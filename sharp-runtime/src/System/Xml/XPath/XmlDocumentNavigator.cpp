// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XPath/XmlDocumentNavigator.hpp"

#include <algorithm>
#include <optional>
#include <set>

#include "System/NotSupportedException.hpp"
#include "System/Xml/XmlAttribute.hpp"
#include "System/Xml/XmlAttributeCollection.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlElement.hpp"

namespace System::Xml::XPath {

    namespace {

        std::optional<XPathNodeType> ToXPathNodeType(System::Xml::XmlNodeType t) {
            switch (t) {
                case System::Xml::XmlNodeType::Document: return XPathNodeType::Root;
                case System::Xml::XmlNodeType::Element: return XPathNodeType::Element;
                case System::Xml::XmlNodeType::Attribute: return XPathNodeType::Attribute;
                case System::Xml::XmlNodeType::Text: return XPathNodeType::Text;
                case System::Xml::XmlNodeType::CDATA: return XPathNodeType::Text;
                case System::Xml::XmlNodeType::Whitespace: return XPathNodeType::Whitespace;
                case System::Xml::XmlNodeType::SignificantWhitespace: return XPathNodeType::SignificantWhitespace;
                case System::Xml::XmlNodeType::Comment: return XPathNodeType::Comment;
                case System::Xml::XmlNodeType::ProcessingInstruction: return XPathNodeType::ProcessingInstruction;
                default: return std::nullopt; // DocumentType/XmlDeclaration/EntityReference/DocumentFragment/Notation/EndElement/EndEntity/Entity/None
            }
        }

        bool HasXPathRepresentation(System::Xml::XmlNode* n) {
            return n && ToXPathNodeType(n->getNodeTypeProperty()).has_value();
        }

        bool IsXmlnsAttributeName(const std::string& name, std::string* outPrefix) {
            if (name == "xmlns") { if (outPrefix) *outPrefix = ""; return true; }
            if (name.rfind("xmlns:", 0) == 0) { if (outPrefix) *outPrefix = name.substr(6); return true; }
            return false;
        }

    } // namespace

    System::Xml::XmlNode* XmlDocumentNavigator::GetNode() const {
        if (pos_ == Pos::Node) return node_;
        if (pos_ == Pos::Attribute) return attr_;
        return nullptr; // namespace nodes have no distinct DOM node representation in this runtime
    }

    XPathNodeType XmlDocumentNavigator::getNodeTypeProperty() const {
        if (pos_ == Pos::Attribute) return XPathNodeType::Attribute;
        if (pos_ == Pos::Namespace) return XPathNodeType::Namespace;
        auto mapped = ToXPathNodeType(node_->getNodeTypeProperty());
        if (!mapped.has_value())
            throw System::NotSupportedException(
                "XmlDocumentNavigator: this DOM node type has no XPath data model equivalent.");
        return *mapped;
    }

    std::string XmlDocumentNavigator::getNameProperty() const {
        if (pos_ == Pos::Attribute) return attr_->getNameProperty();
        if (pos_ == Pos::Namespace) return nsList_[static_cast<size_t>(nsIndex_)].first;
        switch (node_->getNodeTypeProperty()) {
            case System::Xml::XmlNodeType::Element:
            case System::Xml::XmlNodeType::ProcessingInstruction:
                return node_->getNameProperty();
            default:
                return {};
        }
    }

    std::string XmlDocumentNavigator::getLocalNameProperty() const {
        if (pos_ == Pos::Attribute) return attr_->getLocalNameProperty();
        if (pos_ == Pos::Namespace) return nsList_[static_cast<size_t>(nsIndex_)].first;
        switch (node_->getNodeTypeProperty()) {
            case System::Xml::XmlNodeType::Element:
            case System::Xml::XmlNodeType::ProcessingInstruction:
                return node_->getLocalNameProperty();
            default:
                return {};
        }
    }

    std::string XmlDocumentNavigator::getNamespaceURIProperty() const {
        if (pos_ == Pos::Attribute) return attr_->getNamespaceURIProperty();
        if (pos_ == Pos::Namespace) return {};
        if (node_->getNodeTypeProperty() == System::Xml::XmlNodeType::Element) return node_->getNamespaceURIProperty();
        return {};
    }

    std::string XmlDocumentNavigator::getPrefixProperty() const {
        if (pos_ == Pos::Attribute) return attr_->getPrefixProperty();
        if (pos_ == Pos::Namespace) return {};
        if (node_->getNodeTypeProperty() == System::Xml::XmlNodeType::Element) return node_->getPrefixProperty();
        return {};
    }

    std::string XmlDocumentNavigator::getValueProperty() const {
        if (pos_ == Pos::Attribute) return attr_->getValueProperty();
        if (pos_ == Pos::Namespace) return nsList_[static_cast<size_t>(nsIndex_)].second;
        // Root/Element: XPath string-value is the concatenation of all descendant text, which is
        // exactly what XmlNode::getInnerTextProperty() already computes. Text/Comment/PI/
        // Whitespace/SignificantWhitespace: their own value, via the DOM's own getValueProperty().
        //
        // KNOWN GAP (audited, not fixed -- see DocumentXPathNavigator.cs's ValueText/
        // CalibrateText for the real .NET behavior this diverges from): per the XPath 1.0 data
        // model, adjacent Text/CDATA/Whitespace/SignificantWhitespace DOM sibling nodes (e.g. a
        // CDATA section immediately followed by plain text, or two Text nodes left un-normalized
        // after DOM manipulation) collapse into a SINGLE XPath text node -- real .NET's
        // ValueText concatenates forward through all adjacent text-like siblings, and
        // CalibrateText() ensures navigation never lands mid-run (always on the first node of a
        // run). This port has neither: MoveToFirstChild/MoveToNext/MoveToPrevious can land on a
        // non-first member of an adjacent text-like run (they only skip nodes with no XPath
        // representation at all, not "continuation" text-like siblings), and this method returns
        // only the positioned-on node's own value, not the merged run. A correct fix needs
        // coordinated changes across those three navigation methods (each with different
        // directional run-boundary logic) plus this method -- out of scope for a single-pass
        // fix; documented rather than rushed. Reachable only when adjacent text-like DOM
        // siblings exist without an intervening element/comment/PI (uncommon: most parsers
        // coalesce adjacent text automatically; requires explicit un-normalized DOM construction
        // or CDATA directly adjacent to plain text).
        switch (node_->getNodeTypeProperty()) {
            case System::Xml::XmlNodeType::Document:
            case System::Xml::XmlNodeType::Element:
                return node_->getInnerTextProperty();
            default:
                return node_->getValueProperty();
        }
    }

    bool XmlDocumentNavigator::getIsEmptyElementProperty() const {
        if (pos_ != Pos::Node || node_->getNodeTypeProperty() != System::Xml::XmlNodeType::Element) return false;
        return !node_->getHasChildNodesProperty();
    }

    bool XmlDocumentNavigator::MoveToParent() {
        if (pos_ == Pos::Attribute || pos_ == Pos::Namespace) {
            pos_ = Pos::Node;
            attr_ = nullptr;
            nsList_.clear();
            nsIndex_ = -1;
            return true;
        }
        System::Xml::XmlNode* parent = node_->getParentNodeProperty();
        if (!parent) return false;
        node_ = parent;
        return true;
    }

    bool XmlDocumentNavigator::MoveToFirstChild() {
        if (pos_ != Pos::Node) return false;
        System::Xml::XmlNode* child = node_->getFirstChildProperty();
        while (child && !HasXPathRepresentation(child)) child = child->getNextSiblingProperty();
        if (!child) return false;
        node_ = child;
        return true;
    }

    bool XmlDocumentNavigator::MoveToNext() {
        if (pos_ != Pos::Node) return false;
        System::Xml::XmlNode* sib = node_->getNextSiblingProperty();
        while (sib && !HasXPathRepresentation(sib)) sib = sib->getNextSiblingProperty();
        if (!sib) return false;
        node_ = sib;
        return true;
    }

    bool XmlDocumentNavigator::MoveToPrevious() {
        if (pos_ != Pos::Node) return false;
        System::Xml::XmlNode* sib = node_->getPreviousSiblingProperty();
        while (sib && !HasXPathRepresentation(sib)) sib = sib->getPreviousSiblingProperty();
        if (!sib) return false;
        node_ = sib;
        return true;
    }

    std::vector<System::Xml::XmlAttribute*> XmlDocumentNavigator::FilteredAttributes() const {
        std::vector<System::Xml::XmlAttribute*> result;
        if (node_->getNodeTypeProperty() != System::Xml::XmlNodeType::Element) return result;
        auto* el = static_cast<System::Xml::XmlElement*>(node_);
        System::Xml::XmlAttributeCollection* coll = el->getAttributesProperty();
        for (SharpRuntime::intcs i = 0; i < coll->getCountProperty(); ++i) {
            System::Xml::XmlAttribute* a = (*coll)[i];
            if (!IsXmlnsAttributeName(a->getNameProperty(), nullptr)) result.push_back(a);
        }
        return result;
    }

    bool XmlDocumentNavigator::MoveToFirstAttribute() {
        if (pos_ != Pos::Node) return false;
        auto attrs = FilteredAttributes();
        if (attrs.empty()) return false;
        pos_ = Pos::Attribute;
        attr_ = attrs.front();
        return true;
    }

    bool XmlDocumentNavigator::MoveToNextAttribute() {
        if (pos_ != Pos::Attribute) return false;
        auto attrs = FilteredAttributes();
        auto it = std::find(attrs.begin(), attrs.end(), attr_);
        if (it == attrs.end()) return false;
        ++it;
        if (it == attrs.end()) return false;
        attr_ = *it;
        return true;
    }

    bool XmlDocumentNavigator::MoveToFirstNamespace(XPathNamespaceScope scope) {
        if (pos_ != Pos::Node || node_->getNodeTypeProperty() != System::Xml::XmlNodeType::Element) return false;

        std::vector<std::pair<std::string, std::string>> list;
        std::set<std::string> seenPrefixes;
        for (System::Xml::XmlNode* cur = node_; cur; cur = cur->getParentNodeProperty()) {
            if (cur->getNodeTypeProperty() != System::Xml::XmlNodeType::Element) break;
            auto* el = static_cast<System::Xml::XmlElement*>(cur);
            System::Xml::XmlAttributeCollection* coll = el->getAttributesProperty();
            for (SharpRuntime::intcs i = 0; i < coll->getCountProperty(); ++i) {
                System::Xml::XmlAttribute* a = (*coll)[i];
                std::string prefix;
                if (!IsXmlnsAttributeName(a->getNameProperty(), &prefix)) continue;
                if (!seenPrefixes.insert(prefix).second) continue;
                std::string uri = a->getValueProperty();
                // Exclude "xmlns=''" (default-namespace cancellation) unless scope == Local, matching .NET.
                if (!prefix.empty() || !uri.empty() || scope == XPathNamespaceScope::Local)
                    list.emplace_back(prefix, uri);
            }
            if (scope == XPathNamespaceScope::Local) break;
        }
        if (scope == XPathNamespaceScope::All && seenPrefixes.insert("xml").second)
            list.emplace_back("xml", "http://www.w3.org/XML/1998/namespace");

        if (list.empty()) return false;
        nsList_ = std::move(list);
        nsScope_ = scope;
        nsIndex_ = 0;
        pos_ = Pos::Namespace;
        return true;
    }

    bool XmlDocumentNavigator::MoveToNextNamespace(XPathNamespaceScope scope) {
        if (pos_ != Pos::Namespace || scope != nsScope_) return false;
        if (nsIndex_ + 1 >= static_cast<SharpRuntime::intcs>(nsList_.size())) return false;
        ++nsIndex_;
        return true;
    }

    bool XmlDocumentNavigator::IsSamePosition(const XPathNavigator& other) const {
        const auto* o = dynamic_cast<const XmlDocumentNavigator*>(&other);
        if (!o) return false;
        if (pos_ != o->pos_ || node_ != o->node_) return false;
        switch (pos_) {
            case Pos::Node: return true;
            case Pos::Attribute: return attr_ == o->attr_;
            case Pos::Namespace:
                return nsIndex_ >= 0 && o->nsIndex_ >= 0 &&
                       nsList_[static_cast<size_t>(nsIndex_)].first == o->nsList_[static_cast<size_t>(o->nsIndex_)].first;
        }
        return false;
    }

} // namespace System::Xml::XPath
