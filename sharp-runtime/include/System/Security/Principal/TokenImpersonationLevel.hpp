// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Security::Principal {

    /**
     * @brief Defines security impersonation levels for a Windows access token.
     *
     * C++ counterpart of .NET System.Security.Principal.TokenImpersonationLevel.
     */
    enum class TokenImpersonationLevel {
        None = 0,
        Anonymous = 1,
        Identification = 2,
        Impersonation = 3,
        Delegation = 4,
    };

} // namespace System::Security::Principal
