// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Threading {

    /** Indicates whether an EventWaitHandle resets automatically or manually after being signalled. */
    enum class EventResetMode {
        /** The event resets automatically after releasing a single waiting thread. */
        AutoReset   = 0,
        /** The event must be reset manually. */
        ManualReset = 1,
    };

} // namespace System::Threading
