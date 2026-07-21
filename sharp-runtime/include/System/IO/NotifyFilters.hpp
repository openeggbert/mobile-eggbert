// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /**
     * @brief Specifies the changes to watch for in a file or folder.
     *
     * C++ counterpart of .NET System.IO.NotifyFilters.
     * Values may be combined with the | operator.
     */
    enum class NotifyFilters {
        /** The name of the file. */
        FileName      = 0x00000001,
        /** The name of the directory. */
        DirectoryName = 0x00000002,
        /** The attributes of the file or folder. */
        Attributes    = 0x00000004,
        /** The size of the file or folder. */
        Size          = 0x00000008,
        /** The date the file or folder last had anything written to it. */
        LastWrite     = 0x00000010,
        /** The date the file or folder was last opened. */
        LastAccess    = 0x00000020,
        /** The time the file or folder was created. */
        CreationTime  = 0x00000040,
        /** The security settings of the file or folder. */
        Security      = 0x00000100,
    };

    /**
     * @brief Combines two NotifyFilters values with bitwise OR.
     * @param a First value.
     * @param b Second value.
     * @return The combined value.
     */
    inline NotifyFilters operator|(NotifyFilters a, NotifyFilters b) {
        return static_cast<NotifyFilters>(static_cast<int>(a) | static_cast<int>(b));
    }

    /**
     * @brief Masks two NotifyFilters values with bitwise AND.
     * @param a First value.
     * @param b Second value.
     * @return The masked value.
     */
    inline NotifyFilters operator&(NotifyFilters a, NotifyFilters b) {
        return static_cast<NotifyFilters>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::IO
