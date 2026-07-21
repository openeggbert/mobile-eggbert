// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Provides information about the current registration for full GC notifications.
     *
     * C++ counterpart of .NET System.GCNotificationStatus.
     */
    enum class GCNotificationStatus {
        /** @brief The notification was successful. */
        Succeeded     = 0,
        /** @brief The notification failed for any reason. */
        Failed        = 1,
        /** @brief The notification was canceled by the user. */
        Canceled      = 2,
        /** @brief The time specified by the millisecondsTimeout parameter elapsed. */
        Timeout       = 3,
        /** @brief This result can be caused by registering with a concurrent GC enabled. */
        NotApplicable = 4,
    };

} // namespace System
