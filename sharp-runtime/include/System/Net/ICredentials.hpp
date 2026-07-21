// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include "System/Uri.hpp"

namespace System::Net {

    class NetworkCredential;

    /**
     * Provides the base authentication interface for Web client authentication.
     *
     * C++ counterpart of .NET System.Net.ICredentials.
     */
    class ICredentials {
    public:
        virtual ~ICredentials() = default;

        /**
         * Returns a NetworkCredential object that is associated with the supplied uri
         * and authentication type. By convention, this method should return nullptr if no
         * information is available for the specified uri and authentication type.
         */
        [[nodiscard]] virtual std::shared_ptr<NetworkCredential> GetCredential(const System::Uri& uri, const std::string& authType) = 0;
    };

} // namespace System::Net
