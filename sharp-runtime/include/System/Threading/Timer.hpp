// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
#  include "System/PlatformNotSupportedException.hpp"
#endif

namespace System::Threading {

    using SharpRuntime::intcs;

    /** Delegate type for timer callbacks — counterpart of .NET System.Threading.TimerCallback. */
    using TimerCallback = std::function<void(void*)>;

    /**
     * @brief Provides a mechanism for executing a method at specified intervals on a thread-pool thread.
     *
     * Partial C++ counterpart of .NET System.Threading.Timer.
     * Uses a dedicated std::thread instead of a thread pool.
     *
     * @note Status: Partial — no thread-pool; uses one dedicated std::thread per Timer instead.
     * Only the `(callback, state, int dueTime, int period)` constructor and matching
     * `Change(int, int)` are implemented; the TimeSpan/uint/long-typed constructor and Change
     * overloads, the callback-only (self-registering) constructor, static `ActiveCount`,
     * `Dispose(WaitHandle)`, and `DisposeAsync()` are not ported. `Change()` returns `void` here
     * vs. real .NET's `bool` success indicator. The implemented core (due-time/period rescheduling,
     * pause via dueTime=-1, mid-callback Change() handling) is verified against real .NET's
     * TimerQueueTimer behavior and uses shared_ptr-based state ownership specifically to avoid a
     * dangling-`this` hazard in the background thread (contrast with System::Timers::Timer, a
     * different type in a different namespace, which has a documented `this`-capture hazard).
     */
    class Timer {
        // Shared state owned jointly by the Timer and the background thread.
        // Using shared_ptr ensures the State outlives the Timer object even after
        // Dispose() detaches the thread — eliminates the dangling-this UB.
        struct State {
            std::atomic<bool>          running{false};
            std::function<void(void*)> callback;
            void*                      arg     = nullptr;
            // dueTime/period are protected by mtx (not atomic): run() needs to atomically
            // check-and-wait on them together via cv, which a pair of independent atomics can't
            // express without a race between the check and the wait.
            std::mutex                 mtx;
            std::condition_variable    cv;
            intcs                      dueTime = 0;
            intcs                      period  = 0;
            // Verified against TimerQueue.UpdateTimer: every Change() call reschedules the
            // *next* fire relative to when Change() itself is called (`nowTicks + dueTime`),
            // regardless of whether the timer thread is currently mid-sleep between fires or
            // mid-callback. Bumped on every Change() call so run()'s wait loop can detect "the
            // schedule changed since I started this cycle" and react immediately instead of
            // finishing a now-stale sleep or clobbering a fresh dueTime after the callback.
            std::uint64_t              generation = 0;
        };

        std::shared_ptr<State> state_;
        std::thread            thread_;

    public:
        /**
         * @brief Initializes the timer with the given callback, state, initial delay, and repeat period (in milliseconds).
         * @throws System::ArgumentOutOfRangeException if @p dueTime or @p period is less than -1.
         */
        Timer(std::function<void(void*)> callback, void* state, intcs dueTime, intcs period)
            : state_(std::make_shared<State>())
        {
            System::ArgumentOutOfRangeException::ThrowIfLessThan(dueTime, static_cast<intcs>(-1), "dueTime");
            System::ArgumentOutOfRangeException::ThrowIfLessThan(period, static_cast<intcs>(-1), "period");
#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
            (void)callback; (void)state; (void)dueTime; (void)period;
            throw System::PlatformNotSupportedException("Timer requires pthreads (not available in Emscripten single-threaded build)");
#else
            state_->callback = std::move(callback);
            state_->arg      = state;
            state_->dueTime  = dueTime;
            state_->period   = period;
            state_->running  = true;
            thread_ = std::thread([s = state_]() { run(s); });
#endif
        }

        /** Destroys the Timer and stops the background thread. */
        ~Timer() { Dispose(); }

        /** Copying is not allowed. */
        Timer(const Timer&) = delete;
        /** Copy assignment is not allowed. */
        Timer& operator=(const Timer&) = delete;

        /**
         * @brief Changes the timer's due time and period. Pass -1 to disable.
         * @throws System::ArgumentOutOfRangeException if @p dueTime or @p period is less than -1.
         */
        void Change(intcs dueTime, intcs period) {
            System::ArgumentOutOfRangeException::ThrowIfLessThan(dueTime, static_cast<intcs>(-1), "dueTime");
            System::ArgumentOutOfRangeException::ThrowIfLessThan(period, static_cast<intcs>(-1), "period");
            {
                std::lock_guard<std::mutex> lock(state_->mtx);
                state_->dueTime = dueTime;
                state_->period  = period;
                ++state_->generation;
            }
            state_->cv.notify_all();
        }

        /** Stops the timer and releases the background thread. */
        void Dispose() {
            if (state_) {
                state_->running = false;
                state_->cv.notify_all();
            }
            if (thread_.joinable()) thread_.detach();
        }

    private:
        // Verified against TimerQueueTimer.Change()/TimerQueue.UpdateTimer: a timer whose
        // dueTime is Timeout.Infinite (-1) is never scheduled to fire -- not just when never
        // started, but also when Change(-1, ...) pauses an already-active timer
        // (TimerQueueTimer.Change() removes the timer from the queue whenever the new dueTime
        // is UnsignedInfinite). This port previously only special-cased "dueTime > 0" before
        // the very first wait, so -1 fell through and fired on the first loop iteration just
        // like 0 does, and had no way to represent "paused" once past the first fire.
        static void run(std::shared_ptr<State> s) {
            std::unique_lock<std::mutex> lock(s->mtx);
            while (s->running) {
                while (s->running && s->dueTime == -1) {
                    s->cv.wait(lock);
                }
                if (!s->running) break;

                // Snapshot which "schedule generation" this cycle is waiting/firing for. Any
                // Change() call bumps this, whether it happens while we're waiting below or
                // while the callback (invoked with the lock released, further down) is running.
                std::uint64_t genAtStart = s->generation;
                intcs due = s->dueTime;
                if (due > 0) {
                    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(due);
                    // wait_until (not sleep_for) so a concurrent Change() can interrupt this
                    // wait immediately via the generation bump, instead of only taking effect
                    // after this now-stale deadline naturally elapses.
                    s->cv.wait_until(lock, deadline, [&] { return !s->running || s->generation != genAtStart; });
                }
                if (!s->running) break;
                // Change() ran during the wait above (including a pause via dueTime=-1) --
                // restart the outer loop to pick up the fresh schedule from scratch rather than
                // firing on stale timing.
                if (s->generation != genAtStart) continue;

                if (s->callback) {
                    lock.unlock();
                    s->callback(s->arg);
                    lock.lock();
                }
                // Only derive the next dueTime from `period` if no Change() call happened while
                // we didn't hold the lock (i.e. during the callback). If one did, it already set
                // the dueTime/period it wants for the next cycle -- overwriting that here was
                // the original bug (a mid-callback Change(newDueTime, ...) call's newDueTime was
                // silently discarded).
                if (s->generation == genAtStart) {
                    if (s->period <= 0) {
                        // Single-shot: disable further firing until a future Change() re-arms it
                        // (rather than exiting the loop/thread entirely, so a later Change() call
                        // can still bring this timer back to life, matching real .NET).
                        s->dueTime = -1;
                    } else {
                        s->dueTime = s->period; // schedule the next cycle using the period
                    }
                }
            }
        }
    };

} // namespace System::Threading
