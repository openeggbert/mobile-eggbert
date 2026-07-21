// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlNode.hpp"

#include <tinyxml2/tinyxml2.h>

#include <memory>

#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Xml/XPath/XPathNodeIterator.hpp"
#include "System/Xml/XPath/XmlDocumentNavigator.hpp"
#include "System/Xml/IHasXmlNode.hpp"
#include "System/Xml/XmlAttribute.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlDocumentFragment.hpp"
#include "System/Xml/XmlElement.hpp"
#include "System/Xml/XmlNodeList.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml {

    namespace {
        // Verified against XmlNode.cs's InsertBefore/InsertAfter/AppendChild: real .NET throws
        // ArgumentException("Cannot insert a node or any ancestor of that node as a child of
        // itself.") when the node being inserted is `this` itself or one of this's ancestors --
        // otherwise the reparent creates a cycle. tinyxml2 performs no such check itself: its
        // InsertChildPreamble() unconditionally unlinks the node from its current parent and
        // reattaches it, so inserting an ancestor produces a genuine two-node parent/child cycle
        // (the node ends up as both its own descendant and its own ancestor).
        void ThrowIfSelfOrAncestor(const tinyxml2::XMLNode* thisNative, const tinyxml2::XMLNode* candidate) {
            for (const tinyxml2::XMLNode* n = thisNative; n; n = n->Parent()) {
                if (n == candidate)
                    throw System::ArgumentException("Cannot insert a node or any ancestor of that node as a child of itself.");
            }
        }

        // Verified against XmlNode.cs's InsertBefore/InsertAfter/AppendChild: real .NET throws
        // ArgumentException("The node to be inserted is from a different document context.")
        // rather than silently doing nothing. tinyxml2's InsertEndChild/InsertFirstChild/
        // InsertAfterChild already refuse a cross-document insert internally (comparing
        // XMLNode::_document), but they do so by silently returning nullptr -- this wrapper
        // never checked that return value, so a cross-document insert previously reported
        // success (returning newChild unchanged) while actually doing nothing.
        void ThrowIfDifferentDocument(const XmlNode* thisNode, const XmlDocument* thisDoc, const XmlDocument* childDoc) {
            if (childDoc && childDoc != thisDoc && static_cast<const void*>(childDoc) != static_cast<const void*>(thisNode))
                throw System::ArgumentException("The node to be inserted is from a different document context.");
        }

        // Verified against XmlNode.cs's Normalize(): real .NET recurses into the full depth of
        // the sub-tree (`case XmlNodeType.Element: crtChild.Normalize(); goto default;`), not
        // just direct children -- this port previously only merged adjacent text nodes among
        // `this`'s immediate children, silently leaving un-merged runs inside any descendant
        // element untouched.
        void NormalizeNative(tinyxml2::XMLNode* native, XmlDocument* doc) {
            tinyxml2::XMLNode* child = native->FirstChild();
            while (child) {
                tinyxml2::XMLNode* next = child->NextSibling();
                if (child->ToElement()) {
                    NormalizeNative(child, doc);
                    child = next;
                    continue;
                }
                auto* text = child->ToText();
                if (text && !text->CData() && next) {
                    auto* nextText = next->ToText();
                    if (nextText && !nextText->CData()) {
                        std::string merged = (text->Value() ? text->Value() : std::string());
                        merged += (nextText->Value() ? nextText->Value() : std::string());
                        text->SetValue(merged.c_str());
                        if (doc) doc->PurgeCache(next);
                        native->DeleteChild(next); // genuinely discarded (merged away), not detached for reuse
                        continue; // re-check this node against its new next sibling
                    }
                }
                child = next;
            }
        }
    }

    XmlNode::~XmlNode() = default;

    std::string XmlNode::getNameProperty() const {
        return native_ && native_->Value() ? native_->Value() : "";
    }

    std::string XmlNode::getLocalNameProperty() const {
        std::string name = getNameProperty();
        size_t colon = name.find(':');
        return colon == std::string::npos ? name : name.substr(colon + 1);
    }

    std::string XmlNode::getPrefixProperty() const {
        std::string name = getNameProperty();
        size_t colon = name.find(':');
        return colon == std::string::npos ? "" : name.substr(0, colon);
    }

    std::string XmlNode::getNamespaceURIProperty() const {
        std::string prefix = getPrefixProperty();
        std::string attrName = prefix.empty() ? "xmlns" : "xmlns:" + prefix;
        // Walk up through ancestor elements looking for the nearest xmlns/xmlns:prefix declaration.
        for (tinyxml2::XMLNode* n = native_; n; n = n->Parent()) {
            if (auto* el = n->ToElement()) {
                const char* v = el->Attribute(attrName.c_str());
                if (v) return v;
            }
        }
        return {};
    }

    std::string XmlNode::getValueProperty() const { return {}; }

    void XmlNode::setValueProperty(const std::string&) {
        throw System::InvalidOperationException("Value setting is not implemented on node type " +
                                                 std::to_string(static_cast<int>(getNodeTypeProperty())) + ".");
    }

    std::string XmlNode::getInnerTextProperty() const {
        std::string result;
        if (!native_) return result;
        XmlDocument* doc = GetDocument();
        for (tinyxml2::XMLNode* child = native_->FirstChild(); child; child = child->NextSibling()) {
            if (auto* txt = child->ToText()) {
                result += txt->Value() ? txt->Value() : "";
            } else if (child->ToElement()) {
                if (auto* wrapped = doc ? doc->WrapNode(child) : nullptr)
                    result += wrapped->getInnerTextProperty();
            }
        }
        return result;
    }

    void XmlNode::setInnerTextProperty(const std::string& text) {
        RemoveAllChildren();
        XmlDocument* doc = GetDocument();
        if (!text.empty() && doc) AppendChild(doc->CreateTextNode(text));
    }

    std::string XmlNode::getInnerXmlProperty() const {
        if (!native_) return {};
        // compact=true: real .NET's InnerXml serializes exact markup with no inserted
        // whitespace between nodes. tinyxml2::XMLPrinter defaults to compact=false (pretty-
        // printed with inserted newlines/indentation), which this port previously left
        // unchanged, silently reformatting the tree's own markup instead of reproducing it.
        tinyxml2::XMLPrinter printer(nullptr, /*compact=*/true);
        for (tinyxml2::XMLNode* child = native_->FirstChild(); child; child = child->NextSibling())
            child->Accept(&printer);
        return printer.CStr() ? printer.CStr() : "";
    }

    void XmlNode::setInnerXmlProperty(const std::string& xml) {
        RemoveAllChildren();
        XmlDocument* doc = GetDocument();
        if (xml.empty() || !doc) return;
        tinyxml2::XMLDocument fragmentDoc;
        std::string wrapped = "<root>" + xml + "</root>";
        fragmentDoc.Parse(wrapped.c_str());
        auto* root = fragmentDoc.RootElement();
        if (!root) return;
        for (tinyxml2::XMLNode* child = root->FirstChild(); child; child = child->NextSibling()) {
            auto* cloned = child->DeepClone(&doc->getNativeDocument());
            native_->InsertEndChild(cloned);
        }
    }

    std::string XmlNode::getOuterXmlProperty() const {
        if (!native_) return {};
        // See getInnerXmlProperty()'s comment: compact=true matches real .NET's OuterXml
        // producing exact markup with no inserted whitespace.
        tinyxml2::XMLPrinter printer(nullptr, /*compact=*/true);
        native_->Accept(&printer);
        return printer.CStr() ? printer.CStr() : "";
    }

    XmlNode* XmlNode::getParentNodeProperty() const {
        XmlDocument* doc = GetDocument();
        if (!native_ || !doc) return nullptr;
        auto* parent = native_->Parent();
        if (doc->IsDetached(parent)) return nullptr;
        return doc->WrapNode(parent);
    }

    XmlNode* XmlNode::getFirstChildProperty() const {
        XmlDocument* doc = GetDocument();
        return native_ && doc ? doc->WrapNode(native_->FirstChild()) : nullptr;
    }

    XmlNode* XmlNode::getLastChildProperty() const {
        XmlDocument* doc = GetDocument();
        return native_ && doc ? doc->WrapNode(native_->LastChild()) : nullptr;
    }

    XmlNode* XmlNode::getPreviousSiblingProperty() const {
        XmlDocument* doc = GetDocument();
        return native_ && doc ? doc->WrapNode(native_->PreviousSibling()) : nullptr;
    }

    XmlNode* XmlNode::getNextSiblingProperty() const {
        XmlDocument* doc = GetDocument();
        return native_ && doc ? doc->WrapNode(native_->NextSibling()) : nullptr;
    }

    XmlNodeList* XmlNode::getChildNodesProperty() const {
        std::vector<XmlNode*> children;
        XmlDocument* doc = GetDocument();
        if (native_ && doc)
            for (tinyxml2::XMLNode* child = native_->FirstChild(); child; child = child->NextSibling())
                children.push_back(doc->WrapNode(child));
        childNodesSnapshot_ = std::make_unique<XmlNodeList>(std::move(children));
        return childNodesSnapshot_.get();
    }

    bool XmlNode::getHasChildNodesProperty() const {
        return native_ && native_->FirstChild() != nullptr;
    }

    XmlNode* XmlNode::PrependChild(XmlNode* newChild) {
        if (!native_ || !newChild) return newChild;
        ThrowIfSelfOrAncestor(native_, newChild->native_);
        ThrowIfDifferentDocument(this, GetDocument(), newChild->GetDocument());
        if (auto* frag = dynamic_cast<XmlDocumentFragment*>(newChild)) {
            XmlNode* firstInserted = nullptr;
            XmlNode* child = frag->getLastChildProperty();
            while (child) {
                XmlNode* prev = child->getPreviousSiblingProperty();
                ThrowIfSelfOrAncestor(native_, child->native_);
                frag->RemoveChild(child);
                native_->InsertFirstChild(child->native_);
                firstInserted = child;
                child = prev;
            }
            return firstInserted;
        }
        native_->InsertFirstChild(newChild->native_);
        return newChild;
    }

    XmlNode* XmlNode::AppendChild(XmlNode* newChild) {
        if (!native_ || !newChild) return newChild;
        ThrowIfSelfOrAncestor(native_, newChild->native_);
        ThrowIfDifferentDocument(this, GetDocument(), newChild->GetDocument());
        if (auto* frag = dynamic_cast<XmlDocumentFragment*>(newChild)) {
            XmlNode* lastInserted = nullptr;
            XmlNode* child = frag->getFirstChildProperty();
            while (child) {
                XmlNode* next = child->getNextSiblingProperty();
                ThrowIfSelfOrAncestor(native_, child->native_);
                frag->RemoveChild(child);
                native_->InsertEndChild(child->native_);
                lastInserted = child;
                child = next;
            }
            return lastInserted;
        }
        native_->InsertEndChild(newChild->native_);
        return newChild;
    }

    XmlNode* XmlNode::InsertBefore(XmlNode* newChild, XmlNode* refChild) {
        if (!refChild) return AppendChild(newChild);
        if (!native_ || !newChild) return newChild;
        ThrowIfSelfOrAncestor(native_, newChild->native_);
        ThrowIfDifferentDocument(this, GetDocument(), newChild->GetDocument());
        tinyxml2::XMLNode* prevSibling = refChild->native_ ? refChild->native_->PreviousSibling() : nullptr;
        if (!prevSibling) { native_->InsertFirstChild(newChild->native_); return newChild; }
        native_->InsertAfterChild(prevSibling, newChild->native_);
        return newChild;
    }

    XmlNode* XmlNode::InsertAfter(XmlNode* newChild, XmlNode* refChild) {
        if (!refChild) return PrependChild(newChild);
        if (!native_ || !newChild) return newChild;
        ThrowIfSelfOrAncestor(native_, newChild->native_);
        ThrowIfDifferentDocument(this, GetDocument(), newChild->GetDocument());
        native_->InsertAfterChild(refChild->native_, newChild->native_);
        return newChild;
    }

    XmlNode* XmlNode::RemoveChild(XmlNode* oldChild) {
        XmlDocument* doc = GetDocument();
        if (!native_ || !oldChild || !oldChild->native_ || !doc) return oldChild;
        doc->DetachNode(oldChild->native_);
        return oldChild;
    }

    XmlNode* XmlNode::ReplaceChild(XmlNode* newChild, XmlNode* oldChild) {
        InsertBefore(newChild, oldChild);
        RemoveChild(oldChild);
        return oldChild;
    }

    void XmlNode::RemoveAllChildren() {
        XmlDocument* doc = GetDocument();
        if (!native_ || !doc) return;
        for (tinyxml2::XMLNode* child = native_->FirstChild(); child; child = child->NextSibling())
            doc->PurgeCache(child);
        native_->DeleteChildren();
    }

    void XmlNode::RemoveAll() {
        RemoveAllChildren();
    }

    XmlNode* XmlNode::CloneNode(bool deep) const {
        XmlDocument* doc = GetDocument();
        if (!native_ || !doc) return nullptr;
        tinyxml2::XMLNode* cloned = deep
            ? native_->DeepClone(&doc->getNativeDocument())
            : native_->ShallowClone(&doc->getNativeDocument());
        return doc->WrapNode(cloned);
    }

    void XmlNode::Normalize() {
        if (!native_) return;
        NormalizeNative(native_, GetDocument());
    }

    XmlElement* XmlNode::Item(const std::string& name) const {
        XmlDocument* doc = GetDocument();
        if (!native_ || !doc) return nullptr;
        for (tinyxml2::XMLNode* child = native_->FirstChild(); child; child = child->NextSibling())
            if (auto* el = child->ToElement())
                if (el->Name() && name == el->Name())
                    return static_cast<XmlElement*>(doc->WrapNode(child));
        return nullptr;
    }

    bool XmlNode::Supports(const std::string&, const std::string&) const { return false; }

    void XmlNode::WriteTo(XmlWriter& writer) const {
        WriteContentTo(writer);
    }

    void XmlNode::WriteContentTo(XmlWriter& writer) const {
        XmlDocument* doc = GetDocument();
        if (!native_ || !doc) return;
        for (tinyxml2::XMLNode* child = native_->FirstChild(); child; child = child->NextSibling())
            if (auto* wrapped = doc->WrapNode(child))
                wrapped->WriteTo(writer);
    }

    XPath::XPathNavigator* XmlNode::CreateNavigator() const {
        XmlDocument* doc = GetDocument();
        if (getNodeTypeProperty() == XmlNodeType::Attribute) {
            // Attributes are not part of the tinyxml2 node tree, so XmlDocumentNavigator tracks
            // them as (owning element, attribute identity) rather than a native_-bearing node;
            // re-locate this attribute from its owner so the navigator lands in the Attribute
            // position instead of misreporting it as a bare (parentless) content node.
            const auto* attr = static_cast<const XmlAttribute*>(this);
            XmlElement* owner = attr->getOwnerElementProperty();
            if (!owner)
                throw System::InvalidOperationException("Cannot create a navigator for a detached attribute.");
            auto* nav = new XPath::XmlDocumentNavigator(doc, owner);
            if (nav->MoveToAttribute(attr->getLocalNameProperty(), attr->getNamespaceURIProperty())) return nav;
            delete nav;
            throw System::InvalidOperationException("Cannot create a navigator for a detached attribute.");
        }
        return new XPath::XmlDocumentNavigator(doc, const_cast<XmlNode*>(this));
    }

    XmlNode* XmlNode::SelectSingleNode(const std::string& xpath) const {
        std::unique_ptr<XPath::XPathNavigator> nav(CreateNavigator());
        std::unique_ptr<XPath::XPathNavigator> result(nav->SelectSingleNode(xpath));
        if (!result) return nullptr;
        const auto* hasNode = dynamic_cast<const IHasXmlNode*>(result.get());
        return hasNode ? hasNode->GetNode() : nullptr;
    }

    XmlNodeList* XmlNode::SelectNodes(const std::string& xpath) const {
        std::unique_ptr<XPath::XPathNavigator> nav(CreateNavigator());
        std::unique_ptr<XPath::XPathNodeIterator> it(nav->Select(xpath));
        std::vector<XmlNode*> nodes;
        while (it->MoveNext()) {
            const auto* hasNode = dynamic_cast<const IHasXmlNode*>(it->getCurrentProperty());
            if (hasNode && hasNode->GetNode()) nodes.push_back(hasNode->GetNode());
        }
        return new XmlNodeList(std::move(nodes));
    }

} // namespace System::Xml
