// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Threading/ApartmentState.hpp"
#include "System/Threading/ThreadPriority.hpp"
#include "System/Threading/ThreadState.hpp"
#include "System/Threading/ThreadStateException.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /**
     * @brief Creates and controls a thread, sets its priority, and gets its status.
     *
     * C++ counterpart of .NET System.Threading.Thread.
     * Wraps std::thread. The thread does NOT start at construction — call Start()
     * exactly once. Calling Start() a second time throws System::Threading::ThreadStateException.
     *
     * Obsolete .NET APIs (Abort, Suspend, Resume, VolatileRead/Write) are omitted.
     * Apartment-state and compressed-stack APIs are stubs.
     */
    class Thread {
        // Managed thread IDs start at 2, matching .NET's convention that the main/first
        // thread is ID 1 (see currentThreadState_'s default below) — the first Thread object
        // constructed by user code must not collide with that.
        inline static std::atomic<intcs> nextManagedId_{2};

        // Verified: the spawned-thread lambda previously captured `this` by raw pointer and
        // wrote into `finished_`/read `currentThread_->...` after ~Thread() (which detach()es,
        // not join()s -- a deliberate design letting the OS thread outlive the wrapper). A
        // Thread object destroyed before its OS thread finishes left the lambda touching freed
        // memory: a genuine use-after-free, the same bug class fixed elsewhere this session
        // (Channel::ReadAsync/WriteAsync's raw-`this` capture, RegisteredWaitHandle::
        // Unregister()'s detach-without-join). Fixed by moving everything the spawned thread
        // touches into a heap-allocated RunState the lambda captures by shared_ptr, so it stays
        // alive for the OS thread's full lifetime regardless of the owning Thread object's.
        struct RunState {
            std::atomic<bool> finished{false};
            std::atomic<bool> isBackground{false};
            intcs             managedThreadId = 0;
        };

        // Tracks which running thread's RunState (if any) belongs to the calling OS thread, so
        // CurrentThread() can report the correct ManagedThreadId/IsBackground instead of an
        // unrelated hash of the OS thread handle. Threads not started via this class (the
        // main thread, or any other externally-created thread) see nullptr and report the
        // .NET-convention main-thread ID of 1. Holding the shared_ptr here (rather than a raw
        // Thread*) means CurrentThreadProxy never dereferences the (possibly already-destroyed)
        // Thread object itself.
        inline static thread_local std::shared_ptr<RunState> currentThreadState_;

        std::shared_ptr<RunState> state_ = std::make_shared<RunState>();
        std::function<void()>  fn_;
        std::thread            thread_;
        std::string            name_;
        bool                   isThreadPoolThread_ = false;
        ThreadPriority         priority_          = ThreadPriority::Normal;
        std::atomic<bool>      started_{false};

    public:
        /**
         * @brief Constructs a Thread with the given parameterless start function.
         * @param start Function to execute on the new thread.
         */
        explicit Thread(std::function<void()> start)
            : fn_(std::move(start))
        {
            state_->managedThreadId = nextManagedId_.fetch_add(1);
        }

        ~Thread() {
            if (thread_.joinable()) thread_.detach();
        }

        Thread(const Thread&)            = delete;
        Thread& operator=(const Thread&) = delete;

        // -----------------------------------------------------------------------
        // Control
        // -----------------------------------------------------------------------

        /**
         * @brief Starts the thread.
         * @throws System::Threading::ThreadStateException if Start() has already been called.
         */
        void Start() {
            if (started_.exchange(true))
                throw System::Threading::ThreadStateException("Thread is running or terminated; it cannot restart.");
            thread_ = std::thread([state = state_, fn = std::move(fn_)]() mutable {
                currentThreadState_ = state;
                fn();
                state->finished.store(true);
            });
        }

        /**
         * @brief Starts the thread, passing @p parameter to a ParameterizedThreadStart function.
         * @param parameter Argument forwarded to the thread function (stored as void*).
         * @throws System::Threading::ThreadStateException if Start() has already been called.
         */
        void Start(void* parameter) {
            if (started_.exchange(true))
                throw System::Threading::ThreadStateException("Thread is running or terminated; it cannot restart.");
            thread_ = std::thread([state = state_, fn = std::move(fn_), parameter]() mutable {
                currentThreadState_ = state;
                (void)parameter;
                fn();
                state->finished.store(true);
            });
        }

        /**
         * @brief Blocks the calling thread until this thread terminates.
         * @throws System::Threading::ThreadStateException if this thread has not been started.
         */
        void Join() {
            if (!started_.load())
                throw System::Threading::ThreadStateException("Thread has not been started.");
            if (thread_.joinable()) thread_.join();
        }

        /**
         * @brief Blocks the calling thread until this thread terminates or
         * @p millisecondsTimeout elapses.
         * @param millisecondsTimeout Maximum wait time in milliseconds, or -1 (Timeout.Infinite)
         *        to block until the thread terminates.
         * @return true if the thread terminated; false if the timeout elapsed.
         * @throws System::ArgumentOutOfRangeException if @p millisecondsTimeout is less than -1.
         * @throws System::Threading::ThreadStateException if this thread has not been started.
         */
        bool Join(intcs millisecondsTimeout) {
            if (millisecondsTimeout < -1)
                throw System::ArgumentOutOfRangeException("millisecondsTimeout",
                    "Number must be either non-negative and less than or equal to Int32.MaxValue or -1.");
            if (!started_.load())
                throw System::Threading::ThreadStateException("Thread has not been started.");
            if (!thread_.joinable()) return true;
            if (millisecondsTimeout == -1) {
                thread_.join();
                return true;
            }
            auto deadline = std::chrono::steady_clock::now()
                          + std::chrono::milliseconds(millisecondsTimeout);
            while (!state_->finished.load()) {
                if (std::chrono::steady_clock::now() >= deadline) return false;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            if (thread_.joinable()) thread_.join();
            return true;
        }

        /** @brief No-op stub — Interrupt() is not supported in this port. */
        void Interrupt() noexcept {}

        // -----------------------------------------------------------------------
        // Properties
        // -----------------------------------------------------------------------

        /**
         * @brief Returns true while the OS thread is live (started and not yet joined).
         * @return true if the thread is running.
         */
        [[nodiscard]] bool getIsAliveProperty() const { return thread_.joinable() && !state_->finished.load(); }

        /**
         * @brief Returns the unique managed thread ID assigned at construction.
         * @return Managed thread ID.
         */
        [[nodiscard]] intcs getManagedThreadIdProperty() const { return state_->managedThreadId; }

        /** @brief Returns true if this is a background thread. */
        [[nodiscard]] bool getIsBackgroundProperty() const { return state_->isBackground.load(); }
        /** @brief Sets the background status of this thread. */
        void setIsBackgroundProperty(bool v) { state_->isBackground.store(v); }

        /** @brief Returns true if this thread was created by the thread pool. Always false for user-created threads. */
        [[nodiscard]] bool getIsThreadPoolThreadProperty() const { return isThreadPoolThread_; }

        /** @brief Returns the name of this thread. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        /** @brief Sets the name of this thread. */
        void setNameProperty(const std::string& name) { name_ = name; }

        /** @brief Returns the scheduling priority of this thread. */
        [[nodiscard]] ThreadPriority getPriorityProperty() const { return priority_; }
        /** @brief Sets the scheduling priority (stored but not applied to the OS thread). */
        void setPriorityProperty(ThreadPriority p) { priority_ = p; }

        /**
         * @brief Returns the current execution state of this thread.
         * @return ThreadState reflecting started/running/stopped flags.
         */
        [[nodiscard]] ThreadState getThreadStateProperty() const {
            if (!started_.load())          return ThreadState::Unstarted;
            if (state_->finished.load())   return ThreadState::Stopped;
            if (!thread_.joinable())       return ThreadState::Stopped;
            return state_->isBackground.load() ? (ThreadState::Running | ThreadState::Background)
                                                : ThreadState::Running;
        }

        // -----------------------------------------------------------------------
        // Apartment state (stub — COM apartments are not meaningful in C++)
        // -----------------------------------------------------------------------

        /** @brief Returns ApartmentState::Unknown (COM apartments not supported in C++). */
        [[nodiscard]] ApartmentState GetApartmentState() const noexcept {
            return ApartmentState::Unknown;
        }
        /** @brief No-op stub — COM apartment state cannot be set. */
        void SetApartmentState(ApartmentState) noexcept {}
        /** @brief Always returns false — COM apartment state cannot be set. */
        [[nodiscard]] bool TrySetApartmentState(ApartmentState) noexcept { return false; }

        // -----------------------------------------------------------------------
        // Static helpers
        // -----------------------------------------------------------------------

        /**
         * @brief Suspends the current thread for @p milliseconds.
         * @param milliseconds Duration in milliseconds (0 yields the scheduler), or -1
         *        (Timeout.Infinite) to sleep until the process ends (Interrupt() is a
         *        no-op stub in this port, so an infinite sleep cannot be woken early).
         * @throws System::ArgumentOutOfRangeException if @p milliseconds is less than -1.
         */
        static void Sleep(intcs milliseconds) {
            if (milliseconds < -1)
                throw System::ArgumentOutOfRangeException("millisecondsTimeout",
                    "Number must be either non-negative and less than or equal to Int32.MaxValue or -1.");
            if (milliseconds == -1) {
                std::this_thread::sleep_until(std::chrono::steady_clock::time_point::max());
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        }

        /**
         * @brief Causes the operating system to change the state of the current
         * instance to WaitSleepJoin for the specified duration.
         * @param timeout Time span specifying the sleep duration.
         */
        static void Sleep(std::chrono::milliseconds timeout) {
            std::this_thread::sleep_for(timeout);
        }

        /**
         * @brief Spins for @p iterations tight-loop iterations.
         * @param iterations Number of spin iterations.
         */
        static void SpinWait(intcs iterations) {
            for (intcs i = 0; i < iterations; ++i)
                std::atomic_signal_fence(std::memory_order_seq_cst);
        }

        /** @brief Issues a full memory fence (sequentially consistent). */
        static void MemoryBarrier() {
            std::atomic_thread_fence(std::memory_order_seq_cst);
        }

        /**
         * @brief Causes the current thread to yield execution to another thread.
         * @return true if the operating system switched to another thread.
         */
        static bool Yield() {
            std::this_thread::yield();
            return true;
        }

        /**
         * @brief Returns the ID of the processor on which the current thread is running.
         * @return Processor ID (best-effort; may be stale immediately).
         */
        [[nodiscard]] static intcs GetCurrentProcessorId();

        // -----------------------------------------------------------------------
        // CurrentThread proxy
        // -----------------------------------------------------------------------

        /**
         * @brief Lightweight proxy representing the calling thread.
         *
         * Returned by Thread::CurrentThread(). Provides read-only access to
         * properties of the calling thread without owning its lifetime.
         */
        struct CurrentThreadProxy {
            /**
             * @brief Returns the managed thread ID of the calling thread.
             *
             * Resolves to the same ManagedThreadId the owning Thread object reports, when the
             * calling thread was started via Thread::Start(); otherwise (the main thread, or
             * any thread not created through this class) returns 1, matching .NET's
             * main-thread convention.
             */
            [[nodiscard]] intcs getManagedThreadIdProperty() const {
                return currentThreadState_ ? currentThreadState_->managedThreadId : 1;
            }
            /** @brief Returns whether the calling thread's owning Thread object is marked background. */
            [[nodiscard]] bool getIsBackgroundProperty() const {
                return currentThreadState_ && currentThreadState_->isBackground.load();
            }
            /** @brief Suspends the calling thread for @p ms milliseconds. */
            static void Sleep(intcs ms) {
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            }
        };

        /**
         * @brief Returns a proxy for the calling thread.
         * @return CurrentThreadProxy for the thread calling this method.
         */
        static CurrentThreadProxy CurrentThread() { return CurrentThreadProxy{}; }
    };

} // namespace System::Threading
