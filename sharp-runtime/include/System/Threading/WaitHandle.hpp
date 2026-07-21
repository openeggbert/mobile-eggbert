// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <chrono>
#include <cstddef>
#include <thread>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/IDisposable.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /** Abstract base class for OS synchronisation handles. */
    class WaitHandle : public System::IDisposable {
    public:
        /** Return value from a timed wait that expired before the handle was signalled. */
        static constexpr intcs WaitTimeout = 258;   // WAIT_TIMEOUT on Windows
        /** Sentinel for an invalid native handle. */
        static constexpr intcs InvalidHandle = -1;

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

        /** Closes the WaitHandle by calling Dispose. */
        void Close() { Dispose(); }

        /**
         * @brief Waits for all the elements of @p handles to receive a signal.
         *
         * @note Deliberate simplification: waits on each handle sequentially rather than
         * atomically multiplexing on the underlying OS objects (WaitHandle exposes no native
         * handle to multiplex on in this port). This is observably equivalent for the common
         * case of independent handles, but does not detect abandoned-mutex ordering the way
         * a true WaitForMultipleObjects call would.
         */
        static bool WaitAll(const std::vector<WaitHandle*>& handles) {
            for (WaitHandle* h : handles)
                if (h) h->WaitOne();
            return true;
        }

        /**
         * @brief Waits for all the elements of @p handles to receive a signal, or until millisecondsTimeout elapses.
         *
         * -1 (Timeout.Infinite) waits indefinitely for each handle in turn. A deadline
         * computed as now()+milliseconds(-1) would already be in the past, causing every
         * handle to time out immediately, so -1 must be special-cased.
         */
        static bool WaitAll(const std::vector<WaitHandle*>& handles, intcs millisecondsTimeout) {
            if (millisecondsTimeout == -1) {
                for (WaitHandle* h : handles)
                    if (h) h->WaitOne();
                return true;
            }
            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(millisecondsTimeout);
            for (WaitHandle* h : handles) {
                if (!h) continue;
                auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now());
                if (remaining.count() < 0 || !h->WaitOne(static_cast<intcs>(remaining.count())))
                    return false;
            }
            return true;
        }

        /**
         * @brief Waits for any of the elements of @p handles to receive a signal and returns its index.
         *
         * @note Deliberate simplification: polls each handle with a short non-blocking wait in a
         * round-robin loop rather than an atomic OS-level multiplex; see WaitAll for the same caveat.
         */
        static intcs WaitAny(const std::vector<WaitHandle*>& handles) {
            for (;;) {
                for (std::size_t i = 0; i < handles.size(); ++i) {
                    if (handles[i] && handles[i]->WaitOne(0))
                        return static_cast<intcs>(i);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        /**
         * @brief Waits for any of the elements of @p handles to receive a signal, or until millisecondsTimeout elapses.
         *
         * -1 (Timeout.Infinite) polls indefinitely with no deadline. A deadline computed as
         * now()+milliseconds(-1) would already be in the past, causing an immediate
         * WaitTimeout, so -1 must be special-cased.
         */
        static intcs WaitAny(const std::vector<WaitHandle*>& handles, intcs millisecondsTimeout) {
            if (millisecondsTimeout == -1) {
                for (;;) {
                    for (std::size_t i = 0; i < handles.size(); ++i) {
                        if (handles[i] && handles[i]->WaitOne(0))
                            return static_cast<intcs>(i);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(millisecondsTimeout);
            for (;;) {
                for (std::size_t i = 0; i < handles.size(); ++i) {
                    if (handles[i] && handles[i]->WaitOne(0))
                        return static_cast<intcs>(i);
                }
                if (std::chrono::steady_clock::now() >= deadline)
                    return WaitTimeout;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    };

} // namespace System::Threading
