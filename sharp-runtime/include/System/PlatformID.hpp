// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Identifies the operating system, or platform, supported by an assembly.
     *
     * C++ counterpart of .NET System.PlatformID.
     * Used by System.OperatingSystem and System.Environment.OSVersion to identify
     * the platform on which the application is running.
     */
    enum class PlatformID {
        /** @brief The operating system is Win32S (legacy, deprecated). */
        Win32S       = 0,

        /** @brief The operating system is Windows 95 or Windows 98 (legacy, deprecated). */
        Win32Windows = 1,

        /** @brief The operating system is Windows NT or later. */
        Win32NT      = 2,

        /** @brief The operating system is Windows CE (legacy, deprecated). */
        WinCE        = 3,

        /** @brief The operating system is Unix. */
        Unix         = 4,

        /** @brief The development platform is Xbox 360 (legacy, deprecated). */
        Xbox         = 5,

        /** @brief The operating system is macOS (legacy alias; use Unix on modern .NET). */
        MacOSX       = 6,

        /** @brief Any other operating system or platform. */
        Other        = 7,
    };

} // namespace System
