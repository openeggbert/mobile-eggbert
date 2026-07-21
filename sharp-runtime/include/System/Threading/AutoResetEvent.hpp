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
     * Represents a thread synchronization event that resets automatically after releasing a single waiting thread.
     * 
     * Wraps std::condition_variable. Partial C++ counterpart of .NET System.Threading.AutoResetEvent.
     * 
     * @note Status: Implemented
     */
    class AutoResetEvent {
        std::mutex              mutex_;
        std::condition_variable cv_;
        bool                    signaled_;
    public:
        /** @param initialState If true, the event starts in the signaled state. */
        explicit AutoResetEvent(bool initialState = false) : signaled_(initialState) {}

        /** Sets the event to signaled, releasing one waiting thread; the event then resets automatically. */
        void Set() {
            { std::lock_guard<std::mutex> lk(mutex_); signaled_ = true; }
            cv_.notify_one();
        }

        /** Resets the event to non-signaled. */
        void Reset() {
            std::lock_guard<std::mutex> lk(mutex_);
            signaled_ = false;
        }

        /** Blocks until the event is signaled (then auto-resets). */
        void WaitOne() {
            std::unique_lock<std::mutex> lk(mutex_);
            cv_.wait(lk, [this]{ return signaled_; });
            signaled_ = false;
        }

        /**
         * Blocks until signaled or timeout elapses.
         * @param milliseconds Maximum time to wait.
         * @return True if the event was signaled (then auto-resets); false on timeout.
         * @throws System::ArgumentOutOfRangeException if @p milliseconds is less than -1.
         * @note Verified against every sibling wait-handle type in this codebase
         * (ManualResetEvent, EventWaitHandle, Mutex, Semaphore, SemaphoreSlim, CountdownEvent,
         * ManualResetEventSlim), all of which call WaitHandle::ValidateTimeout(...) here. This
         * was previously the one call site missing it: a timeout below -1 (e.g. -2) silently
         * reached cv_.wait_for(..., milliseconds(-2), ...), which per C++'s negative-duration
         * semantics returns almost immediately instead of throwing.
         */
        bool WaitOne(intcs milliseconds) {
            WaitHandle::ValidateTimeout(milliseconds);
            std::unique_lock<std::mutex> lk(mutex_);
            // -1 (Timeout.Infinite) waits indefinitely; std::chrono's wait_for treats a
            // negative duration as already-expired, so it must be special-cased.
            bool ok;
            if (milliseconds == -1) {
                cv_.wait(lk, [this]{ return signaled_; });
                ok = true;
            } else {
                ok = cv_.wait_for(lk, std::chrono::milliseconds(milliseconds), [this]{ return signaled_; });
            }
            if (ok) signaled_ = false;
            return ok;
        }

        /** Releases resources (no-op; provided for .NET API compatibility). */
        void Close() {}
    };

} // namespace System::Threading
