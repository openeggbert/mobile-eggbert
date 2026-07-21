// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies whether a DateTime object represents a local time, a
     * Coordinated Universal Time (UTC), or is not specified as either local time
     * or UTC.
     *
     * C++ counterpart of .NET System.DateTimeKind.
     */
    enum class DateTimeKind {
        /** @brief The time represented is not specified as either local time or UTC. */
        Unspecified = 0,
        /** @brief The time represented is UTC. */
        Utc         = 1,
        /** @brief The time represented is local time. */
        Local       = 2,
    };

} // namespace System
