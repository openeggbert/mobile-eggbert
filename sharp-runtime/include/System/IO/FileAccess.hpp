// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /**
     * @brief Defines constants for read, write, or read/write access to a file.
     *
     * @note Status: Implemented
     */
    enum class FileAccess : int {
        /** Opens the file for reading only. */
        Read      = 1,
        /** Opens the file for writing only. */
        Write     = 2,
        /** Opens the file for both reading and writing. */
        ReadWrite = 3
    };

    /**
     * @brief Combines two FileAccess values with bitwise OR.
     * @param a First value.
     * @param b Second value.
     * @return The combined value.
     */
    inline FileAccess operator|(FileAccess a, FileAccess b) {
        return static_cast<FileAccess>(static_cast<int>(a) | static_cast<int>(b));
    }

    /**
     * @brief Masks two FileAccess values with bitwise AND.
     * @param a First value.
     * @param b Second value.
     * @return The masked value.
     */
    inline FileAccess operator&(FileAccess a, FileAccess b) {
        return static_cast<FileAccess>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::IO
