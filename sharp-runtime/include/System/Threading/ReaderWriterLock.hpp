// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ApplicationException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/TimeSpan.hpp"
#include "System/Threading/LockCookie.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /**
     * @brief A reader/writer lock supporting nested reader and writer acquisitions on the same thread.
     *
     * C++ counterpart of .NET System.Threading.ReaderWriterLock — the legacy (pre-Slim) reader/writer
     * lock. Simplified relative to .NET's lock-free bit-packed state machine: this port uses a
     * std::shared_timed_mutex plus a thread-local recursion-level map keyed on `this`, which is
     * sufficient to match the public contract (nested Acquire/Release, upgrade/downgrade, and
     * ReleaseLock/RestoreLock across a full lock hand-off) without reproducing .NET's exact
     * internal spin/wait-node machinery. New code should prefer ReaderWriterLockSlim.
     */
    class ReaderWriterLock {
        mutable std::shared_timed_mutex mtx_;
        std::atomic<intcs> writerThreadId_{-1};
        intcs writerLevel_ = 0;
        intcs writerSeqNum_ = 1;

        static intcs CurrentThreadId() {
            return static_cast<intcs>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
        }

        static std::unordered_map<const ReaderWriterLock*, intcs>& ReaderLevels() {
            thread_local std::unordered_map<const ReaderWriterLock*, intcs> levels;
            return levels;
        }

        intcs& MyReaderLevel() const { return ReaderLevels()[this]; }

    public:
        /** Constructs a ReaderWriterLock. */
        ReaderWriterLock() = default;

        /** Returns whether the calling thread holds a reader lock. */
        [[nodiscard]] bool getIsReaderLockHeldProperty() const {
            auto it = ReaderLevels().find(this);
            return it != ReaderLevels().end() && it->second > 0;
        }
        /** Returns whether the calling thread holds the writer lock. */
        [[nodiscard]] bool getIsWriterLockHeldProperty() const { return writerThreadId_.load() == CurrentThreadId(); }
        /** Returns the sequence number incremented each time the writer lock is released. */
        [[nodiscard]] intcs getWriterSeqNumProperty() const { return writerSeqNum_; }

        /** Returns true if any thread has acquired the writer lock since @p seqNum was captured. */
        [[nodiscard]] bool AnyWritersSince(intcs seqNum) const {
            if (writerThreadId_.load() == CurrentThreadId()) ++seqNum;
            return static_cast<SharpRuntime::uintcs>(writerSeqNum_) > static_cast<SharpRuntime::uintcs>(seqNum);
        }

        /**
         * @brief Acquires a reader lock, blocking up to millisecondsTimeout milliseconds (-1 = infinite).
         * @throws System::ApplicationException if the timeout elapses before the lock is acquired.
         */
        void AcquireReaderLock(intcs millisecondsTimeout) {
            if (millisecondsTimeout < -1)
                throw System::ArgumentOutOfRangeException("millisecondsTimeout");
            intcs& level = MyReaderLevel();
            if (level > 0) { ++level; return; }
            if (writerThreadId_.load() == CurrentThreadId()) { AcquireWriterLock(millisecondsTimeout); return; }
            if (millisecondsTimeout < 0) mtx_.lock_shared();
            // Verified against ReaderWriterLock.cs's GetTimeoutException(): real .NET throws on
            // timeout instead of returning as if the lock had been acquired. This port previously
            // just returned, leaving the caller to proceed without holding the lock it asked for.
            else if (!mtx_.try_lock_shared_for(std::chrono::milliseconds(millisecondsTimeout)))
                throw System::ApplicationException("The operation has timed out.");
            level = 1;
        }
        /** Acquires a reader lock, blocking up to @p timeout. */
        void AcquireReaderLock(const System::TimeSpan& timeout) {
            AcquireReaderLock(static_cast<intcs>(timeout.getTotalMillisecondsProperty()));
        }

        /**
         * @brief Releases one level of the reader lock acquired by the calling thread.
         * @throws System::ApplicationException if the calling thread doesn't hold the lock.
         * @note Verified against ReaderWriterLock.cs's GetNotOwnerException(): real .NET's
         * private ReaderWriterLockApplicationException derives from ApplicationException, not
         * SynchronizationLockException -- the same exception family as the already-correct
         * timeout path in this class (AcquireReaderLock/AcquireWriterLock).
         */
        void ReleaseReaderLock() {
            intcs& level = MyReaderLevel();
            if (level <= 0)
                throw System::ApplicationException("Attempt to release a lock that is not owned by the calling thread.");
            if (--level == 0) mtx_.unlock_shared();
        }

        /**
         * @brief Acquires the writer lock, blocking up to millisecondsTimeout milliseconds (-1 = infinite).
         * @throws System::ApplicationException if the timeout elapses before the lock is acquired.
         */
        void AcquireWriterLock(intcs millisecondsTimeout) {
            if (millisecondsTimeout < -1)
                throw System::ArgumentOutOfRangeException("millisecondsTimeout");
            if (writerThreadId_.load() == CurrentThreadId()) { ++writerLevel_; return; }
            if (millisecondsTimeout < 0) mtx_.lock();
            // Verified against ReaderWriterLock.cs's GetTimeoutException(): real .NET throws on
            // timeout instead of returning as if the lock had been acquired.
            else if (!mtx_.try_lock_for(std::chrono::milliseconds(millisecondsTimeout)))
                throw System::ApplicationException("The operation has timed out.");
            writerThreadId_.store(CurrentThreadId());
            writerLevel_ = 1;
        }
        /** Acquires the writer lock, blocking up to @p timeout. */
        void AcquireWriterLock(const System::TimeSpan& timeout) {
            AcquireWriterLock(static_cast<intcs>(timeout.getTotalMillisecondsProperty()));
        }

        /**
         * @brief Releases one level of the writer lock acquired by the calling thread.
         * @throws System::ApplicationException if the calling thread doesn't hold the lock.
         * @note See ReleaseReaderLock's doc comment for the exception-type rationale.
         */
        void ReleaseWriterLock() {
            if (writerThreadId_.load() != CurrentThreadId())
                throw System::ApplicationException("Attempt to release a lock that is not owned by the calling thread.");
            if (--writerLevel_ == 0) {
                writerThreadId_.store(-1);
                ++writerSeqNum_;
                mtx_.unlock();
            }
        }

        /** Releases all reader/writer locks held by the calling thread; returns a cookie for RestoreLock. */
        LockCookie ReleaseLock() {
            LockCookie cookie;
            if (writerThreadId_.load() == CurrentThreadId()) {
                cookie.writerLevel_ = writerLevel_;
                writerLevel_ = 1;
                ReleaseWriterLock();
            } else {
                intcs& level = MyReaderLevel();
                cookie.readerLevel_ = level;
                if (level > 0) { level = 1; ReleaseReaderLock(); }
            }
            return cookie;
        }

        /** Restores the reader/writer lock state captured by a prior ReleaseLock call. */
        void RestoreLock(LockCookie& cookie) {
            if (cookie.writerLevel_ > 0) {
                AcquireWriterLock(-1);
                writerLevel_ = cookie.writerLevel_;
            } else if (cookie.readerLevel_ > 0) {
                AcquireReaderLock(-1);
                MyReaderLevel() = cookie.readerLevel_;
            }
        }

        /** Upgrades the calling thread's reader lock (if held) to the writer lock; returns a cookie for DowngradeFromWriterLock. */
        LockCookie UpgradeToWriterLock(intcs millisecondsTimeout) {
            LockCookie cookie;
            intcs& level = MyReaderLevel();
            cookie.readerLevel_ = level;
            if (level > 0) { level = 1; ReleaseReaderLock(); }
            AcquireWriterLock(millisecondsTimeout);
            return cookie;
        }
        /** Upgrades the calling thread's reader lock (if held) to the writer lock, blocking up to @p timeout. */
        LockCookie UpgradeToWriterLock(const System::TimeSpan& timeout) {
            return UpgradeToWriterLock(static_cast<intcs>(timeout.getTotalMillisecondsProperty()));
        }

        /** Restores the reader lock state captured by a prior UpgradeToWriterLock call. */
        void DowngradeFromWriterLock(LockCookie& cookie) {
            writerLevel_ = 1;
            ReleaseWriterLock();
            if (cookie.readerLevel_ > 0) {
                AcquireReaderLock(-1);
                MyReaderLevel() = cookie.readerLevel_;
            }
        }
    };

} // namespace System::Threading
