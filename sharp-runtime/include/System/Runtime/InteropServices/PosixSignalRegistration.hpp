// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Runtime/InteropServices/PosixSignal.hpp"
#include "System/Runtime/InteropServices/PosixSignalContext.hpp"

namespace System::Runtime::InteropServices {

    /**
     * @brief Handles a PosixSignal.
     *
     * C++ counterpart of .NET System.Runtime.InteropServices.PosixSignalRegistration.
     * Move-only (matches the .NET type's single-owner `IDisposable`/`using` usage pattern).
     * Handlers are invoked on a dedicated background watcher thread -- never from inside the raw
     * OS signal handler itself, which may only safely call a small, fixed set of
     * async-signal-safe C functions (POSIX.1 defines this set; arbitrary C++ code, including
     * memory allocation and exceptions, is explicitly excluded from it). The watcher thread is
     * woken via the standard self-pipe trick: the signal handler does nothing but set a
     * `volatile sig_atomic_t` flag and write one byte to a pipe (both async-signal-safe), and the
     * watcher thread blocks on reading that pipe.
     */
    class PosixSignalRegistration {
    public:
        /** @brief The handler signature. C++ counterpart of .NET Action&lt;PosixSignalContext&gt;. */
        using Handler = std::function<void(PosixSignalContext&)>;

        PosixSignalRegistration(const PosixSignalRegistration&) = delete;
        PosixSignalRegistration& operator=(const PosixSignalRegistration&) = delete;
        PosixSignalRegistration(PosixSignalRegistration&& other) noexcept;
        PosixSignalRegistration& operator=(PosixSignalRegistration&& other) noexcept;

        /** @brief Unregisters the handler, if still registered. */
        ~PosixSignalRegistration();

        /**
         * @brief Registers @p handler to be invoked when @p signal occurs.
         *
         * C++ counterpart of .NET PosixSignalRegistration.Create(PosixSignal, Action&lt;PosixSignalContext&gt;).
         * Handlers registered for the same signal are invoked in reverse order of registration,
         * matching real .NET. Setting PosixSignalContext::setCancelProperty(true) from any handler
         * suppresses the OS's default disposition for that signal delivery (e.g. process
         * termination for SIGTERM/SIGINT/SIGHUP/SIGQUIT); if no handler cancels it, the default
         * disposition runs after all handlers return.
         * @throws System::ArgumentNullException if @p handler is empty.
         * @throws System::PlatformNotSupportedException if @p signal cannot be caught (SIGKILL),
         *         or on a platform with no signal-handling support (e.g. Emscripten).
         * @throws System::IO::IOException if installing the OS-level signal handler fails.
         */
        static PosixSignalRegistration Create(PosixSignal signal, Handler handler);

        /** @brief Unregisters the handler. C++ counterpart of .NET PosixSignalRegistration.Dispose(). */
        void Dispose();

    private:
        PosixSignalRegistration(PosixSignal signal, SharpRuntime::intcs token) : signal_(signal), token_(token) {}

        PosixSignal signal_;
        SharpRuntime::intcs token_; // internal registration id; -1 once disposed/moved-from
    };

} // namespace System::Runtime::InteropServices
