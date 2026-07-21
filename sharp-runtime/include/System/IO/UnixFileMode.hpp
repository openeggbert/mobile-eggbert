// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::IO {

    /** Represents the POSIX file permission bits. */
    enum class UnixFileMode {
        /** No permissions. */
        None             = 0,
        /** Execute permission for others. */
        OtherExecute     = 0001,
        /** Write permission for others. */
        OtherWrite       = 0002,
        /** Read permission for others. */
        OtherRead        = 0004,
        /** Execute permission for the group. */
        GroupExecute     = 0010,
        /** Write permission for the group. */
        GroupWrite       = 0020,
        /** Read permission for the group. */
        GroupRead        = 0040,
        /** Execute permission for the owner. */
        UserExecute      = 0100,
        /** Write permission for the owner. */
        UserWrite        = 0200,
        /** Read permission for the owner. */
        UserRead         = 0400,
        /** Sticky bit. */
        StickyBit        = 01000,
        /** Set-group-ID bit. */
        SetGroup         = 02000,
        /** Set-user-ID bit. */
        SetUser          = 04000,
    };

    /** Returns the bitwise OR of two UnixFileMode values. */
    inline UnixFileMode operator|(UnixFileMode a, UnixFileMode b) {
        return static_cast<UnixFileMode>(static_cast<int>(a) | static_cast<int>(b));
    }
    /** Returns the bitwise AND of two UnixFileMode values. */
    inline UnixFileMode operator&(UnixFileMode a, UnixFileMode b) {
        return static_cast<UnixFileMode>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::IO
