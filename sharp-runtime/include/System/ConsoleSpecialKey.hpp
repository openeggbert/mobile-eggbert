// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

    /**
     * @brief Specifies combinations of modifier and console keys that can interrupt
     * the current process.
     *
     * C++ counterpart of .NET System.ConsoleSpecialKey.
     */
    enum class ConsoleSpecialKey {
        /** @brief The CTRL+C key combination. */
        ControlC     = 0,
        /** @brief The CTRL+BREAK key combination. */
        ControlBreak = 1,
    };

} // namespace System
