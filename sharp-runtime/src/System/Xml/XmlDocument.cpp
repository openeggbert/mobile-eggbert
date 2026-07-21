// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlDocument.hpp"

#include <cctype>
#include <tinyxml2/tinyxml2.h>

#include "System/ArgumentException.hpp"
#include "System/Xml/XmlConvert.hpp"
#include "System/Xml/XmlException.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml {

    namespace {
        // VersionNum ::= '1.' [0-9]+  (XmlDeclaration.cs IsValidXmlVersion)
        bool IsValidXmlVersion(const std::string& ver) {
            if (ver.size() < 3 || ver[0] != '1' || ver[1] != '.') return false;
            for (size_t i = 2; i < ver.size(); ++i)
                if (!std::isdigit(static_cast<unsigned char>(ver[i]))) return false;
            return true;
        }

        // Matches XmlCharType.IsOnlyWhitespace / the XML S production: exactly tab, LF, CR,
        // space -- used by XmlWhitespace.cs/XmlSignificantWhiteSpace.cs's constructors
        // (CheckOnData) to reject non-whitespace content. An empty string is vacuously valid.
        bool IsOnlyXmlWhitespace(const std::string& s) {
            for (char c : s)
                if (c != '\t' && c != '\n' && c != '\r' && c != ' ') return false;
            return true;
        }

        bool StartsWithDoctype(const char* v) {
            if (!v) return false;
            std::string s(v);
            return s.rfind("DOCTYPE", 0) == 0;
        }

        // Best-effort DOCTYPE tokenizer: "DOCTYPE name [PUBLIC "pub" "sys" | SYSTEM "sys"]".
        // See XmlDocumentType's doc-comment for why this can't be fully correct for
        // internal-subset content (tinyxml2 itself can't parse past an internal ">" there).
        void ParseDoctype(const std::string& raw, std::string& name, std::string& publicId, std::string& systemId) {
            size_t pos = raw.find("DOCTYPE");
            pos += 7;
            auto skipWs = [&](size_t p) { while (p < raw.size() && std::isspace(static_cast<unsigned char>(raw[p]))) ++p; return p; };
            pos = skipWs(pos);
            size_t nameEnd = raw.find_first_of(" \t\r\n", pos);
            name = raw.substr(pos, nameEnd == std::string::npos ? std::string::npos : nameEnd - pos);
            pos = nameEnd == std::string::npos ? raw.size() : skipWs(nameEnd);

            auto readQuoted = [&](size_t& p) -> std::string {
                if (p >= raw.size() || (raw[p] != '"' && raw[p] != '\'')) return {};
                char q = raw[p];
                size_t start = p + 1;
                size_t end = raw.find(q, start);
                if (end == std::string::npos) { p = raw.size(); return {}; }
                p = end + 1;
                return raw.substr(start, end - start);
            };

            if (raw.compare(pos, 6, "PUBLIC") == 0) {
                pos = skipWs(pos + 6);
                publicId = readQuoted(pos);
                pos = skipWs(pos);
                systemId = readQuoted(pos);
            } else if (raw.compare(pos, 6, "SYSTEM") == 0) {
                pos = skipWs(pos + 6);
                systemId = readQuoted(pos);
            }
        }
    }

    XmlDocument::XmlDocument()
        : nameTable_(std::make_shared<NameTable>()) {
        implementation_ = std::make_unique<XmlImplementation>(nameTable_);
        native_ = &doc_;
        ownerDocument_ = nullptr;
        detachedHolder_ = doc_.NewElement("#detached-holder");
    }

    XmlDocument::XmlDocument(std::shared_ptr<XmlNameTable> nt)
        : nameTable_(std::move(nt)) {
        implementation_ = std::make_unique<XmlImplementation>(nameTable_);
        native_ = &doc_;
        ownerDocument_ = nullptr;
        detachedHolder_ = doc_.NewElement("#detached-holder");
    }

    XmlDocument::~XmlDocument() = default;

    void XmlDocument::PurgeCache(tinyxml2::XMLNode* native) {
        if (!native) return;
        for (tinyxml2::XMLNode* child = native->FirstChild(); child; child = child->NextSibling())
            PurgeCache(child);
        nodeCache_.erase(native);
    }

    void XmlDocument::DetachNode(tinyxml2::XMLNode* native) {
        if (!native || !detachedHolder_) return;
        detachedHolder_->InsertEndChild(native);
    }

    bool XmlDocument::IsDetached(const tinyxml2::XMLNode* native) const {
        return native == static_cast<const tinyxml2::XMLNode*>(detachedHolder_) ||
               (native && native->Parent() == static_cast<const tinyxml2::XMLNode*>(detachedHolder_));
    }

    void XmlDocument::ReleaseUnattachedNode(XmlNode* node) {
        for (auto it = unattachedNodes_.begin(); it != unattachedNodes_.end(); ++it) {
            if (it->get() == node) {
                it->release();
                unattachedNodes_.erase(it);
                return;
            }
        }
    }

    XmlNode* XmlDocument::WrapNode(tinyxml2::XMLNode* native) {
        if (!native) return nullptr;
        if (native == static_cast<tinyxml2::XMLNode*>(&doc_)) return this;

        auto it = nodeCache_.find(native);
        if (it != nodeCache_.end()) return it->second.get();

        std::unique_ptr<XmlNode> wrapper;
        if (native->ToElement()) {
            wrapper = std::make_unique<XmlElement>(native, this);
        } else if (auto* txt = native->ToText()) {
            wrapper = txt->CData() ? std::unique_ptr<XmlNode>(std::make_unique<XmlCDataSection>(native, this))
                                   : std::unique_ptr<XmlNode>(std::make_unique<XmlText>(native, this));
        } else if (native->ToComment()) {
            wrapper = std::make_unique<XmlComment>(native, this);
        } else if (auto* decl = native->ToDeclaration()) {
            const char* v = decl->Value();
            bool isXmlDecl = v && (std::string(v).rfind("xml", 0) == 0) &&
                             (v[3] == '\0' || std::isspace(static_cast<unsigned char>(v[3])));
            wrapper = isXmlDecl ? std::unique_ptr<XmlNode>(std::make_unique<XmlDeclaration>(native, this))
                                : std::unique_ptr<XmlNode>(std::make_unique<XmlProcessingInstruction>(native, this));
        } else if (auto* unk = native->ToUnknown()) {
            std::string name, publicId, systemId;
            if (StartsWithDoctype(unk->Value())) {
                ParseDoctype(unk->Value(), name, publicId, systemId);
            }
            wrapper = std::make_unique<XmlDocumentType>(native, this, name, publicId, systemId);
        } else {
            return nullptr;
        }

        auto* raw = wrapper.get();
        nodeCache_.emplace(native, std::move(wrapper));
        return raw;
    }

    XmlElement* XmlDocument::getDocumentElementProperty() const {
        auto* root = const_cast<tinyxml2::XMLDocument&>(doc_).RootElement();
        return root ? static_cast<XmlElement*>(const_cast<XmlDocument*>(this)->WrapNode(root)) : nullptr;
    }

    XmlDocumentType* XmlDocument::getDocumentTypeProperty() const {
        for (const tinyxml2::XMLNode* n = doc_.FirstChild(); n; n = n->NextSibling())
            if (auto* unk = n->ToUnknown())
                if (StartsWithDoctype(unk->Value()))
                    return static_cast<XmlDocumentType*>(const_cast<XmlDocument*>(this)->WrapNode(const_cast<tinyxml2::XMLNode*>(n)));
        return nullptr;
    }

    XmlAttribute* XmlDocument::CreateAttribute(const std::string& name) {
        // Verified against XmlDocument.cs's CreateElement/CreateAttribute -> XmlDocument.CheckName
        // (called on the split prefix and local name): both throw XmlException for a
        // malformed XML name (e.g. empty, or starting with a digit) instead of silently
        // accepting it and producing unparseable markup on write-out.
        (void)XmlConvert::VerifyName(name);
        // Owned by this document (unattachedNodes_) until XmlElement::SetAttributeNode claims it
        // via ReleaseUnattachedNode -- see unattachedNodes_'s own comment for why (XmlAttribute
        // has no native tinyxml2 node to key nodeCache_ by, unlike every other Create* method).
        auto attr = std::make_unique<XmlAttribute>(this, name);
        auto* raw = attr.get();
        unattachedNodes_.push_back(std::move(attr));
        return raw;
    }

    XmlElement* XmlDocument::CreateElement(const std::string& name) {
        (void)XmlConvert::VerifyName(name);
        auto* native = doc_.NewElement(name.c_str());
        return static_cast<XmlElement*>(WrapNode(native));
    }

    XmlText* XmlDocument::CreateTextNode(const std::string& text) {
        auto* native = doc_.NewText(text.c_str());
        return static_cast<XmlText*>(WrapNode(native));
    }

    XmlComment* XmlDocument::CreateComment(const std::string& data) {
        auto* native = doc_.NewComment(data.c_str());
        return static_cast<XmlComment*>(WrapNode(native));
    }

    XmlCDataSection* XmlDocument::CreateCDataSection(const std::string& data) {
        auto* native = doc_.NewText(data.c_str());
        native->SetCData(true);
        return static_cast<XmlCDataSection*>(WrapNode(native));
    }

    XmlProcessingInstruction* XmlDocument::CreateProcessingInstruction(const std::string& target, const std::string& data) {
        // Matches XmlProcessingInstruction.cs's constructor: only an empty target is rejected
        // (ArgumentException.ThrowIfNullOrEmpty) -- unlike CreateElement/CreateAttribute, the
        // DOM does not additionally require target to be a well-formed NCName at construction.
        if (target.empty())
            throw System::ArgumentException("Value cannot be null or empty.", "target");
        std::string text = data.empty() ? target : target + " " + data;
        auto* native = doc_.NewDeclaration(text.c_str());
        return static_cast<XmlProcessingInstruction*>(WrapNode(native));
    }

    XmlDeclaration* XmlDocument::CreateXmlDeclaration(const std::string& version, const std::string& encoding, const std::string& standalone) {
        if (!IsValidXmlVersion(version))
            throw System::ArgumentException("Wrong XML version information. The XML must match production \"VersionNum ::= '1.' [0-9]+\".");
        if (!standalone.empty() && standalone != "yes" && standalone != "no")
            throw System::ArgumentException("Wrong value for the XML declaration standalone attribute of '" + standalone + "'.");
        std::string text = "xml version=\"" + version + "\"";
        if (!encoding.empty()) text += " encoding=\"" + encoding + "\"";
        if (!standalone.empty()) text += " standalone=\"" + standalone + "\"";
        auto* native = doc_.NewDeclaration(text.c_str());
        return static_cast<XmlDeclaration*>(WrapNode(native));
    }

    XmlDocumentFragment* XmlDocument::CreateDocumentFragment() {
        // Scratch element that is never inserted into the visible tree; see the class doc-comment.
        auto* native = doc_.NewElement("#document-fragment");
        auto wrapper = std::make_unique<XmlDocumentFragment>(native, this);
        auto* raw = wrapper.get();
        nodeCache_.emplace(native, std::move(wrapper));
        return raw;
    }

    XmlDocumentType* XmlDocument::CreateDocumentType(const std::string& name, const std::string& publicId,
                                                     const std::string& systemId, const std::string& /*internalSubset*/) {
        std::string text = "DOCTYPE " + name;
        if (!publicId.empty()) text += " PUBLIC \"" + publicId + "\" \"" + systemId + "\"";
        else if (!systemId.empty()) text += " SYSTEM \"" + systemId + "\"";
        auto* native = doc_.NewUnknown(text.c_str());
        auto wrapper = std::make_unique<XmlDocumentType>(native, this, name, publicId, systemId);
        auto* raw = wrapper.get();
        nodeCache_.emplace(native, std::move(wrapper));
        return raw;
    }

    XmlEntityReference* XmlDocument::CreateEntityReference(const std::string& name) {
        // Matches XmlEntityReference.cs's constructor: entity reference names are not required
        // to be well-formed NCNames (they may reference arbitrary DTD-declared entities) -- the
        // only rejected form is one starting with '#', which would collide with a character
        // reference (e.g. "&#65;").
        if (!name.empty() && name[0] == '#')
            throw System::ArgumentException("An entity reference must not start with '#'.", "name");
        // Owned by this document (unattachedNodes_) for the same reason as CreateAttribute above.
        // Unlike XmlAttribute, nothing in this port currently attaches an XmlEntityReference to a
        // tree (it has no native tinyxml2 backing and AppendChild/InsertBefore/etc. all require
        // one), so in practice this entry is never released via ReleaseUnattachedNode and simply
        // lives until the document itself is destroyed -- still correct, just always the
        // "never claimed" branch of the same ownership mechanism.
        auto ref = std::make_unique<XmlEntityReference>(this, name);
        auto* raw = ref.get();
        unattachedNodes_.push_back(std::move(ref));
        return raw;
    }

    XmlWhitespace* XmlDocument::CreateWhitespace(const std::string& text) {
        // Matches XmlWhitespace.cs's constructor: `if (!doc.IsLoading && !CheckOnData(strData))
        // throw new ArgumentException(SR.Xdom_WS_Char);` -- content must be only XML whitespace
        // characters when created programmatically (not during parsing).
        if (!IsOnlyXmlWhitespace(text))
            throw System::ArgumentException("The string for whitespace contains an invalid character.");
        auto* native = doc_.NewText(text.c_str());
        auto wrapper = std::make_unique<XmlWhitespace>(native, this);
        auto* raw = wrapper.get();
        nodeCache_.emplace(native, std::move(wrapper));
        return raw;
    }

    XmlSignificantWhitespace* XmlDocument::CreateSignificantWhitespace(const std::string& text) {
        // Matches XmlSignificantWhiteSpace.cs's constructor: same CheckOnData validation as
        // XmlWhitespace above.
        if (!IsOnlyXmlWhitespace(text))
            throw System::ArgumentException("The string for whitespace contains an invalid character.");
        auto* native = doc_.NewText(text.c_str());
        auto wrapper = std::make_unique<XmlSignificantWhitespace>(native, this);
        auto* raw = wrapper.get();
        nodeCache_.emplace(native, std::move(wrapper));
        return raw;
    }

    namespace {
        void CollectDocElementsByName(tinyxml2::XMLNode* node, const std::string& name,
                                      XmlDocument* doc, std::vector<XmlNode*>& out) {
            for (tinyxml2::XMLNode* child = node->FirstChild(); child; child = child->NextSibling()) {
                if (auto* el = child->ToElement()) {
                    if (name == "*" || (el->Name() && name == el->Name()))
                        out.push_back(doc->WrapNode(child));
                    CollectDocElementsByName(child, name, doc, out);
                }
            }
        }

        const tinyxml2::XMLElement* FindElementById(const tinyxml2::XMLElement* el, const std::string& id) {
            const char* v = el->Attribute("id");
            if (v && id == v) return el;
            for (const tinyxml2::XMLElement* child = el->FirstChildElement(); child; child = child->NextSiblingElement()) {
                if (auto* found = FindElementById(child, id)) return found;
            }
            return nullptr;
        }
    }

    XmlNodeList* XmlDocument::GetElementsByTagName(const std::string& name) const {
        std::vector<XmlNode*> results;
        CollectDocElementsByName(const_cast<tinyxml2::XMLDocument*>(&doc_), name, const_cast<XmlDocument*>(this), results);
        return new XmlNodeList(std::move(results)); // ownership: caller's responsibility, matching XmlNode::getChildNodesProperty's leak-free
                                                      // pattern is not applied here since XmlDocument has no equivalent cached member;
                                                      // documented as a minor known simplification (rare, one-shot query use).
    }

    XmlElement* XmlDocument::GetElementById(const std::string& elementId) const {
        auto* root = doc_.RootElement();
        if (!root) return nullptr;
        auto* found = FindElementById(root, elementId);
        return found ? static_cast<XmlElement*>(const_cast<XmlDocument*>(this)->WrapNode(const_cast<tinyxml2::XMLElement*>(found))) : nullptr;
    }

    void XmlDocument::Load(const std::string& filename) {
        nodeCache_.clear();
        if (doc_.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS)
            throw XmlException("XmlDocument::Load: failed to load '" + filename + "': " +
                               (doc_.ErrorStr() ? doc_.ErrorStr() : "unknown error"));
        // tinyxml2::XMLDocument::LoadFile() clears and frees every previously-allocated node
        // (including detachedHolder_ from the constructor, or a prior Load/LoadXml call) before
        // parsing; recreate it now, or IsDetached()/getParentNodeProperty() etc. would compare
        // against a dangling pointer into freed (and likely reused) memory.
        detachedHolder_ = doc_.NewElement("#detached-holder");
    }

    void XmlDocument::LoadXml(const std::string& xml) {
        nodeCache_.clear();
        if (doc_.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS)
            throw XmlException(std::string("XmlDocument::LoadXml: parse error: ") +
                               (doc_.ErrorStr() ? doc_.ErrorStr() : "unknown error"));
        // See the comment in Load() above: Parse() also clears and frees prior nodes.
        detachedHolder_ = doc_.NewElement("#detached-holder");
    }

    void XmlDocument::Save(const std::string& filename) const {
        if (const_cast<tinyxml2::XMLDocument&>(doc_).SaveFile(filename.c_str()) != tinyxml2::XML_SUCCESS)
            throw XmlException("XmlDocument::Save: failed to save '" + filename + "'.");
    }

    void XmlDocument::Save(XmlWriter& writer) const {
        WriteTo(writer);
    }

    std::string XmlDocument::getInnerXmlProperty() const {
        return XmlNode::getInnerXmlProperty();
    }

    void XmlDocument::setInnerXmlProperty(const std::string& xml) {
        LoadXml(xml);
    }

    void XmlDocument::WriteTo(XmlWriter& writer) const {
        WriteContentTo(writer);
    }

    void XmlDocument::WriteContentTo(XmlWriter& writer) const {
        XmlNode::WriteContentTo(writer);
    }

} // namespace System::Xml
