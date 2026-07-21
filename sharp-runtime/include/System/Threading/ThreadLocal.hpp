// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "System/IDisposable.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"

namespace System::Threading {

    /**
     * @brief Provides thread-local storage of data.
     *
     * C++ counterpart of .NET System.Threading.ThreadLocal<T>. Each instance owns an
     * independent per-thread slot, keyed by a monotonically-increasing per-instance ID (not
     * `this`) within a thread_local map, so distinct ThreadLocal<T> instances never share state.
     *
     * @note Keying by `this` (the previous implementation) meant that once an instance was
     * destroyed, every *other* thread's thread_local map retained a stale entry under that
     * now-dangling pointer value forever (a destructor can only clean up the destroying thread's
     * own map) -- if a new ThreadLocal<T> was later allocated at the same address (routine with
     * heap reuse), other threads would silently read/write the old, unrelated instance's stale
     * entry instead of the new instance's, corrupting data across two logically-unrelated
     * ThreadLocal<T> objects. IDs are never reused, so a new instance can never collide with a
     * stale entry left by a destroyed one; the trade-off is that another thread's stale entry
     * for a destroyed instance is not proactively cleaned up (a bounded per-ID leak until that
     * thread exits or next touches its own map) rather than corrupting future reads.
     */
    template<typename T>
    class ThreadLocal : public System::IDisposable {
        static std::atomic<std::uint64_t>& nextId() {
            static std::atomic<std::uint64_t> id{0};
            return id;
        }
        std::uint64_t id_ = nextId().fetch_add(1, std::memory_order_relaxed);

        static std::unordered_map<std::uint64_t, std::unique_ptr<T>>& storageMap() {
            static thread_local std::unordered_map<std::uint64_t, std::unique_ptr<T>> map;
            return map;
        }

        // Verified against ThreadLocal.cs's GetValueSlow()/ThreadLocal_Value_RecursiveCallsToValue:
        // real .NET detects a value factory that reentrantly ends up accessing this same
        // instance's Value while the factory is still running and throws
        // InvalidOperationException rather than let it recurse. This port previously had no such
        // guard: factory_() ran before the map entry was inserted, so a reentrant
        // getValueProperty() call on the same thread would find no entry, invoke factory_() again,
        // and recurse until a (uncatchable) stack overflow.
        static std::unordered_set<std::uint64_t>& inProgress() {
            static thread_local std::unordered_set<std::uint64_t> set;
            return set;
        }

        std::function<T()> factory_;
        bool trackAllValues_ = false;
        bool disposed_ = false;

        void ThrowIfDisposed() const {
            if (disposed_) throw System::ObjectDisposedException("ThreadLocal`1");
        }

    public:
        /** Constructs a ThreadLocal with default-constructed values. */
        ThreadLocal() = default;
        /** Constructs a ThreadLocal; the trackAllValues flag is accepted but not used. */
        explicit ThreadLocal(bool trackAllValues) : trackAllValues_(trackAllValues) {}
        /** Constructs a ThreadLocal that calls valueFactory to create the initial value. */
        explicit ThreadLocal(std::function<T()> valueFactory)
            : factory_(std::move(valueFactory)) {}
        /** Constructs a ThreadLocal with a value factory and a tracking flag. */
        ThreadLocal(std::function<T()> valueFactory, bool trackAllValues)
            : factory_(std::move(valueFactory)), trackAllValues_(trackAllValues) {}

        /** Releases this instance's slot in the current thread's storage on destruction. */
        ~ThreadLocal() override { storageMap().erase(id_); }

        /** Returns true if the value has been initialised for the current thread. */
        [[nodiscard]] bool getIsValueCreatedProperty() const {
            return storageMap().find(id_) != storageMap().end();
        }

        /**
         * @brief Returns the value for the current thread, initialising it on first access.
         * @throws System::ObjectDisposedException if this instance has been disposed.
         * @throws System::InvalidOperationException if the value factory reentrantly accesses
         * this instance's value while still running.
         */
        [[nodiscard]] T& getValueProperty() {
            ThrowIfDisposed();
            auto& map = storageMap();
            auto it = map.find(id_);
            if (it == map.end()) {
                auto& active = inProgress();
                if (!active.insert(id_).second)
                    throw System::InvalidOperationException(
                        "ValueFactory attempted to access the Value property of this instance.");
                std::unique_ptr<T> value;
                try {
                    value = std::make_unique<T>(factory_ ? factory_() : T{});
                } catch (...) {
                    active.erase(id_);
                    throw;
                }
                active.erase(id_);
                it = map.emplace(id_, std::move(value)).first;
            }
            return *it->second;
        }

        /**
         * @brief Sets the value for the current thread.
         * @throws System::ObjectDisposedException if this instance has been disposed.
         */
        void setValueProperty(const T& v) {
            ThrowIfDisposed();
            auto& map = storageMap();
            auto it = map.find(id_);
            if (it == map.end()) map.emplace(id_, std::make_unique<T>(v));
            else *it->second = v;
        }

        /** Returns the value for the current thread (alias for getValueProperty). */
        [[nodiscard]] T& Value() { return getValueProperty(); }

        /** Releases resources for the current thread's value. */
        void Dispose() override {
            disposed_ = true;
            storageMap().erase(id_);
        }
    };

} // namespace System::Threading
