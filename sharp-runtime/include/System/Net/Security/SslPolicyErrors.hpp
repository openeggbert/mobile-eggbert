// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::Security {

    /**
     * @brief Flags indicating the reasons a certificate validation failed, or None if it succeeded.
     *
     * C++ counterpart of .NET System.Net.Security.SslPolicyErrors ([Flags] enum).
     */
    enum class SslPolicyErrors {
        None = 0x0,
        RemoteCertificateNotAvailable = 0x1,
        RemoteCertificateNameMismatch = 0x2,
        RemoteCertificateChainErrors = 0x4,
    };

    /** @brief Bitwise OR for the [Flags] SslPolicyErrors enum. */
    constexpr SslPolicyErrors operator|(SslPolicyErrors a, SslPolicyErrors b) {
        return static_cast<SslPolicyErrors>(static_cast<int>(a) | static_cast<int>(b));
    }

    /** @brief Bitwise AND for the [Flags] SslPolicyErrors enum. */
    constexpr SslPolicyErrors operator&(SslPolicyErrors a, SslPolicyErrors b) {
        return static_cast<SslPolicyErrors>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Net::Security
