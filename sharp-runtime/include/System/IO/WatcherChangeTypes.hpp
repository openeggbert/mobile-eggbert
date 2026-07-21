// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /**
     * @brief Changes that may occur to a file or directory.
     *
     * C++ counterpart of .NET System.IO.WatcherChangeTypes.
     * Values may be combined with the | operator.
     */
    enum class WatcherChangeTypes {
        /** The creation of a file or folder. */
        Created = 1,
        /** The deletion of a file or folder. */
        Deleted = 2,
        /** The change of a file or folder. */
        Changed = 4,
        /** The renaming of a file or folder. */
        Renamed = 8,
        /** All of the above (Created | Deleted | Changed | Renamed). */
        All     = 1 | 2 | 4 | 8,
    };

    /**
     * @brief Combines two WatcherChangeTypes values with bitwise OR.
     * @param a First value.
     * @param b Second value.
     * @return The combined value.
     */
    inline WatcherChangeTypes operator|(WatcherChangeTypes a, WatcherChangeTypes b) {
        return static_cast<WatcherChangeTypes>(static_cast<int>(a) | static_cast<int>(b));
    }

    /**
     * @brief Masks two WatcherChangeTypes values with bitwise AND.
     * @param a First value.
     * @param b Second value.
     * @return The masked value.
     */
    inline WatcherChangeTypes operator&(WatcherChangeTypes a, WatcherChangeTypes b) {
        return static_cast<WatcherChangeTypes>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::IO
