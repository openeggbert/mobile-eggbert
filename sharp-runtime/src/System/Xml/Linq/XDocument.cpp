// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/Linq/XDocument.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Xml/Linq/XCData.hpp"
#include "System/Xml/Linq/XComment.hpp"
#include "System/Xml/Linq/XProcessingInstruction.hpp"
#include "System/Xml/Linq/XText.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlException.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml::Linq {

    using System::Xml::XmlNodeType;

    // ---------------------------------------------------------------------------
    // Parsing: builds an XDocument tree from the tinyxml2-backed System::Xml::XmlDocument DOM.
    //
    // Note: no XML namespace-URI resolution is performed — parsed XName local names are taken
    // directly from the element/attribute's qualified tag text, matching the documented
    // simplification already present in XName (see XName's doc-comment).
    // ---------------------------------------------------------------------------
    namespace {

        bool IsAllWhitespace(const std::string& s) {
            return std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c) != 0; });
        }

        std::shared_ptr<XElement> ConvertElement(System::Xml::XmlElement* srcEl, LoadOptions options);

        void ConvertChildrenInto(XContainer& target, System::Xml::XmlNode* parentSrc, LoadOptions options) {
            for (System::Xml::XmlNode* child = parentSrc->getFirstChildProperty(); child != nullptr;
                 child = child->getNextSiblingProperty()) {
                switch (child->getNodeTypeProperty()) {
                    case XmlNodeType::Element:
                        target.Add(ConvertElement(static_cast<System::Xml::XmlElement*>(child), options));
                        break;
                    case XmlNodeType::Text: {
                        std::string v = child->getValueProperty();
                        if ((options & LoadOptions::PreserveWhitespace) != LoadOptions::None || !IsAllWhitespace(v)) {
                            target.Add(std::make_shared<XText>(v));
                        }
                        break;
                    }
                    case XmlNodeType::CDATA:
                        target.Add(std::make_shared<XCData>(child->getValueProperty()));
                        break;
                    case XmlNodeType::Comment:
                        target.Add(std::make_shared<XComment>(child->getValueProperty()));
                        break;
                    case XmlNodeType::ProcessingInstruction: {
                        auto* pi = static_cast<System::Xml::XmlProcessingInstruction*>(child);
                        target.Add(std::make_shared<XProcessingInstruction>(pi->getTargetProperty(), pi->getDataProperty()));
                        break;
                    }
                    default:
                        break; // entity references, notations, etc. — out of scope
                }
            }
        }

        std::shared_ptr<XElement> ConvertElement(System::Xml::XmlElement* srcEl, LoadOptions options) {
            auto el = std::make_shared<XElement>(XName(srcEl->getNameProperty()));
            if (auto* attrs = srcEl->getAttributesProperty()) {
                for (SharpRuntime::intcs i = 0; i < attrs->getCountProperty(); ++i) {
                    auto* a = (*attrs)[i];
                    if (a != nullptr) el->Add(std::make_shared<XAttribute>(XName(a->getNameProperty()), a->getValueProperty()));
                }
            }
            ConvertChildrenInto(*el, srcEl, options);
            return el;
        }

        std::shared_ptr<XDocument> ConvertDocument(System::Xml::XmlDocument& dom, LoadOptions options) {
            auto doc = std::make_shared<XDocument>();
            for (System::Xml::XmlNode* child = dom.getFirstChildProperty(); child != nullptr;
                 child = child->getNextSiblingProperty()) {
                switch (child->getNodeTypeProperty()) {
                    case XmlNodeType::XmlDeclaration: {
                        auto* decl = static_cast<System::Xml::XmlDeclaration*>(child);
                        doc->setDeclarationProperty(std::make_shared<XDeclaration>(
                            decl->getVersionProperty(), decl->getEncodingProperty(), decl->getStandaloneProperty()));
                        break;
                    }
                    case XmlNodeType::DocumentType: {
                        auto* dt = static_cast<System::Xml::XmlDocumentType*>(child);
                        doc->Add(std::make_shared<XDocumentType>(dt->getNameProperty(), dt->getPublicIdProperty(),
                                                                   dt->getSystemIdProperty(), dt->getInternalSubsetProperty()));
                        break;
                    }
                    case XmlNodeType::Comment:
                        doc->Add(std::make_shared<XComment>(child->getValueProperty()));
                        break;
                    case XmlNodeType::ProcessingInstruction: {
                        auto* pi = static_cast<System::Xml::XmlProcessingInstruction*>(child);
                        doc->Add(std::make_shared<XProcessingInstruction>(pi->getTargetProperty(), pi->getDataProperty()));
                        break;
                    }
                    case XmlNodeType::Element:
                        doc->Add(ConvertElement(static_cast<System::Xml::XmlElement*>(child), options));
                        break;
                    default:
                        break;
                }
            }
            return doc;
        }

    } // namespace

    // ---------------------------------------------------------------------------
    // XDocument
    // ---------------------------------------------------------------------------

    std::shared_ptr<XElement> XDocument::getRootProperty() const {
        for (auto& c : children_) {
            if (c->getNodeTypeProperty() == XmlNodeType::Element) return std::static_pointer_cast<XElement>(c);
        }
        return nullptr;
    }

    void XDocument::setRootProperty(std::shared_ptr<XElement> root) {
        if (auto existing = getRootProperty()) {
            RemoveNode(existing.get());
        }
        if (root) Add(std::move(root));
    }

    std::shared_ptr<XDocumentType> XDocument::getDocumentTypeProperty() const {
        for (auto& c : children_) {
            if (c->getNodeTypeProperty() == XmlNodeType::DocumentType) return std::static_pointer_cast<XDocumentType>(c);
        }
        return nullptr;
    }

    void XDocument::ValidateNode(const XNode& node) const {
        // Verified against XDocument.cs's ValidateNode/ValidateString: two corrections from the
        // previous version. (1) Exception type: real .NET throws ArgumentException for CDATA/
        // nested-Document (a fundamentally wrong node *kind* for this container -- a caller
        // bug), reserving InvalidOperationException for Element/DocumentType (a structurally
        // valid kind that conflicts with this specific document's *current state*, e.g. already
        // has a root). (2) Whitespace-only text: real .NET's Text case calls ValidateString,
        // which only rejects a string containing any character *other than* space/tab/CR/LF --
        // whitespace-only text (e.g. indentation between top-level nodes) is legal at document
        // level and was previously rejected unconditionally by the old default: case.
        switch (node.getNodeTypeProperty()) {
            case XmlNodeType::Text: {
                const std::string& text = static_cast<const XText&>(node).getValueProperty();
                if (text.find_first_not_of(" \t\r\n") != std::string::npos)
                    throw System::ArgumentException("Cannot add non-white space text to content.");
                break;
            }
            case XmlNodeType::Element:
                if (getRootProperty() != nullptr) {
                    throw System::InvalidOperationException("This document already has a root element.");
                }
                break;
            case XmlNodeType::DocumentType:
                if (getDocumentTypeProperty() != nullptr) {
                    throw System::InvalidOperationException("This document already has a document type.");
                }
                break;
            case XmlNodeType::CDATA:
                throw System::ArgumentException("This operation would create an incorrectly structured document (cannot add CDATA to an XDocument).");
            case XmlNodeType::Document:
                throw System::ArgumentException("This operation would create an incorrectly structured document (cannot add an XDocument as a child of an XDocument).");
            default:
                // Comment/ProcessingInstruction (and anything else): unrestricted, matching
                // real .NET's switch, which has no case (and no default throw) for them.
                break;
        }
    }

    void XDocument::WriteTo(System::Xml::XmlWriter& writer) const {
        // Verified against XDocument.cs's WriteTo(): real .NET always calls
        // WriteStartDocument()/WriteEndDocument() as a matched pair, regardless of whether an
        // explicit XDeclaration was set (the declaration's Standalone value, when present,
        // only selects *which* WriteStartDocument overload -- it never skips the call). This
        // port previously wrote a declaration only when declaration_ was non-null and never
        // called WriteEndDocument() at all.
        writer.WriteStartDocument();
        for (auto& c : children_) c->WriteTo(writer);
        writer.WriteEndDocument();
    }

    void XDocument::Save(const std::string& filePath, SaveOptions options) const {
        std::ofstream ofs(filePath, std::ios::out | std::ios::trunc);
        if (!ofs) throw System::Xml::XmlException("XDocument::Save: failed to open '" + filePath + "'.");
        bool indent = (options & SaveOptions::DisableFormatting) == SaveOptions::None;
        SerializeTo(ofs, 0, indent);
    }

    void XDocument::Save(System::Xml::XmlWriter& writer) const {
        WriteTo(writer);
    }

    std::shared_ptr<XDocument> XDocument::Parse(const std::string& xml, LoadOptions options) {
        System::Xml::XmlDocument dom;
        dom.LoadXml(xml); // throws System::Xml::XmlException on malformed XML
        return ConvertDocument(dom, options);
    }

    std::shared_ptr<XDocument> XDocument::Load(const std::string& filePath, LoadOptions options) {
        System::Xml::XmlDocument dom;
        dom.Load(filePath); // throws System::Xml::XmlException if missing/malformed
        return ConvertDocument(dom, options);
    }

    void XDocument::SerializeTo(std::ostream& os, int depth, bool indent) const {
        (void)depth;
        bool first = true;
        if (declaration_) {
            os << declaration_->ToString();
            first = false;
        }
        for (auto& c : children_) {
            if (!first) os << "\n";
            c->SerializeTo(os, 0, indent);
            first = false;
        }
    }

    SharpRuntime::intcs XDocument::GetDeepHashCode() const {
        auto root = getRootProperty();
        return root ? root->GetDeepHashCode() : 0;
    }

    bool XDocument::DeepEqualsCore(const XNode& other) const {
        const auto& o = static_cast<const XDocument&>(other);
        return XNode::DeepEquals(getRootProperty().get(), o.getRootProperty().get());
    }

} // namespace System::Xml::Linq
