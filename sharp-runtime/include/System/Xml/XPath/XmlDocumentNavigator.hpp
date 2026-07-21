// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "System/Xml/IHasXmlNode.hpp"
#include "System/Xml/XPath/XPathNamespaceScope.hpp"
#include "System/Xml/XPath/XPathNavigator.hpp"

namespace System::Xml {
    class XmlDocument;
    class XmlNode;
    class XmlAttribute;
} // namespace System::Xml

namespace System::Xml::XPath {

    /**
     * @brief The concrete XPathNavigator implementation over this runtime's classic
     * System::Xml::XmlDocument DOM (constructed via XmlNode::CreateNavigator() /
     * XmlDocument::CreateNavigator()).
     *
     * C++ counterpart of .NET's internal DocumentXPathNavigator (the concrete navigator real
     * .NET's XmlDocument.CreateNavigator() returns). Per the user's 2026-07-07 scope decision,
     * this is the only concrete XPathNavigator in this runtime — there is deliberately no
     * separate navigator over System.Xml.Linq's XDocument/XElement tree (that is separate,
     * concurrent work this port does not touch).
     *
     * Position is represented as either a DOM node (Root/Element/Text/Comment/etc.), an
     * attribute of an element (attributes are not part of the tinyxml2 node tree, so they are
     * tracked as an owning element + a stable XmlAttribute* identity), or a namespace
     * declaration of an element (synthesized from that element's ancestor "xmlns"/"xmlns:*"
     * attributes — there is no dedicated DOM node type for these, matching how .NET's own
     * DocumentXPathNavigator materializes namespace nodes on demand).
     */
    class XmlDocumentNavigator final : public XPathNavigator, public System::Xml::IHasXmlNode {
        enum class Pos { Node, Attribute, Namespace };

        // Owning document, mirroring .NET's DocumentXPathNavigator; not read by the currently
        // implemented (read-only, tree-navigation) subset of XPathNavigator.
        [[maybe_unused]] System::Xml::XmlDocument* doc_ = nullptr;
        Pos pos_ = Pos::Node;
        System::Xml::XmlNode* node_ = nullptr;         // Pos::Node: current node. Pos::Attribute/Namespace: owning element.
        System::Xml::XmlAttribute* attr_ = nullptr;    // Pos::Attribute: current attribute (stable identity).
        std::vector<std::pair<std::string, std::string>> nsList_; // Pos::Namespace: cached (prefix, uri) list for node_ at nsScope_.
        SharpRuntime::intcs nsIndex_ = -1;
        XPathNamespaceScope nsScope_ = XPathNamespaceScope::All;

    public:
        XmlDocumentNavigator(System::Xml::XmlDocument* doc, System::Xml::XmlNode* node) : doc_(doc), node_(node) {}
        XmlDocumentNavigator(const XmlDocumentNavigator&) = default;

        [[nodiscard]] XPathNavigator* Clone() const override { return new XmlDocumentNavigator(*this); }

        // IHasXmlNode
        [[nodiscard]] System::Xml::XmlNode* GetNode() const override;

        [[nodiscard]] XPathNodeType getNodeTypeProperty() const override;
        [[nodiscard]] std::string getNameProperty() const override;
        [[nodiscard]] std::string getLocalNameProperty() const override;
        [[nodiscard]] std::string getNamespaceURIProperty() const override;
        [[nodiscard]] std::string getPrefixProperty() const override;
        [[nodiscard]] std::string getValueProperty() const override;
        [[nodiscard]] std::string getBaseURIProperty() const override { return {}; }
        [[nodiscard]] bool getIsEmptyElementProperty() const override;

        [[nodiscard]] bool MoveToParent() override;
        [[nodiscard]] bool MoveToFirstChild() override;
        [[nodiscard]] bool MoveToNext() override;
        [[nodiscard]] bool MoveToPrevious() override;
        [[nodiscard]] bool MoveToFirstAttribute() override;
        [[nodiscard]] bool MoveToNextAttribute() override;
        [[nodiscard]] bool MoveToFirstNamespace(XPathNamespaceScope scope = XPathNamespaceScope::All) override;
        [[nodiscard]] bool MoveToNextNamespace(XPathNamespaceScope scope = XPathNamespaceScope::All) override;

        [[nodiscard]] bool IsSamePosition(const XPathNavigator& other) const override;

    private:
        [[nodiscard]] std::vector<System::Xml::XmlAttribute*> FilteredAttributes() const;
    };

} // namespace System::Xml::XPath
