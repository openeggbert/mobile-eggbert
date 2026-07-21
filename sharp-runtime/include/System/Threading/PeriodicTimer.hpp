// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/IDisposable.hpp"
#include "System/Threading/Timeout.hpp"
#include "System/TimeSpan.hpp"

namespace System::Threading {

    /** A timer that fires at a fixed interval; WaitForNextTick blocks until the next tick or disposal. */
    class PeriodicTimer : public System::IDisposable {
        // Matches Timer.cs's Timer.MaxSupportedTimeout (0xfffffffe == uint.MaxValue - 1).
        static constexpr long long MaxSupportedTimeoutMs = 0xFFFFFFFEULL;

        bool infinite_;
        std::chrono::milliseconds period_;
        std::chrono::steady_clock::time_point next_;
        std::mutex mtx_;
        std::condition_variable cv_;
        std::atomic<bool> disposed_{false};

    public:
        /**
         * @brief Constructs a PeriodicTimer with the specified period.
         * @throws System::ArgumentOutOfRangeException unless @p period is
         * Timeout::InfiniteTimeSpan or represents a whole number of milliseconds in
         * [1, uint.MaxValue - 1].
         * @note Verified against PeriodicTimer.cs's TryGetMilliseconds: real .NET validates the
         * period exactly this way. This port previously performed no validation at all --
         * TimeSpan::Zero or a negative (non-Infinite) period silently constructed a timer whose
         * next_ was already <= now(), so WaitForNextTick() would busy-loop at 100% CPU
         * (sleep_until a point already in the past returns immediately) instead of throwing a
         * clear error.
         */
        explicit PeriodicTimer(const System::TimeSpan& period)
            : infinite_(period.getTicksProperty() == System::Threading::Timeout::InfiniteTimeSpan) {
            double ms = period.getTotalMillisecondsProperty();
            if (!infinite_ && !(ms >= 1.0 && ms <= static_cast<double>(MaxSupportedTimeoutMs)))
                throw System::ArgumentOutOfRangeException("period");
            period_ = std::chrono::milliseconds(static_cast<long long>(ms));
            next_ = std::chrono::steady_clock::now() + period_;
        }

        /**
         * @brief Blocks until the next scheduled tick; returns false if the timer has been disposed.
         * @note A timer constructed with Timeout::InfiniteTimeSpan never ticks on its own --
         * this blocks until Dispose() is called, matching real .NET (a Timeout.InfiniteTimeSpan
         * PeriodicTimer never schedules a tick, but Dispose() still unblocks a pending wait via
         * State.Signal(stopping: true)).
         */
        bool WaitForNextTick() {
            std::unique_lock<std::mutex> lock(mtx_);
            if (disposed_.load()) return false;
            if (infinite_) {
                cv_.wait(lock, [this] { return disposed_.load(); });
                return false;
            }
            cv_.wait_until(lock, next_, [this] { return disposed_.load(); });
            if (disposed_.load()) return false;
            next_ += period_;
            return true;
        }

        /** Stops the timer so that WaitForNextTick returns false, unblocking any in-progress wait. */
        void Dispose() override {
            {
                std::lock_guard<std::mutex> lock(mtx_);
                disposed_.store(true);
            }
            cv_.notify_all();
        }
    };

} // namespace System::Threading
