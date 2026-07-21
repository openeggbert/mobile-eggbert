// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies options for applicable String.Split method overloads.
     *
     * C++ counterpart of .NET System.StringSplitOptions.
     * Values can be combined with | to apply multiple options simultaneously.
     */
    enum class StringSplitOptions {
        /** @brief The return value includes array elements that contain an empty string. */
        None               = 0,
        /** @brief The return value does not include array elements that contain an empty string. */
        RemoveEmptyEntries = 1,
        /** @brief The return value includes array elements whose entries are trimmed of leading and trailing whitespace. */
        TrimEntries        = 2,
    };

    inline StringSplitOptions operator|(StringSplitOptions a, StringSplitOptions b) {
        return static_cast<StringSplitOptions>(static_cast<int>(a) | static_cast<int>(b));
    }
    inline StringSplitOptions operator&(StringSplitOptions a, StringSplitOptions b) {
        return static_cast<StringSplitOptions>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System
