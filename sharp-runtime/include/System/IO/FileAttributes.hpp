// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /** File and directory attributes. */
    enum class FileAttributes {
        /** The file has no special attributes. */
        None               = 0x00000,
        /** The file is read-only. */
        ReadOnly           = 0x00001,
        /** The file is hidden. */
        Hidden             = 0x00002,
        /** The file is a system file. */
        System             = 0x00004,
        /** The file is a directory. */
        Directory          = 0x00010,
        /** The file is a candidate for backup or removal. */
        Archive            = 0x00020,
        /** The file is reserved for system use. */
        Device             = 0x00040,
        /** The file has no other attributes set. */
        Normal             = 0x00080,
        /** The file is temporary. */
        Temporary          = 0x00100,
        /** The file is a sparse file. */
        SparseFile         = 0x00200,
        /** The file contains a reparse point. */
        ReparsePoint       = 0x00400,
        /** The file or directory is compressed. */
        Compressed         = 0x00800,
        /** The file data is not immediately available. */
        Offline            = 0x01000,
        /** The file will not be indexed by the content indexing service. */
        NotContentIndexed  = 0x02000,
        /** The file or directory is encrypted. */
        Encrypted          = 0x04000,
        /** The file data is integrity-protected. */
        IntegrityStream    = 0x08000,
        /** The file data should not be scrubbed. */
        NoScrubData        = 0x20000,
    };

    /** Returns the bitwise OR of two FileAttributes values. */
    inline FileAttributes operator|(FileAttributes a, FileAttributes b) {
        return static_cast<FileAttributes>(static_cast<int>(a) | static_cast<int>(b));
    }
    /** Returns the bitwise AND of two FileAttributes values. */
    inline FileAttributes operator&(FileAttributes a, FileAttributes b) {
        return static_cast<FileAttributes>(static_cast<int>(a) & static_cast<int>(b));
    }
    /** Returns the bitwise complement of a FileAttributes value. */
    inline FileAttributes operator~(FileAttributes a) {
        return static_cast<FileAttributes>(~static_cast<int>(a));
    }

} // namespace System::IO
