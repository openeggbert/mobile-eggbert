// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <memory>
#include <thread>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Threading/WaitHandle.hpp"
#include "System/Threading/WaitOrTimerCallback.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /**
     * @brief Represents a handle registered via ThreadPool::RegisterWaitForSingleObject.
     *
     * C++ counterpart of .NET System.Threading.RegisteredWaitHandle. Owns the background thread
     * that waits on the registered WaitHandle and invokes the callback; Unregister stops it.
     * Instances are only created by ThreadPool::RegisterWaitForSingleObject.
     */
    class RegisteredWaitHandle {
        struct State {
            std::atomic<bool> unregistered{false};
        };
        std::shared_ptr<State> state_;
        std::thread thread_;

        friend class ThreadPool;

        RegisteredWaitHandle(WaitHandle* waitObject, WaitOrTimerCallback callback, void* callbackState,
                              intcs millisecondsTimeOutInterval, bool executeOnlyOnce)
            : state_(std::make_shared<State>())
        {
            // Polls in short slices (rather than a single unbounded WaitOne) so Unregister()
            // takes effect promptly instead of waiting out the full timeout/signal.
            thread_ = std::thread([state = state_, waitObject, callback = std::move(callback),
                                    callbackState, millisecondsTimeOutInterval, executeOnlyOnce]() {
                constexpr intcs pollSliceMs = 50;
                while (!state->unregistered.load()) {
                    intcs elapsed = 0;
                    bool signaled = false;
                    for (;;) {
                        if (state->unregistered.load()) return;
                        signaled = waitObject->WaitOne(pollSliceMs);
                        if (signaled) break;
                        if (millisecondsTimeOutInterval >= 0) {
                            elapsed += pollSliceMs;
                            if (elapsed >= millisecondsTimeOutInterval) break;
                        }
                    }
                    if (state->unregistered.load()) return;
                    if (callback) callback(callbackState, !signaled);
                    if (executeOnlyOnce) return;
                }
            });
        }

    public:
        RegisteredWaitHandle(const RegisteredWaitHandle&) = delete;
        RegisteredWaitHandle& operator=(const RegisteredWaitHandle&) = delete;
        ~RegisteredWaitHandle() { Unregister(nullptr); }

        /**
         * @brief Cancels the registered wait; the waitObject parameter is accepted for API
         * compatibility and ignored (real .NET signals it once unregistration completes, but
         * this port has no user-visible use for it beyond that -- see the blocking note below).
         *
         * @note Deliberately blocks (joins the background wait thread) before returning, unlike
         * real .NET's `Unregister(null)`, which does not wait for an in-flight `WaitOne()` call
         * to return. Real .NET is safe to not block because its wait thread operates on a
         * ref-counted `SafeWaitHandle` (`DangerousAddRef`), keeping the underlying OS handle
         * alive even if the caller's `WaitHandle` wrapper is garbage-collected immediately after
         * `Unregister()` returns. This port's background thread instead calls
         * `waitObject->WaitOne(...)` directly on the caller-owned, non-reference-counted pointer
         * -- without blocking here, a caller that deletes `waitObject` right after `Unregister()`
         * returns would race the background thread's in-flight `WaitOne()` call on that same
         * (now freed) object: a genuine use-after-free, not just a parity gap. Blocking is the
         * simpler, safe alternative to reproducing .NET's ref-counted-handle machinery. The one
         * exception is a self-unregistering callback (this method called with `this_thread`
         * being the wait thread itself) — joining there would deadlock, so it detaches instead;
         * the thread is already about to exit on its own.
         */
        bool Unregister(WaitHandle* /*waitObject*/) {
            if (!state_ || state_->unregistered.exchange(true)) return false;
            if (thread_.joinable()) {
                if (thread_.get_id() == std::this_thread::get_id()) {
                    thread_.detach();
                } else {
                    thread_.join();
                }
            }
            return true;
        }
    };

} // namespace System::Threading
