// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies the location where an environment variable is stored or retrieved.
     *
     * C++ counterpart of .NET System.EnvironmentVariableTarget.
     */
    enum class EnvironmentVariableTarget {
        /** @brief The environment variable is stored or retrieved from the process environment block. */
        Process = 0,
        /** @brief The environment variable is stored or retrieved from the HKEY_CURRENT_USER registry key (Windows only; validated but otherwise a no-op on other platforms, matching .NET). */
        User    = 1,
        /** @brief The environment variable is stored or retrieved from the HKEY_LOCAL_MACHINE registry key (Windows only; validated but otherwise a no-op on other platforms, matching .NET). */
        Machine = 2,
    };

} // namespace System
