// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO
{
    /**
     * @brief Specifies how the operating system should open a file.
     *
     * C++ counterpart of .NET System.IO.FileMode.
     *
     * @note Status: Implemented — all six .NET FileMode values are present.
     */
    enum class FileMode
    {
        /** @brief Creates a new file, failing if it already exists. */
        CreateNew    = 1,
        /** @brief Creates a new file; overwrites if it already exists. */
        Create       = 2,
        /** @brief Opens an existing file. Throws if not found. */
        Open         = 3,
        /** @brief Opens if exists, otherwise creates. */
        OpenOrCreate = 4,
        /** @brief Opens and truncates an existing file to zero bytes. */
        Truncate     = 5,
        /** @brief Opens or creates a file and seeks to end before each write. */
        Append       = 6
    };
}