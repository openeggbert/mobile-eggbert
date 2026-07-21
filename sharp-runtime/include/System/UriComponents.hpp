// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies the parts of a URI.
     *
     * C++ counterpart of .NET System.UriComponents.
     * Values are flags that can be combined with bitwise OR.
     */
    enum class UriComponents : unsigned int {
        Scheme                  = 0x1,          /**< The scheme component. */
        UserInfo                = 0x2,          /**< The user-info component. */
        Host                    = 0x4,          /**< The host component. */
        Port                    = 0x8,          /**< The port component. */
        Path                    = 0x10,         /**< The path component. */
        Query                   = 0x20,         /**< The query component. */
        Fragment                = 0x40,         /**< The fragment component. */
        StrongPort              = 0x80,         /**< The port, even if it is the default for the scheme. */
        NormalizedHost          = 0x100,        /**< The normalised host name. */
        KeepDelimiter           = 0x40000000,   /**< Specifies that the delimiter should be included. */
        SerializationInfoString = 0x80000000,   /**< The complete URI. */
        AbsoluteUri             = Scheme | UserInfo | Host | Port | Path | Query | Fragment, /**< All absolute URI components. */
        HostAndPort             = Host | StrongPort,  /**< The host and port, always including the port even if it is the default for the scheme. */
        StrongAuthority         = UserInfo | Host | StrongPort, /**< The user-info, host, and port, always including the port. */
        SchemeAndServer         = Scheme | Host | Port, /**< The scheme, host, and port. */
        HttpRequestUrl          = Scheme | Host | Port | Path | Query, /**< The components needed to submit a request. */
        PathAndQuery            = Path | Query,  /**< The path and query. */
    };

    inline UriComponents operator|(UriComponents a, UriComponents b) {
        return static_cast<UriComponents>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
    }
    inline UriComponents operator&(UriComponents a, UriComponents b) {
        return static_cast<UriComponents>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b));
    }

} // namespace System
