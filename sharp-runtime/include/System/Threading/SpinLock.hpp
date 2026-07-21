// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <chrono>
#include <climits>
#include <thread>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/TimeSpan.hpp"
#include "System/Threading/LockRecursionException.hpp"
#include "System/Threading/SynchronizationLockException.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /**
     * @brief A lightweight mutual-exclusion lock optimised for short wait times.
     *
     * C++ counterpart of .NET System.Threading.SpinLock. This port models the public
     * ownership-tracking contract (IsHeld/IsHeldByCurrentThread/IsThreadOwnerTrackingEnabled,
     * LockRecursionException on same-thread re-entry, SynchronizationLockException on Exit()
     * without ownership) with a simple atomic-owner spin loop rather than reproducing .NET's
     * bit-packed `_owner` field and waiter-count fast paths -- the public behavior matches;
     * the internal spin/backoff strategy is simpler.
     */
    class SpinLock {
        std::atomic<std::thread::id> owner_{};      // used when tracking is enabled; default id() == unowned
        std::atomic<bool> heldAnonymous_{false};     // used when tracking is disabled
        bool trackingEnabled_;

        [[nodiscard]] static bool isUnowned(std::thread::id id) noexcept { return id == std::thread::id(); }

        // Verified against SpinLock.cs's TryEnter(int millisecondsTimeout, ...): -1 means wait
        // indefinitely, any other negative value is invalid, and the wait is bounded by
        // millisecondsTimeout otherwise.
        bool acquire(intcs millisecondsTimeout) {
            using clock = std::chrono::steady_clock;
            const bool hasTimeout = millisecondsTimeout != -1;
            const auto deadline = hasTimeout
                ? clock::now() + std::chrono::milliseconds(millisecondsTimeout)
                : clock::time_point{};

            if (trackingEnabled_) {
                const auto self = std::this_thread::get_id();
                if (owner_.load(std::memory_order_relaxed) == self)
                    throw LockRecursionException("The calling thread already holds the lock.");
                while (true) {
                    std::thread::id expected{};
                    if (owner_.compare_exchange_weak(expected, self, std::memory_order_acquire, std::memory_order_relaxed))
                        return true;
                    if (millisecondsTimeout == 0) return false;
                    if (hasTimeout && clock::now() >= deadline) return false;
                    std::this_thread::yield();
                }
            } else {
                while (true) {
                    bool expected = false;
                    if (heldAnonymous_.compare_exchange_weak(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
                        return true;
                    if (millisecondsTimeout == 0) return false;
                    if (hasTimeout && clock::now() >= deadline) return false;
                    std::this_thread::yield();
                }
            }
        }

        static void ValidateLockTaken(bool lockTaken) {
            if (lockTaken)
                throw System::ArgumentException("The lockTaken argument must be set to false before calling this method.", "lockTaken");
        }

        static void ValidateTimeout(intcs millisecondsTimeout) {
            if (millisecondsTimeout < -1)
                throw System::ArgumentOutOfRangeException("millisecondsTimeout", "The timeout must be a value between -1 and Int32.MaxValue, inclusive.");
        }

    public:
        /**
         * @brief Constructs a SpinLock, optionally tracking which thread owns it.
         *
         * Matches .NET's default: the parameterless constructor is equivalent to
         * `SpinLock(true)` (thread ownership tracking enabled).
         */
        explicit SpinLock(bool enableThreadOwnerTracking = true) : trackingEnabled_(enableThreadOwnerTracking) {}

        /**
         * @brief Acquires the lock, spinning until it is available; sets lockTaken to true on success.
         * @throws System::ArgumentException if lockTaken was not initialized to false.
         * @throws LockRecursionException if thread ownership tracking is enabled and the calling
         *         thread already holds this lock.
         */
        void Enter(bool& lockTaken) {
            ValidateLockTaken(lockTaken);
            lockTaken = acquire(-1);
        }

        /**
         * @brief Attempts to acquire the lock without spinning; sets lockTaken and returns true on success.
         * @throws System::ArgumentException if lockTaken was not initialized to false.
         * @throws LockRecursionException if thread ownership tracking is enabled and the calling
         *         thread already holds this lock.
         */
        bool TryEnter(bool& lockTaken) {
            ValidateLockTaken(lockTaken);
            lockTaken = acquire(0);
            return lockTaken;
        }

        /**
         * @brief Attempts to acquire the lock within the given millisecond timeout.
         * @throws System::ArgumentException if lockTaken was not initialized to false.
         * @throws System::ArgumentOutOfRangeException if millisecondsTimeout is less than -1.
         * @throws LockRecursionException if thread ownership tracking is enabled and the calling
         *         thread already holds this lock.
         */
        bool TryEnter(intcs millisecondsTimeout, bool& lockTaken) {
            ValidateLockTaken(lockTaken);
            ValidateTimeout(millisecondsTimeout);
            lockTaken = acquire(millisecondsTimeout);
            return lockTaken;
        }

        /**
         * @brief Attempts to acquire the lock within the given timeout.
         * @throws System::ArgumentException if lockTaken was not initialized to false.
         * @throws System::ArgumentOutOfRangeException if timeout represents a negative duration
         *         other than -1ms, or more than Int32.MaxValue milliseconds.
         * @throws LockRecursionException if thread ownership tracking is enabled and the calling
         *         thread already holds this lock.
         */
        bool TryEnter(const System::TimeSpan& timeout, bool& lockTaken) {
            double totalMs = timeout.getTotalMillisecondsProperty();
            if (totalMs < -1.0 || totalMs > static_cast<double>(INT_MAX))
                throw System::ArgumentOutOfRangeException("timeout", "The timeout must be a value between -1 and Int32.MaxValue, inclusive.");
            return TryEnter(static_cast<intcs>(totalMs), lockTaken);
        }

        /**
         * @brief Releases the lock.
         * @throws SynchronizationLockException if thread ownership tracking is enabled and the
         *         calling thread does not own this lock.
         */
        void Exit(bool /*useMemoryBarrier*/ = true) {
            if (trackingEnabled_) {
                if (owner_.load(std::memory_order_relaxed) != std::this_thread::get_id())
                    throw SynchronizationLockException();
                owner_.store(std::thread::id(), std::memory_order_release);
            } else {
                heldAnonymous_.store(false, std::memory_order_release);
            }
        }

        /** @return true if the lock is currently held by any thread. */
        [[nodiscard]] bool getIsHeldProperty() const noexcept {
            return trackingEnabled_ ? !isUnowned(owner_.load(std::memory_order_relaxed))
                                     : heldAnonymous_.load(std::memory_order_relaxed);
        }

        /**
         * @brief Returns true if the lock is held by the calling thread.
         * @throws System::InvalidOperationException if thread ownership tracking is disabled.
         */
        [[nodiscard]] bool getIsHeldByCurrentThreadProperty() const {
            if (!trackingEnabled_)
                throw System::InvalidOperationException("Thread tracking is disabled.");
            return owner_.load(std::memory_order_relaxed) == std::this_thread::get_id();
        }

        /** @return true if this instance tracks which thread owns the lock. */
        [[nodiscard]] bool getIsThreadOwnerTrackingEnabledProperty() const noexcept { return trackingEnabled_; }
    };

} // namespace System::Threading
