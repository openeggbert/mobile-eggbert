// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Collections/Generic/ICollection.hpp"

namespace System::Collections::Generic {

/**
 * @brief Provides the base interface for the abstraction of sets.
 *
 * C++ counterpart of .NET System.Collections.Generic.ISet<T>.
 * Extends ICollection<T> with set-mutation operations (union, intersection, etc.)
 * and set-relation queries.
 *
 * @tparam T The type of elements in the set.
 */
template<typename T>
class ISet : public ICollection<T> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~ISet() = default;

    /**
     * @brief Adds an element to the set and returns whether it was newly inserted.
     *
     * C++ counterpart of .NET ISet<T>.Add(T).
     * Unlike ICollection<T>.Add which returns void, this returns a bool indicating
     * whether the element was added (true) or was already present (false).
     * @param item The element to add.
     * @return true if the element was added; false if it was already present.
     */
    virtual bool TryAdd(const T& item) = 0;

    /**
     * @brief Adds an element to the set (void override satisfying ICollection<T>).
     *
     * Delegates to TryAdd(). Provided so that ICollection<T>.Add(T) is satisfied.
     * @param item The element to add.
     */
    void Add(const T& item) override { (void)TryAdd(item); }

    /**
     * @brief Modifies the set so it contains all elements present in itself or the specified collection.
     *
     * C++ counterpart of .NET ISet<T>.UnionWith(IEnumerable<T>).
     * @param other The collection of items to add to the set.
     */
    virtual void UnionWith(const IEnumerable<T>& other) = 0;

    /**
     * @brief Modifies the set so it contains only elements also present in the specified collection.
     *
     * C++ counterpart of .NET ISet<T>.IntersectWith(IEnumerable<T>).
     * @param other The collection of items to retain in the set.
     */
    virtual void IntersectWith(const IEnumerable<T>& other) = 0;

    /**
     * @brief Removes all elements in the specified collection from the set.
     *
     * C++ counterpart of .NET ISet<T>.ExceptWith(IEnumerable<T>).
     * @param other The collection of items to remove from the set.
     */
    virtual void ExceptWith(const IEnumerable<T>& other) = 0;

    /**
     * @brief Modifies the set so it contains only elements present in either itself or the specified
     *        collection, but not both.
     *
     * C++ counterpart of .NET ISet<T>.SymmetricExceptWith(IEnumerable<T>).
     * @param other The collection to compare to.
     */
    virtual void SymmetricExceptWith(const IEnumerable<T>& other) = 0;

    /**
     * @brief Determines whether the set is a subset of the specified collection.
     *
     * C++ counterpart of .NET ISet<T>.IsSubsetOf(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if every element of the set is in @p other; otherwise false.
     */
    [[nodiscard]] virtual bool IsSubsetOf(const IEnumerable<T>& other) const = 0;

    /**
     * @brief Determines whether the set is a superset of the specified collection.
     *
     * C++ counterpart of .NET ISet<T>.IsSupersetOf(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if every element of @p other is in the set; otherwise false.
     */
    [[nodiscard]] virtual bool IsSupersetOf(const IEnumerable<T>& other) const = 0;

    /**
     * @brief Determines whether the set is a proper (strict) subset of the specified collection.
     *
     * C++ counterpart of .NET ISet<T>.IsProperSubsetOf(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if the set is a subset of @p other and @p other has at least one more element.
     */
    [[nodiscard]] virtual bool IsProperSubsetOf(const IEnumerable<T>& other) const = 0;

    /**
     * @brief Determines whether the set is a proper (strict) superset of the specified collection.
     *
     * C++ counterpart of .NET ISet<T>.IsProperSupersetOf(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if the set is a superset of @p other and the set has at least one more element.
     */
    [[nodiscard]] virtual bool IsProperSupersetOf(const IEnumerable<T>& other) const = 0;

    /**
     * @brief Determines whether the set and the specified collection share any common elements.
     *
     * C++ counterpart of .NET ISet<T>.Overlaps(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if at least one element is common to both; otherwise false.
     */
    [[nodiscard]] virtual bool Overlaps(const IEnumerable<T>& other) const = 0;

    /**
     * @brief Determines whether the set contains exactly the same elements as the specified collection.
     *
     * C++ counterpart of .NET ISet<T>.SetEquals(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if the set and @p other contain the same elements; otherwise false.
     */
    [[nodiscard]] virtual bool SetEquals(const IEnumerable<T>& other) const = 0;
};

} // namespace System::Collections::Generic
