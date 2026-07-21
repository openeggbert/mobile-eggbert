// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Defines host name types for the System::Uri.CheckHostName method.
     *
     * C++ counterpart of .NET System.UriHostNameType.
     */
    enum class UriHostNameType {
        Unknown = 0, /**< The host name type is not supplied. */
        Basic   = 1, /**< The host name is set, but the type cannot be determined. */
        Dns     = 2, /**< The host name is a DNS-style host name. */
        IPv4    = 3, /**< The host name is an Internet Protocol (IP) version 4 host address. */
        IPv6    = 4, /**< The host name is an Internet Protocol (IP) version 6 host address. */
    };

} // namespace System
