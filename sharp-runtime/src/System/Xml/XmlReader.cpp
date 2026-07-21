// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlReader.hpp"
#include <tinyxml2/tinyxml2.h>
#include "System/Xml/XmlException.hpp"
#include <cctype>
#include <stack>

namespace System::Xml {

// ---------------------------------------------------------------------------
// Internal event model
// ---------------------------------------------------------------------------

struct XmlEvent {
    XmlNodeType                              type         = XmlNodeType::None;
    std::string                              name;
    std::string                              value;
    bool                                     isEmptyElement = false;
    std::vector<std::pair<std::string,std::string>> attributes;
};

// ---------------------------------------------------------------------------
// Opaque state
// ---------------------------------------------------------------------------

struct XmlReaderState {
    tinyxml2::XMLDocument              doc;
    std::vector<XmlEvent>              events;
    int                                pos       = -1;  // before first Read()
    int                                attrIndex = -1;  // attribute cursor (-1 = on element)
    ReadState                          readState = ReadState::Initial;
};

// ---------------------------------------------------------------------------
// DOM → flat event list
// ---------------------------------------------------------------------------

// tinyxml2 has no distinct "processing instruction" node kind: both the real
// `<?xml ...?>` declaration and every other `<?target data?>` PI parse as XMLDeclaration,
// with Value() holding the raw "target data" text. Split the leading whitespace-delimited
// target token from the rest so the two can be told apart (target == "xml", the only
// reserved PI target per the XML spec, means a real declaration).
static void splitTargetAndData(const std::string& raw, std::string& target, std::string& data) {
    size_t i = 0;
    while (i < raw.size() && !std::isspace(static_cast<unsigned char>(raw[i]))) ++i;
    target = raw.substr(0, i);
    while (i < raw.size() && std::isspace(static_cast<unsigned char>(raw[i]))) ++i;
    data = raw.substr(i);
}

static void buildEvents(tinyxml2::XMLNode* node, std::vector<XmlEvent>& out) {
    if (auto* decl = node->ToDeclaration()) {
        std::string target, data;
        splitTargetAndData(decl->Value() ? decl->Value() : "", target, data);
        XmlEvent e;
        bool isRealDeclaration = target.size() == 3 &&
            (target[0] == 'x' || target[0] == 'X') &&
            (target[1] == 'm' || target[1] == 'M') &&
            (target[2] == 'l' || target[2] == 'L');
        e.type  = isRealDeclaration ? XmlNodeType::XmlDeclaration : XmlNodeType::ProcessingInstruction;
        e.name  = target;
        e.value = data;
        out.push_back(std::move(e));
        return;
    }
    if (auto* el = node->ToElement()) {
        XmlEvent e;
        e.type          = XmlNodeType::Element;
        e.name          = el->Name() ? el->Name() : "";
        e.isEmptyElement = el->ClosingType() == tinyxml2::XMLElement::CLOSED;
        for (const tinyxml2::XMLAttribute* a = el->FirstAttribute(); a; a = a->Next())
            e.attributes.emplace_back(a->Name() ? a->Name() : "",
                                      a->Value() ? a->Value() : "");
        out.push_back(std::move(e));

        for (tinyxml2::XMLNode* child = el->FirstChild(); child; child = child->NextSibling())
            buildEvents(child, out);

        if (!e.isEmptyElement) {
            XmlEvent ee;
            ee.type = XmlNodeType::EndElement;
            ee.name = el->Name() ? el->Name() : "";
            out.push_back(std::move(ee));
        }
        return;
    }
    if (auto* txt = node->ToText()) {
        XmlEvent e;
        e.type  = txt->CData() ? XmlNodeType::CDATA : XmlNodeType::Text;
        e.value = txt->Value() ? txt->Value() : "";
        out.push_back(std::move(e));
        return;
    }
    if (auto* cmt = node->ToComment()) {
        XmlEvent e;
        e.type  = XmlNodeType::Comment;
        e.value = cmt->Value() ? cmt->Value() : "";
        out.push_back(std::move(e));
        return;
    }
    if (auto* unk = node->ToUnknown()) {
        // tinyxml2 parses every other `<!...>` form (DOCTYPE, and anything else it
        // doesn't specifically recognize) as XMLUnknown, with Value() holding the raw
        // text between "<!" and the next ">". DOCTYPE's Name is the root element name
        // per the DOM spec; anything else is reported best-effort with the raw text as
        // Name rather than silently dropped.
        std::string raw = unk->Value() ? unk->Value() : "";
        XmlEvent e;
        e.type = XmlNodeType::DocumentType;
        if (raw.rfind("DOCTYPE", 0) == 0) {
            std::string keyword, rest;
            splitTargetAndData(raw, keyword, rest);
            std::string name;
            splitTargetAndData(rest, name, rest);
            e.name = name;
        } else {
            e.name = raw;
        }
        out.push_back(std::move(e));
        return;
    }
}

// ---------------------------------------------------------------------------
// Constructor / destructor
// ---------------------------------------------------------------------------

XmlReader::XmlReader(std::unique_ptr<XmlReaderState> s) : state_(std::move(s)) {}
XmlReader::~XmlReader() = default;

// ---------------------------------------------------------------------------
// Property accessors
// ---------------------------------------------------------------------------

XmlNodeType XmlReader::getNodeTypeProperty() const {
    if (!state_ || state_->pos < 0 ||
        state_->pos >= static_cast<int>(state_->events.size()))
        return XmlNodeType::None;
    if (state_->attrIndex >= 0)
        return XmlNodeType::Attribute;
    return state_->events[static_cast<size_t>(state_->pos)].type;
}

std::string XmlReader::getNameProperty() const {
    if (!state_ || state_->pos < 0 ||
        state_->pos >= static_cast<int>(state_->events.size()))
        return {};
    const auto& ev = state_->events[static_cast<size_t>(state_->pos)];
    if (state_->attrIndex >= 0 &&
        state_->attrIndex < static_cast<int>(ev.attributes.size()))
        return ev.attributes[static_cast<size_t>(state_->attrIndex)].first;
    return ev.name;
}

std::string XmlReader::getValueProperty() const {
    if (!state_ || state_->pos < 0 ||
        state_->pos >= static_cast<int>(state_->events.size()))
        return {};
    const auto& ev = state_->events[static_cast<size_t>(state_->pos)];
    if (state_->attrIndex >= 0 &&
        state_->attrIndex < static_cast<int>(ev.attributes.size()))
        return ev.attributes[static_cast<size_t>(state_->attrIndex)].second;
    return ev.value;
}

bool XmlReader::getIsEmptyElementProperty() const {
    if (!state_ || state_->pos < 0 ||
        state_->pos >= static_cast<int>(state_->events.size()))
        return false;
    return state_->events[static_cast<size_t>(state_->pos)].isEmptyElement;
}

ReadState XmlReader::getReadStateProperty() const {
    return state_ ? state_->readState : ReadState::Closed;
}

// ---------------------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------------------

bool XmlReader::Read() {
    if (!state_) return false;
    state_->attrIndex = -1;
    ++state_->pos;
    if (state_->pos >= static_cast<int>(state_->events.size())) {
        state_->readState = ReadState::EndOfFile;
        return false;
    }
    state_->readState = ReadState::Interactive;
    return true;
}

bool XmlReader::MoveToElement() {
    if (!state_ || state_->pos < 0 ||
        state_->pos >= static_cast<int>(state_->events.size()))
        return false;
    state_->attrIndex = -1;
    return state_->events[static_cast<size_t>(state_->pos)].type == XmlNodeType::Element;
}

bool XmlReader::MoveToNextAttribute() {
    if (!state_ || state_->pos < 0 ||
        state_->pos >= static_cast<int>(state_->events.size()))
        return false;
    const auto& ev = state_->events[static_cast<size_t>(state_->pos)];
    if (ev.type != XmlNodeType::Element) return false;
    int next = state_->attrIndex + 1;
    if (next >= static_cast<int>(ev.attributes.size())) return false;
    state_->attrIndex = next;
    return true;
}

std::string XmlReader::GetAttribute(const std::string& name) const {
    if (!state_ || state_->pos < 0 ||
        state_->pos >= static_cast<int>(state_->events.size()))
        return {};
    const auto& ev = state_->events[static_cast<size_t>(state_->pos)];
    for (const auto& [k, v] : ev.attributes)
        if (k == name) return v;
    return {};
}

std::string XmlReader::ReadElementContentAsString() {
    if (!state_) return {};
    std::string result;
    // advance past the Element node we're sitting on
    Read();
    // Tracks nesting depth of any child elements encountered along the way. Without this, a
    // nested child element's OWN EndElement event was indistinguishable from the enclosing
    // element's end -- e.g. for "<a><b>x</b>y</a>", the loop would hit <b>'s EndElement first,
    // treat it as </a>, break early (returning "x" instead of "xy"), and leave the reader
    // positioned mid-content (on the Text("y") node) rather than past </a> -- corrupting every
    // subsequent Read() call, not just the returned string.
    int depth = 0;
    while (state_->pos >= 0 &&
           state_->pos < static_cast<int>(state_->events.size())) {
        const auto& ev = state_->events[static_cast<size_t>(state_->pos)];
        if (ev.type == XmlNodeType::EndElement) {
            if (depth == 0) {
                Read(); // consume our own end-element
                break;
            }
            --depth;
        } else if (ev.type == XmlNodeType::Element) {
            if (!ev.isEmptyElement) ++depth; // self-closing elements have no matching EndElement
        } else if (depth == 0 && (ev.type == XmlNodeType::Text || ev.type == XmlNodeType::CDATA)) {
            result += ev.value;
        }
        Read();
    }
    return result;
}

void XmlReader::ReadStartElement() {
    if (!state_ || state_->pos < 0 ||
        state_->pos >= static_cast<int>(state_->events.size()) ||
        state_->events[static_cast<size_t>(state_->pos)].type != XmlNodeType::Element)
        throw XmlException("ReadStartElement: not on an element node");
    Read();
}

void XmlReader::ReadEndElement() {
    if (!state_ || state_->pos < 0 ||
        state_->pos >= static_cast<int>(state_->events.size()) ||
        state_->events[static_cast<size_t>(state_->pos)].type != XmlNodeType::EndElement)
        throw XmlException("ReadEndElement: not on an end-element node");
    Read();
}

void XmlReader::Close() {
    if (state_) state_->readState = ReadState::Closed;
}

// ---------------------------------------------------------------------------
// Factory helpers
// ---------------------------------------------------------------------------

static XmlReader* createFromDoc(std::unique_ptr<XmlReaderState> st) {
    if (st->doc.Error())
        throw XmlException(std::string("XmlReader: parse error: ") +
                                 (st->doc.ErrorStr() ? st->doc.ErrorStr() : ""));
    // Walk all top-level nodes (declaration + root element)
    for (tinyxml2::XMLNode* n = st->doc.FirstChild(); n; n = n->NextSibling())
        buildEvents(n, st->events);
    return new XmlReader(std::move(st));
}

XmlReader* XmlReader::Create(const std::string& inputUri) {
    auto st = std::make_unique<XmlReaderState>();
    // Heuristic: treat as file path if it ends with .xml or contains a path separator --
    // UNLESS the (whitespace-trimmed) input starts with '<', which is an unambiguous, much
    // stronger signal that this is XML content rather than a path (a file path essentially
    // never starts with '<'). Without this check, ANY XML content containing a '/' character
    // -- extremely common: self-closing tags like "<br/>", URLs in attribute/text content, XML
    // namespace URIs almost always being "http://..." -- was misclassified as a file path and
    // sent to LoadFile(), which fails (no such file) and throws a misleading "parse error" for
    // perfectly valid XML text.
    std::size_t firstNonSpace = inputUri.find_first_not_of(" \t\r\n");
    bool looksLikeContent = firstNonSpace != std::string::npos && inputUri[firstNonSpace] == '<';
    bool isFile = !looksLikeContent &&
                  (inputUri.find('/') != std::string::npos ||
                   inputUri.find('\\') != std::string::npos ||
                   (inputUri.size() >= 4 &&
                    inputUri.substr(inputUri.size() - 4) == ".xml"));
    if (isFile)
        st->doc.LoadFile(inputUri.c_str());
    else
        st->doc.Parse(inputUri.c_str());
    return createFromDoc(std::move(st));
}

XmlReader* XmlReader::CreateFromString(const std::string& xmlContent) {
    auto st = std::make_unique<XmlReaderState>();
    st->doc.Parse(xmlContent.c_str());
    return createFromDoc(std::move(st));
}

} // namespace System::Xml
