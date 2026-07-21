// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Defines the parts of a URI for the System::Uri::GetLeftPart method.
     *
     * C++ counterpart of .NET System.UriPartial.
     */
    enum class UriPartial {
        Scheme    = 0, /**< The scheme and delimiter only (e.g. "https://"). */
        Authority = 1, /**< The scheme, delimiter, and authority (e.g. "https://example.com"). */
        Path      = 2, /**< The scheme, authority, and path. */
        Query     = 3, /**< The scheme, authority, path, and query. */
    };

} // namespace System
