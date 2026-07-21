// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlWriter.hpp"
#include <tinyxml2/tinyxml2.h>
#include "System/Xml/XmlException.hpp"
#include <stack>

namespace System::Xml {

// ---------------------------------------------------------------------------
// Opaque state
// ---------------------------------------------------------------------------

struct XmlWriterState {
    tinyxml2::XMLDocument          doc;
    std::stack<tinyxml2::XMLNode*> nodeStack;  // top = current parent
    std::string                    filePath;
    bool                           hasDeclaration = false;
    XmlWriterSettings              settings;
};

// ---------------------------------------------------------------------------
// Constructor / destructor
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Well-formedness self-healing (matches XmlEncodedRawTextWriter.WriteCommentOrPi /
// WriteCDataSection): real .NET never throws for a comment containing "--", a PI
// containing "?>", or a CDATA section containing "]]>" -- it silently inserts a
// protective character (or, for CDATA, splits into adjacent sections) so the emitted
// markup stays well-formed while preserving the original content on read-back.
// ---------------------------------------------------------------------------

static std::string sanitizeCommentText(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        out += text[i];
        if (text[i] == '-' && (i + 1 == text.size() || text[i + 1] == '-'))
            out += ' '; // avoid "--" inside the comment or "-" abutting the closing "-->"
    }
    return out;
}

static std::string sanitizeProcessingInstructionText(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        out += text[i];
        if (text[i] == '?' && i + 1 < text.size() && text[i + 1] == '>')
            out += ' '; // avoid "?>" prematurely closing the PI
    }
    return out;
}

static std::string sanitizeCDataText(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    size_t i = 0;
    while (i < text.size()) {
        if (i + 2 < text.size() && text[i] == ']' && text[i + 1] == ']' && text[i + 2] == '>') {
            // Close the current CDATA section right before the embedded "]]>" and
            // immediately reopen a new one, so the terminator never appears mid-content.
            out += "]]]]><![CDATA[>";
            i += 3;
        } else {
            out += text[i];
            ++i;
        }
    }
    return out;
}

XmlWriter::XmlWriter(std::unique_ptr<XmlWriterState> s) : state_(std::move(s)) {
    // Start with the document as the root parent
    state_->nodeStack.push(&state_->doc);
}

// Best-effort, non-throwing (audit finding A-02, 2026-07-14) -- see DeflateStream::~DeflateStream's
// identical doc-comment for the full rationale and confirmed std::terminate repro. Close() can
// throw via Flush() when tinyxml2's SaveFile() fails (e.g. an unwritable path or a full disk).
XmlWriter::~XmlWriter() { try { Close(); } catch (...) {} }

// ---------------------------------------------------------------------------
// Write methods
// ---------------------------------------------------------------------------

void XmlWriter::WriteStartDocument() {
    if (!state_ || state_->hasDeclaration) return;
    state_->doc.InsertFirstChild(state_->doc.NewDeclaration());
    state_->hasDeclaration = true;
}

void XmlWriter::WriteEndDocument() {
    Flush();
}

void XmlWriter::WriteStartElement(const std::string& localName) {
    if (!state_ || state_->nodeStack.empty()) return;
    tinyxml2::XMLElement* el = state_->doc.NewElement(localName.c_str());
    state_->nodeStack.top()->InsertEndChild(el);
    state_->nodeStack.push(el);
}

void XmlWriter::WriteEndElement() {
    if (!state_ || state_->nodeStack.size() <= 1) return; // never pop the document
    state_->nodeStack.pop();
}

void XmlWriter::WriteAttributeString(const std::string& name, const std::string& value) {
    if (!state_ || state_->nodeStack.empty()) return;
    auto* el = state_->nodeStack.top()->ToElement();
    if (el) el->SetAttribute(name.c_str(), value.c_str());
}

void XmlWriter::WriteString(const std::string& text) {
    if (!state_ || state_->nodeStack.empty()) return;
    tinyxml2::XMLText* tn = state_->doc.NewText(text.c_str());
    state_->nodeStack.top()->InsertEndChild(tn);
}

void XmlWriter::WriteElementString(const std::string& name, const std::string& value) {
    WriteStartElement(name);
    WriteString(value);
    WriteEndElement();
}

void XmlWriter::WriteComment(const std::string& text) {
    if (!state_ || state_->nodeStack.empty()) return;
    tinyxml2::XMLComment* cmt = state_->doc.NewComment(sanitizeCommentText(text).c_str());
    state_->nodeStack.top()->InsertEndChild(cmt);
}

void XmlWriter::WriteCData(const std::string& text) {
    if (!state_ || state_->nodeStack.empty()) return;
    tinyxml2::XMLText* tn = state_->doc.NewText(sanitizeCDataText(text).c_str());
    tn->SetCData(true);
    state_->nodeStack.top()->InsertEndChild(tn);
}

void XmlWriter::WriteProcessingInstruction(const std::string& target, const std::string& data) {
    if (!state_ || state_->nodeStack.empty()) return;
    std::string sanitizedData = sanitizeProcessingInstructionText(data);
    std::string text = sanitizedData.empty() ? target : (target + " " + sanitizedData);
    tinyxml2::XMLDeclaration* pi = state_->doc.NewDeclaration(text.c_str());
    state_->nodeStack.top()->InsertEndChild(pi);
}

void XmlWriter::WriteDocType(const std::string& name, const std::string& publicId,
                              const std::string& systemId, const std::string& internalSubset) {
    if (!state_ || state_->nodeStack.empty()) return;
    std::string text = "DOCTYPE " + name;
    if (!publicId.empty()) {
        text += " PUBLIC \"" + publicId + "\" \"" + systemId + "\"";
    } else if (!systemId.empty()) {
        text += " SYSTEM \"" + systemId + "\"";
    }
    if (!internalSubset.empty()) text += " [" + internalSubset + "]";
    tinyxml2::XMLUnknown* dt = state_->doc.NewUnknown(text.c_str());
    state_->nodeStack.top()->InsertEndChild(dt);
}

std::string XmlWriter::ToString() const {
    if (!state_) return {};
    tinyxml2::XMLPrinter printer(nullptr, /*compact=*/!state_->settings.Indent);
    state_->doc.Print(&printer);
    return printer.CStr() ? printer.CStr() : "";
}

void XmlWriter::Flush() {
    if (!state_ || state_->filePath.empty()) return;
    if (state_->doc.SaveFile(state_->filePath.c_str(), /*compact=*/!state_->settings.Indent) !=
        tinyxml2::XML_SUCCESS)
        throw XmlException("XmlWriter: failed to save file: " + state_->filePath);
}

void XmlWriter::Close() {
    if (!state_) return;
    Flush();
    // clear the stack
    while (!state_->nodeStack.empty()) state_->nodeStack.pop();
}

// ---------------------------------------------------------------------------
// Factory methods
// ---------------------------------------------------------------------------

XmlWriter* XmlWriter::Create(const std::string& outputFileName, const XmlWriterSettings& settings) {
    auto st = std::make_unique<XmlWriterState>();
    st->filePath = outputFileName;
    st->settings = settings;
    return new XmlWriter(std::move(st));
}

XmlWriter* XmlWriter::CreateToString(const XmlWriterSettings& settings) {
    auto st = std::make_unique<XmlWriterState>();
    st->settings = settings;
    return new XmlWriter(std::move(st));
}

} // namespace System::Xml
