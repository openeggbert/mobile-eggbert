// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlNamespaceManager.hpp"
#include "System/ArgumentException.hpp"

namespace System::Xml {

    namespace {
        constexpr const char* kNsXml = "http://www.w3.org/XML/1998/namespace";
        constexpr const char* kNsXmlNs = "http://www.w3.org/2000/xmlns/";
    }

    XmlNamespaceManager::XmlNamespaceManager(std::shared_ptr<XmlNameTable> nameTable)
        : nameTable_(std::move(nameTable))
    {
        // Scope 0 holds the permanent, implicit declarations; PopScope() never removes it.
        scopes_.emplace_back();
        scopes_[0][""] = "";
        scopes_[0]["xmlns"] = kNsXmlNs;
        scopes_[0]["xml"] = kNsXml;
    }

    std::string XmlNamespaceManager::getDefaultNamespaceProperty() const {
        auto ns = LookupNamespace("");
        return ns.value_or(std::string());
    }

    void XmlNamespaceManager::PushScope() {
        scopes_.emplace_back();
    }

    bool XmlNamespaceManager::PopScope() {
        if (scopes_.size() <= 1) return false;
        scopes_.pop_back();
        return true;
    }

    void XmlNamespaceManager::AddNamespace(const std::string& prefix, const std::string& uri) {
        if (prefix == "xml" && uri != kNsXml)
            throw System::ArgumentException(
                "Prefix \"xml\" is reserved for use by XML and can be mapped only to namespace name "
                "\"http://www.w3.org/XML/1998/namespace\".");
        if (prefix == "xmlns")
            throw System::ArgumentException("Prefix \"xmlns\" is reserved for use by XML.");
        scopes_.back()[prefix] = uri;
    }

    void XmlNamespaceManager::RemoveNamespace(const std::string& prefix, const std::string& uri) {
        auto& scope = scopes_.back();
        auto it = scope.find(prefix);
        if (it != scope.end() && it->second == uri)
            scope.erase(it);
    }

    std::optional<std::string> XmlNamespaceManager::LookupNamespace(const std::string& prefix) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = it->find(prefix);
            if (found != it->end()) return found->second;
        }
        return std::nullopt;
    }

    std::optional<std::string> XmlNamespaceManager::LookupPrefix(const std::string& namespaceName) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it)
            for (const auto& [prefix, uri] : *it)
                if (uri == namespaceName) return prefix;
        return std::nullopt;
    }

    bool XmlNamespaceManager::HasNamespace(const std::string& prefix) const {
        return scopes_.back().find(prefix) != scopes_.back().end();
    }

    std::map<std::string, std::string> XmlNamespaceManager::GetNamespacesInScope(XmlNamespaceScope scope) const {
        // The permanent ""->"" and "xmlns"->NsXmlNs declarations are never enumerated, matching
        // .NET (they are implicit/fixed, not user-visible scope entries); "xml" is included for
        // All/Local but not ExcludeXml.
        std::map<std::string, std::string> result;
        size_t startScope = (scope == XmlNamespaceScope::Local) ? scopes_.size() - 1 : 0;
        for (size_t i = startScope; i < scopes_.size(); ++i)
            for (const auto& [prefix, uri] : scopes_[i]) {
                if (prefix.empty() || prefix == "xmlns") continue;
                if (prefix == "xml" && scope == XmlNamespaceScope::ExcludeXml) continue;
                if (prefix == "xml" && scope == XmlNamespaceScope::Local && scopes_.size() != 1) continue;
                result[prefix] = uri;
            }
        return result;
    }

} // namespace System::Xml
