// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <functional>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"
#include "System/Collections/Generic/KeyValuePair.hpp"
#include "System/InvalidOperationException.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Represents a collection of key/value pairs that are accessible by key or index,
 *        preserving insertion order.
 *
 * C++ counterpart of .NET System.Collections.Generic.OrderedDictionary<TKey,TValue> (.NET 9+).
 * Backed by a std::vector (preserves order) and an std::unordered_map (O(1) key lookup).
 * Remove and RemoveAt are O(n) due to index rebuilding.
 *
 * begin()/end() (used by range-for) detect structural modification during iteration
 * via a version counter and throw System::InvalidOperationException, matching real
 * .NET's fail-fast enumerator contract (see ticket 1713) -- important here since
 * `entries_` is a std::vector, so a mutation mid-iteration can reallocate and leave a
 * raw iterator genuinely dangling, not just semantically stale.
 *
 * @tparam TKey   The type of keys in the dictionary.
 * @tparam TValue The type of values in the dictionary.
 */
template<typename TKey, typename TValue>
class OrderedDictionary {
    std::vector<std::pair<TKey, TValue>>       entries_;
    std::unordered_map<TKey, std::size_t>      keyIndex_;
    intcs version_ = 0;

    // Builds the replacement index in a local temporary and swaps it in only after every
    // insertion succeeds (audit finding A-05, 2026-07-14) -- previously cleared keyIndex_
    // FIRST and repopulated it in place, so a throw partway through (e.g. a user-supplied
    // std::hash<TKey>, a throwing key copy, or an allocation failure) left keyIndex_
    // partially populated and inconsistent with entries_, confirmed via a standalone repro.
    // unordered_map::swap is noexcept for the default (always-equal) allocator this class
    // uses, so this call is strongly exception-safe: either keyIndex_ becomes the fully
    // rebuilt index, or (if the loop above throws) keyIndex_ is left completely unchanged.
    // Callers that mutate entries_ before calling this must roll that mutation back on a
    // caught exception -- see Insert/Remove/RemoveAt.
    void rebuildIndex() {
        std::unordered_map<TKey, std::size_t> newIndex;
        newIndex.reserve(entries_.size());
        for (std::size_t i = 0; i < entries_.size(); ++i)
            newIndex[entries_[i].first] = i;
        keyIndex_.swap(newIndex);
    }

public:
    /**
     * @brief A version-checked wrapper over the backing vector's iterator that throws
     * System::InvalidOperationException on dereference/increment if the owning
     * OrderedDictionary was structurally modified since this iterator was obtained --
     * matching real .NET's fail-fast enumerator contract (ticket 1713). Also guards
     * against genuine iterator invalidation from vector reallocation, which the raw
     * STL iterator this replaces could not.
     */
    class Iterator {
        typename std::vector<std::pair<TKey, TValue>>::const_iterator it_;
        const OrderedDictionary* owner_;
        intcs version_;
        std::size_t index_;

        void checkVersion() const {
            if (version_ != owner_->version_)
                throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
        }

    public:
        Iterator(std::size_t index, const OrderedDictionary* owner)
            : it_(), owner_(owner), version_(owner->version_), index_(index) {}

        const std::pair<TKey, TValue>& operator*() const {
            checkVersion();
            return owner_->entries_[index_];
        }
        const std::pair<TKey, TValue>* operator->() const {
            checkVersion();
            return &owner_->entries_[index_];
        }
        Iterator& operator++() { checkVersion(); ++index_; return *this; }
        bool operator==(const Iterator& other) const { return index_ == other.index_; }
        bool operator!=(const Iterator& other) const { return index_ != other.index_; }
    };

    /** @brief Initializes a new empty OrderedDictionary. */
    OrderedDictionary() = default;

    /**
     * @brief Initializes a new empty OrderedDictionary with the specified initial capacity.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>(int capacity).
     * @param capacity The initial number of elements the dictionary can hold without resizing.
     * @throws System::ArgumentOutOfRangeException if @p capacity is negative.
     */
    explicit OrderedDictionary(intcs capacity) {
        if (capacity < 0)
            throw System::ArgumentOutOfRangeException("capacity");
        entries_.reserve(static_cast<std::size_t>(capacity));
        keyIndex_.reserve(static_cast<std::size_t>(capacity));
    }

    /**
     * @brief Gets the number of key/value pairs in the dictionary.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.Count.
     * @return The number of elements.
     */
    [[nodiscard]] intcs getCountProperty() const {
        return static_cast<intcs>(entries_.size());
    }

    /**
     * @brief Gets the value associated with the specified key.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.this[TKey] getter.
     * @param key The key whose value to get.
     * @return A const reference to the associated value.
     * @throws System::Collections::Generic::KeyNotFoundException if the key is not found.
     */
    [[nodiscard]] const TValue& operator[](const TKey& key) const {
        auto it = keyIndex_.find(key);
        if (it == keyIndex_.end()) throw KeyNotFoundException("The given key was not present in the dictionary.");
        return entries_[it->second].second;
    }

    /**
     * @brief A proxy for `dict[key]` that throws on read of a missing key while still
     * supporting `dict[key] = value` to insert-or-update, matching .NET's split get/set
     * indexer semantics on top of a single C++ `operator[]`.
     *
     * Fixed 2026-07-14 (duplicated-implementation audit finding): the previous non-const
     * `operator[]` returned a plain `TValue&`, which cannot distinguish a read from a
     * write -- so `orderedDict[missingKey]` used purely as a READ silently inserted a
     * default-constructed value instead of throwing `KeyNotFoundException`, unlike every
     * sibling dictionary type in this codebase (`Dictionary`/`SortedDictionary`/
     * `SortedList`/`ConcurrentDictionary` all already have this exact fix -- see
     * `Dictionary::ValueProxy` for the precedent this mirrors). Real .NET's
     * `OrderedDictionary<TKey,TValue>` indexer getter and setter are genuinely separate
     * accessor methods (`get_Item`/`set_Item`), so only the setter upserts.
     */
    class ValueProxy {
        OrderedDictionary* owner_;
        TKey key_;
    public:
        ValueProxy(OrderedDictionary* owner, const TKey& key) : owner_(owner), key_(key) {}

        /** Reads the current value; throws KeyNotFoundException if the key is absent. */
        operator const TValue&() const {
            auto it = owner_->keyIndex_.find(key_);
            if (it == owner_->keyIndex_.end())
                throw KeyNotFoundException("The given key was not present in the dictionary.");
            return owner_->entries_[it->second].second;
        }

        /**
         * Inserts or overwrites the value for this key, adding at the end if new. Bumps
         * the version counter only when a NEW key is inserted, matching
         * `Dictionary::ValueProxy::operator=`'s identical convention.
         */
        ValueProxy& operator=(const TValue& value) {
            auto it = owner_->keyIndex_.find(key_);
            if (it != owner_->keyIndex_.end()) {
                owner_->entries_[it->second].second = value;
                return *this;
            }
            // Mutate entries_ (the operation more likely to throw -- the pair's move/copy
            // into the vector) BEFORE keyIndex_ -- see the class's own prior exception-
            // safety note (a genuine ASan heap-buffer-overflow was confirmed here via a
            // standalone repro using a TValue whose copy constructor throws, before that
            // fix). This order is exception-neutral: if emplace_back throws, keyIndex_ is
            // untouched and the dictionary is unchanged.
            std::size_t newIndex = owner_->entries_.size();
            owner_->entries_.emplace_back(key_, value);
            try {
                owner_->keyIndex_[key_] = newIndex;
            } catch (...) {
                // Roll back the vector append so entries_ and keyIndex_ stay consistent
                // (audit finding A-05, 2026-07-14) -- e.g. a throwing std::hash<TKey> during
                // map insertion used to leave entries_ with the new item but keyIndex_
                // without it, so Count reported one more element than ContainsKey(key)
                // could ever find. pop_back() is noexcept, so this rollback cannot itself
                // throw a second exception.
                owner_->entries_.pop_back();
                throw;
            }
            ++owner_->version_;
            return *this;
        }
    };

    /**
     * @brief Gets or sets the value associated with the specified key.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.this[TKey]. The getter
     * throws System::Collections::Generic::KeyNotFoundException if @p key is absent (it
     * does NOT insert a default); the setter inserts (at the end) or overwrites. See
     * ValueProxy for why this isn't a plain `TValue&`.
     * @param key The key whose value to get or set.
     */
    [[nodiscard]] ValueProxy operator[](const TKey& key) { return ValueProxy(this, key); }

    /**
     * @brief Gets the key/value pair at the specified index.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.GetAt(int).
     * @param index The zero-based index of the element.
     * @return The KeyValuePair at the specified index.
     * @throws System::ArgumentOutOfRangeException if the index is out of range.
     */
    [[nodiscard]] KeyValuePair<TKey, TValue> GetAt(intcs index) const {
        if (index < 0 || static_cast<std::size_t>(index) >= entries_.size())
            throw System::ArgumentOutOfRangeException("index");
        auto i = static_cast<std::size_t>(index);
        return {entries_[i].first, entries_[i].second};
    }

    /**
     * @brief Adds the specified key and value to the dictionary.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.Add(TKey, TValue).
     * @param key   The key of the element to add.
     * @param value The value of the element to add.
     * @throws System::ArgumentException if the key already exists.
     */
    void Add(const TKey& key, const TValue& value) {
        if (keyIndex_.count(key)) throw System::ArgumentException("An item with the same key has already been added.");
        // See operator[]'s comment: entries_ must be mutated before keyIndex_ is updated,
        // so a throwing TKey/TValue copy leaves keyIndex_ untouched instead of desynced.
        std::size_t newIndex = entries_.size();
        entries_.emplace_back(key, value);
        try {
            keyIndex_[key] = newIndex;
        } catch (...) {
            // Roll back the append (audit finding A-05, 2026-07-14) -- see ValueProxy::
            // operator='s identical rollback for the full rationale.
            entries_.pop_back();
            throw;
        }
        ++version_;
    }

    /**
     * @brief Attempts to add the specified key and value to the dictionary.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.TryAdd(TKey, TValue).
     * @param key   The key of the element to add.
     * @param value The value of the element to add.
     * @return true if the element was added; false if the key already existed.
     */
    bool TryAdd(const TKey& key, const TValue& value) {
        if (keyIndex_.count(key)) return false;
        // See operator[]'s comment: entries_ must be mutated before keyIndex_ is updated.
        std::size_t newIndex = entries_.size();
        entries_.emplace_back(key, value);
        try {
            keyIndex_[key] = newIndex;
        } catch (...) {
            // Roll back the append (audit finding A-05, 2026-07-14) -- see ValueProxy::
            // operator='s identical rollback for the full rationale.
            entries_.pop_back();
            throw;
        }
        ++version_;
        return true;
    }

    /**
     * @brief Determines whether the dictionary contains an element with the specified key.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.ContainsKey(TKey).
     * @param key The key to locate.
     * @return true if the key is found; otherwise false.
     */
    [[nodiscard]] bool ContainsKey(const TKey& key) const {
        return keyIndex_.count(key) > 0;
    }

    /**
     * @brief Determines whether the dictionary contains an element with the specified value.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.ContainsValue(TValue).
     * @param value The value to locate.
     * @return true if the value is found in any entry; otherwise false.
     */
    [[nodiscard]] bool ContainsValue(const TValue& value) const {
        for (const auto& e : entries_)
            if (e.second == value) return true;
        return false;
    }

    /**
     * @brief Gets the value associated with the specified key.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.TryGetValue(TKey, out TValue).
     * @param key   The key to locate.
     * @param value Receives the value if the key is found.
     * @return true if the key was found; otherwise false.
     */
    [[nodiscard]] bool TryGetValue(const TKey& key, TValue& value) const {
        auto it = keyIndex_.find(key);
        if (it == keyIndex_.end()) return false;
        value = entries_[it->second].second;
        return true;
    }

    /**
     * @brief Returns the zero-based index of the specified key, or -1 if not found.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.IndexOf(TKey).
     * @param key The key to locate.
     * @return The insertion-order index, or -1 if not present.
     */
    [[nodiscard]] intcs IndexOf(const TKey& key) const {
        auto it = keyIndex_.find(key);
        if (it == keyIndex_.end()) return -1;
        return static_cast<intcs>(it->second);
    }

    /**
     * @brief Inserts a key/value pair at the specified index.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.Insert(int, TKey, TValue).
     * @param index The zero-based index at which to insert.
     * @param key   The key of the element to insert.
     * @param value The value of the element to insert.
     * @throws System::ArgumentException if the key already exists.
     * @throws System::ArgumentOutOfRangeException if the index is out of range.
     */
    void Insert(intcs index, const TKey& key, const TValue& value) {
        if (index < 0 || static_cast<std::size_t>(index) > entries_.size())
            throw System::ArgumentOutOfRangeException("index");
        if (keyIndex_.count(key)) throw System::ArgumentException("An item with the same key has already been added.");
        entries_.insert(entries_.begin() + index, {key, value});
        try {
            rebuildIndex();
        } catch (...) {
            // Roll back the vector insertion so entries_ and keyIndex_ stay consistent
            // (audit finding A-05, 2026-07-14) -- see rebuildIndex()'s own doc-comment:
            // keyIndex_ is left completely unchanged by the failed call, so it still
            // matches the original (pre-insert) entries_ once this erase restores it.
            entries_.erase(entries_.begin() + index);
            throw;
        }
        ++version_;
    }

    /**
     * @brief Updates the value of the element at the specified index.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.SetAt(int, TValue).
     * @param index The zero-based index of the element to update.
     * @param value The new value.
     * @throws System::ArgumentOutOfRangeException if the index is out of range.
     */
    void SetAt(intcs index, const TValue& value) {
        if (index < 0 || static_cast<std::size_t>(index) >= entries_.size())
            throw System::ArgumentOutOfRangeException("index");
        entries_[static_cast<std::size_t>(index)].second = value;
        ++version_;
    }

    /**
     * @brief Removes the element with the specified key from the dictionary.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.Remove(TKey).
     * @param key The key of the element to remove.
     * @return true if the element was found and removed; otherwise false.
     */
    bool Remove(const TKey& key) {
        auto it = keyIndex_.find(key);
        if (it == keyIndex_.end()) return false;
        auto pos = static_cast<std::ptrdiff_t>(it->second);
        // Copy the entry before erasing (audit finding A-05, 2026-07-14) so it can be
        // restored if rebuildIndex() below throws; if this copy itself throws, nothing has
        // been mutated yet and the dictionary is left unchanged.
        auto erased = entries_[static_cast<std::size_t>(pos)];
        entries_.erase(entries_.begin() + pos);
        try {
            rebuildIndex();
        } catch (...) {
            // Roll back the erase so entries_ and keyIndex_ stay consistent -- see
            // rebuildIndex()'s own doc-comment: keyIndex_ is left completely unchanged by
            // the failed call, so it still matches the original (pre-erase) entries_ once
            // this re-insertion restores it.
            entries_.insert(entries_.begin() + pos, erased);
            throw;
        }
        ++version_;
        return true;
    }

    /**
     * @brief Removes the element at the specified index from the dictionary.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.RemoveAt(int).
     * @param index The zero-based index of the element to remove.
     * @throws System::ArgumentOutOfRangeException if the index is out of range.
     */
    void RemoveAt(intcs index) {
        if (index < 0 || static_cast<std::size_t>(index) >= entries_.size())
            throw System::ArgumentOutOfRangeException("index");
        // See Remove()'s identical comment on why the entry is copied before erasing (audit
        // finding A-05, 2026-07-14).
        auto erased = entries_[static_cast<std::size_t>(index)];
        entries_.erase(entries_.begin() + index);
        try {
            rebuildIndex();
        } catch (...) {
            entries_.insert(entries_.begin() + index, erased);
            throw;
        }
        ++version_;
    }

    /**
     * @brief Removes all key/value pairs from the dictionary.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.Clear().
     */
    void Clear() {
        entries_.clear();
        keyIndex_.clear();
        ++version_;
    }

    /**
     * @brief Ensures the internal storage can hold at least @p capacity elements.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.EnsureCapacity(int).
     * @param capacity The minimum capacity to ensure.
     * @return The new capacity.
     * @throws System::ArgumentOutOfRangeException if @p capacity is negative.
     *
     * @note Bumps version_ only when capacity actually grows -- matching real .NET's own
     * `if (Capacity < capacity) { ...; _version++; }` exactly (verified against
     * OrderedDictionary.cs), fixed 2026-07-14 (duplicated-implementation audit finding: this
     * method previously never bumped version_ at all, unlike its sibling
     * `Dictionary::EnsureCapacity`, which always bumps -- a fail-fast contract gap versus both
     * real .NET and this codebase's own established pattern).
     */
    intcs EnsureCapacity(intcs capacity) {
        if (capacity < 0)
            throw System::ArgumentOutOfRangeException("capacity");
        auto cap = static_cast<std::size_t>(capacity);
        if (entries_.capacity() < cap) {
            entries_.reserve(cap);
            ++version_;
        }
        return static_cast<intcs>(entries_.capacity());
    }

    /**
     * @brief Reduces the internal storage capacity to fit the current element count.
     *
     * C++ counterpart of .NET OrderedDictionary<TKey,TValue>.TrimExcess().
     */
    void TrimExcess() { entries_.shrink_to_fit(); }

    /**
     * @brief Returns a version-checked iterator to the first entry (range-for).
     * @throws System::InvalidOperationException from dereference/increment if the
     *         dictionary is structurally modified while this iterator is in use.
     */
    Iterator begin() const { return Iterator(0, this); }
    /** @brief Returns a version-checked iterator past the last entry (range-for). */
    Iterator end()   const { return Iterator(entries_.size(), this); }
};

} // namespace System::Collections::Generic
