// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <memory>
#include <unordered_map>

#include "System/Xml/XmlAttributeCollection.hpp"
#include "System/Xml/XmlLinkedNode.hpp"

namespace tinyxml2 { class XMLElement; }

namespace System::Xml {

    /**
     * @brief Represents an XML element.
     *
     * C++ counterpart of .NET System.Xml.XmlElement. Maintains a name-keyed cache of
     * XmlAttribute wrappers so repeated GetAttributeNode() calls for the same attribute name
     * return the same wrapper identity. getAttributesProperty()/GetElementsByTagName() return a
     * snapshot collection owned by this element and replaced on the next call to the same
     * method — do not retain the returned pointer across another call to it.
     */
    class XmlElement : public XmlLinkedNode {
        std::unordered_map<std::string, std::unique_ptr<XmlAttribute>> attrCache_;
        mutable std::unique_ptr<XmlAttributeCollection> attrsSnapshot_;
        mutable std::unique_ptr<XmlNodeList> elementsByTagNameSnapshot_;

        tinyxml2::XMLElement* nativeElement() const;
        XmlAttribute* GetOrCreateAttrWrapper(const std::string& name);

    public:
        XmlElement(tinyxml2::XMLNode* native, XmlDocument* ownerDocument)
            : XmlLinkedNode(native, ownerDocument) {}

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const override { return XmlNodeType::Element; }

        /** @return true if this element has no children. */
        [[nodiscard]] bool getIsEmptyProperty() const { return !getHasChildNodesProperty(); }

        /** @return This element's attributes, as a fresh snapshot collection (see class doc-comment). */
        [[nodiscard]] XmlAttributeCollection* getAttributesProperty() const override;
        /** @return true if this element has at least one attribute. */
        [[nodiscard]] bool getHasAttributesProperty() const;

        /** @return The value of attribute @p name, or "" if not present. */
        [[nodiscard]] std::string GetAttribute(const std::string& name) const;
        /** @brief Sets attribute @p name to @p value, creating it if absent. */
        void SetAttribute(const std::string& name, const std::string& value);
        /** @brief Removes attribute @p name if present (no-op otherwise). */
        void RemoveAttribute(const std::string& name);
        /** @return The XmlAttribute node named @p name, or nullptr if not present. */
        [[nodiscard]] XmlAttribute* GetAttributeNode(const std::string& name);
        /** @brief Attaches @p newAttr to this element (replacing any existing attribute of the same name). @return The replaced attribute, or nullptr. */
        XmlAttribute* SetAttributeNode(XmlAttribute* newAttr);
        /** @brief Detaches @p oldAttr from this element. @return @p oldAttr, or nullptr if it wasn't attached here. */
        XmlAttribute* RemoveAttributeNode(XmlAttribute* oldAttr);
        /** @return true if attribute @p name is present. */
        [[nodiscard]] bool HasAttribute(const std::string& name) const;

        /** @brief Removes the attribute at index @p i. @return The removed attribute, or nullptr if out of range. */
        XmlNode* RemoveAttributeAt(SharpRuntime::intcs i);
        /** @brief Removes all attributes. */
        void RemoveAllAttributes();
        void RemoveAll() override;

        /** @return All descendant elements named @p name (or "*" for all), in document order. */
        [[nodiscard]] XmlNodeList* GetElementsByTagName(const std::string& name) const;

        [[nodiscard]] std::string getInnerXmlProperty() const override;
        void setInnerXmlProperty(const std::string& xml) override;

        void WriteTo(XmlWriter& writer) const override;
        void WriteContentTo(XmlWriter& writer) const override;
    };

} // namespace System::Xml
