// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Threading {

    /** Contains constants for specifying infinite wait durations. */
    class Timeout {
    public:
        /** Prevents instantiation — all members are static. */
        Timeout() = delete;

        /** Represents an infinite timeout as a millisecond value (-1). */
        static constexpr int Infinite = -1;
        /** Represents an infinite timeout as a TimeSpan tick value (-1 ms in 100-ns ticks). */
        static constexpr long long InfiniteTimeSpan = -10000LL; // -1 ms in TimeSpan ticks
    };

} // namespace System::Threading
