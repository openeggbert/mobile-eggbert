// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /**
     * @brief Specifies the type of a file.
     *
     * C++ counterpart of .NET System.IO.FileHandleType.
     */
    enum class FileHandleType {
        /** The file type is unknown. */
        Unknown         = 0,
        /** The file is a regular file. */
        RegularFile     = 1,
        /** The file is a pipe (FIFO). */
        Pipe            = 2,
        /** The file is a socket. */
        Socket          = 3,
        /** The file is a character device. */
        CharacterDevice = 4,
        /** The file is a directory. */
        Directory       = 5,
        /** The file is a symbolic link. */
        SymbolicLink    = 6,
        /** The file is a block device. */
        BlockDevice     = 7,
    };

} // namespace System::IO
