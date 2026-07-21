// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::Security {

    /**
     * @brief Specifies the encryption policy to use for a Ssl/TlsStream connection.
     *
     * C++ counterpart of .NET System.Net.Security.EncryptionPolicy.
     *
     * @note `AllowNoEncryption`/`NoEncryption` are obsolete in .NET (SYSLIB0040, reduce security)
     * but are reproduced here for API parity; no code in this runtime acts on them differently.
     */
    enum class EncryptionPolicy {
        RequireEncryption = 0,
        AllowNoEncryption = 1,
        NoEncryption = 2,
    };

} // namespace System::Net::Security
