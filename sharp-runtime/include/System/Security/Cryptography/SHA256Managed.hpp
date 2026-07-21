// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/SHA256.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Legacy managed-implementation name for SHA256 — functionally identical in this runtime.
     * C++ counterpart of .NET System.Security.Cryptography.SHA256Managed.
     */
    class SHA256Managed : public SHA256 {};

} // namespace System::Security::Cryptography
