// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Xml/Linq/XName.hpp"

namespace System::Xml::Linq {

    /**
     * @brief Represents an XML namespace, used to qualify XName instances.
     *
     * C++ counterpart of .NET System.Xml.Linq.XNamespace.
     *
     * @note .NET interns XNamespace instances (weak-reference hashtable) so `==` is reference
     * equality. This port compares by value (the namespace URI string) instead — simpler, and
     * observably equivalent since XNamespace is otherwise immutable. See XName's doc comment for
     * the same rationale.
     */
    class XNamespace {
        std::string namespaceName_;

    public:
        XNamespace() = default;

        /** @brief Constructs directly from a namespace URI. Prefer XNamespace::Get() for parity with .NET. */
        explicit XNamespace(std::string namespaceName) : namespaceName_(std::move(namespaceName)) {}

        /** @return The namespace URI string. */
        [[nodiscard]] const std::string& getNamespaceNameProperty() const { return namespaceName_; }

        [[nodiscard]] std::string ToString() const { return namespaceName_; }

        [[nodiscard]] bool Equals(const XNamespace& o) const { return namespaceName_ == o.namespaceName_; }
        bool operator==(const XNamespace& o) const { return Equals(o); }
        bool operator!=(const XNamespace& o) const { return !Equals(o); }

        /** @brief Builds the XName {this}localName. Matches .NET's `XNamespace + string` operator. */
        [[nodiscard]] XName GetName(const std::string& localName) const { return XName(namespaceName_, localName); }

        /** @brief Builds the XName {ns}localName. Matches .NET's `operator +(XNamespace, string)`. */
        friend XName operator+(const XNamespace& ns, const std::string& localName) { return ns.GetName(localName); }

        /** @brief Parses/creates an XNamespace for the given URI (no true interning — see class doc comment). */
        [[nodiscard]] static XNamespace Get(const std::string& namespaceName) { return XNamespace(namespaceName); }

        /** @brief The empty (no) namespace. */
        static const XNamespace None;
        /** @brief The predefined "xml" namespace (http://www.w3.org/XML/1998/namespace). */
        static const XNamespace Xml;
        /** @brief The predefined "xmlns" namespace (http://www.w3.org/2000/xmlns/). */
        static const XNamespace Xmlns;
    };

    inline const XNamespace XNamespace::None{std::string()};
    inline const XNamespace XNamespace::Xml{std::string("http://www.w3.org/XML/1998/namespace")};
    inline const XNamespace XNamespace::Xmlns{std::string("http://www.w3.org/2000/xmlns/")};

    inline XNamespace XName::getNamespaceProperty() const { return XNamespace(namespaceName_); }

} // namespace System::Xml::Linq
