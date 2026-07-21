// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cmath>
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Represents a generic collection of key/value pairs.
 *
 * C++ counterpart of .NET System.Collections.Generic.Dictionary<TKey,TValue>.
 * Backed by std::unordered_map; provides O(1) average-case lookup, insertion, and removal.
 *
 * @note begin()/end() return a version-checked iterator that throws
 * System::InvalidOperationException("Collection was modified; enumeration operation may not "
 * "execute.") if the dictionary is structurally modified while iteration is in progress,
 * matching .NET's Dictionary<TKey,TValue> enumerator fail-fast contract in spirit. The
 * version-bump rule matches real .NET's actual _version semantics (verified against
 * Dictionary.cs) for Add() and the indexer setter: both bump on inserting a NEW key, but
 * overwriting an EXISTING key's value does NOT (not a structural change). TWO deliberate
 * deviations from .NET's exact _version rules, both for C++ memory safety rather than pure
 * .NET parity: (1) Remove() DOES bump version_ here, unlike real .NET's Dictionary.Remove
 * (which doesn't, since its array-backed entries use a free-list so index-based enumeration is
 * unaffected by a removal) -- this port's iterator wraps a real std::unordered_map iterator,
 * and erasing the element the iterator currently points at invalidates that specific iterator
 * even though other iterators remain valid (confirmed via an ASan use-after-free repro before
 * this fix); not bumping would silently let an in-progress enumeration crash instead of
 * throwing cleanly. (2) Clear() also bumps version_ here, unlike real .NET's Dictionary.Clear()
 * (which also skips the bump), because std::unordered_map::clear() invalidates every iterator
 * into the map, unlike .NET's array-based entries which are simply zeroed in place. Both
 * deviations trade being slightly stricter than real .NET (an extra InvalidOperationException
 * real .NET wouldn't throw in these two specific scenarios) for guaranteed memory safety.
 *
 * @tparam TKey   The type of the keys.
 * @tparam TValue The type of the values.
 */
template<typename TKey, typename TValue>
class Dictionary {
    std::unordered_map<TKey, TValue> map_;
    intcs version_ = 0;

    template<typename InnerIt>
    class VersionCheckedIterator {
        const Dictionary* owner_;
        intcs version_;
        InnerIt it_;
    public:
        VersionCheckedIterator(const Dictionary* owner, InnerIt it)
            : owner_(owner), version_(owner->version_), it_(it) {}

        VersionCheckedIterator& operator++() {
            if (version_ != owner_->version_)
                throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            ++it_;
            return *this;
        }
        decltype(auto) operator*() const {
            if (version_ != owner_->version_)
                throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            return *it_;
        }
        auto operator->() const { return it_.operator->(); }
        bool operator!=(const VersionCheckedIterator& other) const { return it_ != other.it_; }
        bool operator==(const VersionCheckedIterator& other) const { return it_ == other.it_; }
    };

public:
    /**
     * @brief Initializes a new empty Dictionary.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>().
     */
    Dictionary() = default;

    /**
     * @brief Gets the number of key/value pairs contained in the dictionary.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>.Count.
     */
    [[nodiscard]] intcs getCountProperty() const {
        return static_cast<intcs>(map_.size());
    }

    /**
     * @brief Adds the specified key and value to the dictionary.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>.Add(TKey, TValue).
     * @param key   The key of the element to add.
     * @param value The value of the element to add.
     * @throws System::ArgumentException if a key with the same value already exists.
     */
    void Add(const TKey& key, const TValue& value) {
        if (map_.count(key))
            throw System::ArgumentException("An item with the same key has already been added.");
        map_[key] = value;
        ++version_;
    }

    /**
     * @brief Removes the element with the specified key from the dictionary.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>.Remove(TKey).
     *
     * @note DEVIATION from real .NET: Dictionary.cs's Remove does NOT bump `_version` (its
     * array-backed entries use a free-list, so index-based enumeration is unaffected by a
     * removal). This port's version-checked iterator wraps a real std::unordered_map iterator,
     * where erasing the element the iterator is CURRENTLY positioned on invalidates that
     * specific iterator (confirmed via an ASan use-after-free repro) even though other
     * iterators remain valid per the standard -- not bumping version_ here would silently let
     * an in-progress enumeration crash instead of throwing cleanly. Bumping unconditionally on
     * a successful removal trades exact `_version` parity for guaranteed memory safety.
     * @param key The key of the element to remove.
     * @return true if the element was found and removed; otherwise false.
     */
    bool Remove(const TKey& key) {
        bool removed = map_.erase(key) > 0;
        if (removed) ++version_;
        return removed;
    }

    /**
     * @brief Removes the element with the specified key and copies its value to the output parameter.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>.Remove(TKey, out TValue). See the other
     * Remove(TKey) overload's doc-comment for why this port bumps version_ on removal, unlike
     * real .NET's Dictionary.Remove.
     * @param key   The key of the element to remove.
     * @param value Receives the removed value if found.
     * @return true if the element was found and removed; otherwise false.
     */
    bool Remove(const TKey& key, TValue& value) {
        auto it = map_.find(key);
        if (it == map_.end()) return false;
        value = std::move(it->second);
        map_.erase(it);
        ++version_;
        return true;
    }

    /**
     * @brief Determines whether the dictionary contains the specified key.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>.ContainsKey(TKey).
     * @param key The key to locate.
     * @return true if the key is found; otherwise false.
     */
    [[nodiscard]] bool ContainsKey(const TKey& key) const {
        return map_.count(key) > 0;
    }

    /**
     * @brief Determines whether the dictionary contains a specific value.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>.ContainsValue(TValue).
     * This is an O(n) operation.
     * @param value The value to locate.
     * @return true if an entry with the value is found; otherwise false.
     */
    [[nodiscard]] bool ContainsValue(const TValue& value) const {
        for (const auto& kv : map_)
            if (kv.second == value) return true;
        return false;
    }

    /**
     * @brief Gets the value associated with the specified key.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>.TryGetValue(TKey, out TValue).
     * @param key      The key whose value to get.
     * @param outValue Receives the value if the key is found.
     * @return true if the key was found; otherwise false.
     */
    bool TryGetValue(const TKey& key, TValue& outValue) const {
        auto it = map_.find(key);
        if (it == map_.end()) return false;
        outValue = it->second;
        return true;
    }

    /**
     * @brief Adds a key/value pair only if the key does not already exist.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>.TryAdd(TKey, TValue).
     * @param key   The key to add.
     * @param value The value to associate with the key.
     * @return true if the pair was added; false if the key already existed.
     */
    bool TryAdd(const TKey& key, const TValue& value) {
        if (map_.count(key)) return false;
        map_[key] = value;
        ++version_;
        return true;
    }

    /**
     * @brief Gets the value for a key, or a default if the key is absent.
     *
     * C++ counterpart of .NET CollectionExtensions.GetValueOrDefault extension method.
     * @param key          The key to look up.
     * @param defaultValue Value to return if the key is absent.
     * @return The associated value, or defaultValue.
     */
    [[nodiscard]] TValue GetValueOrDefault(const TKey& key, const TValue& defaultValue = TValue{}) const {
        auto it = map_.find(key);
        return it != map_.end() ? it->second : defaultValue;
    }

    /**
     * @brief Removes all key/value pairs from the dictionary.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>.Clear().
     */
    void Clear() { map_.clear(); ++version_; }

    /**
     * @brief Gets a vector containing the keys of the dictionary.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>.Keys.
     */
    [[nodiscard]] std::vector<TKey> getKeysProperty() const {
        std::vector<TKey> keys;
        keys.reserve(map_.size());
        for (const auto& kv : map_) keys.push_back(kv.first);
        return keys;
    }

    /**
     * @brief Gets a vector containing the values of the dictionary.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>.Values.
     */
    [[nodiscard]] std::vector<TValue> getValuesProperty() const {
        std::vector<TValue> vals;
        vals.reserve(map_.size());
        for (const auto& kv : map_) vals.push_back(kv.second);
        return vals;
    }

    /**
     * @brief Ensures the internal bucket count can hold at least capacity entries without rehashing.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>.EnsureCapacity(int), which returns the
     * resulting capacity (`_entries.Length` after the call, or the pre-existing capacity if it
     * was already >= the requested value) rather than void -- added to match that signature.
     * std::unordered_map has no direct equivalent of .NET's internal entries-array length, so
     * bucket_count() after reserve() is returned as the closest honest approximation.
     * @param capacity The minimum number of entries the dictionary should be able to hold.
     * @return The resulting capacity.
     * @throws System::ArgumentOutOfRangeException if @p capacity is negative.
     */
    intcs EnsureCapacity(intcs capacity) {
        if (capacity < 0)
            throw System::ArgumentOutOfRangeException("capacity");
        map_.reserve(static_cast<std::size_t>(capacity));
        ++version_;
        return static_cast<intcs>(map_.bucket_count());
    }

    /**
     * @brief Reduces internal memory by resizing the bucket array to fit the current entry count.
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue>.TrimExcess().
     */
    void TrimExcess() {
        map_.rehash(static_cast<std::size_t>(
            std::ceil(static_cast<double>(map_.size()) / map_.max_load_factor())));
        ++version_;
    }

    /**
     * @brief A proxy for `dict[key]` that throws on read of a missing key while still
     * supporting `dict[key] = value` to insert-or-update, matching .NET's split get/set
     * indexer semantics on top of a single C++ `operator[]`.
     *
     * A plain `TValue&` return (the prior implementation) could not distinguish a read
     * from a write, so it always inserted a default-constructed value on a missing key --
     * matching std::unordered_map::operator[]'s convention, not .NET's. Real .NET's
     * Dictionary<TKey,TValue> indexer getter throws KeyNotFoundException unconditionally
     * on a missing key; only the setter inserts. This mirrors the same fix already applied
     * to ConcurrentDictionary::ValueProxy (see that class for the precedent).
     */
    class ValueProxy {
        Dictionary* owner_;
        TKey key_;
    public:
        ValueProxy(Dictionary* owner, const TKey& key) : owner_(owner), key_(key) {}

        /** Reads the current value; throws KeyNotFoundException if the key is absent. */
        operator const TValue&() const {
            auto it = owner_->map_.find(key_);
            if (it == owner_->map_.end())
                throw KeyNotFoundException("The given key was not present in the dictionary.");
            return it->second;
        }

        /**
         * Inserts or overwrites the value for this key. Matches real .NET's Dictionary
         * indexer setter exactly: bumps the version counter only when a NEW key is inserted,
         * not when an existing key's value is merely overwritten (verified against
         * Dictionary.cs's TryInsert -- the OverwriteExisting branch returns before its
         * _version++).
         */
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
     * C++ counterpart of .NET Dictionary<TKey,TValue> indexer. The getter throws
     * System::Collections::Generic::KeyNotFoundException if @p key is absent (it does NOT
     * insert a default, unlike std::unordered_map::operator[]); the setter inserts or
     * overwrites. See ValueProxy for why this isn't a plain `TValue&`.
     */
    [[nodiscard]] ValueProxy operator[](const TKey& key) { return ValueProxy(this, key); }

    /**
     * @brief Gets the value associated with the specified key (const).
     *
     * C++ counterpart of .NET Dictionary<TKey,TValue> indexer getter.
     * @throws System::Collections::Generic::KeyNotFoundException if the key is not found.
     */
    [[nodiscard]] const TValue& operator[](const TKey& key) const {
        auto it = map_.find(key);
        if (it == map_.end())
            throw KeyNotFoundException("The given key was not present in the dictionary.");
        return it->second;
    }

    using iterator = VersionCheckedIterator<typename std::unordered_map<TKey, TValue>::iterator>;
    using const_iterator = VersionCheckedIterator<typename std::unordered_map<TKey, TValue>::const_iterator>;

    /**
     * @brief Returns a version-checked iterator to the first element (for range-based for).
     * Throws System::InvalidOperationException from operator++/operator* if the dictionary is
     * structurally modified during iteration -- see the class doc-comment for the exact rules.
     */
    iterator begin()        { return iterator(this, map_.begin()); }
    /** @brief Returns a version-checked iterator past the last element (for range-based for). */
    iterator end()          { return iterator(this, map_.end()); }
    /** @brief Returns a const version-checked iterator to the first element (for range-based for). */
    [[nodiscard]] const_iterator begin() const { return const_iterator(this, map_.cbegin()); }
    /** @brief Returns a const version-checked iterator past the last element (for range-based for). */
    [[nodiscard]] const_iterator end()   const { return const_iterator(this, map_.cend()); }

    /**
     * @brief Returns a const reference to the underlying std::unordered_map.
     *
     * Provides direct STL interoperability when needed.
     */
    [[nodiscard]] const std::unordered_map<TKey, TValue>& ToMap() const { return map_; }

    /**
     * @brief Returns a reference to the underlying std::unordered_map.
     *
     * Provides direct STL interoperability when needed.
     */
    std::unordered_map<TKey, TValue>& ToMap() { return map_; }
};

} // namespace System::Collections::Generic
