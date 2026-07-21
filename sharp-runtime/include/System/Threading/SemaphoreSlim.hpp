// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/Threading/SemaphoreFullException.hpp"
#include "System/Threading/WaitHandle.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /**
     * @brief A lightweight alternative to Semaphore that limits the number of threads that can access a resource.
     *
     * Wraps std::condition_variable. Partial C++ counterpart of .NET System.Threading.SemaphoreSlim.
     *
     * @note Status: Implemented
     */
    class SemaphoreSlim {
        std::mutex              mutex_;
        std::condition_variable cv_;
        intcs                   count_;
        intcs                   maxCount_;
        // Verified against SemaphoreSlim.cs's CheckDispose()/ObjectDisposedException.ThrowIf:
        // real .NET's Wait/Release all reject calls after Dispose(). This port's sibling
        // primitives (Barrier, CountdownEvent, ManualResetEventSlim, ReaderWriterLockSlim) all
        // already have this guard; SemaphoreSlim was previously missed, with Dispose() a true
        // no-op and no disposal check anywhere.
        bool                    disposed_ = false;

        void ThrowIfDisposed() const {
            if (disposed_) throw System::ObjectDisposedException("SemaphoreSlim");
        }

    public:
        /** Constructs a SemaphoreSlim with the given initial and maximum counts. */
        explicit SemaphoreSlim(intcs initialCount, intcs maxCount = 0x7fffffff)
            : count_(initialCount), maxCount_(maxCount) {
            if (maxCount < 1)
                throw System::ArgumentOutOfRangeException("maxCount");
            if (initialCount < 0 || initialCount > maxCount)
                throw System::ArgumentOutOfRangeException("initialCount");
        }

        /** Returns the current count of the semaphore. */
        [[nodiscard]] intcs getCurrentCountProperty() const { return count_; }

        /**
         * @brief Blocks until the semaphore can be entered.
         * @throws System::ObjectDisposedException if this instance has been disposed.
         */
        void Wait() {
            ThrowIfDisposed();
            std::unique_lock<std::mutex> lk(mutex_);
            cv_.wait(lk, [this]{ return count_ > 0; });
            --count_;
        }

        /**
         * @brief Tries to enter the semaphore within a timeout. Returns true on success.
         * @throws System::ArgumentOutOfRangeException if @p milliseconds is less than -1.
         * @throws System::ObjectDisposedException if this instance has been disposed.
         */
        bool Wait(intcs milliseconds) {
            WaitHandle::ValidateTimeout(milliseconds);
            ThrowIfDisposed();
            std::unique_lock<std::mutex> lk(mutex_);
            // -1 (Timeout.Infinite) waits indefinitely; std::chrono's wait_for treats a
            // negative duration as already-expired, so it must be special-cased.
            bool ok;
            if (milliseconds == -1) {
                cv_.wait(lk, [this]{ return count_ > 0; });
                ok = true;
            } else {
                ok = cv_.wait_for(lk, std::chrono::milliseconds(milliseconds), [this]{ return count_ > 0; });
            }
            if (ok) --count_;
            return ok;
        }

        /**
         * @brief Releases the semaphore once.
         * @throws System::ObjectDisposedException if this instance has been disposed.
         */
        intcs Release() { return Release(1); }

        /**
         * @brief Releases the semaphore a specified number of times. Returns previous count.
         *
         * Verified against SemaphoreSlim.cs's Release(int): the disposal check runs *before*
         * the releaseCount validation (the reverse order from Wait(int), which validates its
         * timeout argument before checking disposal -- matching each method's own .NET source).
         *
         * @throws System::ObjectDisposedException if this instance has been disposed.
         * @throws System::ArgumentOutOfRangeException if @p releaseCount is less than 1.
         */
        intcs Release(intcs releaseCount) {
            ThrowIfDisposed();
            if (releaseCount < 1)
                throw System::ArgumentOutOfRangeException("releaseCount");
            std::lock_guard<std::mutex> lk(mutex_);
            if (count_ + releaseCount > maxCount_)
                throw System::Threading::SemaphoreFullException();
            intcs prev = count_;
            count_ += releaseCount;
            cv_.notify_all();
            return prev;
        }

        /** Releases resources used by the SemaphoreSlim. */
        void Dispose() { disposed_ = true; }
    };

} // namespace System::Threading
