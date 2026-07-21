// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Collections::Immutable {

using SharpRuntime::intcs;

/**
 * @brief Represents an immutable unordered set with no duplicate elements.
 *
 * C++ counterpart of .NET System.Collections.Immutable.ImmutableHashSet<T>.
 * Internally shares the underlying unordered_set via shared_ptr; mutations return new instances.
 *
 * The hash and equality functions are stored as std::function so a custom comparer
 * (WithComparer()) can be supplied at runtime without adding a comparer type parameter to
 * this class template -- matching real .NET, where IEqualityComparer<T> is a runtime object,
 * not a compile-time type parameter, and keeping every existing `ImmutableHashSet<T>` call
 * site (single type argument) source-compatible.
 *
 * @tparam T The type of elements in the set.
 */
template<typename T>
class ImmutableHashSet {
    using HashFn = std::function<size_t(const T&)>;
    using EqFn   = std::function<bool(const T&, const T&)>;
    using SetT   = std::unordered_set<T, HashFn, EqFn>;
    std::shared_ptr<const SetT> data_;

    explicit ImmutableHashSet(std::shared_ptr<const SetT> data) : data_(std::move(data)) {}

    /**
     * @brief Constructs an empty backing set using the given hash/equality functions.
     *
     * Centralizes every "fresh empty set" construction in this class so a comparer is
     * always supplied explicitly -- std::unordered_set's own default-constructed
     * std::function hasher/key_eq is empty and throws std::bad_function_call on first use,
     * so no code path in this class may construct a SetT without going through this helper
     * (or copy-constructing from an existing SetT, which already carries a valid comparer).
     */
    static std::shared_ptr<SetT> makeEmpty(HashFn hash, EqFn eq) {
        return std::make_shared<SetT>(0, std::move(hash), std::move(eq));
    }

    static HashFn defaultHash() { return HashFn(std::hash<T>{}); }
    static EqFn defaultEq() { return EqFn(std::equal_to<T>{}); }

public:
    /** @brief Default-constructs an empty ImmutableHashSet using the default hash/equality. */
    ImmutableHashSet() : data_(makeEmpty(defaultHash(), defaultEq())) {}

    /**
     * @brief Returns an empty ImmutableHashSet.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.Empty.
     * @return An empty ImmutableHashSet<T>.
     */
    static ImmutableHashSet<T> Empty() { return ImmutableHashSet<T>(); }

    /**
     * @brief Creates an ImmutableHashSet from an initializer list.
     *
     * C++ counterpart of .NET ImmutableHashSet.Create<T>(params T[] items).
     * @param items The initial elements.
     * @return A new ImmutableHashSet containing the given elements.
     */
    static ImmutableHashSet<T> Create(std::initializer_list<T> items) {
        auto s = makeEmpty(defaultHash(), defaultEq());
        s->insert(items);
        return ImmutableHashSet<T>(std::move(s));
    }

    /**
     * @brief Creates an ImmutableHashSet from an existing vector.
     *
     * C++ counterpart of .NET ImmutableHashSet.CreateRange<T>(IEnumerable<T>).
     * @param items The initial elements.
     * @return A new ImmutableHashSet containing the given elements.
     */
    static ImmutableHashSet<T> Create(const std::vector<T>& items) {
        auto s = makeEmpty(defaultHash(), defaultEq());
        s->insert(items.begin(), items.end());
        return ImmutableHashSet<T>(std::move(s));
    }

    /**
     * @brief Creates an empty ImmutableHashSet using a custom equality comparer.
     *
     * C++ counterpart of .NET ImmutableHashSet.Create<T>(IEqualityComparer<T>).
     * @param hash The hash function for elements.
     * @param equal The equality function for elements.
     * @return An empty ImmutableHashSet using the given comparer.
     */
    static ImmutableHashSet<T> Create(HashFn hash, EqFn equal) {
        return ImmutableHashSet<T>(makeEmpty(std::move(hash), std::move(equal)));
    }

    /**
     * @brief Creates an ImmutableHashSet from a vector using a custom equality comparer.
     *
     * C++ counterpart of .NET ImmutableHashSet.CreateRange<T>(IEqualityComparer<T>, IEnumerable<T>).
     * @param hash  The hash function for elements.
     * @param equal The equality function for elements.
     * @param items The initial elements.
     * @return A new ImmutableHashSet using the given comparer, containing @p items.
     */
    static ImmutableHashSet<T> Create(HashFn hash, EqFn equal, const std::vector<T>& items) {
        auto s = makeEmpty(std::move(hash), std::move(equal));
        s->insert(items.begin(), items.end());
        return ImmutableHashSet<T>(std::move(s));
    }

    /**
     * @brief Returns an equivalent set that uses the specified comparer instead.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.WithComparer(IEqualityComparer<T>).
     * All existing elements are re-hashed under the new comparer, so duplicates under the
     * new equality function collapse (matching real .NET's WithComparer semantics).
     * @param hash  The new hash function.
     * @param equal The new equality function.
     * @return A new ImmutableHashSet with the same elements under the new comparer.
     */
    [[nodiscard]] ImmutableHashSet<T> WithComparer(HashFn hash, EqFn equal) const {
        auto s = makeEmpty(std::move(hash), std::move(equal));
        s->insert(data_->begin(), data_->end());
        return ImmutableHashSet<T>(std::move(s));
    }

    /**
     * @brief Gets the number of elements in the set.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.Count.
     * @return The number of elements.
     */
    [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(data_->size()); }

    /**
     * @brief Gets a value indicating whether the set is empty.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.IsEmpty.
     * @return true if the set contains no elements; otherwise false.
     */
    [[nodiscard]] bool getIsEmptyProperty() const { return data_->empty(); }

    /**
     * @brief Determines whether the set contains the specified element.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.Contains(T).
     * @param item The element to locate.
     * @return true if found; otherwise false.
     */
    [[nodiscard]] bool Contains(const T& item) const {
        return data_->find(item) != data_->end();
    }

    /**
     * @brief Returns a new set with @p item added.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.Add(T).
     * @param item The element to add.
     * @return A new ImmutableHashSet with the element added (duplicate is ignored).
     */
    [[nodiscard]] ImmutableHashSet<T> Add(const T& item) const {
        auto s = std::make_shared<SetT>(*data_);
        s->insert(item);
        return ImmutableHashSet<T>(std::move(s));
    }

    /**
     * @brief Returns a new set with @p item removed.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.Remove(T).
     * @param item The element to remove.
     * @return A new ImmutableHashSet without the specified element.
     */
    [[nodiscard]] ImmutableHashSet<T> Remove(const T& item) const {
        auto s = std::make_shared<SetT>(*data_);
        s->erase(item);
        return ImmutableHashSet<T>(std::move(s));
    }

    /**
     * @brief Returns an empty ImmutableHashSet using this set's comparer.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.Clear().
     * @return An empty ImmutableHashSet<T> using the same comparer as this instance.
     */
    [[nodiscard]] ImmutableHashSet<T> Clear() const {
        return ImmutableHashSet<T>(makeEmpty(data_->hash_function(), data_->key_eq()));
    }

    /**
     * @brief Returns a new set containing all elements from both this set and @p other.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.Union(IEnumerable<T>).
     * @param other The set to union with.
     * @return A new ImmutableHashSet containing all elements from both sets.
     */
    [[nodiscard]] ImmutableHashSet<T> Union(const ImmutableHashSet<T>& other) const {
        auto s = std::make_shared<SetT>(*data_);
        for (const auto& x : *other.data_) s->insert(x);
        return ImmutableHashSet<T>(std::move(s));
    }

    /**
     * @brief Returns a new set containing only elements present in both sets.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.Intersect(IEnumerable<T>).
     * Verified against ImmutableHashSet_1.cs's private Intersect: iterates @p other's raw
     * elements and tests membership via *this* set's Contains (i.e. this set's comparer),
     * not @p other's -- so if the two sets use different comparers, the result and its
     * membership test are governed entirely by this set's equality notion.
     * @param other The set to intersect with.
     * @return A new ImmutableHashSet containing only common elements, using this set's comparer.
     */
    [[nodiscard]] ImmutableHashSet<T> Intersect(const ImmutableHashSet<T>& other) const {
        auto s = makeEmpty(data_->hash_function(), data_->key_eq());
        for (const auto& x : *other.data_) if (Contains(x)) s->insert(x);
        return ImmutableHashSet<T>(std::move(s));
    }

    /**
     * @brief Returns a new set containing elements in this set that are not in @p other.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.Except(IEnumerable<T>).
     * Verified against ImmutableHashSet_1.cs's private Except: starts from a copy of this
     * set and removes each of @p other's raw elements from it using this set's own
     * comparer (via erase, which always uses the container's own hasher/key_eq) -- not
     * @p other's comparer.
     * @param other The set whose elements to exclude.
     * @return A new ImmutableHashSet without the elements from @p other.
     */
    [[nodiscard]] ImmutableHashSet<T> Except(const ImmutableHashSet<T>& other) const {
        auto s = std::make_shared<SetT>(*data_);
        for (const auto& x : *other.data_) s->erase(x);
        return ImmutableHashSet<T>(std::move(s));
    }

    /**
     * @brief Returns a new set containing only elements present in exactly one of the two sets.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.SymmetricExcept(IEnumerable<T>).
     * Verified against ImmutableHashSet_1.cs's private SymmetricExcept: @p other is first
     * rehashed into a set under *this* set's comparer (`ImmutableHashSet.CreateRange(
     * origin.EqualityComparer, other)`) before the difference is computed. This port previously
     * toggled directly over @p other's raw elements (iterated under @p other's own comparer): if
     * two of @p other's elements are distinct under @p other's comparer but collapse to the same
     * logical element under *this* set's comparer, the toggle runs twice for that logical
     * element and cancels itself out, silently dropping it from the result. Confirmed via a
     * standalone repro (this: case-insensitive empty set; other: case-sensitive {"A","a"}) --
     * toggling over other's raw elements produced an empty result where real .NET (and the
     * rehash-first fix here) correctly produces {"A"}. Same bug class as the SetEquals fix
     * above, just for the toggle-based set operation instead of a size/membership comparison.
     * @param other The other set.
     * @return A new ImmutableHashSet with the symmetric difference.
     */
    [[nodiscard]] ImmutableHashSet<T> SymmetricExcept(const ImmutableHashSet<T>& other) const {
        auto otherRehashed = makeEmpty(data_->hash_function(), data_->key_eq());
        otherRehashed->insert(other.data_->begin(), other.data_->end());
        auto s = std::make_shared<SetT>(*data_);
        for (const auto& x : *otherRehashed) {
            if (s->count(x)) s->erase(x);
            else s->insert(x);
        }
        return ImmutableHashSet<T>(std::move(s));
    }

    /**
     * @brief Determines whether this set and @p other contain the same elements.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.SetEquals(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if both sets are equal; otherwise false.
     */
    [[nodiscard]] bool SetEquals(const ImmutableHashSet<T>& other) const {
        // Matches ImmutableHashSet_1.cs's SetEquals: when the two sets don't share a
        // comparer, `other`'s elements are rehashed under *this* set's comparer
        // (`new HashSet<T>(other, origin.EqualityComparer)`) before comparing -- the same
        // pattern IsSubsetOf/IsSupersetOf in this file already use, just missed here. The
        // previous version compared raw sizes and tested membership via `other`'s own
        // comparer, which gives a wrong result when the two sets' equality notions differ --
        // confirmed via a standalone repro before this fix (a case-insensitive {"Hello"} vs.
        // a case-sensitive {"HELLO","hello"}, which collapses to the identical single logical
        // element once rehashed case-insensitively, compared unequal). Comparing sizes AFTER
        // rehashing (not before) correctly handles this collapsing case.
        auto otherRehashed = makeEmpty(data_->hash_function(), data_->key_eq());
        otherRehashed->insert(other.data_->begin(), other.data_->end());
        if (otherRehashed->size() != data_->size()) return false;
        for (const auto& x : *data_) if (!otherRehashed->count(x)) return false;
        return true;
    }

    /**
     * @brief Determines whether every element of this set is in @p other.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.IsSubsetOf(IEnumerable<T>).
     * Verified against ImmutableHashSet_1.cs's private IsSubsetOf: rehashes @p other's raw
     * elements under *this* set's comparer first (`new HashSet<T>(other,
     * origin.EqualityComparer)`), then checks membership -- so the test is governed by
     * this set's equality notion, not @p other's.
     * @param other The set to compare to.
     * @return true if every element of this set is in @p other; otherwise false.
     */
    [[nodiscard]] bool IsSubsetOf(const ImmutableHashSet<T>& other) const {
        auto otherRehashed = makeEmpty(data_->hash_function(), data_->key_eq());
        otherRehashed->insert(other.data_->begin(), other.data_->end());
        for (const auto& x : *data_) if (!otherRehashed->count(x)) return false;
        return true;
    }

    /**
     * @brief Determines whether every element of @p other is in this set.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.IsSupersetOf(IEnumerable<T>).
     * Verified against ImmutableHashSet_1.cs's private IsSupersetOf: iterates @p other's
     * raw elements and tests membership via *this* set's Contains directly (this set's
     * comparer), unlike the delegate-to-IsSubsetOf shortcut this port used to take -- that
     * shortcut happened to produce the same comparer precedence, but is replaced here with
     * a direct implementation for clarity and to avoid relying on IsSubsetOf's own
     * (now rehashing) implementation detail.
     * @param other The set to compare to.
     * @return true if every element of @p other is in this set; otherwise false.
     */
    [[nodiscard]] bool IsSupersetOf(const ImmutableHashSet<T>& other) const {
        for (const auto& x : *other.data_) if (!Contains(x)) return false;
        return true;
    }

    /**
     * @brief Determines whether this set is a proper (strict) subset of @p other.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.IsProperSubsetOf(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if this set is a proper subset of @p other; otherwise false.
     */
    [[nodiscard]] bool IsProperSubsetOf(const ImmutableHashSet<T>& other) const {
        return IsSubsetOf(other) && !SetEquals(other);
    }

    /**
     * @brief Determines whether this set is a proper (strict) superset of @p other.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.IsProperSupersetOf(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if this set is a proper superset of @p other; otherwise false.
     */
    [[nodiscard]] bool IsProperSupersetOf(const ImmutableHashSet<T>& other) const {
        return IsSupersetOf(other) && !SetEquals(other);
    }

    /**
     * @brief Determines whether this set and @p other share any common elements.
     *
     * C++ counterpart of .NET ImmutableHashSet<T>.Overlaps(IEnumerable<T>).
     * @param other The set to compare to.
     * @return true if at least one element is common; otherwise false.
     */
    [[nodiscard]] bool Overlaps(const ImmutableHashSet<T>& other) const {
        for (const auto& x : *other.data_) if (Contains(x)) return true;
        return false;
    }

    /** @brief Returns a const iterator to the beginning of the set (STL interop). */
    auto begin() const { return data_->begin(); }
    /** @brief Returns a const iterator past the end of the set (STL interop). */
    auto end()   const { return data_->end(); }
};

} // namespace System::Collections::Immutable
