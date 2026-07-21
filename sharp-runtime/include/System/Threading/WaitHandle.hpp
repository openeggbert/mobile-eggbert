// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/IDisposable.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /** Abstract base class for OS synchronisation handles. */
    class WaitHandle : public System::IDisposable {
    public:
        /**
         * @brief Validates a millisecondsTimeout argument for WaitOne-style methods.
         *
         * C++ counterpart of the bounds check in .NET WaitHandle.WaitOne(int), shared by
         * every timed wait method in this hierarchy (and by the non-WaitHandle-derived
         * Mutex/Semaphore/SemaphoreSlim/ManualResetEventSlim wait methods).
         * @throws System::ArgumentOutOfRangeException if @p millisecondsTimeout is less than -1.
         */
        static void ValidateTimeout(intcs millisecondsTimeout) {
            System::ArgumentOutOfRangeException::ThrowIfLessThan(millisecondsTimeout, static_cast<intcs>(-1), "millisecondsTimeout");
        }

        /** Destroys the WaitHandle. */
        virtual ~WaitHandle() = default;

        /** Blocks the current thread until the handle is signalled. */
        virtual bool WaitOne() = 0;
        /** Blocks the current thread until the handle is signalled or millisecondsTimeout elapses. */
        virtual bool WaitOne(intcs millisecondsTimeout) = 0;

        /** Releases resources held by this WaitHandle. */
        void Dispose() override {}
    };

} // namespace System::Threading
