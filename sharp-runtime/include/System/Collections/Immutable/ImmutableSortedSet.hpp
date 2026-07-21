// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <memory>
#include <set>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Collections::Immutable {

using SharpRuntime::intcs;

/**
 * @brief Represents an immutable sorted set with no duplicate elements.
 *
 * C++ counterpart of .NET System.Collections.Immutable.ImmutableSortedSet<T>.
 * Backed by std::set; provides O(log n) Add, Remove, and Contains with sorted iteration.
 * All mutating operations return a new instance; the original is unchanged.
 *
 * The ordering function is stored as std::function so a custom comparer (WithComparer())
 * can be supplied at runtime without adding a comparer type parameter to this class
 * template -- matching real .NET, where IComparer<T> is a runtime object, not a
 * compile-time type parameter, and keeping every existing `ImmutableSortedSet<T>` call site
 * (single type argument) source-compatible.
 *
 * @tparam T The type of elements (must support operator< for the default ordering).
 */
template<typename T>
class ImmutableSortedSet {
    using CompareFn = std::function<bool(const T&, const T&)>;
    using SetT = std::set<T, CompareFn>;
    std::shared_ptr<const SetT> data_;

    explicit ImmutableSortedSet(std::shared_ptr<const SetT> data) : data_(std::move(data)) {}

    /**
     * @brief Constructs an empty backing set using the given ordering function.
     *
     * Centralizes every "fresh empty set" construction in this class so a comparer is
     * always supplied explicitly -- std::set's own default-constructed std::function
     * comparator is empty and throws std::bad_function_call on first use, so no code path
     * in this class may construct a SetT without going through this helper (or
     * copy-constructing from an existing SetT, which already carries a valid comparer).
     */
    static std::shared_ptr<SetT> makeEmpty(CompareFn less) {
        return std::make_shared<SetT>(std::move(less));
    }

    static CompareFn defaultLess() { return CompareFn(std::less<T>{}); }

public:
    /** @brief Default-constructs an empty ImmutableSortedSet using operator< ordering. */
    ImmutableSortedSet() : data_(makeEmpty(defaultLess())) {}

    /**
     * @brief Returns an empty ImmutableSortedSet.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.Empty.
     * @return An empty ImmutableSortedSet<T>.
     */
    static ImmutableSortedSet<T> Empty() { return ImmutableSortedSet<T>(); }

    /**
     * @brief Creates an ImmutableSortedSet from an initializer list.
     *
     * C++ counterpart of .NET ImmutableSortedSet.Create<T>(params T[] items).
     * @param items The initial elements.
     * @return A new ImmutableSortedSet containing the given elements.
     */
    static ImmutableSortedSet<T> Create(std::initializer_list<T> items) {
        auto s = makeEmpty(defaultLess());
        s->insert(items);
        return ImmutableSortedSet<T>(std::move(s));
    }

    /**
     * @brief Creates an ImmutableSortedSet from an existing vector.
     *
     * C++ counterpart of .NET ImmutableSortedSet.CreateRange<T>(IEnumerable<T>).
     * @param items The initial elements.
     * @return A new ImmutableSortedSet containing the given elements.
     */
    static ImmutableSortedSet<T> Create(const std::vector<T>& items) {
        auto s = makeEmpty(defaultLess());
        s->insert(items.begin(), items.end());
        return ImmutableSortedSet<T>(std::move(s));
    }

    /**
     * @brief Creates an empty ImmutableSortedSet using a custom ordering comparer.
     *
     * C++ counterpart of .NET ImmutableSortedSet.Create<T>(IComparer<T>).
     * @param less A strict-weak-ordering "less than" function for elements.
     * @return An empty ImmutableSortedSet using the given comparer.
     */
    static ImmutableSortedSet<T> Create(CompareFn less) {
        return ImmutableSortedSet<T>(makeEmpty(std::move(less)));
    }

    /**
     * @brief Creates an ImmutableSortedSet from a vector using a custom ordering comparer.
     *
     * C++ counterpart of .NET ImmutableSortedSet.CreateRange<T>(IComparer<T>, IEnumerable<T>).
     * @param less  A strict-weak-ordering "less than" function for elements.
     * @param items The initial elements.
     * @return A new ImmutableSortedSet using the given comparer, containing @p items.
     */
    static ImmutableSortedSet<T> Create(CompareFn less, const std::vector<T>& items) {
        auto s = makeEmpty(std::move(less));
        s->insert(items.begin(), items.end());
        return ImmutableSortedSet<T>(std::move(s));
    }

    /**
     * @brief Returns an equivalent set that uses the specified comparer instead.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.WithComparer(IComparer<T>).
     * @param less A strict-weak-ordering "less than" function for elements.
     * @return A new ImmutableSortedSet with the same elements under the new comparer.
     */
    [[nodiscard]] ImmutableSortedSet<T> WithComparer(CompareFn less) const {
        auto s = makeEmpty(std::move(less));
        s->insert(data_->begin(), data_->end());
        return ImmutableSortedSet<T>(std::move(s));
    }

    /**
     * @brief Gets the number of elements in the set.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.Count.
     * @return The number of elements.
     */
    [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(data_->size()); }

    /**
     * @brief Gets a value indicating whether the set is empty.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.IsEmpty.
     * @return true if the set contains no elements; otherwise false.
     */
    [[nodiscard]] bool getIsEmptyProperty() const { return data_->empty(); }

    /**
     * @brief Gets the smallest element in the set, or a default-constructed T if empty.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.Min, which returns default(T) rather
     * than throwing when the set is empty.
     */
    [[nodiscard]] T getMinProperty() const { return data_->empty() ? T{} : *data_->begin(); }

    /**
     * @brief Gets the largest element in the set, or a default-constructed T if empty.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.Max, which returns default(T) rather
     * than throwing when the set is empty.
     */
    [[nodiscard]] T getMaxProperty() const { return data_->empty() ? T{} : *data_->rbegin(); }

    /**
     * @brief Determines whether the set contains the specified element.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.Contains(T).
     * @param item The element to locate.
     * @return true if found; otherwise false.
     */
    [[nodiscard]] bool Contains(const T& item) const {
        return data_->find(item) != data_->end();
    }

    /**
     * @brief Searches for a value equal to @p equalValue and returns the stored value.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.TryGetValue(T, out T).
     * @param equalValue  The value to search for.
     * @param actualValue Receives the stored value if found.
     * @return true if a matching value was found; otherwise false.
     */
    bool TryGetValue(const T& equalValue, T& actualValue) const {
        auto it = data_->find(equalValue);
        if (it == data_->end()) return false;
        actualValue = *it;
        return true;
    }

    /**
     * @brief Returns a new set with @p item added.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.Add(T).
     * @param item The element to add.
     * @return A new ImmutableSortedSet with the element added (duplicate is ignored).
     */
    [[nodiscard]] ImmutableSortedSet<T> Add(const T& item) const {
        auto s = std::make_shared<SetT>(*data_);
        s->insert(item);
        return ImmutableSortedSet<T>(std::move(s));
    }

    /**
     * @brief Returns a new set with @p item removed.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.Remove(T).
     * @param item The element to remove.
     * @return A new ImmutableSortedSet without the specified element.
     */
    [[nodiscard]] ImmutableSortedSet<T> Remove(const T& item) const {
        auto s = std::make_shared<SetT>(*data_);
        s->erase(item);
        return ImmutableSortedSet<T>(std::move(s));
    }

    /**
     * @brief Returns an empty ImmutableSortedSet using this set's comparer.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.Clear().
     * @return An empty ImmutableSortedSet<T> using the same comparer as this instance.
     */
    [[nodiscard]] ImmutableSortedSet<T> Clear() const {
        return ImmutableSortedSet<T>(makeEmpty(data_->key_comp()));
    }

    /**
     * @brief Returns a new set containing all elements from both this set and @p other.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.Union(IEnumerable<T>).
     * @param other The set to union with.
     * @return A new ImmutableSortedSet containing all elements from both sets.
     */
    [[nodiscard]] ImmutableSortedSet<T> Union(const ImmutableSortedSet<T>& other) const {
        auto s = std::make_shared<SetT>(*data_);
        for (const auto& x : *other.data_) s->insert(x);
        return ImmutableSortedSet<T>(std::move(s));
    }

    /**
     * @brief Returns a new set containing only elements present in both sets.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.Intersect(IEnumerable<T>).
     * Verified against ImmutableSortedSet_1.cs's Intersect: iterates @p other's raw
     * elements and tests membership via *this* set's Contains (this set's comparer), not
     * @p other's -- so if the two sets use different comparers, the result and its
     * membership test are governed entirely by this set's ordering/equivalence notion.
     * @param other The set to intersect with.
     * @return A new ImmutableSortedSet containing only common elements, using this set's comparer.
     */
    [[nodiscard]] ImmutableSortedSet<T> Intersect(const ImmutableSortedSet<T>& other) const {
        auto s = makeEmpty(data_->key_comp());
        for (const auto& x : *other.data_) if (Contains(x)) s->insert(x);
        return ImmutableSortedSet<T>(std::move(s));
    }

    /**
     * @brief Returns a new set containing elements in this set that are not in @p other.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.Except(IEnumerable<T>).
     * Verified against ImmutableSortedSet_1.cs's Except: starts from this set's own tree
     * and removes each of @p other's raw elements using this set's own comparer (`_comparer`)
     * -- not @p other's comparer.
     * @param other The set whose elements to exclude.
     * @return A new ImmutableSortedSet without the elements from @p other.
     */
    [[nodiscard]] ImmutableSortedSet<T> Except(const ImmutableSortedSet<T>& other) const {
        auto s = std::make_shared<SetT>(*data_);
        for (const auto& x : *other.data_) s->erase(x);
        return ImmutableSortedSet<T>(std::move(s));
    }

    /**
     * @brief Returns a new set containing only elements present in exactly one of the two sets.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.SymmetricExcept(IEnumerable<T>).
     * Verified against ImmutableSortedSet_1.cs's SymmetricExcept: @p other is first rehashed
     * into a set under *this* set's comparer before the difference is computed -- same pattern
     * as SetEquals/IsSubsetOf above. This port previously toggled directly over @p other's raw
     * elements (ordered under @p other's own comparer): if two of @p other's elements are
     * distinct under @p other's comparer but equivalent under *this* set's comparer, the toggle
     * runs twice for that element and cancels out, silently dropping it from the result. Same
     * bug class as the SetEquals fix, just for the toggle-based operation; see
     * ImmutableHashSet::SymmetricExcept's comment for the confirmed repro (case-insensitive vs.
     * case-sensitive {"A","a"}) demonstrating the failure mode.
     * @param other The other set.
     * @return A new ImmutableSortedSet with the symmetric difference.
     */
    [[nodiscard]] ImmutableSortedSet<T> SymmetricExcept(const ImmutableSortedSet<T>& other) const {
        auto otherRehashed = makeEmpty(data_->key_comp());
        otherRehashed->insert(other.data_->begin(), other.data_->end());
        auto s = std::make_shared<SetT>(*data_);
        for (const auto& x : *otherRehashed) {
            if (s->count(x)) s->erase(x);
            else s->insert(x);
        }
        return ImmutableSortedSet<T>(std::move(s));
    }

    /**
     * @brief Determines whether this set and @p other contain the same elements.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.SetEquals(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if both sets are equal; otherwise false.
     */
    [[nodiscard]] bool SetEquals(const ImmutableSortedSet<T>& other) const {
        // Matches ImmutableSortedSet_1.cs's SetEquals: when the two sets don't share a
        // comparer, `other`'s elements are rehashed under *this* set's comparer
        // (`new SortedSet<T>(other, this.KeyComparer)`) before comparing -- the same pattern
        // already applied to IsSubsetOf/Intersect/Except in this file, just missed here. A
        // direct std::set::operator== compares elements position-wise after each set's own
        // internal ordering, which gives a wrong (false-negative) result when the two sets use
        // different comparers even though they contain the identical elements -- confirmed via
        // a standalone repro before this fix (ascending {1,2,3} vs. descending {1,2,3} compared
        // unequal). Comparing sizes AFTER rehashing (not before) also correctly catches the
        // rarer case where a different equivalence notion collapses distinct elements together.
        auto otherRehashed = makeEmpty(data_->key_comp());
        otherRehashed->insert(other.data_->begin(), other.data_->end());
        if (otherRehashed->size() != data_->size()) return false;
        return *data_ == *otherRehashed;
    }

    /**
     * @brief Determines whether every element of this set is in @p other.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.IsSubsetOf(IEnumerable<T>).
     * Verified against ImmutableSortedSet_1.cs's IsSubsetOf: rehashes @p other's raw
     * elements under *this* set's comparer first (`new SortedSet<T>(other,
     * this.KeyComparer)`), then checks membership -- so the test is governed by this set's
     * ordering/equivalence notion, not @p other's.
     * @param other The set to compare to.
     * @return true if every element of this set is in @p other; otherwise false.
     */
    [[nodiscard]] bool IsSubsetOf(const ImmutableSortedSet<T>& other) const {
        auto otherRehashed = makeEmpty(data_->key_comp());
        otherRehashed->insert(other.data_->begin(), other.data_->end());
        for (const auto& x : *data_) if (!otherRehashed->count(x)) return false;
        return true;
    }

    /**
     * @brief Determines whether every element of @p other is in this set.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.IsSupersetOf(IEnumerable<T>).
     * Verified against ImmutableSortedSet_1.cs's IsSupersetOf: iterates @p other's raw
     * elements and tests membership via *this* set's Contains directly (this set's
     * comparer), replacing the delegate-to-IsSubsetOf shortcut this port used to take.
     * @param other The set to compare to.
     * @return true if every element of @p other is in this set; otherwise false.
     */
    [[nodiscard]] bool IsSupersetOf(const ImmutableSortedSet<T>& other) const {
        for (const auto& x : *other.data_) if (!Contains(x)) return false;
        return true;
    }

    /**
     * @brief Determines whether this set is a proper (strict) subset of @p other.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.IsProperSubsetOf(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if this set is a proper subset of @p other; otherwise false.
     */
    [[nodiscard]] bool IsProperSubsetOf(const ImmutableSortedSet<T>& other) const {
        return IsSubsetOf(other) && !SetEquals(other);
    }

    /**
     * @brief Determines whether this set is a proper (strict) superset of @p other.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.IsProperSupersetOf(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if this set is a proper superset of @p other; otherwise false.
     */
    [[nodiscard]] bool IsProperSupersetOf(const ImmutableSortedSet<T>& other) const {
        return IsSupersetOf(other) && !SetEquals(other);
    }

    /**
     * @brief Determines whether this set and @p other share any common elements.
     *
     * C++ counterpart of .NET ImmutableSortedSet<T>.Overlaps(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if at least one element is common; otherwise false.
     */
    [[nodiscard]] bool Overlaps(const ImmutableSortedSet<T>& other) const {
        for (const auto& x : *other.data_) if (Contains(x)) return true;
        return false;
    }

    /** @brief Returns a const iterator to the beginning of the set (STL interop). */
    auto begin() const { return data_->begin(); }
    /** @brief Returns a const iterator past the end of the set (STL interop). */
    auto end()   const { return data_->end(); }
};

} // namespace System::Collections::Immutable
