// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <condition_variable>
#include <mutex>
#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Threading/WaitHandle.hpp"
#include "System/Threading/SemaphoreFullException.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /** A counting semaphore that limits the number of threads accessing a resource concurrently. */
    class Semaphore : public WaitHandle {
        intcs count_;
        intcs maxCount_;
        std::mutex mtx_;
        std::condition_variable cv_;

        // Verified against Semaphore.cs's ValidateArguments: real .NET throws
        // ArgumentOutOfRangeException for each count individually out of range
        // (initialCount < 0, maximumCount < 1), but a plain ArgumentException
        // (Argument_SemaphoreInitialMaximum) when both are individually valid yet
        // initialCount > maximumCount -- this previously used ArgumentOutOfRangeException for
        // that case too.
        static void ValidateCounts(intcs initialCount, intcs maximumCount) {
            if (initialCount < 0)
                throw System::ArgumentOutOfRangeException("initialCount");
            if (maximumCount < 1)
                throw System::ArgumentOutOfRangeException("maximumCount");
            if (initialCount > maximumCount)
                throw System::ArgumentException(
                    "The initial count for the semaphore must be greater than or equal to zero and less than the maximum count.");
        }

    public:
        /** Constructs a Semaphore with the given initial and maximum counts. */
        Semaphore(intcs initialCount, intcs maximumCount)
            : count_(initialCount), maxCount_(maximumCount) {
            ValidateCounts(initialCount, maximumCount);
        }
        /** Constructs a named Semaphore (naming is not truly cross-process; the name is ignored). */
        Semaphore(intcs initialCount, intcs maximumCount, const std::string& /*name*/)
            : count_(initialCount), maxCount_(maximumCount) {
            ValidateCounts(initialCount, maximumCount);
        }

        /** Blocks until the semaphore count is greater than zero, then decrements it. */
        bool WaitOne() override {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this]{ return count_ > 0; });
            --count_;
            return true;
        }

        /**
         * @brief Blocks until the count is greater than zero or the timeout elapses; returns true on success.
         * @throws System::ArgumentOutOfRangeException if @p milliseconds is less than -1.
         */
        bool WaitOne(intcs milliseconds) override {
            ValidateTimeout(milliseconds);
            std::unique_lock<std::mutex> lock(mtx_);
            // -1 (Timeout.Infinite) waits indefinitely; std::chrono's wait_for treats a
            // negative duration as already-expired, so it must be special-cased.
            bool ok;
            if (milliseconds == -1) {
                cv_.wait(lock, [this]{ return count_ > 0; });
                ok = true;
            } else {
                ok = cv_.wait_for(lock, std::chrono::milliseconds(milliseconds),
                    [this]{ return count_ > 0; });
            }
            if (ok) --count_;
            return ok;
        }

        /** Releases the semaphore once and returns the count before the release. */
        intcs Release() { return Release(1); }

        /** Releases the semaphore releaseCount times and returns the count before the release. */
        intcs Release(intcs releaseCount) {
            if (releaseCount < 1)
                throw System::ArgumentOutOfRangeException("releaseCount");
            std::unique_lock<std::mutex> lock(mtx_);
            if (count_ + releaseCount > maxCount_)
                throw SemaphoreFullException();
            intcs prev = count_;
            count_ += releaseCount;
            cv_.notify_all();
            return prev;
        }
    };

} // namespace System::Threading
