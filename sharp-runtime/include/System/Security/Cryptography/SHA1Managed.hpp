// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/SHA1.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Legacy managed-implementation name for SHA1 — functionally identical in this runtime.
     * C++ counterpart of .NET System.Security.Cryptography.SHA1Managed.
     */
    class SHA1Managed : public SHA1 {};

} // namespace System::Security::Cryptography
