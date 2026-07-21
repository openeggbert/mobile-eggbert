// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Threading/SynchronizationLockException.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /**
     * @brief Provides a mechanism that synchronizes access to objects.
     *
     * Partial C++ counterpart of .NET System.Threading.Monitor. .NET associates a monitor with
     * an object's identity via a per-object sync block in the object header; C++ has no object
     * header, so this port keys a global registry on the caller-supplied pointer's identity
     * instead. Each distinct pointer gets its own recursive_timed_mutex + condition_variable_any
     * pair, created lazily on first use and never freed — a documented, deliberate simplification
     * (matching .NET's lock(obj){...} re-entrancy semantics for the common case, at the cost of a
     * registry entry leak for pointers that go out of use during the process lifetime).
     *
     * @note Status: Implemented (registry-backed); not a no-op stub — Enter/Exit/TryEnter/Wait/
     * Pulse/PulseAll provide real mutual exclusion and wait-set semantics keyed by pointer identity.
     */
    class Monitor {
        struct State {
            std::recursive_timed_mutex mutex;
            std::condition_variable_any cv;
            std::atomic<std::thread::id> owner{};
            std::atomic<int> depth{0};

            void onAcquired() {
                if (depth.fetch_add(1, std::memory_order_relaxed) == 0)
                    owner.store(std::this_thread::get_id(), std::memory_order_relaxed);
            }
            void onReleasing() {
                if (depth.fetch_sub(1, std::memory_order_relaxed) == 1)
                    owner.store(std::thread::id(), std::memory_order_relaxed);
            }
            [[nodiscard]] bool heldByCurrentThread() const {
                return owner.load(std::memory_order_relaxed) == std::this_thread::get_id();
            }
        };

        static std::mutex& RegistryMutex() {
            static std::mutex m;
            return m;
        }

        static std::unordered_map<const void*, std::shared_ptr<State>>& Registry() {
            static std::unordered_map<const void*, std::shared_ptr<State>> registry;
            return registry;
        }

        static std::shared_ptr<State> GetOrCreate(const void* obj) {
            std::lock_guard<std::mutex> lock(RegistryMutex());
            auto& slot = Registry()[obj];
            if (!slot) slot = std::make_shared<State>();
            return slot;
        }

    public:
        /** Prevents instantiation — all members are static. */
        Monitor() = delete;

        /** Acquires an exclusive lock on obj, blocking until it is available. Re-entrant on the same thread. */
        static void Enter(const void* obj) { auto s = GetOrCreate(obj); s->mutex.lock(); s->onAcquired(); }
        /** Acquires a lock on obj and sets lockTaken to true once acquired. */
        static void Enter(const void* obj, bool& lockTaken) {
            auto s = GetOrCreate(obj);
            s->mutex.lock();
            s->onAcquired();
            lockTaken = true;
        }

        /**
         * @brief Releases one level of the exclusive lock on obj.
         * @throws System::Threading::SynchronizationLockException if the calling thread does not hold the lock.
         */
        static void Exit(const void* obj) {
            auto s = GetOrCreate(obj);
            if (!s->heldByCurrentThread()) throw System::Threading::SynchronizationLockException();
            s->onReleasing();
            s->mutex.unlock();
        }

        /** Attempts to acquire an exclusive lock on obj without blocking; returns true on success. */
        static bool TryEnter(const void* obj) {
            auto s = GetOrCreate(obj);
            bool ok = s->mutex.try_lock();
            if (ok) s->onAcquired();
            return ok;
        }
        /** Attempts to acquire a lock on obj without blocking; sets lockTaken and returns the same value. */
        static void TryEnter(const void* obj, bool& lockTaken) { lockTaken = TryEnter(obj); }
        /**
         * @brief Attempts to acquire an exclusive lock on obj within the given timeout; returns true on success.
         *
         * Verified against Lock.cs's doc comment: -1 (Timeout.Infinite) waits indefinitely,
         * matching every other timed-wait method in this codebase. std::chrono's own
         * try_lock_for treats a negative duration as already-expired, so -1 must be
         * special-cased to an untimed lock() rather than passed straight through.
         */
        static bool TryEnter(const void* obj, intcs millisecondsTimeout) {
            auto s = GetOrCreate(obj);
            bool ok;
            if (millisecondsTimeout == -1) {
                s->mutex.lock();
                ok = true;
            } else {
                ok = s->mutex.try_lock_for(std::chrono::milliseconds(millisecondsTimeout));
            }
            if (ok) s->onAcquired();
            return ok;
        }

        /**
         * @brief Releases the lock on obj and blocks until it is reacquired via Pulse/PulseAll.
         * @note Assumes the calling thread holds the lock at recursion depth 1 (the common case for
         * lock(obj){ Monitor.Wait(obj); } patterns); deeper recursion is not unwound and reacquired.
         * @throws System::Threading::SynchronizationLockException if the calling thread does not hold the lock.
         */
        static bool Wait(const void* obj) {
            auto state = GetOrCreate(obj);
            if (!state->heldByCurrentThread()) throw System::Threading::SynchronizationLockException();
            std::unique_lock<std::recursive_timed_mutex> lock(state->mutex, std::adopt_lock);
            // cv.wait() internally unlocks state->mutex for the duration of the wait and
            // relocks it before returning; mirror that in our owner/depth tracking so a
            // different thread's Enter() during the wait window is correctly recognized
            // as the new owner.
            state->onReleasing();
            state->cv.wait(lock);
            state->onAcquired();
            lock.release();
            return true;
        }

        /**
         * @brief Releases the lock on obj and blocks until reacquired or millisecondsTimeout elapses.
         * @throws System::Threading::SynchronizationLockException if the calling thread does not hold the lock.
         */
        static bool Wait(const void* obj, intcs millisecondsTimeout) {
            auto state = GetOrCreate(obj);
            if (!state->heldByCurrentThread()) throw System::Threading::SynchronizationLockException();
            std::unique_lock<std::recursive_timed_mutex> lock(state->mutex, std::adopt_lock);
            state->onReleasing();
            bool ok;
            if (millisecondsTimeout == -1) {
                state->cv.wait(lock);
                ok = true;
            } else {
                ok = state->cv.wait_for(lock, std::chrono::milliseconds(millisecondsTimeout)) == std::cv_status::no_timeout;
            }
            state->onAcquired();
            lock.release();
            return ok;
        }

        /**
         * @brief Notifies a thread in the waiting queue of a change in the locked object's state.
         * @throws System::Threading::SynchronizationLockException if the calling thread does not hold the lock.
         */
        static void Pulse(const void* obj) {
            auto s = GetOrCreate(obj);
            if (!s->heldByCurrentThread()) throw System::Threading::SynchronizationLockException();
            s->cv.notify_one();
        }
        /**
         * @brief Notifies all waiting threads of a change in the locked object's state.
         * @throws System::Threading::SynchronizationLockException if the calling thread does not hold the lock.
         */
        static void PulseAll(const void* obj) {
            auto s = GetOrCreate(obj);
            if (!s->heldByCurrentThread()) throw System::Threading::SynchronizationLockException();
            s->cv.notify_all();
        }
    };

} // namespace System::Threading
