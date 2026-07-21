// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net {

    using SharpRuntime::intcs;

    class NetworkCredential;

    /**
     * Provides the base authentication interface for Web client authentication,
     * keyed by host/port rather than by Uri.
     *
     * C++ counterpart of .NET System.Net.ICredentialsByHost.
     */
    class ICredentialsByHost {
    public:
        virtual ~ICredentialsByHost() = default;

        /**
         * Returns a NetworkCredential object that is associated with the supplied host, port,
         * and authentication type. By convention, this method should return nullptr if no
         * information is available for the specified host and authentication type.
         */
        [[nodiscard]] virtual std::shared_ptr<NetworkCredential> GetCredential(const std::string& host, intcs port, const std::string& authenticationType) = 0;
    };

} // namespace System::Net
