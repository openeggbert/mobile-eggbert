// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Controls when host names are converted to Internationalizing Domain Name (IDN) form.
     *
     * C++ counterpart of .NET System.UriIdnScope.
     * Used to configure IDN host-name normalization policy.
     */
    enum class UriIdnScope {
        None,               /**< IDN is never used. */
        AllExceptIntranet,  /**< IDN is used for Internet hosts but not intranet hosts. */
        All,                /**< IDN is used for both Internet and intranet hosts. */
    };

} // namespace System
