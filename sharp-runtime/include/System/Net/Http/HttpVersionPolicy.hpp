// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::Http {

    /**
     * @brief Determines behavior when selecting and negotiating HTTP version for a request.
     *
     * C++ counterpart of .NET System.Net.Http.HttpVersionPolicy.
     *
     * @note This runtime's HttpClient does not perform ALPN/Alt-Svc version negotiation
     * (see HttpClient's implementation notes), so this enum exists for API-surface
     * compatibility; its values are not currently consulted by request execution.
     */
    enum class HttpVersionPolicy {
        /** Default behavior: use the requested version or downgrade to a lower one. */
        RequestVersionOrLower,
        /** Try to use the highest available version, downgrading only to the requested version, not below. */
        RequestVersionOrHigher,
        /** Use only the requested version. */
        RequestVersionExact,
    };

} // namespace System::Net::Http
