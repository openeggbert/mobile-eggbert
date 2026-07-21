// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"

namespace System::Xml::Linq {

    class XNamespace;

    /**
     * @brief Represents a qualified XML name (local name + optional namespace URI).
     *
     * C++ counterpart of .NET System.Xml.Linq.XName.
     *
     * @note .NET's real XName is interned via a weak-reference hashtable per XNamespace, so
     * `==` is reference equality. This port compares by value (namespace + local name strings)
     * instead — simpler, and observably equivalent for any code that doesn't rely on reference
     * identity specifically (which ported game/XNA code realistically never does for XName).
     * The implicit `string -> XName` conversion (accepting both plain "local" and expanded
     * "{namespace}local" forms) is preserved, matching .NET's `implicit operator XName(string)`.
     */
    class XName {
        std::string localName_;
        std::string namespaceName_;

    public:
        /** Default constructor — produces an empty name. */
        XName() = default;

        /**
         * @brief Constructs from a plain "local" name or an expanded "{namespace}local" name.
         * Intentionally not `explicit`, matching .NET's `implicit operator XName(string)`.
         */
        XName(const std::string& expandedOrLocalName) : XName(Get(expandedOrLocalName)) {}

        /** @brief Constructs from a C string literal (see the std::string overload). */
        XName(const char* expandedOrLocalName) : XName(std::string(expandedOrLocalName)) {}

        /** Constructs a qualified name from @p namespaceName and @p localName. */
        XName(const std::string& namespaceName, const std::string& localName)
            : localName_(localName), namespaceName_(namespaceName) {}

        /** @return The local (unqualified) part of the name. */
        [[nodiscard]] const std::string& getLocalNameProperty() const { return localName_; }

        /** @return The namespace URI, or empty string if unqualified. */
        [[nodiscard]] const std::string& getNamespaceNameProperty() const { return namespaceName_; }

        /** @return The XNamespace component of this name. */
        [[nodiscard]] XNamespace getNamespaceProperty() const;

        /** @return The expanded form: "{namespace}localName", or just localName when unqualified. */
        [[nodiscard]] std::string ToString() const {
            if (namespaceName_.empty()) return localName_;
            return "{" + namespaceName_ + "}" + localName_;
        }

        /** Equality — both local name and namespace URI must match. */
        [[nodiscard]] bool Equals(const XName& o) const {
            return localName_ == o.localName_ && namespaceName_ == o.namespaceName_;
        }
        bool operator==(const XName& o) const { return Equals(o); }
        bool operator!=(const XName& o) const { return !Equals(o); }

        [[nodiscard]] SharpRuntime::intcs GetHashCode() const {
            return static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(namespaceName_ + "|" + localName_));
        }

        /**
         * @brief Parses an expanded name of the form "{namespace}localName" or plain "localName".
         *
         * Matches XName.cs's Get(string): splits on the *last* '}' (not the first -- a
         * namespace URI may itself legally contain '}'), and rejects a malformed expanded
         * name (empty namespace "{}x", or no local name after the closing brace "{ns}").
         *
         * @param expandedName The expanded XML name string.
         * @return Parsed XName.
         * @throws System::ArgumentException if @p expandedName is empty or malformed.
         */
        [[nodiscard]] static XName Get(const std::string& expandedName) {
            if (expandedName.empty())
                throw System::ArgumentException("The value cannot be an empty string.", "expandedName");
            if (expandedName[0] == '{') {
                size_t end = expandedName.rfind('}');
                if (end == std::string::npos || end <= 1 || end == expandedName.size() - 1)
                    throw System::ArgumentException("The format of value '" + expandedName + "' is invalid.");
                return XName(expandedName.substr(1, end - 1), expandedName.substr(end + 1));
            }
            return XName(std::string(), expandedName);
        }

        /** @brief Constructs an XName from a namespace name and local name (matches XNamespace::Get). */
        [[nodiscard]] static XName Get(const std::string& localName, const std::string& namespaceName) {
            return XName(namespaceName, localName);
        }
    };

} // namespace System::Xml::Linq

/** @brief Specialization so XName can be used as an unordered_map/unordered_set key. */
template<>
struct std::hash<System::Xml::Linq::XName> {
    std::size_t operator()(const System::Xml::Linq::XName& name) const noexcept {
        return static_cast<std::size_t>(name.GetHashCode());
    }
};
