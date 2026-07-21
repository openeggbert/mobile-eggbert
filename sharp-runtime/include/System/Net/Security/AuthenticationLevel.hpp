// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::Security {

    /**
     * @brief WebRequest-specific authentication flags.
     *
     * C++ counterpart of .NET System.Net.Security.AuthenticationLevel.
     */
    enum class AuthenticationLevel {
        None = 0,
        MutualAuthRequested = 1,
        MutualAuthRequired = 2,
    };

} // namespace System::Net::Security
