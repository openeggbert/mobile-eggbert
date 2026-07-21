// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <condition_variable>
#include <limits>
#include <mutex>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/Threading/WaitHandle.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /** Represents a synchronisation primitive that is signalled when its count reaches zero. */
    class CountdownEvent {
        intcs initialCount_;
        intcs currentCount_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        bool disposed_ = false;

        void ThrowIfDisposed() const {
            if (disposed_) throw System::ObjectDisposedException("CountdownEvent");
        }

    public:
        /**
         * @brief Constructs a CountdownEvent with the specified initial count.
         * @throws System::ArgumentOutOfRangeException if @p initialCount is negative.
         */
        explicit CountdownEvent(intcs initialCount) : initialCount_(initialCount), currentCount_(initialCount) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(initialCount, "initialCount");
        }

        /** Returns the current count. */
        [[nodiscard]] intcs getCurrentCountProperty()   const { std::unique_lock lock(mutex_); return currentCount_; }
        /** Returns the initial count supplied at construction. */
        [[nodiscard]] intcs getInitialCountProperty()   const { return initialCount_; }
        /** Returns true when the current count has reached zero. */
        [[nodiscard]] bool getIsSetProperty()          const { std::unique_lock lock(mutex_); return currentCount_ == 0; }

        /**
         * @brief Decrements the current count; returns true when the count reaches zero.
         * @throws System::ArgumentOutOfRangeException if @p signalCount is not positive.
         * @throws System::InvalidOperationException if the count is already zero.
         * @throws System::ObjectDisposedException if this instance has been disposed.
         */
        bool Signal(intcs signalCount = 1) {
            System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(signalCount, "signalCount");
            ThrowIfDisposed();
            std::unique_lock lock(mutex_);
            if (currentCount_ == 0 || currentCount_ < signalCount)
                throw System::InvalidOperationException("Invalid attempt made to decrement the event's count below zero.");
            currentCount_ -= signalCount;
            bool set = (currentCount_ == 0);
            if (set) cv_.notify_all();
            return set;
        }

        /**
         * @brief Increments the current count by signalCount.
         * @throws System::ArgumentOutOfRangeException if @p signalCount is not positive.
         * @throws System::InvalidOperationException if the count is already zero, or if
         * incrementing would overflow intcs::max().
         * @throws System::ObjectDisposedException if this instance has been disposed.
         */
        void AddCount(intcs signalCount = 1) {
            System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(signalCount, "signalCount");
            ThrowIfDisposed();
            std::unique_lock lock(mutex_);
            if (currentCount_ == 0)
                throw System::InvalidOperationException("The event is already signaled and cannot be incremented.");
            // Verified against CountdownEvent.cs's TryAddCount: real .NET checks
            // observedCount > (int.MaxValue - signalCount) before adding, to avoid signed
            // overflow (UB in C++, and .NET itself treats it as an error rather than wrapping).
            if (currentCount_ > std::numeric_limits<intcs>::max() - signalCount)
                throw System::InvalidOperationException("The increment operation would cause the CurrentCount to overflow.");
            currentCount_ += signalCount;
        }

        /**
         * @brief Resets both CurrentCount and InitialCount to InitialCount's current value.
         * @throws System::ObjectDisposedException if this instance has been disposed.
         */
        void Reset() { Reset(initialCount_); }

        /**
         * @brief Resets both CurrentCount and InitialCount to @p count.
         *
         * Verified against CountdownEvent.cs's Reset(int): unlike this method's previous
         * signature here (a single `Reset(intcs count = -1)` using -1 as a sentinel for "use
         * InitialCount"), real .NET has two separate overloads and Reset(int) rejects *any*
         * negative count -- including -1 -- with ArgumentOutOfRangeException. The sentinel
         * previously made an explicit `Reset(-1)` call silently reset to InitialCount instead
         * of throwing as real .NET does.
         *
         * @throws System::ArgumentOutOfRangeException if @p count is negative.
         * @throws System::ObjectDisposedException if this instance has been disposed.
         */
        void Reset(intcs count) {
            ThrowIfDisposed();
            System::ArgumentOutOfRangeException::ThrowIfNegative(count, "count");
            std::unique_lock lock(mutex_);
            initialCount_ = count;
            currentCount_ = count;
        }

        /**
         * @brief Blocks until the current count reaches zero.
         * @throws System::ObjectDisposedException if this instance has been disposed.
         */
        void Wait() {
            ThrowIfDisposed();
            std::unique_lock lock(mutex_);
            cv_.wait(lock, [this]{ return currentCount_ == 0; });
        }

        /**
         * @brief Blocks until the count reaches zero or the timeout elapses; returns true on success.
         * @throws System::ArgumentOutOfRangeException if @p millisecondsTimeout is less than -1.
         * @throws System::ObjectDisposedException if this instance has been disposed.
         */
        bool Wait(intcs millisecondsTimeout) {
            WaitHandle::ValidateTimeout(millisecondsTimeout);
            ThrowIfDisposed();
            std::unique_lock lock(mutex_);
            // -1 (Timeout.Infinite) waits indefinitely; std::chrono's wait_for treats a
            // negative duration as already-expired, so it must be special-cased.
            if (millisecondsTimeout == -1) {
                cv_.wait(lock, [this]{ return currentCount_ == 0; });
                return true;
            }
            return cv_.wait_for(lock, std::chrono::milliseconds(millisecondsTimeout),
                                [this]{ return currentCount_ == 0; });
        }

        /** Releases resources used by the CountdownEvent. */
        void Dispose() { disposed_ = true; }
    };

} // namespace System::Threading
