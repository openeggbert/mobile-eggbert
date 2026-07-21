// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Runtime::InteropServices {

    /**
     * @brief Specifies a POSIX signal number.
     *
     * C++ counterpart of .NET System.Runtime.InteropServices.PosixSignal. Values are negative
     * (matching real .NET exactly), decoupling this portable enum from the actual, platform-
     * specific `SIGxxx` macro values looked up internally by PosixSignalRegistration.
     *
     * Member names deliberately deviate from .NET's literal ALL-CAPS spelling (`SIGHUP`,
     * `SIGINT`, ...): those exact identifiers are `#define`d as macros by `<signal.h>`/
     * `<csignal>`, and the C++ preprocessor has no notion of C++ scoping, so any translation unit
     * that includes both this header and a POSIX signal header (directly, or transitively --
     * e.g. GoogleTest's own death-test support pulls in `<signal.h>`) would have every
     * `PosixSignal::SIGHUP`-shaped token silently corrupted into `PosixSignal::-1` before the
     * compiler's C++ front end ever sees it. This is not a style choice; it is a forced,
     * documented deviation from literal name parity to keep the type usable at all in a C++
     * translation unit that also touches POSIX signal APIs. Values match .NET exactly; only the
     * spelling (capitalize only the first letter after "Sig") differs.
     */
    enum class PosixSignal : SharpRuntime::intcs {
        /** @brief Hangup. C++ counterpart of .NET PosixSignal.SIGHUP. */
        Sighup = -1,
        /** @brief Interrupt. C++ counterpart of .NET PosixSignal.SIGINT. */
        Sigint = -2,
        /** @brief Quit. C++ counterpart of .NET PosixSignal.SIGQUIT. */
        Sigquit = -3,
        /** @brief Termination. C++ counterpart of .NET PosixSignal.SIGTERM. */
        Sigterm = -4,
        /** @brief Child stopped. C++ counterpart of .NET PosixSignal.SIGCHLD. */
        Sigchld = -5,
        /** @brief Continue if stopped. C++ counterpart of .NET PosixSignal.SIGCONT. */
        Sigcont = -6,
        /** @brief Window resized. C++ counterpart of .NET PosixSignal.SIGWINCH. */
        Sigwinch = -7,
        /** @brief Terminal input for background process. C++ counterpart of .NET PosixSignal.SIGTTIN. */
        Sigttin = -8,
        /** @brief Terminal output for background process. C++ counterpart of .NET PosixSignal.SIGTTOU. */
        Sigttou = -9,
        /** @brief Stop typed at terminal. C++ counterpart of .NET PosixSignal.SIGTSTP. */
        Sigtstp = -10,
        /** @brief Kill (cannot be caught or ignored). C++ counterpart of .NET PosixSignal.SIGKILL. */
        Sigkill = -11
    };

} // namespace System::Runtime::InteropServices
