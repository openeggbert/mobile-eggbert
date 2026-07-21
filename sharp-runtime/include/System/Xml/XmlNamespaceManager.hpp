// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "System/Xml/IXmlNamespaceResolver.hpp"
#include "System/Xml/XmlNameTable.hpp"

namespace System::Xml {

    /**
     * @brief Resolves, adds, and removes namespaces to a collection and provides scope
     * management for these namespaces.
     *
     * C++ counterpart of .NET System.Xml.XmlNamespaceManager. Implemented as a stack of
     * prefix-to-URI scopes (rather than .NET's flat declaration array with scope-id
     * bookkeeping) since this runtime does not need to replicate that specific internal
     * memory layout; the externally observable PushScope/PopScope/LookupNamespace/LookupPrefix
     * semantics match.
     */
    class XmlNamespaceManager : public IXmlNamespaceResolver {
        std::shared_ptr<XmlNameTable> nameTable_;
        std::vector<std::unordered_map<std::string, std::string>> scopes_;

    public:
        /** @brief Constructs a manager with the predefined "xml"/"xmlns" namespaces already in scope 0. */
        explicit XmlNamespaceManager(std::shared_ptr<XmlNameTable> nameTable);

        ~XmlNamespaceManager() override = default;

        /** @return The XmlNameTable associated with this manager. */
        [[nodiscard]] std::shared_ptr<XmlNameTable> getNameTableProperty() const { return nameTable_; }

        /** @return The namespace URI mapped to the empty prefix in the current scope, or "" if none. */
        [[nodiscard]] std::string getDefaultNamespaceProperty() const;

        /** @brief Pushes a new, empty namespace scope. */
        void PushScope();
        /** @brief Pops the most recently pushed scope. @return false if there was no scope to pop (already at the root scope). */
        bool PopScope();

        /**
         * @brief Adds the given namespace to the current (innermost) scope.
         * @throws System::ArgumentException if @p prefix is "xml" and @p uri is not the reserved
         *         XML namespace URI, or if @p prefix is "xmlns" (always reserved).
         */
        void AddNamespace(const std::string& prefix, const std::string& uri);
        /** @brief Removes a namespace declaration matching both @p prefix and @p uri from the current scope, if present. */
        void RemoveNamespace(const std::string& prefix, const std::string& uri);

        /** @return The namespace mapped to @p prefix, searching from the innermost scope outward, or std::nullopt if not found. */
        [[nodiscard]] std::optional<std::string> LookupNamespace(const std::string& prefix) const override;
        /** @return The first prefix mapped to @p namespaceName found searching from the innermost scope outward, or std::nullopt if not found. */
        [[nodiscard]] std::optional<std::string> LookupPrefix(const std::string& namespaceName) const override;
        /** @return true if @p prefix is declared in any scope. */
        [[nodiscard]] bool HasNamespace(const std::string& prefix) const;

        /** @return All prefix-to-URI mappings visible in the requested @p scope. */
        [[nodiscard]] std::map<std::string, std::string> GetNamespacesInScope(XmlNamespaceScope scope) const override;
    };

} // namespace System::Xml
