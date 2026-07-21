// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies the day of the week.
     *
     * C++ counterpart of .NET System.DayOfWeek.
     * The integer values match those of .NET: Sunday = 0, Saturday = 6.
     */
    enum class DayOfWeek {
        /** @brief Indicates Sunday (0). */
        Sunday    = 0,
        /** @brief Indicates Monday (1). */
        Monday    = 1,
        /** @brief Indicates Tuesday (2). */
        Tuesday   = 2,
        /** @brief Indicates Wednesday (3). */
        Wednesday = 3,
        /** @brief Indicates Thursday (4). */
        Thursday  = 4,
        /** @brief Indicates Friday (5). */
        Friday    = 5,
        /** @brief Indicates Saturday (6). */
        Saturday  = 6,
    };

} // namespace System
