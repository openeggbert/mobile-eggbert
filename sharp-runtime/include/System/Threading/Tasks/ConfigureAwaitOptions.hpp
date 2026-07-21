// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Threading::Tasks {

    /**
     * @brief Options to control behavior when awaiting.
     *
     * C++ counterpart of .NET System.Threading.Tasks.ConfigureAwaitOptions. Provided for API
     * name/value compatibility; this runtime has no compiler-generated async/await state machine,
     * so there is no ConfigureAwait call site that consumes these flags yet.
     */
    enum class ConfigureAwaitOptions {
        /** No options specified. */
        None = 0x0,
        /** Attempt to marshal the continuation back to the originating SynchronizationContext/TaskScheduler. */
        ContinueOnCapturedContext = 0x1,
        /** Avoids throwing an exception at the completion of awaiting a faulted or canceled Task. */
        SuppressThrowing = 0x2,
        /** Forces an await on an already-completed Task to behave as if it were not yet completed. */
        ForceYielding = 0x4,
    };

    /** Returns the bitwise OR of two ConfigureAwaitOptions values. */
    inline ConfigureAwaitOptions operator|(ConfigureAwaitOptions a, ConfigureAwaitOptions b) {
        return static_cast<ConfigureAwaitOptions>(static_cast<int>(a) | static_cast<int>(b));
    }
    /** Returns the bitwise AND of two ConfigureAwaitOptions values. */
    inline ConfigureAwaitOptions operator&(ConfigureAwaitOptions a, ConfigureAwaitOptions b) {
        return static_cast<ConfigureAwaitOptions>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Threading::Tasks
