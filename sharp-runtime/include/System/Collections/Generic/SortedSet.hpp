// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <set>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Represents a collection of objects maintained in sorted order with no duplicate elements.
 *
 * C++ counterpart of .NET System.Collections.Generic.SortedSet<T>.
 * Backed by std::set<T>; provides O(log n) Add, Remove, and Contains.
 *
 * begin()/end() (used by range-for) detect structural modification during iteration
 * via a version counter and throw System::InvalidOperationException, matching real
 * .NET's fail-fast enumerator contract (see ticket 1713).
 *
 * @tparam T The type of elements in the set (must support operator< for ordering).
 */
template<typename T>
class SortedSet {
    std::set<T> data_;
    intcs version_ = 0;

public:
    /**
     * @brief A version-checked wrapper over std::set<T>::iterator that throws
     * System::InvalidOperationException on dereference/increment if the owning
     * SortedSet was structurally modified since this iterator was obtained --
     * matching real .NET's fail-fast enumerator contract (ticket 1713).
     */
    class Iterator {
        typename std::set<T>::const_iterator it_;
        const SortedSet* owner_;
        intcs version_;

        void checkVersion() const {
            if (version_ != owner_->version_)
                throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
        }

    public:
        Iterator(typename std::set<T>::const_iterator it, const SortedSet* owner)
            : it_(it), owner_(owner), version_(owner->version_) {}

        const T& operator*() const { checkVersion(); return *it_; }
        const T* operator->() const { checkVersion(); return &*it_; }
        Iterator& operator++() { checkVersion(); ++it_; return *this; }
        bool operator==(const Iterator& other) const { return it_ == other.it_; }
        bool operator!=(const Iterator& other) const { return it_ != other.it_; }
    };

    /** @brief Initializes a new empty SortedSet. */
    SortedSet() = default;

    /**
     * @brief Initializes a SortedSet with elements from an initializer list.
     * @param items The initial elements.
     */
    explicit SortedSet(std::initializer_list<T> items) : data_(items) {}

    /**
     * @brief Gets the number of elements in the SortedSet.
     *
     * C++ counterpart of .NET SortedSet<T>.Count.
     * @return The number of elements.
     */
    [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(data_.size()); }

    /**
     * @brief Gets a value indicating whether the SortedSet contains no elements.
     * @return true if the set is empty; otherwise false.
     */
    [[nodiscard]] bool getIsEmptyProperty() const { return data_.empty(); }

    /**
     * @brief Gets the minimum value in the SortedSet, or a default-constructed T if empty.
     *
     * C++ counterpart of .NET SortedSet<T>.Min, which returns default(T) (.NET's nullable
     * T?) rather than throwing when the set is empty.
     */
    [[nodiscard]] T getMinProperty() const { return data_.empty() ? T{} : *data_.begin(); }

    /**
     * @brief Gets the maximum value in the SortedSet, or a default-constructed T if empty.
     *
     * C++ counterpart of .NET SortedSet<T>.Max, which returns default(T) (.NET's nullable
     * T?) rather than throwing when the set is empty.
     */
    [[nodiscard]] T getMaxProperty() const { return data_.empty() ? T{} : *data_.rbegin(); }

    /**
     * @brief Adds the specified element to the set.
     *
     * C++ counterpart of .NET SortedSet<T>.Add(T).
     * @param item The element to add.
     * @return true if the element was added; false if it was already present.
     */
    bool Add(const T& item) {
        bool added = data_.insert(item).second;
        if (added) ++version_;
        return added;
    }

    /**
     * @brief Removes the specified element from the set.
     *
     * C++ counterpart of .NET SortedSet<T>.Remove(T).
     * @param item The element to remove.
     * @return true if the element was found and removed; otherwise false.
     */
    bool Remove(const T& item) {
        bool removed = data_.erase(item) > 0;
        if (removed) ++version_;
        return removed;
    }

    /**
     * @brief Determines whether the set contains the specified element.
     *
     * C++ counterpart of .NET SortedSet<T>.Contains(T).
     * @param item The element to locate.
     * @return true if found; otherwise false.
     */
    [[nodiscard]] bool Contains(const T& item) const {
        return data_.find(item) != data_.end();
    }

    /**
     * @brief Removes all elements from the set.
     *
     * C++ counterpart of .NET SortedSet<T>.Clear().
     */
    void Clear() { data_.clear(); ++version_; }

    /**
     * @brief Modifies the set to contain all elements in itself and the specified set.
     *
     * C++ counterpart of .NET SortedSet<T>.UnionWith(IEnumerable<T>).
     * @param other The set of elements to add.
     */
    void UnionWith(const SortedSet<T>& other) {
        for (const auto& x : other.data_) data_.insert(x);
        ++version_;
    }

    /**
     * @brief Modifies the set to contain only elements also present in the specified set.
     *
     * C++ counterpart of .NET SortedSet<T>.IntersectWith(IEnumerable<T>).
     * @param other The set to intersect with.
     */
    void IntersectWith(const SortedSet<T>& other) {
        std::set<T> result;
        for (const auto& x : data_) if (other.Contains(x)) result.insert(x);
        data_ = std::move(result);
        ++version_;
    }

    /**
     * @brief Removes all elements in the specified set from the current set.
     *
     * C++ counterpart of .NET SortedSet<T>.ExceptWith(IEnumerable<T>).
     * @param other The set of elements to remove.
     */
    void ExceptWith(const SortedSet<T>& other) {
        // Same self-aliasing hazard as System::Collections::Generic::HashSet<T>::ExceptWith
        // (ticket 324): without this guard, `s.ExceptWith(s)` iterates other.data_ while
        // erasing from the exact same std::set, using an iterator the standard says is
        // invalidated by that very erase call -- real UB even where it doesn't reliably crash
        // under a given std::set implementation. Real .NET's SortedSet<T>.ExceptWith has the
        // identical `if (other == this) { Clear(); return; }` special case.
        if (&other == this) { data_.clear(); ++version_; return; }
        for (const auto& x : other.data_) data_.erase(x);
        ++version_;
    }

    /**
     * @brief Modifies the set so it contains only elements present in one set but not both.
     *
     * C++ counterpart of .NET SortedSet<T>.SymmetricExceptWith(IEnumerable<T>).
     * @param other The set to compare to.
     */
    void SymmetricExceptWith(const SortedSet<T>& other) {
        // Same self-aliasing hazard and fix as ExceptWith above -- real .NET's
        // SortedSet<T>.SymmetricExceptWith also special-cases `other == this`.
        if (&other == this) { data_.clear(); ++version_; return; }
        for (const auto& x : other.data_) {
            auto it = data_.find(x);
            if (it != data_.end()) data_.erase(it);
            else data_.insert(x);
        }
        ++version_;
    }

    /**
     * @brief Determines whether the set is a subset of the specified set.
     *
     * C++ counterpart of .NET SortedSet<T>.IsSubsetOf(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if every element of this set is in @p other; otherwise false.
     */
    [[nodiscard]] bool IsSubsetOf(const SortedSet<T>& other) const {
        for (const auto& x : data_) if (!other.Contains(x)) return false;
        return true;
    }

    /**
     * @brief Determines whether the set is a superset of the specified set.
     *
     * C++ counterpart of .NET SortedSet<T>.IsSupersetOf(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if every element of @p other is in this set; otherwise false.
     */
    [[nodiscard]] bool IsSupersetOf(const SortedSet<T>& other) const {
        return other.IsSubsetOf(*this);
    }

    /**
     * @brief Determines whether the set is a proper (strict) subset of the specified set.
     *
     * C++ counterpart of .NET SortedSet<T>.IsProperSubsetOf(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if this set is a subset of @p other and @p other has more elements.
     */
    [[nodiscard]] bool IsProperSubsetOf(const SortedSet<T>& other) const {
        return IsSubsetOf(other) && !SetEquals(other);
    }

    /**
     * @brief Determines whether the set is a proper (strict) superset of the specified set.
     *
     * C++ counterpart of .NET SortedSet<T>.IsProperSupersetOf(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if this set is a superset of @p other and has more elements.
     */
    [[nodiscard]] bool IsProperSupersetOf(const SortedSet<T>& other) const {
        return IsSupersetOf(other) && !SetEquals(other);
    }

    /**
     * @brief Determines whether the set and the specified set contain the same elements.
     *
     * C++ counterpart of .NET SortedSet<T>.SetEquals(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if the two sets are equal; otherwise false.
     */
    [[nodiscard]] bool SetEquals(const SortedSet<T>& other) const {
        return data_ == other.data_;
    }

    /**
     * @brief Determines whether the set and the specified set share any common elements.
     *
     * C++ counterpart of .NET SortedSet<T>.Overlaps(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if at least one element is common to both; otherwise false.
     */
    [[nodiscard]] bool Overlaps(const SortedSet<T>& other) const {
        for (const auto& x : other.data_)
            if (data_.find(x) != data_.end()) return true;
        return false;
    }

    /**
     * @brief Returns a view of elements in the inclusive range [@p lower, @p upper].
     *
     * C++ counterpart of .NET SortedSet<T>.GetViewBetween(T, T).
     *
     * @warning KNOWN DIVERGENCE FROM .NET, audited and left as documented rather than fixed:
     * real .NET's GetViewBetween returns `new TreeSubSet(this, lowerValue, upperValue, ...)`, a
     * genuine bidirectional LIVE VIEW backed by the SAME underlying tree as the original set --
     * elements added to or removed from the view (within [lower, upper]) are reflected in the
     * original set and vice versa. This port instead returns an independent COPY: mutating the
     * returned SortedSet<T> has no effect on the original, and mutating the original after
     * calling GetViewBetween is not reflected in the already-returned copy either. A faithful
     * live view is not achievable on top of std::set (which has no notion of a mutable,
     * bounded, write-through sub-range view) without replacing this type's entire internal
     * representation with a hand-rolled tree structure matching .NET's own -- an architectural
     * change far beyond a single audit ticket's scope, so this is documented rather than
     * attempted. Ported C# code that relies on GetViewBetween's write-through semantics (e.g.
     * `set.GetViewBetween(a, b).Add(x)` expecting `x` to also appear in the original set) will
     * silently behave differently here.
     * @param lower The minimum value (inclusive).
     * @param upper The maximum value (inclusive).
     * @return A new, independent SortedSet<T> containing a snapshot of elements in the range.
     * @throws System::ArgumentException if @p lower is greater than @p upper.
     */
    [[nodiscard]] SortedSet<T> GetViewBetween(const T& lower, const T& upper) const {
        if (lower > upper)
            throw System::ArgumentException("lowerValue is greater than upperValue.", "lowerValue");
        SortedSet<T> view;
        for (auto it = data_.lower_bound(lower); it != data_.end() && !(*it > upper); ++it)
            view.Add(*it);
        return view;
    }

    /**
     * @brief Copies the set elements to a new vector in sorted order.
     *
     * C++ counterpart of .NET SortedSet<T>.CopyTo(T[]).
     * @return A std::vector<T> containing all elements in ascending order.
     */
    [[nodiscard]] std::vector<T> ToVector() const {
        return std::vector<T>(data_.begin(), data_.end());
    }

    /**
     * @brief Returns a version-checked iterator to the beginning of the set (range-for).
     * @throws System::InvalidOperationException from dereference/increment if the set is
     *         structurally modified while this iterator is in use.
     */
    Iterator begin() const { return Iterator(data_.cbegin(), this); }
    /** @brief Returns a version-checked iterator past the end of the set (range-for). */
    Iterator end()   const { return Iterator(data_.cend(), this); }
};

} // namespace System::Collections::Generic
