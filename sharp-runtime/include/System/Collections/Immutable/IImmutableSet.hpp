// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <vector>
#include "System/Collections/Generic/IReadOnlyCollection.hpp"

namespace System::Collections::Immutable {

using System::Collections::Generic::IReadOnlyCollection;

/**
 * @brief Represents an immutable set of elements with no duplicates.
 *
 * C++ counterpart of .NET System.Collections.Immutable.IImmutableSet<T>.
 * All mutating operations return a new instance; the original is unchanged.
 *
 * @tparam T The type of elements in the set.
 */
template<typename T>
class IImmutableSet : public IReadOnlyCollection<T> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    ~IImmutableSet() override = default;

    /**
     * @brief Returns an empty immutable set.
     *
     * C++ counterpart of .NET IImmutableSet<T>.Clear().
     * @return A shared_ptr to a new empty IImmutableSet.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableSet<T>> Clear() const = 0;

    /**
     * @brief Determines whether the set contains the specified value.
     *
     * C++ counterpart of .NET IImmutableSet<T>.Contains(T).
     * @param value The value to locate.
     * @return true if found; otherwise false.
     */
    [[nodiscard]] virtual bool Contains(const T& value) const = 0;

    /**
     * @brief Returns a new set with the specified value added.
     *
     * C++ counterpart of .NET IImmutableSet<T>.Add(T).
     * @param value The value to add.
     * @return A shared_ptr to a new IImmutableSet with the value added.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableSet<T>> Add(const T& value) const = 0;

    /**
     * @brief Returns a new set with the specified value removed.
     *
     * C++ counterpart of .NET IImmutableSet<T>.Remove(T).
     * @param value The value to remove.
     * @return A shared_ptr to a new IImmutableSet with the value removed.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableSet<T>> Remove(const T& value) const = 0;

    /**
     * @brief Searches for a value equal to @p equalValue and returns the actual stored value.
     *
     * C++ counterpart of .NET IImmutableSet<T>.TryGetValue(T, out T).
     * @param equalValue  The value to search for.
     * @param actualValue Receives the stored value if found.
     * @return true if a matching value was found; otherwise false.
     */
    virtual bool TryGetValue(const T& equalValue, T& actualValue) const = 0;

    /**
     * @brief Returns a new set that is the intersection of this set and @p other.
     *
     * C++ counterpart of .NET IImmutableSet<T>.Intersect(IEnumerable<T>).
     * @param other The elements to intersect with.
     * @return A shared_ptr to a new IImmutableSet containing only common elements.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableSet<T>>
        Intersect(const std::vector<T>& other) const = 0;

    /**
     * @brief Returns a new set with the elements of @p other removed.
     *
     * C++ counterpart of .NET IImmutableSet<T>.Except(IEnumerable<T>).
     * @param other The elements to exclude.
     * @return A shared_ptr to a new IImmutableSet without the specified elements.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableSet<T>>
        Except(const std::vector<T>& other) const = 0;

    /**
     * @brief Returns a new set containing only elements present in exactly one of the two sets.
     *
     * C++ counterpart of .NET IImmutableSet<T>.SymmetricExcept(IEnumerable<T>).
     * @param other The other set of elements.
     * @return A shared_ptr to a new IImmutableSet with the symmetric difference.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableSet<T>>
        SymmetricExcept(const std::vector<T>& other) const = 0;

    /**
     * @brief Returns a new set that is the union of this set and @p other.
     *
     * C++ counterpart of .NET IImmutableSet<T>.Union(IEnumerable<T>).
     * @param other The elements to union with.
     * @return A shared_ptr to a new IImmutableSet containing all elements from both.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableSet<T>>
        Union(const std::vector<T>& other) const = 0;

    /**
     * @brief Determines whether this set and @p other contain the same elements.
     *
     * C++ counterpart of .NET IImmutableSet<T>.SetEquals(IEnumerable<T>).
     * @param other The elements to compare to.
     * @return true if both sets are equal; otherwise false.
     */
    [[nodiscard]] virtual bool SetEquals(const std::vector<T>& other) const = 0;

    /**
     * @brief Determines whether this set is a proper subset of @p other.
     *
     * C++ counterpart of .NET IImmutableSet<T>.IsProperSubsetOf(IEnumerable<T>).
     * @param other The elements to compare to.
     * @return true if this set is a proper subset of @p other; otherwise false.
     */
    [[nodiscard]] virtual bool IsProperSubsetOf(const std::vector<T>& other) const = 0;

    /**
     * @brief Determines whether this set is a proper superset of @p other.
     *
     * C++ counterpart of .NET IImmutableSet<T>.IsProperSupersetOf(IEnumerable<T>).
     * @param other The elements to compare to.
     * @return true if this set is a proper superset of @p other; otherwise false.
     */
    [[nodiscard]] virtual bool IsProperSupersetOf(const std::vector<T>& other) const = 0;

    /**
     * @brief Determines whether this set is a subset of @p other.
     *
     * C++ counterpart of .NET IImmutableSet<T>.IsSubsetOf(IEnumerable<T>).
     * @param other The elements to compare to.
     * @return true if every element in this set is in @p other; otherwise false.
     */
    [[nodiscard]] virtual bool IsSubsetOf(const std::vector<T>& other) const = 0;

    /**
     * @brief Determines whether this set is a superset of @p other.
     *
     * C++ counterpart of .NET IImmutableSet<T>.IsSupersetOf(IEnumerable<T>).
     * @param other The elements to compare to.
     * @return true if every element in @p other is in this set; otherwise false.
     */
    [[nodiscard]] virtual bool IsSupersetOf(const std::vector<T>& other) const = 0;

    /**
     * @brief Determines whether this set and @p other share any common elements.
     *
     * C++ counterpart of .NET IImmutableSet<T>.Overlaps(IEnumerable<T>).
     * @param other The elements to compare to.
     * @return true if at least one element is common; otherwise false.
     */
    [[nodiscard]] virtual bool Overlaps(const std::vector<T>& other) const = 0;
};

} // namespace System::Collections::Immutable
