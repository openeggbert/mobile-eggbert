// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/IO/WatcherChangeTypes.hpp"

namespace System::IO {

    /**
     * @brief Represents the result of waiting for a file system change with FileSystemWatcher.WaitForChanged.
     *
     * C++ counterpart of .NET System.IO.WaitForChangedResult.
     */
    struct WaitForChangedResult {
        /** The type of change that occurred. */
        WatcherChangeTypes ChangeType = static_cast<WatcherChangeTypes>(0);
        /** The name of the file or directory that changed, or an empty string if not applicable. */
        std::string Name;
        /** The previous name of the file or directory for a rename, or an empty string if not applicable. */
        std::string OldName;
        /** True if the wait timed out before a change occurred. */
        bool TimedOut = false;

        /** @brief The shared result representing a timed-out wait. C++ counterpart of .NET's internal TimedOutResult. */
        static WaitForChangedResult TimedOutResult() {
            WaitForChangedResult r;
            r.TimedOut = true;
            return r;
        }
    };

} // namespace System::IO
