// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <csignal>
#include <string>
#include "System/Exception.hpp"

namespace System::Diagnostics {

    /**
     * @brief Enables communication with a debugger.
     *
     * Partial C++ counterpart of .NET System.Diagnostics.Debugger.
     */
    class Debugger {
    public:
        /** @brief Not instantiable — all members are static. */
        Debugger() = delete;

        /**
         * @brief The default category of message, used by Log(). Real .NET's value is `null`;
         * this port uses an empty string as the closest C++ equivalent.
         */
        inline static const std::string DefaultCategory = "";

        /**
         * @brief Signals a breakpoint to an attached debugger with @p exception's details, if a
         * .NET debugger with break-on-user-unhandled-exception is attached and a method
         * attributed with DebuggerDisableUserUnhandledExceptionsAttribute calls this method.
         *
         * C++ counterpart of .NET Debugger.BreakForUserUnhandledException(Exception). A no-op
         * here, matching real .NET's own empty method body -- the JIT/debugger recognizes this
         * method by its metadata identity rather than any code it executes.
         * @param exception The user-unhandled exception (unused).
         */
        static void BreakForUserUnhandledException(const System::Exception& exception) { (void)exception; }

        /**
         * @return true if a debugger is attached to the process; always false in this
         * implementation. Real attach detection (e.g. parsing /proc/self/status's
         * TracerPid on Linux, or IsDebuggerPresent() on Windows) is not implemented.
         */
        [[nodiscard]] static bool getIsAttachedProperty() noexcept { return false; }

        /**
         * @brief Signals the debugger to break (triggers a debug trap).
         * 
         * Uses `__debugbreak()` on MSVC, `__builtin_trap()` on GCC/Clang,
         * or SIGTRAP elsewhere.
         */
        static void Break() noexcept {
#if defined(_MSC_VER)
            __debugbreak();
#elif defined(__GNUC__) || defined(__clang__)
            __builtin_trap();
#else
            std::raise(SIGTRAP);
#endif
        }

        /**
         * @brief Attempts to launch a debugger and attach it to the process.
         * @return Always false — launching is not supported in this implementation.
         */
        static bool Launch() noexcept { return false; }

        /**
         * @brief Sends a log message to the attached debugger; no-op if none is attached.
         * @param level    Severity level (ignored in this implementation).
         * @param category Log category (ignored in this implementation).
         * @param message  Message text (ignored in this implementation).
         */
        static void Log(int /*level*/, const std::string& /*category*/ = DefaultCategory, const std::string& /*message*/ = "") {}

        /** @return false — logging to the debugger is not supported in this implementation. */
        static bool IsLogging() noexcept { return false; }
    };

} // namespace System::Diagnostics
