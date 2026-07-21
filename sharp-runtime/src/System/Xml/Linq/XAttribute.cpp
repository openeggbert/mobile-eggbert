// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/Linq/XAttribute.hpp"
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Xml/Linq/XElement.hpp"
#include "System/Xml/Linq/XNamespace.hpp"

namespace System::Xml::Linq {

    namespace {
        // Verified against XAttribute.cs's ValidateAttribute: enforces the XML Namespaces
        // spec's constraints on namespace-declaration attributes (xmlns="..." /
        // xmlns:prefix="..."). A no-op for any ordinary (non-namespace-declaration) attribute.
        void ValidateAttribute(const XName& name, const std::string& value) {
            const std::string& namespaceName = name.getNamespaceNameProperty();
            const std::string& localName = name.getLocalNameProperty();
            if (namespaceName == XNamespace::Xmlns.getNamespaceNameProperty()) {
                // xmlns:localName="value" -- localName is the prefix being declared.
                if (value.empty()) {
                    throw System::ArgumentException(
                        "The empty namespace name cannot be declared with a prefixed namespace "
                        "declaration such as xmlns:" + localName + "='...'.");
                } else if (value == XNamespace::Xml.getNamespaceNameProperty()) {
                    // 'http://www.w3.org/XML/1998/namespace' can only be declared by the
                    // 'xml' prefix namespace declaration.
                    if (localName != "xml")
                        throw System::ArgumentException(
                            "The 'http://www.w3.org/XML/1998/namespace' namespace name can "
                            "only be declared with the 'xml' prefix.");
                } else if (value == XNamespace::Xmlns.getNamespaceNameProperty()) {
                    // 'http://www.w3.org/2000/xmlns/' must not be declared by any namespace
                    // declaration.
                    throw System::ArgumentException(
                        "The 'http://www.w3.org/2000/xmlns/' namespace name cannot be declared.");
                } else if (localName == "xml") {
                    // No other namespace name can be declared by the 'xml' prefix.
                    throw System::ArgumentException(
                        "The 'xml' prefix can only be declared to be "
                        "'http://www.w3.org/XML/1998/namespace'.");
                } else if (localName == "xmlns") {
                    // The 'xmlns' prefix must not be declared.
                    throw System::ArgumentException("The 'xmlns' prefix cannot be declared.");
                }
            } else if (namespaceName.empty() && localName == "xmlns") {
                // xmlns="value" -- the default namespace declaration.
                if (value == XNamespace::Xml.getNamespaceNameProperty())
                    throw System::ArgumentException(
                        "The 'http://www.w3.org/XML/1998/namespace' namespace name can only "
                        "be declared with the 'xml' prefix.");
                if (value == XNamespace::Xmlns.getNamespaceNameProperty())
                    throw System::ArgumentException(
                        "The 'http://www.w3.org/2000/xmlns/' namespace name cannot be declared.");
            }
        }
    }

    XAttribute::XAttribute(const XName& name, const std::string& value)
        : name_(name), value_(value) {
        ValidateAttribute(name_, value_);
    }

    void XAttribute::setValueProperty(const std::string& v) {
        ValidateAttribute(name_, v);
        value_ = v;
    }

    bool XAttribute::getIsNamespaceDeclarationProperty() const {
        const std::string& namespaceName = name_.getNamespaceNameProperty();
        if (namespaceName.empty())
            return name_.getLocalNameProperty() == "xmlns";
        return namespaceName == XNamespace::Xmlns.getNamespaceNameProperty();
    }

    std::string XAttribute::EscapeValue(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                case '"': out += "&quot;"; break;
                // Matches XmlEncodedRawTextWriter.WriteAttributeTextBlock's Tab/LineFeed/
                // CarriageReturnEntity: a literal tab/LF/CR written unescaped into an attribute
                // value is collapsed to a plain space by the XML spec's attribute-value
                // normalization on reload (section 3.3.3) -- character references are not
                // subject to that normalization, so escaping is the only way for the value to
                // round-trip unchanged.
                case '\t': out += "&#x9;"; break;
                case '\n': out += "&#xA;"; break;
                case '\r': out += "&#xD;"; break;
                default: out += c; break;
            }
        }
        return out;
    }

    XAttribute* XAttribute::getPreviousAttributeProperty() const {
        XElement* owner = getParentProperty();
        if (owner == nullptr) return nullptr;
        XAttribute* prev = nullptr;
        for (const auto& a : owner->getAttributesProperty()) {
            if (a.get() == this) return prev;
            prev = a.get();
        }
        return nullptr;
    }

    void XAttribute::Remove() {
        XElement* owner = getParentProperty();
        if (owner == nullptr) throw System::InvalidOperationException("The parent is missing.");
        owner->RemoveAttribute(this);
    }

} // namespace System::Xml::Linq
