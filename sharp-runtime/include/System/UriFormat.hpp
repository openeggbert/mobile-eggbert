// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Controls how special characters are escaped in a URI.
     *
     * C++ counterpart of .NET System.UriFormat.
     */
    enum class UriFormat {
        UriEscaped    = 1, /**< Characters are escaped according to RFC 2396. */
        Unescaped     = 2, /**< No escaping is performed. */
        SafeUnescaped = 3, /**< Characters that have a reserved meaning in the URI are not escaped. */
    };

} // namespace System
