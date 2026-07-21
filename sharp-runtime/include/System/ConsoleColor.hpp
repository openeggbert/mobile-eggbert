// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies constants that define foreground and background colors for the console.
     *
     * C++ counterpart of .NET System.ConsoleColor.
     */
    enum class ConsoleColor {
        /** @brief The color black. */
        Black       =  0,
        /** @brief The color dark blue. */
        DarkBlue    =  1,
        /** @brief The color dark green. */
        DarkGreen   =  2,
        /** @brief The color dark cyan (dark blue-green). */
        DarkCyan    =  3,
        /** @brief The color dark red. */
        DarkRed     =  4,
        /** @brief The color dark magenta (dark purplish-red). */
        DarkMagenta =  5,
        /** @brief The color dark yellow (ochre). */
        DarkYellow  =  6,
        /** @brief The color gray. */
        Gray        =  7,
        /** @brief The color dark gray. */
        DarkGray    =  8,
        /** @brief The color blue. */
        Blue        =  9,
        /** @brief The color green. */
        Green       = 10,
        /** @brief The color cyan (blue-green). */
        Cyan        = 11,
        /** @brief The color red. */
        Red         = 12,
        /** @brief The color magenta (purplish-red). */
        Magenta     = 13,
        /** @brief The color yellow. */
        Yellow      = 14,
        /** @brief The color white. */
        White       = 15
    };

} // namespace System
