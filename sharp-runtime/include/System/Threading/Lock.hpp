// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <chrono>
#include <limits>
#include <mutex>
#include <thread>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/TimeSpan.hpp"
#include "System/Threading/SynchronizationLockException.hpp"
#include "System/Threading/Timeout.hpp"
#include "System/Threading/WaitHandle.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /**
     * @brief A mutual-exclusion lock supporting bounded same-thread reentrancy (.NET 9 Lock type).
     *
     * Wraps std::recursive_timed_mutex plus owner-thread tracking, so Exit() can validate
     * that the calling thread actually holds the lock before releasing it (matching .NET's
     * ThrowHelper.ThrowSynchronizationLockException_LockExit()).
     */
    class Lock {
        std::recursive_timed_mutex mtx_;
        std::atomic<std::thread::id> owner_{};
        std::atomic<int> depth_{0};

        void onAcquired() {
            if (depth_.fetch_add(1, std::memory_order_relaxed) == 0)
                owner_.store(std::this_thread::get_id(), std::memory_order_relaxed);
        }
        void onReleasing() {
            if (depth_.fetch_sub(1, std::memory_order_relaxed) == 1)
                owner_.store(std::thread::id(), std::memory_order_relaxed);
        }

    public:
        /** Acquires the lock, blocking until it is available. */
        void Enter() { mtx_.lock(); onAcquired(); }

        /**
         * @brief Releases the lock.
         * @throws System::Threading::SynchronizationLockException if the calling thread does not hold the lock.
         */
        void Exit() {
            if (!getIsHeldByCurrentThreadProperty())
                throw System::Threading::SynchronizationLockException();
            onReleasing();
            mtx_.unlock();
        }

        /** Attempts to acquire the lock without blocking; returns true if successful. */
        bool TryEnter() {
            bool ok = mtx_.try_lock();
            if (ok) onAcquired();
            return ok;
        }

        /**
         * @brief Attempts to acquire the lock within the given millisecond timeout; returns true if successful.
         *
         * Verified against Lock.cs's doc comment: -1 (Timeout.Infinite) waits indefinitely.
         * std::chrono's try_lock_for treats a negative duration as already-expired, so it
         * must be special-cased rather than passed straight through.
         *
         * @throws System::ArgumentOutOfRangeException if @p millisecondsTimeout is less than -1.
         * Verified against Lock.cs's TryEnter(int): ArgumentOutOfRangeException.ThrowIfLessThan
         * (millisecondsTimeout, -1) -- this port's WaitHandle::ValidateTimeout() implements the
         * identical check, already used by every sibling wait primitive in this namespace; this
         * overload had simply never included WaitHandle.hpp to call it.
         */
        bool TryEnter(intcs millisecondsTimeout) {
            WaitHandle::ValidateTimeout(millisecondsTimeout);
            bool ok;
            if (millisecondsTimeout == -1) {
                mtx_.lock();
                ok = true;
            } else {
                ok = mtx_.try_lock_for(std::chrono::milliseconds(millisecondsTimeout));
            }
            if (ok) onAcquired();
            return ok;
        }

        /**
         * @brief Attempts to acquire the lock within the given timeout; returns true if successful.
         *
         * Verified against Lock.cs's doc comment: Timeout.InfiniteTimeSpan (-1ms) waits
         * indefinitely, the same as the intcs overload above. This previously passed
         * Timeout::InfiniteTimeSpan straight through as a negative std::chrono duration, which
         * try_lock_for treats like a non-blocking try_lock() -- returning almost instantly
         * instead of blocking indefinitely -- the one call site the session's -1/Infinite fix
         * pattern (already applied to the intcs overload) had missed.
         *
         * @throws System::ArgumentOutOfRangeException if @p timeout, converted to milliseconds,
         * is less than -1 or exceeds intcs::max(). Verified against Lock.cs's TryEnter(TimeSpan),
         * which converts via WaitHandle.ToTimeoutMilliseconds(timeout) -- this port had no
         * equivalent conversion-time validation at all.
         */
        bool TryEnter(const System::TimeSpan& timeout) {
            bool ok;
            if (timeout.getTicksProperty() == System::Threading::Timeout::InfiniteTimeSpan) {
                mtx_.lock();
                ok = true;
            } else {
                long long ms = static_cast<long long>(timeout.getTotalMillisecondsProperty());
                if (ms < -1 || ms > std::numeric_limits<intcs>::max())
                    throw System::ArgumentOutOfRangeException("timeout");
                ok = mtx_.try_lock_for(std::chrono::microseconds(timeout.getTicksProperty() / 10));
            }
            if (ok) onAcquired();
            return ok;
        }

        /** @return true if the calling thread currently holds this lock. */
        [[nodiscard]] bool getIsHeldByCurrentThreadProperty() const {
            return owner_.load(std::memory_order_relaxed) == std::this_thread::get_id();
        }

        /** RAII scope helper that holds the lock for its lifetime. */
        class Scope {
            Lock& lock_;
        public:
            /** Acquires the lock on construction. */
            explicit Scope(Lock& lk) : lock_(lk) { lock_.Enter(); }
            /** Releases the lock on destruction. */
            ~Scope() { lock_.Exit(); }
            /** Copying is not allowed. */
            Scope(const Scope&) = delete;
            /** Copy assignment is not allowed. */
            Scope& operator=(const Scope&) = delete;
        };

        /** Acquires the lock and returns a Scope RAII guard that releases it on destruction. */
        Scope EnterScope() { return Scope(*this); }
    };

} // namespace System::Threading
