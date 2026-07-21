// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/SHA512.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Legacy managed-implementation name for SHA512 — functionally identical in this runtime.
     * C++ counterpart of .NET System.Security.Cryptography.SHA512Managed.
     */
    class SHA512Managed : public SHA512 {};

} // namespace System::Security::Cryptography
