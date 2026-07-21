// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/CredentialCache.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include <algorithm>
#include <cctype>

namespace System::Net {

    bool CredentialCache::equalsIgnoreCase(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        return std::equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) {
            return std::tolower(x) == std::tolower(y);
        });
    }

    void CredentialCache::Add(const System::Uri& uriPrefix, const std::string& authType, std::shared_ptr<NetworkCredential> cred) {
        const std::string& absolutePath = uriPrefix.getAbsolutePathProperty();
        intcs prefixLength = static_cast<intcs>(absolutePath.rfind('/'));
        if (absolutePath.rfind('/') == std::string::npos) prefixLength = -1;

        for (const auto& e : uriCache_) {
            if (equalsIgnoreCase(e.authenticationType, authType) &&
                e.scheme == uriPrefix.getSchemeProperty() &&
                e.host == uriPrefix.getHostProperty() &&
                e.port == uriPrefix.getPortProperty() &&
                e.absolutePath == absolutePath) {
                throw System::ArgumentException("An item with the same key has already been added.");
            }
        }

        UriEntry entry;
        entry.scheme = uriPrefix.getSchemeProperty();
        entry.host = uriPrefix.getHostProperty();
        entry.port = uriPrefix.getPortProperty();
        entry.absolutePath = absolutePath;
        entry.prefixLength = prefixLength;
        entry.authenticationType = authType;
        entry.credential = std::move(cred);
        uriCache_.push_back(std::move(entry));
    }

    void CredentialCache::Add(const std::string& host, intcs port, const std::string& authenticationType, std::shared_ptr<NetworkCredential> credential) {
        System::ArgumentException::ThrowIfNullOrEmpty(host, "host");
        System::ArgumentOutOfRangeException::ThrowIfNegative(port, "port");

        for (const auto& e : hostCache_) {
            if (equalsIgnoreCase(e.authenticationType, authenticationType) &&
                equalsIgnoreCase(e.host, host) &&
                e.port == port) {
                throw System::ArgumentException("An item with the same key has already been added.");
            }
        }

        HostEntry entry;
        entry.host = host;
        entry.port = port;
        entry.authenticationType = authenticationType;
        entry.credential = std::move(credential);
        hostCache_.push_back(std::move(entry));
    }

    void CredentialCache::Remove(const System::Uri& uriPrefix, const std::string& authType) {
        const std::string& absolutePath = uriPrefix.getAbsolutePathProperty();
        std::erase_if(uriCache_, [&](const UriEntry& e) {
            return equalsIgnoreCase(e.authenticationType, authType) &&
                   e.scheme == uriPrefix.getSchemeProperty() &&
                   e.host == uriPrefix.getHostProperty() &&
                   e.port == uriPrefix.getPortProperty() &&
                   e.absolutePath == absolutePath;
        });
    }

    void CredentialCache::Remove(const std::string& host, intcs port, const std::string& authenticationType) {
        if (port < 0) return;
        std::erase_if(hostCache_, [&](const HostEntry& e) {
            return equalsIgnoreCase(e.authenticationType, authenticationType) &&
                   equalsIgnoreCase(e.host, host) &&
                   e.port == port;
        });
    }

    std::shared_ptr<NetworkCredential> CredentialCache::GetCredential(const System::Uri& uriPrefix, const std::string& authType) {
        const std::string& absolutePath = uriPrefix.getAbsolutePathProperty();
        intcs uriPrefixLength = static_cast<intcs>(absolutePath.rfind('/'));
        if (absolutePath.rfind('/') == std::string::npos) uriPrefixLength = -1;

        intcs longestMatchPrefix = -1;
        std::shared_ptr<NetworkCredential> mostSpecificMatch;

        for (const auto& e : uriCache_) {
            if (e.prefixLength <= longestMatchPrefix) continue;
            if (!equalsIgnoreCase(e.authenticationType, authType)) continue;
            if (e.scheme != uriPrefix.getSchemeProperty() || e.host != uriPrefix.getHostProperty() || e.port != uriPrefix.getPortProperty()) continue;
            if (e.prefixLength > uriPrefixLength) continue;
            if (!equalsIgnoreCase(e.absolutePath.substr(0, e.prefixLength), absolutePath.substr(0, e.prefixLength))) continue;

            longestMatchPrefix = e.prefixLength;
            mostSpecificMatch = e.credential;
            if (uriPrefixLength == e.prefixLength) break;
        }

        return mostSpecificMatch;
    }

    std::shared_ptr<NetworkCredential> CredentialCache::GetCredential(const std::string& host, intcs port, const std::string& authenticationType) {
        System::ArgumentException::ThrowIfNullOrEmpty(host, "host");
        System::ArgumentOutOfRangeException::ThrowIfNegative(port, "port");

        for (const auto& e : hostCache_) {
            if (equalsIgnoreCase(e.authenticationType, authenticationType) &&
                equalsIgnoreCase(e.host, host) &&
                e.port == port) {
                return e.credential;
            }
        }
        return nullptr;
    }

    std::shared_ptr<ICredentials> CredentialCache::getDefaultCredentialsProperty() {
        static std::shared_ptr<NetworkCredential> s_defaultCredential = std::make_shared<NetworkCredential>();
        return s_defaultCredential;
    }

    std::shared_ptr<NetworkCredential> CredentialCache::getDefaultNetworkCredentialsProperty() {
        static std::shared_ptr<NetworkCredential> s_defaultCredential = std::make_shared<NetworkCredential>();
        return s_defaultCredential;
    }

} // namespace System::Net
