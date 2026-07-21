// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Xml {

    /**
     * @brief Represents an XML qualified name (a local name plus an optional namespace URI).
     *
     * C++ counterpart of .NET System.Xml.XmlQualifiedName.
     */
    class XmlQualifiedName {
        std::string name_;
        std::string ns_;

    public:
        /** @brief The empty qualified name ("", ""). */
        static const XmlQualifiedName Empty;

        /** @brief Constructs an empty qualified name. */
        XmlQualifiedName();
        /** @brief Constructs a qualified name with the given local @p name and no namespace. */
        explicit XmlQualifiedName(const std::string& name);
        /** @brief Constructs a qualified name with the given local @p name and namespace @p ns. */
        XmlQualifiedName(const std::string& name, const std::string& ns);

        /** @return The namespace URI, or "" if none. */
        [[nodiscard]] const std::string& getNamespaceProperty() const { return ns_; }
        /** @return The local name. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        /** @return @c true if both Name and Namespace are empty. */
        [[nodiscard]] bool getIsEmptyProperty() const { return name_.empty() && ns_.empty(); }

        /** @return The .NET-style hash code (based on Name). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const;

        /** @return "Name" if Namespace is empty, otherwise "Namespace:Name". */
        [[nodiscard]] std::string ToString() const;

        /** @return @c true if @p other has the same Name and Namespace. */
        [[nodiscard]] bool Equals(const XmlQualifiedName& other) const;

        bool operator==(const XmlQualifiedName& other) const { return Equals(other); }
        bool operator!=(const XmlQualifiedName& other) const { return !Equals(other); }

        /** @return "name" if @p ns is empty, otherwise "ns:name". */
        [[nodiscard]] static std::string ToString(const std::string& name, const std::string& ns);
    };

} // namespace System::Xml
