// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <mutex>
#include <condition_variable>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Threading/WaitHandle.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /**
     * Represents a thread synchronization event that, when signaled, must be reset manually.
     * 
     * Wraps std::condition_variable. Partial C++ counterpart of .NET System.Threading.ManualResetEvent.
     * 
     * @note Status: Implemented
     */
    class ManualResetEvent {
        std::mutex              mutex_;
        std::condition_variable cv_;
        bool                    signaled_;
    public:
        /** @param initialState If true, the event starts in the signaled state. */
        explicit ManualResetEvent(bool initialState = false) : signaled_(initialState) {}

        /** Sets the event to signaled, releasing all waiting threads. */
        void Set() {
            { std::lock_guard<std::mutex> lk(mutex_); signaled_ = true; }
            cv_.notify_all();
        }

        /** Resets the event to non-signaled. */
        void Reset() {
            std::lock_guard<std::mutex> lk(mutex_);
            signaled_ = false;
        }

        /** Blocks until the event is signaled. */
        void WaitOne() {
            std::unique_lock<std::mutex> lk(mutex_);
            cv_.wait(lk, [this]{ return signaled_; });
        }

        /**
         * Blocks until the event is signaled or the timeout elapses.
         * @param milliseconds Maximum time to wait.
         * @return True if the event was signaled before the timeout.
         * @throws System::ArgumentOutOfRangeException if @p milliseconds is less than -1.
         */
        bool WaitOne(intcs milliseconds) {
            WaitHandle::ValidateTimeout(milliseconds);
            std::unique_lock<std::mutex> lk(mutex_);
            // -1 (Timeout.Infinite) waits indefinitely; std::chrono's wait_for treats a
            // negative duration as already-expired, so it must be special-cased.
            if (milliseconds == -1) {
                cv_.wait(lk, [this]{ return signaled_; });
                return true;
            }
            return cv_.wait_for(lk, std::chrono::milliseconds(milliseconds), [this]{ return signaled_; });
        }

        /** Releases resources (no-op; provided for .NET API compatibility). */
        void Close() {}
    };

} // namespace System::Threading
