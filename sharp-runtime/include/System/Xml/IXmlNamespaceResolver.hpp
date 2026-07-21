// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <map>
#include <optional>
#include <string>

#include "System/Xml/XmlNamespaceScope.hpp"

namespace System::Xml {

    /**
     * @brief Provides read-only access to a set of (prefix, namespace) mappings.
     *
     * Each distinct prefix maps to exactly one namespace, but multiple prefixes may map to the
     * same namespace. C++ counterpart of .NET System.Xml.IXmlNamespaceResolver.
     */
    class IXmlNamespaceResolver {
    public:
        virtual ~IXmlNamespaceResolver() = default;

        /** @return All (prefix, namespace) mappings defined within @p scope. */
        [[nodiscard]] virtual std::map<std::string, std::string>
        GetNamespacesInScope(XmlNamespaceScope scope) const = 0;

        /**
         * @return The namespace mapped to @p prefix, or std::nullopt if @p prefix is not mapped.
         * "xml" always maps to "http://www.w3.org/XML/1998/namespace"; "xmlns" always maps to
         * "http://www.w3.org/2000/xmlns/".
         */
        [[nodiscard]] virtual std::optional<std::string> LookupNamespace(const std::string& prefix) const = 0;

        /**
         * @return A prefix mapped to @p namespaceName, or std::nullopt if none is mapped.
         * Which prefix is returned is unspecified when more than one qualifies.
         */
        [[nodiscard]] virtual std::optional<std::string> LookupPrefix(const std::string& namespaceName) const = 0;
    };

} // namespace System::Xml
