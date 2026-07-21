// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <unordered_map>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/IDisposable.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/Threading/LockRecursionException.hpp"
#include "System/Threading/LockRecursionPolicy.hpp"
#include "System/Threading/SynchronizationLockException.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /**
     * @brief A reader/writer lock that allows multiple simultaneous readers or one exclusive writer.
     *
     * C++ counterpart of .NET System.Threading.ReaderWriterLockSlim.
     *
     * @note Verified against ReaderWriterLockSlim.cs's TryEnterReadLockCore/TryEnterWriteLockCore/
     * TryEnterUpgradeableReadLockCore/Exit*. This port previously (1) discarded the
     * millisecondsTimeout parameter on all three TryEnter* methods, always making a single
     * non-blocking attempt regardless of what the caller passed; (2) ignored the constructor's
     * LockRecursionPolicy entirely, so same-thread recursive acquisition always **deadlocked**
     * (blocking forever waiting for a lock the same thread already held) instead of throwing
     * LockRecursionException under the .NET-default NoRecursion policy; and (3) tracked
     * reader-lock ownership via set *membership* rather than a count, so a legitimately nested
     * EnterReadLock()/EnterReadLock()/ExitReadLock() sequence's second ExitReadLock() call threw
     * SynchronizationLockException (membership already removed by the first exit) instead of
     * decrementing — worse, this also meant the internal reader tally never reached zero from a
     * genuinely-nested acquisition, which would have permanently starved any waiting writer had
     * recursion not otherwise deadlocked first.
     *
     * Reworked to: use real timed condition-variable waits (matching this session's established
     * Timeout.Infinite-aware pattern); track per-thread reader/writer/upgrade *counts* instead of
     * set membership, keyed by a monotonically-increasing per-instance ID rather than `this`
     * (matching the AsyncLocal<T>/ThreadLocal<T> fix earlier this session — avoids the same
     * address-reuse data-corruption risk `this`-pointer-keying has); and apply
     * LockRecursionPolicy's actual rules: cross-type combinations that are always deadlock-prone
     * (write-after-read, upgrade-after-read, upgrade-after-write) always throw
     * LockRecursionException regardless of policy (verified: real .NET performs these specific
     * checks identically in both its NoRecursion and SupportsRecursion code paths), while
     * same-type recursion (read-after-read, write-after-write, upgrade-after-upgrade) only
     * throws under NoRecursion and is allowed (as a tracked, matched-by-count recursion) under
     * SupportsRecursion. The existing upgrade-lock design (an upgrade owner's EnterWriteLock()
     * call waits only for other readers to drain, not for the upgrade slot itself, avoiding the
     * self-deadlock a naive lock_shared()-then-lock() sequence would cause) is preserved.
     */
    class ReaderWriterLockSlim : public System::IDisposable {
        struct ThreadCounts {
            intcs reader = 0;
            intcs writer = 0;
            intcs upgrade = 0;
            // True if this thread's currently-outstanding read acquisition incremented
            // `readers_`. A read acquisition implied by already holding the write or upgrade
            // lock never increments `readers_` (that reader is already accounted for via
            // writerActive_/upgradeableActive_, and NOT counting it lets an upgrade owner's
            // EnterWriteLock() wait for "other readers drained" without waiting on itself).
            bool readerCountsTowardGlobal = false;
        };

        static std::atomic<std::uint64_t>& nextId() {
            static std::atomic<std::uint64_t> id{0};
            return id;
        }
        std::uint64_t id_ = nextId().fetch_add(1, std::memory_order_relaxed);

        static std::unordered_map<std::uint64_t, ThreadCounts>& threadCounts() {
            thread_local std::unordered_map<std::uint64_t, ThreadCounts> map;
            return map;
        }
        [[nodiscard]] ThreadCounts& myCounts() { return threadCounts()[id_]; }

        // Hand-rolled monitor rather than std::shared_mutex: std::shared_mutex has no upgrade
        // primitive, and calling lock() on a thread that already holds lock_shared() on the
        // same shared_mutex (the EnterUpgradeableReadLock() -> EnterWriteLock() pattern that is
        // the entire reason to use the upgradeable lock) is undefined behavior. This state
        // machine lets the upgrade-lock owner acquire the write lock by waiting only for other
        // readers to drain, not for itself.
        mutable std::mutex stateMtx_;
        mutable std::condition_variable cv_;
        intcs readers_ = 0;
        bool writerActive_ = false;
        bool upgradeableActive_ = false;
        bool disposed_ = false;
        LockRecursionPolicy recursionPolicy_ = LockRecursionPolicy::NoRecursion;

        void throwIfDisposed() const {
            if (disposed_) throw System::ObjectDisposedException("ReaderWriterLockSlim");
        }

        [[nodiscard]] bool isReentrant() const { return recursionPolicy_ == LockRecursionPolicy::SupportsRecursion; }

        // Waits for pred() while holding lk, honoring .NET's Timeout.Infinite (-1) convention.
        // Returns false (without throwing) if millisecondsTimeout elapses before pred() is true.
        template<typename Pred>
        bool waitFor(std::unique_lock<std::mutex>& lk, intcs millisecondsTimeout, Pred pred) {
            if (millisecondsTimeout == -1) {
                cv_.wait(lk, pred);
                return true;
            }
            return cv_.wait_for(lk, std::chrono::milliseconds(millisecondsTimeout), pred);
        }

        static void validateTimeout(intcs millisecondsTimeout) {
            System::ArgumentOutOfRangeException::ThrowIfLessThan(millisecondsTimeout, -1, "millisecondsTimeout");
        }

    public:
        /** Constructs a ReaderWriterLockSlim with no-recursion policy. */
        ReaderWriterLockSlim() = default;
        /** Constructs a ReaderWriterLockSlim with the specified recursion policy. */
        explicit ReaderWriterLockSlim(LockRecursionPolicy recursionPolicy) : recursionPolicy_(recursionPolicy) {}

        /** Releases this instance's slot in the current thread's per-thread tracking on destruction. */
        ~ReaderWriterLockSlim() override { threadCounts().erase(id_); }

        /** Returns the recursion policy this instance was constructed with. */
        [[nodiscard]] LockRecursionPolicy getRecursionPolicyProperty() const { return recursionPolicy_; }

        /**
         * @brief Acquires a shared read lock, blocking until it becomes available.
         * @throws LockRecursionException per LockRecursionPolicy — see class doc comment.
         */
        void EnterReadLock() { TryEnterReadLock(-1); }

        /** Releases a shared read lock. @throws System::Threading::SynchronizationLockException if the calling thread does not hold a read lock. */
        void ExitReadLock() {
            auto& map = threadCounts();
            auto it = map.find(id_);
            if (it == map.end() || it->second.reader < 1)
                throw System::Threading::SynchronizationLockException("The read lock is being released without being held.");
            ThreadCounts& counts = it->second;
            --counts.reader;
            if (counts.reader == 0 && counts.readerCountsTowardGlobal) {
                {
                    std::lock_guard<std::mutex> lk(stateMtx_);
                    --readers_;
                }
                counts.readerCountsTowardGlobal = false;
                cv_.notify_all();
            }
        }

        /**
         * @brief Tries to acquire a shared read lock, blocking up to millisecondsTimeout milliseconds (-1 = infinite).
         * @throws LockRecursionException per LockRecursionPolicy — see class doc comment.
         */
        bool TryEnterReadLock(intcs millisecondsTimeout) {
            throwIfDisposed();
            validateTimeout(millisecondsTimeout);
            ThreadCounts& counts = myCounts();

            if (counts.reader > 0) {
                if (!isReentrant())
                    throw LockRecursionException("Recursive read lock acquisitions not allowed in this mode.");
                ++counts.reader;
                return true;
            }
            if (!isReentrant() && counts.writer > 0)
                throw LockRecursionException("A read lock may not be acquired with the write lock held in this mode.");

            // Holding the write or upgrade lock already implies read access (and, under
            // SupportsRecursion, is allowed even for the write-lock case) -- never blocks.
            if (counts.writer > 0 || counts.upgrade > 0) {
                counts.reader = 1;
                counts.readerCountsTowardGlobal = false;
                return true;
            }

            std::unique_lock<std::mutex> lk(stateMtx_);
            if (!waitFor(lk, millisecondsTimeout, [&] { return !writerActive_; })) return false;
            ++readers_;
            counts.reader = 1;
            counts.readerCountsTowardGlobal = true;
            return true;
        }

        /**
         * @brief Acquires an exclusive write lock, blocking until it becomes available.
         * @throws LockRecursionException per LockRecursionPolicy — see class doc comment.
         */
        void EnterWriteLock() { TryEnterWriteLock(-1); }

        /** Releases the exclusive write lock. @throws System::Threading::SynchronizationLockException if the calling thread does not hold the write lock. */
        void ExitWriteLock() {
            auto& map = threadCounts();
            auto it = map.find(id_);
            if (it == map.end() || it->second.writer < 1)
                throw System::Threading::SynchronizationLockException("The write lock is being released without being held.");
            ThreadCounts& counts = it->second;
            if (--counts.writer == 0) {
                {
                    std::lock_guard<std::mutex> lk(stateMtx_);
                    writerActive_ = false;
                }
                cv_.notify_all();
            }
        }

        /**
         * @brief Tries to acquire an exclusive write lock, blocking up to millisecondsTimeout milliseconds (-1 = infinite).
         * @throws LockRecursionException per LockRecursionPolicy — see class doc comment.
         */
        bool TryEnterWriteLock(intcs millisecondsTimeout) {
            throwIfDisposed();
            validateTimeout(millisecondsTimeout);
            ThreadCounts& counts = myCounts();

            if (counts.writer > 0) {
                if (!isReentrant())
                    throw LockRecursionException("Recursive write lock acquisitions not allowed in this mode.");
                ++counts.writer;
                return true;
            }

            bool upgradingToWrite = counts.upgrade > 0;
            // Write-after-read is always disallowed (deadlock-prone), regardless of policy --
            // verified: real .NET performs this exact check in both its NoRecursion and
            // SupportsRecursion branches.
            if (!upgradingToWrite && counts.reader > 0)
                throw LockRecursionException(
                    "Write lock may not be acquired with read lock held. This pattern is prone to "
                    "deadlocks. Please ensure that read locks are released before taking a write "
                    "lock. If an upgrade is necessary, use an upgrade lock in place of the read lock.");

            std::unique_lock<std::mutex> lk(stateMtx_);
            bool acquired = upgradingToWrite
                ? waitFor(lk, millisecondsTimeout, [&] { return readers_ == 0; })
                : waitFor(lk, millisecondsTimeout, [&] { return !writerActive_ && readers_ == 0 && !upgradeableActive_; });
            if (!acquired) return false;
            writerActive_ = true;
            ++counts.writer;
            return true;
        }

        /**
         * @brief Acquires the upgradeable read lock, blocking until it becomes available.
         *
         * At most one thread may hold the upgradeable read lock at a time. While held, the
         * owning thread may subsequently call EnterWriteLock() to upgrade to the exclusive
         * write lock — that call only waits for other readers to drain, not for the
         * upgradeable lock itself, avoiding the self-deadlock that a naive
         * lock_shared()-then-lock() sequence on the same mutex would cause.
         * @throws LockRecursionException per LockRecursionPolicy — see class doc comment.
         */
        void EnterUpgradeableReadLock() { TryEnterUpgradeableReadLock(-1); }

        /** Releases the upgradeable read lock. @throws System::Threading::SynchronizationLockException if the calling thread does not hold it. */
        void ExitUpgradeableReadLock() {
            auto& map = threadCounts();
            auto it = map.find(id_);
            if (it == map.end() || it->second.upgrade < 1)
                throw System::Threading::SynchronizationLockException("The upgradeable lock is being released without being held.");
            ThreadCounts& counts = it->second;
            if (--counts.upgrade == 0) {
                {
                    std::lock_guard<std::mutex> lk(stateMtx_);
                    upgradeableActive_ = false;
                }
                cv_.notify_all();
            }
        }

        /**
         * @brief Tries to acquire the upgradeable read lock, blocking up to millisecondsTimeout milliseconds (-1 = infinite).
         * @throws LockRecursionException per LockRecursionPolicy — see class doc comment.
         */
        bool TryEnterUpgradeableReadLock(intcs millisecondsTimeout) {
            throwIfDisposed();
            validateTimeout(millisecondsTimeout);
            ThreadCounts& counts = myCounts();

            if (counts.upgrade > 0) {
                if (!isReentrant())
                    throw LockRecursionException("Recursive upgradeable lock acquisitions not allowed in this mode.");
                ++counts.upgrade;
                return true;
            }
            if (counts.writer > 0) {
                if (!isReentrant())
                    throw LockRecursionException(
                        "Upgradeable lock may not be acquired with write lock held in this mode. "
                        "Acquiring Upgradeable lock gives the ability to read along with an option "
                        "to upgrade to a writer.");
                // Reentrant: the write lock already held implies exclusive access, so the
                // upgrade slot is granted immediately with no wait.
                std::lock_guard<std::mutex> lk(stateMtx_);
                upgradeableActive_ = true;
                ++counts.upgrade;
                return true;
            }
            // Upgrade-after-read is always disallowed, regardless of policy -- verified: real
            // .NET performs this exact check in both its NoRecursion and SupportsRecursion
            // branches.
            if (counts.reader > 0)
                throw LockRecursionException("Upgradeable lock may not be acquired with read lock held.");

            std::unique_lock<std::mutex> lk(stateMtx_);
            if (!waitFor(lk, millisecondsTimeout, [&] { return !writerActive_ && !upgradeableActive_; })) return false;
            upgradeableActive_ = true;
            ++counts.upgrade;
            return true;
        }

        /** Releases all resources held by the lock. */
        void Dispose() override { disposed_ = true; }

        /** Returns whether the current thread holds a read lock. */
        [[nodiscard]] bool getIsReadLockHeldProperty() const {
            auto& map = threadCounts();
            auto it = map.find(id_);
            return it != map.end() && it->second.reader > 0;
        }
        /** Returns whether the current thread holds the write lock. */
        [[nodiscard]] bool getIsWriteLockHeldProperty() const {
            auto& map = threadCounts();
            auto it = map.find(id_);
            return it != map.end() && it->second.writer > 0;
        }
        /** Returns whether the current thread holds the upgradeable read lock. */
        [[nodiscard]] bool getIsUpgradeableReadLockHeldProperty() const {
            auto& map = threadCounts();
            auto it = map.find(id_);
            return it != map.end() && it->second.upgrade > 0;
        }
    };

} // namespace System::Threading
