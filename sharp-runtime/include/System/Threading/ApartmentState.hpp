// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Threading {

    /**
     * @brief Specifies the apartment state of a managed thread.
     *
     * C++ counterpart of .NET System.Threading.ApartmentState.
     */
    enum class ApartmentState {
        /** @brief The thread will create and enter a single-threaded apartment. */
        STA     = 0,
        /** @brief The thread will create and enter a multi-threaded apartment. */
        MTA     = 1,
        /** @brief The apartment state has not been set. */
        Unknown = 2,
    };

} // namespace System::Threading
