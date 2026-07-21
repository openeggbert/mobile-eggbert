// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Represents the SHIFT, ALT, and CTRL modifier keys on a keyboard.
     *
     * C++ counterpart of .NET System.ConsoleModifiers.
     * This is a [Flags] enum — values can be combined with bitwise OR.
     */
    enum class ConsoleModifiers {
        /** @brief No modifier key was pressed. */
        None    = 0,
        /** @brief The ALT modifier key. */
        Alt     = 1,
        /** @brief The SHIFT modifier key. */
        Shift   = 2,
        /** @brief The CTRL modifier key. */
        Control = 4,
    };

    /** @brief Returns the bitwise OR of two ConsoleModifiers values. */
    inline ConsoleModifiers operator|(ConsoleModifiers a, ConsoleModifiers b) {
        return static_cast<ConsoleModifiers>(static_cast<int>(a) | static_cast<int>(b));
    }
    /** @brief Returns the bitwise AND of two ConsoleModifiers values. */
    inline ConsoleModifiers operator&(ConsoleModifiers a, ConsoleModifiers b) {
        return static_cast<ConsoleModifiers>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System
