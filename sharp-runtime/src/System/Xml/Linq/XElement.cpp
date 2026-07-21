// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/Linq/XElement.hpp"
#include <algorithm>
#include <fstream>
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Xml/Linq/XDocument.hpp"
#include "System/Xml/Linq/XText.hpp"
#include "System/Xml/XmlException.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml::Linq {

    using System::Xml::XmlNodeType;

    void XElement::RelinkAttributes() {
        for (size_t i = 0; i < attributes_.size(); ++i) {
            attributes_[i]->setNextAttributeProperty(i + 1 < attributes_.size() ? attributes_[i + 1].get() : nullptr);
        }
    }

    void XElement::Add(std::shared_ptr<XAttribute> attr) {
        if (!attr) return;
        for (auto& a : attributes_) {
            if (a.get() != attr.get() && a->getNameProperty() == attr->getNameProperty()) {
                throw System::InvalidOperationException("Duplicate attribute: " + attr->getNameProperty().ToString());
            }
        }
        if (XElement* owner = attr->getParentProperty()) {
            owner->RemoveAttribute(attr.get());
        }
        AdoptObject(*attr, this);
        attributes_.push_back(std::move(attr));
        RelinkAttributes();
    }

    void XElement::Add(const std::string& text) {
        // Verified against XContainer.cs's AddString(): if the current last child is already a
        // plain XText (not XCData), real .NET appends into its existing Value rather than
        // creating a new sibling text node -- e.g. two consecutive Add(string) calls produce
        // one merged text node, not two adjacent ones. An empty string is a genuine no-op (no
        // node created at all), matching AddString's `if (s.Length > 0)` guard.
        if (text.empty()) return;
        if (!children_.empty()) {
            auto& last = children_.back();
            if (last->getNodeTypeProperty() == XmlNodeType::Text) {
                auto* lastText = static_cast<XText*>(last.get());
                lastText->setValueProperty(lastText->getValueProperty() + text);
                return;
            }
        }
        XContainer::Add(std::make_shared<XText>(text));
    }

    bool XElement::getHasElementsProperty() const {
        return std::any_of(children_.begin(), children_.end(),
                            [](const std::shared_ptr<XNode>& n) { return n->getNodeTypeProperty() == XmlNodeType::Element; });
    }

    void XElement::RemoveAttributes() {
        for (auto& a : attributes_) {
            AdoptObject(*a, nullptr);
            a->setNextAttributeProperty(nullptr);
        }
        attributes_.clear();
    }

    void XElement::RemoveAttribute(XAttribute* attr) {
        auto it = std::find_if(attributes_.begin(), attributes_.end(),
                                [attr](const std::shared_ptr<XAttribute>& a) { return a.get() == attr; });
        if (it == attributes_.end()) return;
        AdoptObject(**it, nullptr);
        (*it)->setNextAttributeProperty(nullptr);
        attributes_.erase(it);
        RelinkAttributes();
    }

    void XElement::RemoveAll() {
        RemoveAttributes();
        RemoveNodes();
    }

    void XElement::AppendTextValue(std::string& sb) const {
        for (auto& child : children_) {
            auto nt = child->getNodeTypeProperty();
            if (nt == XmlNodeType::Text || nt == XmlNodeType::CDATA) {
                sb += static_cast<const XText*>(child.get())->getValueProperty();
            } else if (nt == XmlNodeType::Element) {
                static_cast<const XElement*>(child.get())->AppendTextValue(sb);
            }
        }
    }

    std::string XElement::getValueProperty() const {
        std::string sb;
        AppendTextValue(sb);
        return sb;
    }

    void XElement::setValueProperty(const std::string& v) {
        RemoveNodes();
        if (!v.empty()) XContainer::Add(std::make_shared<XText>(v));
    }

    void XElement::WriteTo(System::Xml::XmlWriter& writer) const {
        writer.WriteStartElement(name_.getLocalNameProperty());
        for (auto& a : attributes_) {
            // Verified: XName::ToString() returns Clark notation ("{namespace}local") for a
            // namespace-qualified name. That string was previously passed directly as the
            // *attribute name* to WriteAttributeString -- '{'/'}' are not legal in an XML Name
            // production, so this produced literally malformed, unparseable XML for any
            // namespaced attribute (not just a fidelity gap: Save()-then-Parse() would fail or
            // silently corrupt the attribute). This port's XmlWriter has no namespace/prefix-
            // aware WriteAttributeString overload (matching XElement's own getNameProperty()
            // write path a few lines up, which already only ever writes the local name and
            // silently drops the element's own namespace -- a separately-tracked, lower-
            // severity fidelity gap, not corruption). Using the local name here makes
            // attributes consistent with that existing, valid-XML-but-namespace-lossy
            // behavior instead of producing invalid XML.
            writer.WriteAttributeString(a->getNameProperty().getLocalNameProperty(), a->getValueProperty());
        }
        for (auto& c : children_) {
            c->WriteTo(writer);
        }
        writer.WriteEndElement();
    }

    void XElement::Save(const std::string& fileName, SaveOptions options) const {
        std::ofstream ofs(fileName, std::ios::out | std::ios::trunc);
        if (!ofs) throw System::Xml::XmlException("XElement::Save: failed to open '" + fileName + "'.");
        bool indent = (options & SaveOptions::DisableFormatting) == SaveOptions::None;
        SerializeTo(ofs, 0, indent);
    }

    void XElement::Save(System::Xml::XmlWriter& writer) const {
        WriteTo(writer);
    }

    std::shared_ptr<XElement> XElement::Parse(const std::string& xml, LoadOptions options) {
        auto doc = XDocument::Parse(xml, options);
        auto root = doc->getRootProperty();
        if (root) root->Remove(); // detach from the temporary XDocument (see XContainer's doc-comment on lifetime)
        return root;
    }

    std::shared_ptr<XElement> XElement::Load(const std::string& filePath, LoadOptions options) {
        auto doc = XDocument::Load(filePath, options);
        auto root = doc->getRootProperty();
        if (root) root->Remove();
        return root;
    }

    void XElement::SerializeTo(std::ostream& os, int depth, bool indent) const {
        std::string pad = indent ? std::string(static_cast<size_t>(depth) * 2, ' ') : "";
        os << pad << "<" << name_.getLocalNameProperty();
        for (auto& a : attributes_) os << " " << a->ToString();
        if (children_.empty()) {
            os << "/>";
            return;
        }
        os << ">";
        bool multiline = indent && std::any_of(children_.begin(), children_.end(), [](const std::shared_ptr<XNode>& c) {
            auto nt = c->getNodeTypeProperty();
            return nt != XmlNodeType::Text && nt != XmlNodeType::CDATA;
        });
        for (auto& c : children_) {
            auto nt = c->getNodeTypeProperty();
            bool isText = (nt == XmlNodeType::Text || nt == XmlNodeType::CDATA);
            if (multiline && !isText) os << "\n";
            c->SerializeTo(os, depth + 1, indent);
        }
        if (multiline) os << "\n" << pad;
        os << "</" << name_.getLocalNameProperty() << ">";
    }

    SharpRuntime::intcs XElement::GetDeepHashCode() const {
        SharpRuntime::intcs h = static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(name_.ToString()));
        for (auto& a : attributes_) {
            h ^= static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(a->getNameProperty().ToString()));
            h ^= static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(a->getValueProperty()));
        }
        for (auto& c : children_) {
            auto nt = c->getNodeTypeProperty();
            if (nt == XmlNodeType::Comment || nt == XmlNodeType::ProcessingInstruction) continue;
            h ^= c->GetDeepHashCode();
        }
        return h;
    }

    void XElement::ValidateNode(const XNode& node) const {
        // Verified against XElement.cs's ValidateNode: an XDocument or XDocumentType can never
        // be a legal child of an element (only of an XDocument, which enforces its own,
        // separate single-root/single-doctype rules) -- without this check, tinyxml2's own
        // insertion has no such restriction and would silently produce a structurally invalid
        // tree (a nested document, or a doctype declaration inside element content).
        auto nt = node.getNodeTypeProperty();
        if (nt == XmlNodeType::Document)
            throw System::ArgumentException("This operation would create an incorrectly structured document (cannot add an XDocument as a child of an XElement).");
        if (nt == XmlNodeType::DocumentType)
            throw System::ArgumentException("This operation would create an incorrectly structured document (cannot add an XDocumentType as a child of an XElement).");
    }

    bool XElement::DeepEqualsCore(const XNode& other) const {
        const auto& o = static_cast<const XElement&>(other);
        if (name_ != o.name_) return false;
        // Verified against XElement.cs's AttributesEqual: real .NET walks both attribute lists
        // in parallel by *position*, not by name lookup -- two elements with the same
        // attributes in a different order are NOT deep-equal (attribute order is part of an
        // XElement's identity for DeepEquals purposes, even though lookup-by-name is
        // order-independent).
        if (attributes_.size() != o.attributes_.size()) return false;
        for (size_t i = 0; i < attributes_.size(); ++i) {
            if (attributes_[i]->getNameProperty() != o.attributes_[i]->getNameProperty() ||
                attributes_[i]->getValueProperty() != o.attributes_[i]->getValueProperty())
                return false;
        }
        // Ignore comments/processing instructions on both sides; pairwise-compare the rest.
        std::vector<const XNode*> lhs, rhs;
        for (auto& c : children_) {
            auto nt = c->getNodeTypeProperty();
            if (nt != XmlNodeType::Comment && nt != XmlNodeType::ProcessingInstruction) lhs.push_back(c.get());
        }
        for (auto& c : o.children_) {
            auto nt = c->getNodeTypeProperty();
            if (nt != XmlNodeType::Comment && nt != XmlNodeType::ProcessingInstruction) rhs.push_back(c.get());
        }
        if (lhs.size() != rhs.size()) return false;
        for (size_t i = 0; i < lhs.size(); ++i) {
            if (!XNode::DeepEquals(lhs[i], rhs[i])) return false;
        }
        return true;
    }

} // namespace System::Xml::Linq
