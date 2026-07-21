// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Collections/Generic/IReadOnlyCollection.hpp"

namespace System::Collections::Generic {

/**
 * @brief Provides a read-only abstraction of a set.
 *
 * C++ counterpart of .NET System.Collections.Generic.IReadOnlySet<T>.
 * Extends IReadOnlyCollection<T> with set-membership and set-relation queries.
 *
 * @tparam T The type of elements in the set.
 */
template<typename T>
class IReadOnlySet : public IReadOnlyCollection<T> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IReadOnlySet() = default;

    /**
     * @brief Determines whether the set contains the specified element.
     *
     * C++ counterpart of .NET IReadOnlySet<T>.Contains(T).
     * @param item The element to locate.
     * @return true if the element is found; otherwise false.
     */
    [[nodiscard]] virtual bool Contains(const T& item) const = 0;

    /**
     * @brief Determines whether the set is a subset of the specified collection.
     *
     * C++ counterpart of .NET IReadOnlySet<T>.IsSubsetOf(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if every element of the set is in @p other; otherwise false.
     */
    [[nodiscard]] virtual bool IsSubsetOf(const IEnumerable<T>& other) const = 0;

    /**
     * @brief Determines whether the set is a superset of the specified collection.
     *
     * C++ counterpart of .NET IReadOnlySet<T>.IsSupersetOf(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if every element of @p other is in the set; otherwise false.
     */
    [[nodiscard]] virtual bool IsSupersetOf(const IEnumerable<T>& other) const = 0;

    /**
     * @brief Determines whether the set is a proper (strict) subset of the specified collection.
     *
     * C++ counterpart of .NET IReadOnlySet<T>.IsProperSubsetOf(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if the set is a subset of @p other and @p other has at least one more element.
     */
    [[nodiscard]] virtual bool IsProperSubsetOf(const IEnumerable<T>& other) const = 0;

    /**
     * @brief Determines whether the set is a proper (strict) superset of the specified collection.
     *
     * C++ counterpart of .NET IReadOnlySet<T>.IsProperSupersetOf(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if the set is a superset of @p other and the set has at least one more element.
     */
    [[nodiscard]] virtual bool IsProperSupersetOf(const IEnumerable<T>& other) const = 0;

    /**
     * @brief Determines whether the set and the specified collection share any common elements.
     *
     * C++ counterpart of .NET IReadOnlySet<T>.Overlaps(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if at least one element is common to both; otherwise false.
     */
    [[nodiscard]] virtual bool Overlaps(const IEnumerable<T>& other) const = 0;

    /**
     * @brief Determines whether the set and the specified collection contain the same elements.
     *
     * C++ counterpart of .NET IReadOnlySet<T>.SetEquals(IEnumerable<T>).
     * @param other The collection to compare to.
     * @return true if the set contains exactly the same elements as @p other; otherwise false.
     */
    [[nodiscard]] virtual bool SetEquals(const IEnumerable<T>& other) const = 0;
};

} // namespace System::Collections::Generic
