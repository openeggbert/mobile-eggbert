// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <unordered_map>

#include "System/Threading/AsyncLocalValueChangedArgs.hpp"

namespace System::Threading {

    /**
     * @brief Represents ambient data that is local to a given asynchronous control flow; backed
     * by thread-local storage in C++.
     *
     * @note Storage is keyed by a monotonically-increasing per-instance ID rather than by `this`.
     * Keying by `this` (the previous implementation) meant that once an instance was destroyed,
     * every *other* thread's thread_local map retained a stale entry under that now-dangling
     * pointer value forever (a destructor can only clean up the destroying thread's own map) --
     * if a new AsyncLocal<T> was later allocated at the same address (routine with heap reuse),
     * other threads would silently read/write the old, unrelated instance's stale entry instead
     * of the new instance's, corrupting data across two logically-unrelated AsyncLocal<T>
     * objects. IDs are never reused, so a new instance can never collide with a stale entry left
     * by a destroyed one; the trade-off is that another thread's stale entry for a destroyed
     * instance is not proactively cleaned up (a bounded per-ID leak until that thread exits or
     * next touches its own map) rather than corrupting future reads.
     */
    template<typename T>
    class AsyncLocal {
        static std::atomic<std::uint64_t>& nextId() {
            static std::atomic<std::uint64_t> id{0};
            return id;
        }
        std::uint64_t id_ = nextId().fetch_add(1, std::memory_order_relaxed);

        static std::unordered_map<std::uint64_t, T>& storageMap() {
            static thread_local std::unordered_map<std::uint64_t, T> map;
            return map;
        }

        std::function<void(const AsyncLocalValueChangedArgs<T>&)> valueChangedHandler_;

    public:
        /** Constructs an AsyncLocal without a value-changed handler. */
        AsyncLocal() = default;
        /** Constructs an AsyncLocal that invokes handler whenever the value changes. */
        explicit AsyncLocal(std::function<void(const AsyncLocalValueChangedArgs<T>&)> handler)
            : valueChangedHandler_(std::move(handler)) {}

        /** Destroys this instance's slot in the current thread's storage. */
        ~AsyncLocal() { storageMap().erase(id_); }

        /** Returns the current ambient value for the executing thread. */
        [[nodiscard]] const T& getValueProperty() const {
            static const T defaultValue{};
            auto& map = storageMap();
            auto it = map.find(id_);
            return it == map.end() ? defaultValue : it->second;
        }

        /**
         * @brief Sets the ambient value for the executing thread.
         *
         * C++ counterpart of .NET AsyncLocal<T>.Value setter, which is a complete no-op
         * (no mutation, no change notification) when the new value equals the previous one
         * (ExecutionContext.SetLocalValue: `if (previousValue == newValue) return;`).
         */
        void setValueProperty(const T& v) {
            auto& map = storageMap();
            auto it = map.find(id_);
            T previous = (it == map.end()) ? T{} : it->second;
            if (previous == v) return;
            if (valueChangedHandler_) valueChangedHandler_(AsyncLocalValueChangedArgs<T>(previous, v, false));
            map[id_] = v;
        }
    };

} // namespace System::Threading
