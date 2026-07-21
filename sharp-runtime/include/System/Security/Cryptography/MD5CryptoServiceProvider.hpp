// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/MD5.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Legacy CSP-backed MD5 implementation — functionally identical to MD5 in this
     * runtime (there is no separate CSP-backed vs. managed implementation split here).
     *
     * C++ counterpart of .NET System.Security.Cryptography.MD5CryptoServiceProvider.
     */
    class MD5CryptoServiceProvider : public MD5 {};

} // namespace System::Security::Cryptography
