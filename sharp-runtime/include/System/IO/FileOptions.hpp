// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /** Represents advanced options for creating a FileStream object. */
    enum class FileOptions {
        /** No additional options. */
        None           = 0,
        /** Indicates that the system should write through any intermediate cache and go directly to disk. */
        WriteThrough   = static_cast<int>(0x80000000u),
        /** Indicates that the file is to be used for asynchronous reading and writing. */
        Asynchronous   = static_cast<int>(0x40000000u),
        /** Indicates that the file is accessed randomly. */
        RandomAccess   = 0x10000000,
        /** Indicates that the file is automatically deleted when it is no longer in use. */
        DeleteOnClose  = 0x04000000,
        /** Indicates that the file is accessed sequentially from beginning to end. */
        SequentialScan = 0x08000000,
        /** Indicates that the file is encrypted and can be decrypted only using the same user account used for encryption. */
        Encrypted      = 0x00004000,
    };

    /** Returns the bitwise OR of two FileOptions values. */
    inline FileOptions operator|(FileOptions a, FileOptions b) {
        return static_cast<FileOptions>(static_cast<int>(a) | static_cast<int>(b));
    }
    /** Returns the bitwise AND of two FileOptions values. */
    inline FileOptions operator&(FileOptions a, FileOptions b) {
        return static_cast<FileOptions>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::IO
