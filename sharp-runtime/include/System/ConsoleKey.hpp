// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies the standard keys on a console.
     *
     * C++ counterpart of .NET System.ConsoleKey.
     * Values match Windows virtual-key codes.
     */
    enum class ConsoleKey {
        None          = 0x00,
        /** @brief The BACKSPACE key. */
        Backspace     = 0x08,
        /** @brief The TAB key. */
        Tab           = 0x09,
        /** @brief The CLEAR key. */
        Clear         = 0x0C,
        /** @brief The ENTER key. */
        Enter         = 0x0D,
        /** @brief The PAUSE key. */
        Pause         = 0x13,
        /** @brief The ESCAPE key. */
        Escape        = 0x1B,
        /** @brief The SPACEBAR key. */
        Spacebar      = 0x20,
        /** @brief The PAGE UP key. */
        PageUp        = 0x21,
        /** @brief The PAGE DOWN key. */
        PageDown      = 0x22,
        /** @brief The END key. */
        End           = 0x23,
        /** @brief The HOME key. */
        Home          = 0x24,
        /** @brief The LEFT ARROW key. */
        LeftArrow     = 0x25,
        /** @brief The UP ARROW key. */
        UpArrow       = 0x26,
        /** @brief The RIGHT ARROW key. */
        RightArrow    = 0x27,
        /** @brief The DOWN ARROW key. */
        DownArrow     = 0x28,
        /** @brief The SELECT key. */
        Select        = 0x29,
        /** @brief The PRINT key. */
        Print         = 0x2A,
        /** @brief The EXECUTE key. */
        Execute       = 0x2B,
        /** @brief The PRINT SCREEN key. */
        PrintScreen   = 0x2C,
        /** @brief The INSERT key. */
        Insert        = 0x2D,
        /** @brief The DELETE key. */
        Delete        = 0x2E,
        /** @brief The HELP key. */
        Help          = 0x2F,
        D0 = 0x30, D1 = 0x31, D2 = 0x32, D3 = 0x33, D4 = 0x34,
        D5 = 0x35, D6 = 0x36, D7 = 0x37, D8 = 0x38, D9 = 0x39,
        A = 0x41, B = 0x42, C = 0x43, D = 0x44, E = 0x45,
        F = 0x46, G = 0x47, H = 0x48, I = 0x49, J = 0x4A,
        K = 0x4B, L = 0x4C, M = 0x4D, N = 0x4E, O = 0x4F,
        P = 0x50, Q = 0x51, R = 0x52, S = 0x53, T = 0x54,
        U = 0x55, V = 0x56, W = 0x57, X = 0x58, Y = 0x59, Z = 0x5A,
        /** @brief The left Windows logo key (Microsoft Natural Keyboard). */
        LeftWindows   = 0x5B,
        /** @brief The right Windows logo key (Microsoft Natural Keyboard). */
        RightWindows  = 0x5C,
        /** @brief The Application key (Microsoft Natural Keyboard). */
        Applications  = 0x5D,
        /** @brief The Computer Sleep key. */
        Sleep         = 0x5F,
        NumPad0 = 0x60, NumPad1 = 0x61, NumPad2 = 0x62, NumPad3 = 0x63,
        NumPad4 = 0x64, NumPad5 = 0x65, NumPad6 = 0x66, NumPad7 = 0x67,
        NumPad8 = 0x68, NumPad9 = 0x69,
        Multiply  = 0x6A, Add       = 0x6B, Separator = 0x6C,
        Subtract  = 0x6D, Decimal   = 0x6E, Divide    = 0x6F,
        F1  = 0x70, F2  = 0x71, F3  = 0x72, F4  = 0x73,
        F5  = 0x74, F6  = 0x75, F7  = 0x76, F8  = 0x77,
        F9  = 0x78, F10 = 0x79, F11 = 0x7A, F12 = 0x7B,
        F13 = 0x7C, F14 = 0x7D, F15 = 0x7E, F16 = 0x7F,
        F17 = 0x80, F18 = 0x81, F19 = 0x82, F20 = 0x83,
        F21 = 0x84, F22 = 0x85, F23 = 0x86, F24 = 0x87,
        BrowserBack      = 0xA6, BrowserForward   = 0xA7,
        BrowserRefresh   = 0xA8, BrowserStop      = 0xA9,
        BrowserSearch    = 0xAA, BrowserFavorites = 0xAB,
        BrowserHome      = 0xAC,
        VolumeMute = 0xAD, VolumeDown = 0xAE, VolumeUp = 0xAF,
        MediaNext = 0xB0, MediaPrevious = 0xB1,
        MediaStop = 0xB2, MediaPlay    = 0xB3,
        LaunchMail           = 0xB4,
        LaunchMediaSelect    = 0xB5,
        LaunchApp1           = 0xB6,
        LaunchApp2           = 0xB7,
        Oem1        = 0xBA, OemPlus    = 0xBB,
        OemComma    = 0xBC, OemMinus   = 0xBD,
        OemPeriod   = 0xBE, Oem2       = 0xBF,
        Oem3        = 0xC0,
        Oem4        = 0xDB, Oem5       = 0xDC,
        Oem6        = 0xDD, Oem7       = 0xDE,
        Oem8        = 0xDF,
        Oem102      = 0xE2,
        Process     = 0xE5,
        Packet      = 0xE7,
        Attention   = 0xF6,
        CrSel       = 0xF7,
        ExSel       = 0xF8,
        EraseEndOfFile = 0xF9,
        Play        = 0xFA,
        Zoom        = 0xFB,
        NoName      = 0xFC,
        Pa1         = 0xFD,
        OemClear    = 0xFE,
    };

} // namespace System
