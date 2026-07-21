// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /**
     * @brief Contains constants for controlling the kind of access other
     * FileStream objects can have to the same file.
     *
     * @note Status: Implemented
     */
    enum class FileShare : int {
        /** Declines sharing of the current file. */
        None        = 0,
        /** Allows subsequent opening for reading. */
        Read        = 1,
        /** Allows subsequent opening for writing. */
        Write       = 2,
        /** Allows subsequent opening for reading or writing. */
        ReadWrite   = 3,
        /** Allows subsequent deleting of the file. */
        Delete      = 4,
        /** Makes the file handle inheritable by child processes. */
        Inheritable = 16
    };

    /**
     * @brief Combines two FileShare values with bitwise OR.
     * @param a First value.
     * @param b Second value.
     * @return The combined value.
     */
    inline FileShare operator|(FileShare a, FileShare b) {
        return static_cast<FileShare>(static_cast<int>(a) | static_cast<int>(b));
    }

    /**
     * @brief Masks two FileShare values with bitwise AND.
     * @param a First value.
     * @param b Second value.
     * @return The masked value.
     */
    inline FileShare operator&(FileShare a, FileShare b) {
        return static_cast<FileShare>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::IO
