// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies the behavior for a forced garbage collection.
     *
     * C++ counterpart of .NET System.GCCollectionMode.
     */
    enum class GCCollectionMode {
        /** @brief Default mode. Currently equivalent to Forced. */
        Default    = 0,
        /** @brief Force an immediate garbage collection. */
        Forced     = 1,
        /** @brief Allow the garbage collector to determine the optimal time. */
        Optimized  = 2,
        /** @brief Force an aggressive full blocking garbage collection. */
        Aggressive = 3,
    };

} // namespace System
