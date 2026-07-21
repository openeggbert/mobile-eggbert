// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <map>
#include <vector>
#include <stdexcept>
#include "System/ArgumentException.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"
#include "System/Collections/Generic/KeyValuePair.hpp"
#include "System/InvalidOperationException.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Represents a collection of key/value pairs sorted on the key.
 *
 * C++ counterpart of .NET System.Collections.Generic.SortedDictionary<TKey,TValue>.
 * Backed by std::map<TKey,TValue>; provides O(log n) Add, Remove, and lookup.
 *
 * begin()/end() (used by range-for) now detect structural modification during
 * iteration via a version counter and throw System::InvalidOperationException,
 * matching real .NET's fail-fast enumerator contract (see ticket 1713).
 *
 * @tparam TKey   The type of the keys (must support operator<).
 * @tparam TValue The type of the values.
 */
template<typename TKey, typename TValue>
class SortedDictionary {
    std::map<TKey, TValue> map_;
    intcs version_ = 0;

public:
    /**
     * @brief A version-checked wrapper over std::map<TKey,TValue>::iterator that throws
     * System::InvalidOperationException on dereference/increment if the owning
     * SortedDictionary was structurally modified since this iterator was obtained --
     * matching real .NET's fail-fast enumerator contract (ticket 1713).
     */
    class Iterator {
        typename std::map<TKey, TValue>::const_iterator it_;
        const SortedDictionary* owner_;
        intcs version_;

        void checkVersion() const {
            if (version_ != owner_->version_)
                throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
        }

    public:
        Iterator(typename std::map<TKey, TValue>::const_iterator it, const SortedDictionary* owner)
            : it_(it), owner_(owner), version_(owner->version_) {}

        const std::pair<const TKey, TValue>& operator*() const { checkVersion(); return *it_; }
        const std::pair<const TKey, TValue>* operator->() const { checkVersion(); return &*it_; }
        Iterator& operator++() { checkVersion(); ++it_; return *this; }
        bool operator==(const Iterator& other) const { return it_ == other.it_; }
        bool operator!=(const Iterator& other) const { return it_ != other.it_; }
    };

    /** @brief Initializes a new empty SortedDictionary. */
    SortedDictionary() = default;

    /**
     * @brief Gets the number of key/value pairs in the SortedDictionary.
     *
     * C++ counterpart of .NET SortedDictionary<TKey,TValue>.Count.
     * @return The number of elements.
     */
    [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(map_.size()); }

    /**
     * @brief A proxy for `dict[key]` that throws on read of a missing key while still
     * supporting `dict[key] = value` to insert-or-update, matching .NET's split get/set
     * indexer semantics on top of a single C++ `operator[]`.
     *
     * A plain `TValue&` return (the prior implementation) could not distinguish a read
     * from a write, so it always inserted a default-constructed value on a missing key --
     * matching std::map::operator[]'s convention, not .NET's. Real .NET's
     * SortedDictionary<TKey,TValue> indexer getter throws KeyNotFoundException
     * unconditionally on a missing key; only the setter inserts.
     */
    class ValueProxy {
        SortedDictionary* owner_;
        TKey key_;
    public:
        ValueProxy(SortedDictionary* owner, const TKey& key) : owner_(owner), key_(key) {}

        /** Reads the current value; throws KeyNotFoundException if the key is absent. */
        operator const TValue&() const {
            auto it = owner_->map_.find(key_);
            if (it == owner_->map_.end())
                throw KeyNotFoundException("The given key was not present in the dictionary.");
            return it->second;
        }

        /** Inserts or overwrites the value for this key. */
        ValueProxy& operator=(const TValue& value) {
            bool isNewKey = owner_->map_.find(key_) == owner_->map_.end();
            owner_->map_[key_] = value;
            if (isNewKey) ++owner_->version_;
            return *this;
        }
    };

    /**
     * @brief Gets or sets the value associated with the specified key.
     *
     * C++ counterpart of .NET SortedDictionary<TKey,TValue>.Item[TKey]. The getter throws
     * System::Collections::Generic::KeyNotFoundException if @p key is absent (it does NOT
     * insert a default, unlike std::map::operator[]); the setter inserts or overwrites.
     * See ValueProxy for why this isn't a plain `TValue&`.
     * @param key The key whose value to get or set.
     */
    [[nodiscard]] ValueProxy operator[](const TKey& key) { return ValueProxy(this, key); }

    /**
     * @brief Gets the value associated with the specified key (const).
     *
     * C++ counterpart of .NET SortedDictionary<TKey,TValue>.Item[TKey] getter.
     * @param key The key whose value to get.
     * @return A const reference to the associated value.
     * @throws System::Collections::Generic::KeyNotFoundException if the key is not found.
     */
    [[nodiscard]] const TValue& operator[](const TKey& key) const {
        auto it = map_.find(key);
        if (it == map_.end()) throw KeyNotFoundException("The given key was not present in the dictionary.");
        return it->second;
    }

    /**
     * @brief Adds the specified key and value to the dictionary.
     *
     * C++ counterpart of .NET SortedDictionary<TKey,TValue>.Add(TKey, TValue).
     * @param key   The key of the element to add.
     * @param value The value of the element to add.
     * @throws System::ArgumentException if the key already exists.
     */
    void Add(const TKey& key, const TValue& value) {
        if (map_.count(key)) throw System::ArgumentException("An item with the same key has already been added.");
        map_[key] = value;
        ++version_;
    }

    /**
     * @brief Attempts to add the specified key and value to the dictionary.
     *
     * @param key   The key of the element to add.
     * @param value The value of the element to add.
     * @return true if the element was added; false if the key already existed.
     */
    bool TryAdd(const TKey& key, const TValue& value) {
        if (map_.count(key)) return false;
        map_[key] = value;
        ++version_;
        return true;
    }

    /**
     * @brief Removes the element with the specified key.
     *
     * C++ counterpart of .NET SortedDictionary<TKey,TValue>.Remove(TKey).
     * @param key The key of the element to remove.
     * @return true if the element was found and removed; otherwise false.
     */
    bool Remove(const TKey& key) {
        bool removed = map_.erase(key) > 0;
        if (removed) ++version_;
        return removed;
    }

    /**
     * @brief Determines whether the dictionary contains the specified key.
     *
     * C++ counterpart of .NET SortedDictionary<TKey,TValue>.ContainsKey(TKey).
     * @param key The key to locate.
     * @return true if the key is found; otherwise false.
     */
    [[nodiscard]] bool ContainsKey(const TKey& key) const { return map_.count(key) > 0; }

    /**
     * @brief Determines whether any entry has the specified value.
     *
     * C++ counterpart of .NET SortedDictionary<TKey,TValue>.ContainsValue(TValue).
     * @param value The value to locate.
     * @return true if at least one entry has the value; otherwise false.
     */
    [[nodiscard]] bool ContainsValue(const TValue& value) const {
        for (const auto& kv : map_)
            if (kv.second == value) return true;
        return false;
    }

    /**
     * @brief Gets the value associated with the specified key.
     *
     * C++ counterpart of .NET SortedDictionary<TKey,TValue>.TryGetValue(TKey, out TValue).
     * @param key   The key to locate.
     * @param value Receives the value if the key is found.
     * @return true if the key was found; otherwise false.
     */
    bool TryGetValue(const TKey& key, TValue& value) const {
        auto it = map_.find(key);
        if (it == map_.end()) return false;
        value = it->second;
        return true;
    }

    /**
     * @brief Returns the value for the key, or @p defaultValue if not found.
     *
     * @param key          The key to locate.
     * @param defaultValue The value to return if the key is absent.
     * @return The associated value, or @p defaultValue.
     */
    [[nodiscard]] TValue GetValueOrDefault(const TKey& key, const TValue& defaultValue = TValue{}) const {
        auto it = map_.find(key);
        return it != map_.end() ? it->second : defaultValue;
    }

    /**
     * @brief Gets an enumerable collection of all keys in sorted order.
     *
     * C++ counterpart of .NET SortedDictionary<TKey,TValue>.Keys.
     * @return A std::vector<TKey> of all keys in ascending order.
     */
    [[nodiscard]] std::vector<TKey> getKeysProperty() const {
        std::vector<TKey> keys;
        keys.reserve(map_.size());
        for (const auto& kv : map_) keys.push_back(kv.first);
        return keys;
    }

    /**
     * @brief Gets an enumerable collection of all values in key-sorted order.
     *
     * C++ counterpart of .NET SortedDictionary<TKey,TValue>.Values.
     * @return A std::vector<TValue> of all values in key-ascending order.
     */
    [[nodiscard]] std::vector<TValue> getValuesProperty() const {
        std::vector<TValue> vals;
        vals.reserve(map_.size());
        for (const auto& kv : map_) vals.push_back(kv.second);
        return vals;
    }

    /**
     * @brief Removes all key/value pairs from the dictionary.
     *
     * C++ counterpart of .NET SortedDictionary<TKey,TValue>.Clear().
     */
    void Clear() { map_.clear(); ++version_; }

    /** @brief Returns all keys in sorted order (alias for getKeysProperty()). */
    [[nodiscard]] std::vector<TKey> Keys() const { return getKeysProperty(); }
    /** @brief Returns all values in key-sorted order (alias for getValuesProperty()). */
    [[nodiscard]] std::vector<TValue> Values() const { return getValuesProperty(); }

    /**
     * @brief Returns a version-checked iterator to the beginning of the dictionary (range-for).
     * @throws System::InvalidOperationException from dereference/increment if the dictionary
     *         is structurally modified while this iterator is in use.
     */
    Iterator begin() const { return Iterator(map_.cbegin(), this); }
    /** @brief Returns a version-checked iterator past the end of the dictionary (range-for). */
    Iterator end()   const { return Iterator(map_.cend(), this); }
};

} // namespace System::Collections::Generic
