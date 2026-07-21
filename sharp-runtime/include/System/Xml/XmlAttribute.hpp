// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlNode.hpp"

namespace tinyxml2 { class XMLElement; }

namespace System::Xml {

    /**
     * @brief Represents an XML attribute.
     *
     * C++ counterpart of .NET System.Xml.XmlAttribute. tinyxml2 ties attributes directly to
     * their owning XMLElement (there is no API to create a detached tinyxml2::XMLAttribute), so
     * this wrapper stores name/value locally and, once attached to an element (via
     * XmlElement::SetAttributeNode), reads/writes through the owning tinyxml2::XMLElement
     * instead — matching .NET's observable behavior (value changes are visible through the
     * element) without needing to track individual tinyxml2::XMLAttribute pointers, which
     * tinyxml2 may reallocate internally.
     */
    class XmlAttribute : public XmlNode {
        std::string name_;
        std::string localValue_;
        tinyxml2::XMLElement* ownerElementNative_ = nullptr;
        XmlElement* ownerElementWrapper_ = nullptr;

    public:
        XmlAttribute(XmlDocument* ownerDocument, std::string name)
            : name_(std::move(name)) { ownerDocument_ = ownerDocument; }

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const override { return XmlNodeType::Attribute; }
        [[nodiscard]] std::string getNameProperty() const override { return name_; }

        [[nodiscard]] std::string getValueProperty() const override;
        void setValueProperty(const std::string& value) override;
        [[nodiscard]] std::string getInnerTextProperty() const override { return getValueProperty(); }
        void setInnerTextProperty(const std::string& text) override { setValueProperty(text); }

        /**
         * @return The namespace URI declared for this attribute's prefix via the nearest
         * xmlns:prefix declaration on the owning element or one of its ancestors, or "" for an
         * unprefixed attribute (per the XML Namespaces spec, an element's default `xmlns`
         * declaration does not apply to its own unprefixed attributes).
         */
        [[nodiscard]] std::string getNamespaceURIProperty() const override;

        /** @brief Creates a duplicate of this attribute (always a full/"deep" copy, matching .NET — the deep parameter is ignored since an attribute's value has no meaningful shallow form). */
        [[nodiscard]] XmlNode* CloneNode(bool deep) const override;

        /** @return Always nullptr; attributes are not children in the DOM tree sense. */
        [[nodiscard]] XmlNode* getParentNodeProperty() const { return nullptr; }
        /** @return The element that owns this attribute, or nullptr if unattached. */
        [[nodiscard]] XmlElement* getOwnerElementProperty() const { return ownerElementWrapper_; }
        /** @return Always true; this runtime does not track whether an attribute came from a DTD default. */
        [[nodiscard]] bool getSpecifiedProperty() const { return true; }

        /** @brief Internal: attaches a previously-unattached attribute to @p element, pushing its local value onto it for the first time. Used by XmlElement::SetAttributeNode. */
        void AttachTo(XmlElement* elementWrapper, tinyxml2::XMLElement* elementNative);
        /** @brief Internal: binds this wrapper to an attribute that already exists live on @p element, WITHOUT touching its current value. Used by XmlElement to wrap already-set attributes. */
        void BindExisting(XmlElement* elementWrapper, tinyxml2::XMLElement* elementNative);
        /** @brief Internal: detaches this wrapper (keeps the last known value locally). Used by XmlElement. */
        void Detach();
        /** @return true if this attribute is currently attached to an element. */
        [[nodiscard]] bool IsAttached() const { return ownerElementNative_ != nullptr; }
    };

} // namespace System::Xml
