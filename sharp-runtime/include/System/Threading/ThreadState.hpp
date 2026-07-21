// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Threading {

    /**
     * @brief Specifies the execution states of a Thread.
     *
     * C++ counterpart of .NET System.Threading.ThreadState.
     * This is a [Flags] enum — values can be combined with bitwise OR.
     */
    enum class ThreadState {
        /** @brief The thread is running or ready to run. */
        Running          = 0,
        /** @brief The thread is being requested to stop. */
        StopRequested    = 1,
        /** @brief The thread is being requested to suspend. */
        SuspendRequested = 2,
        /** @brief The thread is executing as a background thread. */
        Background       = 4,
        /** @brief The thread has not been started. */
        Unstarted        = 8,
        /** @brief The thread has stopped. */
        Stopped          = 16,
        /** @brief The thread is blocked in a Wait, Sleep, or Join call. */
        WaitSleepJoin    = 32,
        /** @brief The thread has been suspended. */
        Suspended        = 64,
        /** @brief An abort has been requested. */
        AbortRequested   = 128,
        /** @brief The thread has been aborted. */
        Aborted          = 256,
    };

    /** @brief Returns the bitwise OR of two ThreadState values. */
    inline ThreadState operator|(ThreadState a, ThreadState b) {
        return static_cast<ThreadState>(static_cast<int>(a) | static_cast<int>(b));
    }
    /** @brief Returns the bitwise AND of two ThreadState values. */
    inline ThreadState operator&(ThreadState a, ThreadState b) {
        return static_cast<ThreadState>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Threading
