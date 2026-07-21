// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Security::Authentication {

    /**
     * @brief Defines the possible versions of SslStream/TlsStream security protocols. [Flags]
     *
     * C++ counterpart of .NET System.Security.Authentication.SslProtocols.
     *
     * @note Pure data — portable independent of an actual TLS engine, which this runtime does
     * not have (see `SslStream`'s `tobedecided` status in `plan.sqlite3`). `Ssl2`/`Ssl3`/`Tls`/
     * `Tls11`/`Default` are obsolete in .NET itself (deprecated/insecure) but reproduced here for
     * numeric parity.
     */
    enum class SslProtocols {
        None = 0,
        Ssl2 = 0x0C,   // SP_PROT_SSL2 (SERVER|CLIENT)
        Ssl3 = 0x30,   // SP_PROT_SSL3
        Tls = 0xC0,    // SP_PROT_TLS1_0
        Tls11 = 0x300, // SP_PROT_TLS1_1
        Tls12 = 0xC00, // SP_PROT_TLS1_2
        Tls13 = 0x3000, // SP_PROT_TLS1_3
        Default = Ssl3 | Tls,
    };

    /** @brief Bitwise OR for the [Flags] SslProtocols enum. */
    constexpr SslProtocols operator|(SslProtocols a, SslProtocols b) {
        return static_cast<SslProtocols>(static_cast<int>(a) | static_cast<int>(b));
    }

    /** @brief Bitwise AND for the [Flags] SslProtocols enum. */
    constexpr SslProtocols operator&(SslProtocols a, SslProtocols b) {
        return static_cast<SslProtocols>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Security::Authentication
