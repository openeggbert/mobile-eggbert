// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Threading {

    /**
     * @brief Specifies the scheduling priority of a System::Threading::Thread.
     *
     * C++ counterpart of .NET System.Threading.ThreadPriority.
     */
    enum class ThreadPriority {
        /** @brief The thread can be scheduled after threads with any other priority. */
        Lowest      = 0,
        /** @brief The thread can be scheduled after Normal priority threads. */
        BelowNormal = 1,
        /** @brief The thread is scheduled with normal priority. */
        Normal      = 2,
        /** @brief The thread can be scheduled before Normal priority threads. */
        AboveNormal = 3,
        /** @brief The thread is scheduled before threads with any other priority. */
        Highest     = 4,
    };

} // namespace System::Threading
