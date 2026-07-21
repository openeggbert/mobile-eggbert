// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <mutex>
#include <unordered_map>
#include <vector>
#include <functional>
#include <stdexcept>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"

namespace System::Collections::Concurrent {

    using SharpRuntime::intcs;

    /**
     * @brief A thread-safe collection of key/value pairs.
     *
     * Wraps std::unordered_map with std::mutex.
     * Partial C++ counterpart of .NET System.Collections.Concurrent.ConcurrentDictionary<TKey,TValue>.
     *
     */
    template<typename TKey, typename TValue>
    class ConcurrentDictionary {
        mutable std::mutex             mutex_;
        std::unordered_map<TKey,TValue> map_;
    public:
        /** Default-constructs an empty ConcurrentDictionary. */
        ConcurrentDictionary() = default;

        /** Gets the number of key/value pairs in the dictionary (thread-safe). */
        [[nodiscard]] intcs getCountProperty() const {
            std::lock_guard<std::mutex> lk(mutex_);
            return static_cast<intcs>(map_.size());
        }

        /** Returns true if the dictionary contains no elements (thread-safe). */
        [[nodiscard]] bool getIsEmptyProperty() const {
            std::lock_guard<std::mutex> lk(mutex_);
            return map_.empty();
        }

        /** Thread-safely adds key/value only if key is not already present; returns true if added. */
        bool TryAdd(const TKey& key, const TValue& value) {
            std::lock_guard<std::mutex> lk(mutex_);
            return map_.emplace(key, value).second;
        }

        /** Thread-safely retrieves the value for key; returns true if found. */
        bool TryGetValue(const TKey& key, TValue& value) const {
            std::lock_guard<std::mutex> lk(mutex_);
            auto it = map_.find(key);
            if (it == map_.end()) return false;
            value = it->second;
            return true;
        }

        /** Thread-safely removes the entry for key and outputs its value; returns true if removed. */
        bool TryRemove(const TKey& key, TValue& value) {
            std::lock_guard<std::mutex> lk(mutex_);
            auto it = map_.find(key);
            if (it == map_.end()) return false;
            value = it->second;
            map_.erase(it);
            return true;
        }

        /** Thread-safely updates the value for key if the current value equals comparisonValue; returns true if updated. */
        bool TryUpdate(const TKey& key, const TValue& newValue, const TValue& comparisonValue) {
            std::lock_guard<std::mutex> lk(mutex_);
            auto it = map_.find(key);
            if (it == map_.end() || !(it->second == comparisonValue)) return false;
            it->second = newValue;
            return true;
        }

        /**
         * @brief A proxy for `dict[key]` that performs each read or write atomically under the
         * dictionary's lock, instead of exposing a reference into the internal map.
         *
         * C++ counterpart of .NET ConcurrentDictionary<TKey,TValue>'s indexer property
         * (ConcurrentDictionary.cs:1065). Returning `TValue&` directly (the prior
         * implementation) let a reference escape the lock scope, so a concurrent TryRemove
         * on the same key could erase the underlying map node while another thread still
         * held and used that reference -- a dangling-reference bug that cannot occur in
         * real .NET, where the getter returns a copy and the setter is a single locked
         * operation. This proxy restores that guarantee: reading converts to a locked
         * TryGetValue-style copy, assigning performs a single locked upsert.
         */
        class ValueProxy {
            ConcurrentDictionary* owner_;
            TKey key_;
        public:
            ValueProxy(ConcurrentDictionary* owner, const TKey& key) : owner_(owner), key_(key) {}

            /** Reads the current value under lock; throws KeyNotFoundException if @p key is absent. */
            operator TValue() const {
                std::lock_guard<std::mutex> lk(owner_->mutex_);
                auto it = owner_->map_.find(key_);
                if (it == owner_->map_.end())
                    throw System::Collections::Generic::KeyNotFoundException("The given key was not present in the dictionary.");
                return it->second;
            }

            /** Atomically inserts or overwrites the value for this key under lock. */
            ValueProxy& operator=(const TValue& value) {
                std::lock_guard<std::mutex> lk(owner_->mutex_);
                owner_->map_[key_] = value;
                return *this;
            }
        };

        /**
         * @brief Gets or sets the value associated with @p key.
         *
         * C++ counterpart of .NET ConcurrentDictionary<TKey,TValue> indexer. The getter throws
         * System::Collections::Generic::KeyNotFoundException if @p key is absent (it does NOT
         * insert a default, unlike std::unordered_map::operator[]); the setter inserts or
         * overwrites atomically. See ValueProxy for why this isn't a plain `TValue&`.
         */
        ValueProxy operator[](const TKey& key) {
            return ValueProxy(this, key);
        }

        /** Thread-safely returns the value for key if present, otherwise inserts and returns defaultValue. */
        TValue GetOrAdd(const TKey& key, const TValue& defaultValue) {
            std::lock_guard<std::mutex> lk(mutex_);
            auto [it, inserted] = map_.emplace(key, defaultValue);
            return it->second;
        }

        /**
         * @brief Thread-safely returns the value for key if present, otherwise inserts the
         * value produced by factory.
         *
         * @note factory is invoked WITHOUT the dictionary's internal lock held, matching real
         * .NET's documented ConcurrentDictionary.GetOrAdd contract exactly: the delegate may run
         * outside the lock and, under contention, may be invoked more than once (its result is
         * discarded if another thread inserts the same key first) -- callers must not assume the
         * factory call and the insertion are atomic, and the factory must not have side effects
         * that depend on that assumption. This also avoids a self-deadlock if factory reentrantly
         * calls back into this same ConcurrentDictionary instance (unavoidable with a
         * non-recursive std::mutex if the lock were held across the callback).
         */
        TValue GetOrAdd(const TKey& key, std::function<TValue(const TKey&)> factory) {
            {
                std::lock_guard<std::mutex> lk(mutex_);
                auto it = map_.find(key);
                if (it != map_.end()) return it->second;
            }
            TValue v = factory(key);
            std::lock_guard<std::mutex> lk(mutex_);
            auto [it, inserted] = map_.emplace(key, std::move(v));
            return it->second;
        }

        /**
         * @brief Thread-safely adds addValue if key is absent, or replaces the existing value
         * using updateFactory.
         *
         * @note updateFactory is invoked WITHOUT the dictionary's internal lock held (see
         * GetOrAdd's doc-comment for the full rationale -- same contract, same deadlock-avoidance
         * reason). If another thread mutates the entry between the existence check and the write
         * back, this last write wins (a simplification of real .NET's compare-and-retry loop,
         * which requires TValue equality-comparability this port's ConcurrentDictionary doesn't
         * assume) -- acceptable since the primary correctness requirement (no lock held across
         * arbitrary user code) is preserved.
         */
        TValue AddOrUpdate(const TKey& key, const TValue& addValue,
                           std::function<TValue(const TKey&, const TValue&)> updateFactory) {
            bool exists;
            TValue current{};
            {
                std::lock_guard<std::mutex> lk(mutex_);
                auto it = map_.find(key);
                exists = (it != map_.end());
                if (exists) current = it->second;
            }
            if (!exists) {
                std::lock_guard<std::mutex> lk(mutex_);
                auto it = map_.find(key);
                if (it == map_.end()) { map_[key] = addValue; return addValue; }
                current = it->second; // another thread added it first; fall through to update
            }
            TValue updated = updateFactory(key, current);
            std::lock_guard<std::mutex> lk(mutex_);
            map_[key] = updated;
            return updated;
        }

        /**
         * @brief Thread-safely adds a value produced by addFactory if key is absent, or replaces
         * the existing value using updateFactory.
         * C++ counterpart of .NET ConcurrentDictionary.AddOrUpdate(TKey, Func&lt;TKey,TValue&gt;, Func&lt;TKey,TValue,TValue&gt;).
         *
         * @note Neither addFactory nor updateFactory is invoked with the dictionary's internal
         * lock held -- see GetOrAdd's doc-comment for the full rationale.
         */
        TValue AddOrUpdate(const TKey& key, std::function<TValue(const TKey&)> addFactory,
                           std::function<TValue(const TKey&, const TValue&)> updateFactory) {
            bool exists;
            TValue current{};
            {
                std::lock_guard<std::mutex> lk(mutex_);
                auto it = map_.find(key);
                exists = (it != map_.end());
                if (exists) current = it->second;
            }
            if (!exists) {
                TValue v = addFactory(key);
                std::lock_guard<std::mutex> lk(mutex_);
                auto it = map_.find(key);
                if (it == map_.end()) { map_[key] = v; return v; }
                current = it->second; // another thread added it first; fall through to update
            }
            TValue updated = updateFactory(key, current);
            std::lock_guard<std::mutex> lk(mutex_);
            map_[key] = updated;
            return updated;
        }

        /** Returns true if the dictionary contains the specified key (thread-safe). */
        [[nodiscard]] bool ContainsKey(const TKey& key) const {
            std::lock_guard<std::mutex> lk(mutex_);
            return map_.count(key) > 0;
        }

        /** Thread-safely removes all elements from the dictionary. */
        void Clear() {
            std::lock_guard<std::mutex> lk(mutex_);
            map_.clear();
        }

        /**
         * @brief Returns a snapshot vector of all keys (thread-safe).
         * C++ counterpart of .NET ConcurrentDictionary&lt;TKey,TValue&gt;.Keys.
         */
        [[nodiscard]] std::vector<TKey> getKeysProperty() const {
            std::lock_guard<std::mutex> lk(mutex_);
            std::vector<TKey> k;
            for (auto& p : map_) k.push_back(p.first);
            return k;
        }

        /**
         * @brief Returns a snapshot vector of all values (thread-safe).
         * C++ counterpart of .NET ConcurrentDictionary&lt;TKey,TValue&gt;.Values.
         */
        [[nodiscard]] std::vector<TValue> getValuesProperty() const {
            std::lock_guard<std::mutex> lk(mutex_);
            std::vector<TValue> v;
            for (auto& p : map_) v.push_back(p.second);
            return v;
        }
    };

} // namespace System::Collections::Concurrent
