// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Xml/Linq/XName.hpp"
#include "System/Xml/Linq/XObject.hpp"

namespace System::Xml::Linq {

    class XElement;

    /**
     * @brief Represents an XML attribute (name/value pair) on an XElement.
     *
     * C++ counterpart of .NET System.Xml.Linq.XAttribute.
     *
     * @note `next_` is an intrusive next-sibling-attribute link, matching real .NET's internal
     * `XAttribute.next` design (there it's part of a circular list rooted at the owning
     * XElement's `lastAttr`; here it's simply kept in sync with the owning XElement's
     * `attributes_` vector order whenever attributes are added/removed).
     */
    class XAttribute : public XObject {
        friend class XElement;

        XName name_;
        std::string value_;
        XAttribute* next_ = nullptr;

    public:
        /**
         * @brief Constructs an attribute with an XName key and string value (plain strings
         * convert implicitly via XName).
         *
         * @throws System::ArgumentException if @p name/@p value form an invalid namespace
         * declaration (see @c ValidateAttribute in XAttribute.cpp for the exact rules).
         */
        XAttribute(const XName& name, const std::string& value);

        /** @brief Copies @p other's name/value. The clone starts detached (no parent, no next). */
        XAttribute(const XAttribute& other) : name_(other.name_), value_(other.value_) {}

        [[nodiscard]] System::Xml::XmlNodeType getNodeTypeProperty() const override { return System::Xml::XmlNodeType::Attribute; }

        /** @return The qualified name of the attribute. */
        [[nodiscard]] const XName&       getNameProperty()  const { return name_; }

        /** @return The attribute value string. */
        [[nodiscard]] const std::string& getValueProperty() const { return value_; }

        /**
         * @brief Sets the attribute value.
         * @throws System::ArgumentException if this attribute is a namespace declaration and
         * @p v would violate the namespace-declaration rules (see the constructor).
         */
        void setValueProperty(const std::string& v);

        /**
         * @return @c true if this attribute is a namespace declaration (@c xmlns="..." or
         * @c xmlns:prefix="...").
         */
        [[nodiscard]] bool getIsNamespaceDeclarationProperty() const;

        /** @return Pointer to the next sibling attribute (in owning-element order), or nullptr. */
        [[nodiscard]] XAttribute* getNextAttributeProperty() const { return next_; }

        /** Sets the next sibling attribute in the linked list. Exposed for API compatibility; normally maintained automatically by XElement::Add/RemoveAttributes. */
        void setNextAttributeProperty(XAttribute* n)               { next_ = n; }

        /** @return Pointer to the previous sibling attribute (in owning-element order), or nullptr. */
        [[nodiscard]] XAttribute* getPreviousAttributeProperty() const;

        /** @brief Removes this attribute from its owning element. @throws System::InvalidOperationException if this attribute has no parent. */
        void Remove();

        /**
         * @return The attribute serialised as name="value" (value is XML-escaped).
         *
         * @note Uses the local name only, not XName::ToString()'s Clark notation
         * ("{namespace}local") -- real .NET's ToString() resolves a proper namespace prefix via
         * an actual XmlWriter (GetPrefixOfNamespace); this port's XmlWriter has no namespace/
         * prefix-aware WriteAttributeString overload, so writing Clark notation directly as the
         * attribute *name* would produce literally malformed, unparseable XML ('{'/'}' are not
         * legal in an XML Name production) instead of just a namespace-fidelity gap.
         */
        [[nodiscard]] std::string ToString() const {
            return name_.getLocalNameProperty() + "=\"" + EscapeValue(value_) + "\"";
        }

    private:
        [[nodiscard]] static std::string EscapeValue(const std::string& s);
    };

} // namespace System::Xml::Linq
