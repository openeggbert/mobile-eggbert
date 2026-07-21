// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "System/Net/ICredentials.hpp"
#include "System/Net/ICredentialsByHost.hpp"
#include "System/Net/NetworkCredential.hpp"
#include "System/Uri.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net {

    using SharpRuntime::intcs;

    /**
     * A more sophisticated password cache that stores multiple name-password pairs
     * and associates these with a Uri prefix/authentication-type pair, or with a
     * host/port/authentication-type triple.
     *
     * C++ counterpart of .NET System.Net.CredentialCache.
     *
     * @note .NET's IEnumerable/GetEnumerator surface and the internal versioned
     * fail-fast enumerator are not ported: this runtime exposes the cache contents
     * only through GetCredential(); there is no equivalent use case in ported game code.
     */
    class CredentialCache : public ICredentials, public ICredentialsByHost {
        struct UriEntry {
            std::string scheme;
            std::string host;
            intcs port = 0;
            std::string absolutePath;
            intcs prefixLength = -1;
            std::string authenticationType;
            std::shared_ptr<NetworkCredential> credential;
        };

        struct HostEntry {
            std::string host;
            intcs port = 0;
            std::string authenticationType;
            std::shared_ptr<NetworkCredential> credential;
        };

        std::vector<UriEntry> uriCache_;
        std::vector<HostEntry> hostCache_;

        static bool equalsIgnoreCase(const std::string& a, const std::string& b);

    public:
        CredentialCache() = default;
        ~CredentialCache() override = default;

        /** Adds a NetworkCredential instance associated with a Uri prefix and authentication type. */
        void Add(const System::Uri& uriPrefix, const std::string& authType, std::shared_ptr<NetworkCredential> cred);

        /** Adds a NetworkCredential instance associated with a host, port, and authentication type. */
        void Add(const std::string& host, intcs port, const std::string& authenticationType, std::shared_ptr<NetworkCredential> credential);

        /** Removes a NetworkCredential instance associated with a Uri prefix and authentication type. */
        void Remove(const System::Uri& uriPrefix, const std::string& authType);

        /** Removes a NetworkCredential instance associated with a host, port, and authentication type. */
        void Remove(const std::string& host, intcs port, const std::string& authenticationType);

        /** Returns the NetworkCredential instance associated with the most specific Uri prefix match and authentication type. */
        [[nodiscard]] std::shared_ptr<NetworkCredential> GetCredential(const System::Uri& uriPrefix, const std::string& authType) override;

        /** Returns the NetworkCredential instance associated with the given host, port, and authentication type. */
        [[nodiscard]] std::shared_ptr<NetworkCredential> GetCredential(const std::string& host, intcs port, const std::string& authenticationType) override;

        /** A ICredentials instance that provides the system's default network credential. */
        static std::shared_ptr<ICredentials> getDefaultCredentialsProperty();

        /** A NetworkCredential instance that provides the system's default network credential. */
        static std::shared_ptr<NetworkCredential> getDefaultNetworkCredentialsProperty();
    };

} // namespace System::Net
