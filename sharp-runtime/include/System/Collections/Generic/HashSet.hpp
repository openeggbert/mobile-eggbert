// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <unordered_set>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Represents a set of values with no duplicate elements.
 *
 * C++ counterpart of .NET System.Collections.Generic.HashSet<T>.
 * Backed by std::unordered_set; provides O(1) average-case Add, Remove, and Contains.
 *
 * @note begin()/end() return a version-checked iterator that throws
 * System::InvalidOperationException("Collection was modified; enumeration operation may not "
 * "execute.") if the set is structurally modified while iteration is in progress, matching
 * .NET's HashSet<T> enumerator fail-fast contract. Every mutating method here bumps the version
 * counter on an actual change, including Remove() and Clear() -- a deliberate, safety-motivated
 * deviation from real .NET's Dictionary<TKey,TValue>/HashSet<T> `_version` semantics (which
 * don't bump on Remove, since their array-backed entries use a free-list so index-based
 * enumeration is unaffected by a removal). This port's iterator wraps a real
 * std::unordered_set iterator, where erasing the element the iterator currently points at
 * invalidates that specific iterator (confirmed via an ASan use-after-free repro during this
 * fix's development for the analogous Dictionary<TKey,TValue> case) even though other iterators
 * remain valid per the standard -- bumping unconditionally on every successful mutation trades
 * exact `_version` parity (an extra InvalidOperationException real .NET wouldn't throw for a
 * same-position Remove during enumeration) for guaranteed memory safety.
 *
 * @tparam T The type of elements in the set.
 */
template<typename T>
class HashSet {
    std::unordered_set<T> set_;
    intcs version_ = 0;

    template<typename InnerIt>
    class VersionCheckedIterator {
        const HashSet* owner_;
        intcs version_;
        InnerIt it_;
    public:
        VersionCheckedIterator(const HashSet* owner, InnerIt it)
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
     * @brief Initializes a new empty HashSet.
     *
     * C++ counterpart of .NET HashSet<T>().
     */
    HashSet() = default;

    /**
     * @brief Adds the specified element to the set.
     *
     * C++ counterpart of .NET HashSet<T>.Add(T).
     * @param item The element to add.
     * @return true if the element was added; false if it was already present.
     */
    bool Add(const T& item) {
        bool added = set_.insert(item).second;
        if (added) ++version_;
        return added;
    }

    /**
     * @brief Adds the specified element to the set (move overload).
     *
     * C++ counterpart of .NET HashSet<T>.Add(T).
     * @param item The element to add (moved).
     * @return true if the element was added; false if it was already present.
     */
    bool Add(T&& item) {
        bool added = set_.insert(std::move(item)).second;
        if (added) ++version_;
        return added;
    }

    /**
     * @brief Removes the specified element from the set.
     *
     * C++ counterpart of .NET HashSet<T>.Remove(T).
     * @param item The element to remove.
     * @return true if the element was found and removed; otherwise false.
     */
    bool Remove(const T& item) {
        bool removed = set_.erase(item) > 0;
        if (removed) ++version_;
        return removed;
    }

    /**
     * @brief Determines whether the set contains the specified element.
     *
     * C++ counterpart of .NET HashSet<T>.Contains(T).
     * @param item The element to locate.
     * @return true if found; otherwise false.
     */
    [[nodiscard]] bool Contains(const T& item) const { return set_.count(item) > 0; }

    /**
     * @brief Attempts to get the actual stored value equal to the given value.
     *
     * C++ counterpart of .NET HashSet<T>.TryGetValue(T, out T).
     * Useful when the equality comparer distinguishes objects that compare equal.
     * @param equalValue  The value to search for.
     * @param actualValue Receives the stored element if found.
     * @return true if an equal element was found; otherwise false.
     */
    bool TryGetValue(const T& equalValue, T& actualValue) const {
        auto it = set_.find(equalValue);
        if (it == set_.end()) return false;
        actualValue = *it;
        return true;
    }

    /**
     * @brief Removes all elements matching the specified predicate.
     *
     * C++ counterpart of .NET HashSet<T>.RemoveWhere(Predicate<T>).
     * @param match Predicate that returns true for elements to remove.
     * @return The number of elements removed.
     */
    intcs RemoveWhere(const std::function<bool(const T&)>& match) {
        intcs removed = 0;
        for (auto it = set_.begin(); it != set_.end(); ) {
            if (match(*it)) { it = set_.erase(it); ++removed; }
            else ++it;
        }
        if (removed > 0) ++version_;
        return removed;
    }

    /**
     * @brief Determines whether this set and the specified collection share any common elements.
     *
     * C++ counterpart of .NET HashSet<T>.Overlaps(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if at least one element is common to both; otherwise false.
     */
    [[nodiscard]] bool Overlaps(const HashSet<T>& other) const {
        for (const auto& item : other.set_)
            if (Contains(item)) return true;
        return false;
    }

    /**
     * @brief Gets the number of elements contained in the set.
     *
     * C++ counterpart of .NET HashSet<T>.Count.
     */
    [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(set_.size()); }

    /**
     * @brief Removes all elements from the set.
     *
     * C++ counterpart of .NET HashSet<T>.Clear().
     */
    void Clear() { set_.clear(); ++version_; }

    /**
     * @brief Modifies the set to contain all elements present in itself or the specified collection.
     *
     * C++ counterpart of .NET HashSet<T>.UnionWith(IEnumerable<T>).
     * @param other The collection to union with.
     */
    void UnionWith(const HashSet<T>& other) {
        for (const auto& item : other.set_) set_.insert(item);
        ++version_;
    }

    /**
     * @brief Modifies the set to contain only elements also present in the specified collection.
     *
     * C++ counterpart of .NET HashSet<T>.IntersectWith(IEnumerable<T>).
     * @param other The collection to intersect with.
     */
    void IntersectWith(const HashSet<T>& other) {
        for (auto it = set_.begin(); it != set_.end(); ) {
            it = other.Contains(*it) ? ++it : set_.erase(it);
        }
        ++version_;
    }

    /**
     * @brief Removes all elements that are present in the specified collection.
     *
     * C++ counterpart of .NET HashSet<T>.ExceptWith(IEnumerable<T>).
     * @param other The collection of elements to remove.
     */
    void ExceptWith(const HashSet<T>& other) {
        // Real .NET explicitly special-cases `other == this` ("a set minus itself is the
        // empty set") before the general loop -- without it, iterating other.set_ while
        // erasing from set_ is erasing from the exact same std::unordered_set being iterated
        // whenever the caller passes the set itself (e.g. `s.ExceptWith(s)`), which is a real,
        // confirmed heap-use-after-free (ASan) since erase() invalidates the very iterator the
        // range-for loop is using to walk other.set_.
        if (&other == this) { set_.clear(); ++version_; return; }
        for (const auto& item : other.set_) set_.erase(item);
        ++version_;
    }

    /**
     * @brief Modifies the set to contain only elements present in exactly one of the two sets.
     *
     * C++ counterpart of .NET HashSet<T>.SymmetricExceptWith(IEnumerable<T>).
     * @param other The other set.
     */
    void SymmetricExceptWith(const HashSet<T>& other) {
        // Same self-aliasing hazard and fix as ExceptWith above: real .NET special-cases
        // `other == this` ("the symmetric difference of a set with itself is the empty set")
        // before the general loop, which here would otherwise erase from set_ while iterating
        // that exact same container via other.set_ -- confirmed as a genuine ASan
        // heap-use-after-free before this fix.
        if (&other == this) { set_.clear(); ++version_; return; }
        std::vector<T> toAdd;
        for (const auto& item : other.set_) {
            if (!Contains(item)) toAdd.push_back(item);
            else set_.erase(item);
        }
        for (const auto& item : toAdd) set_.insert(item);
        ++version_;
    }

    /**
     * @brief Determines whether this set and the specified collection contain exactly the same elements.
     *
     * C++ counterpart of .NET HashSet<T>.SetEquals(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if both sets contain the same elements; otherwise false.
     */
    [[nodiscard]] bool SetEquals(const HashSet<T>& other) const {
        if (set_.size() != other.set_.size()) return false;
        for (const auto& item : other.set_)
            if (!Contains(item)) return false;
        return true;
    }

    /**
     * @brief Determines whether the set is a subset of the specified collection.
     *
     * C++ counterpart of .NET HashSet<T>.IsSubsetOf(IEnumerable<T>).
     * @param other The collection to check against.
     * @return true if every element of this set is in other; otherwise false.
     */
    [[nodiscard]] bool IsSubsetOf(const HashSet<T>& other) const {
        for (const auto& item : set_)
            if (!other.Contains(item)) return false;
        return true;
    }

    /**
     * @brief Determines whether the set is a superset of the specified collection.
     *
     * C++ counterpart of .NET HashSet<T>.IsSupersetOf(IEnumerable<T>).
     * @param other The collection to check against.
     * @return true if every element of other is in this set; otherwise false.
     */
    [[nodiscard]] bool IsSupersetOf(const HashSet<T>& other) const {
        return other.IsSubsetOf(*this);
    }

    /**
     * @brief Determines whether the set is a proper (strict) subset of the specified collection.
     *
     * C++ counterpart of .NET HashSet<T>.IsProperSubsetOf(IEnumerable<T>).
     * @param other The collection to check against.
     * @return true if this set is a subset of other and not equal to it; otherwise false.
     */
    [[nodiscard]] bool IsProperSubsetOf(const HashSet<T>& other) const {
        return IsSubsetOf(other) && !SetEquals(other);
    }

    /**
     * @brief Determines whether the set is a proper (strict) superset of the specified collection.
     *
     * C++ counterpart of .NET HashSet<T>.IsProperSupersetOf(IEnumerable<T>).
     * @param other The collection to check against.
     * @return true if this set is a superset of other and not equal to it; otherwise false.
     */
    [[nodiscard]] bool IsProperSupersetOf(const HashSet<T>& other) const {
        return IsSupersetOf(other) && !SetEquals(other);
    }

    /**
     * @brief Copies all elements to a new vector.
     *
     * C++ counterpart of .NET HashSet<T>.CopyTo(T[]).
     * @return A new vector containing all elements.
     */
    [[nodiscard]] std::vector<T> ToArray() const {
        return std::vector<T>(set_.begin(), set_.end());
    }

    /**
     * @brief Ensures the bucket count supports at least capacity elements without rehashing.
     *
     * C++ counterpart of .NET HashSet<T>.EnsureCapacity(int), which returns the resulting
     * capacity (`_entries.Length` after the call, or the pre-existing capacity if it was
     * already >= the requested value) rather than void -- added to match that signature.
     * std::unordered_set has no direct equivalent of .NET's internal entries-array length, so
     * bucket_count() after reserve() is returned as the closest honest approximation.
     * @param capacity The minimum number of elements the set should support.
     * @return The resulting capacity.
     * @throws System::ArgumentOutOfRangeException if @p capacity is negative.
     */
    intcs EnsureCapacity(intcs capacity) {
        if (capacity < 0)
            throw System::ArgumentOutOfRangeException("capacity");
        set_.reserve(static_cast<std::size_t>(capacity));
        ++version_;
        return static_cast<intcs>(set_.bucket_count());
    }

    /**
     * @brief Reduces memory usage by shrinking the bucket array to fit the current element count.
     *
     * C++ counterpart of .NET HashSet<T>.TrimExcess().
     */
    void TrimExcess() { set_.rehash(set_.size()); ++version_; }

    using iterator = VersionCheckedIterator<typename std::unordered_set<T>::iterator>;
    using const_iterator = VersionCheckedIterator<typename std::unordered_set<T>::const_iterator>;

    /**
     * @brief Returns a version-checked iterator to the first element (for range-based for).
     * Throws System::InvalidOperationException from operator++/operator* if the set is
     * structurally modified during iteration -- see the class doc-comment for the exact rules.
     */
    iterator begin()       { return iterator(this, set_.begin()); }
    /** @brief Returns a version-checked iterator past the last element (for range-based for). */
    iterator end()         { return iterator(this, set_.end()); }
    /** @brief Returns a const version-checked iterator to the first element (for range-based for). */
    [[nodiscard]] const_iterator begin() const { return const_iterator(this, set_.begin()); }
    /** @brief Returns a const version-checked iterator past the last element (for range-based for). */
    [[nodiscard]] const_iterator end()   const { return const_iterator(this, set_.end()); }
};

} // namespace System::Collections::Generic
