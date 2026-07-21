// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System {

/**
 * @brief Specifies the reason why the runtime crashed or failed fast.
 *
 * C++ counterpart of .NET System.CrashReason (NativeAOT internal enum).
 * Used in crash diagnostics to categorise unhandled failures.
 */
enum class CrashReason {
    /** @brief The crash reason is unknown. */
    Unknown = 0,
    /** @brief The process crashed due to an unhandled exception. */
    UnhandledException = 1,
    /** @brief The process called Environment.FailFast. */
    EnvironmentFailFast = 2,
    /** @brief The runtime itself triggered a fail-fast due to an internal error. */
    InternalFailFast = 3,
};

} // namespace System
