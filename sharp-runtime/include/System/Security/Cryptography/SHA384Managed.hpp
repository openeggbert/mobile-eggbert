// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Security/Cryptography/SHA384.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Legacy managed-implementation name for SHA384 — functionally identical in this runtime.
     * C++ counterpart of .NET System.Security.Cryptography.SHA384Managed.
     */
    class SHA384Managed : public SHA384 {};

} // namespace System::Security::Cryptography
