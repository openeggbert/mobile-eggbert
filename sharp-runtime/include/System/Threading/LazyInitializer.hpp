// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <functional>
#include <mutex>
#include "System/InvalidOperationException.hpp"

namespace System::Threading {

    /**
     * @brief Provides lazy-initialization routines.
     *
     * @note Verified against LazyInitializer.cs's EnsureInitializedCore overloads for `ref T?
     * target`: real .NET does not take any lock at all here — it publishes the new instance via
     * Interlocked.CompareExchange, and if multiple threads race, multiple instances of T may be
     * transiently constructed with all but the winner simply discarded (documented: "this method
     * will not dispose of the objects that were not stored"). This port previously used a
     * `static std::mutex` scoped per template instantiation of T — shared across every unrelated
     * call site initializing a different target of the same type, so reentrant same-thread
     * initialization of a *different* target of the same T (e.g. a value factory that itself
     * calls EnsureInitialized on another T* while the outer call still holds the lock)
     * self-deadlocked on the non-recursive std::mutex. Switched to the same lock-free
     * compare-exchange approach as real .NET (via std::atomic_ref, since `target` is an arbitrary
     * caller-owned T*&, not necessarily a std::atomic<T*> itself); the losing candidate is
     * deleted rather than merely abandoned, since this runtime has no GC to eventually reclaim it.
     */
    class LazyInitializer {
    public:
        /** Prevents instantiation — all members are static. */
        LazyInitializer() = delete;

        /** Initializes target using its default constructor if it is null; returns the initialized value. */
        template<typename T>
        static T& EnsureInitialized(T*& target) {
            if (!target) {
                T* candidate = new T();
                std::atomic_ref<T*> ref(target);
                T* expected = nullptr;
                if (!ref.compare_exchange_strong(expected, candidate)) delete candidate;
            }
            return *target;
        }

        /**
         * @brief Initializes target using valueFactory if it is null; returns the initialized value.
         * @throws System::InvalidOperationException if valueFactory returns nullptr.
         */
        template<typename T>
        static T& EnsureInitialized(T*& target, std::function<T*()> valueFactory) {
            if (!target) {
                T* candidate = valueFactory();
                if (!candidate) throw System::InvalidOperationException("ValueFactory returned null.");
                std::atomic_ref<T*> ref(target);
                T* expected = nullptr;
                if (!ref.compare_exchange_strong(expected, candidate)) delete candidate;
            }
            return *target;
        }
    };

} // namespace System::Threading
