// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/Generic/IEnumerable.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Defines methods to manipulate generic collections.
 *
 * C++ counterpart of .NET System.Collections.Generic.ICollection<T>.
 * Extends IEnumerable<T> with count, read-only check, add, clear, contains,
 * and remove operations.
 *
 * @tparam T The type of elements in the collection.
 */
template<typename T>
class ICollection : public IEnumerable<T> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    ~ICollection() override = default;

    /**
     * @brief Gets the number of elements contained in the collection.
     *
     * C++ counterpart of .NET ICollection<T>.Count.
     */
    [[nodiscard]] virtual intcs getCountProperty() const = 0;

    /**
     * @brief Gets a value indicating whether the collection is read-only.
     *
     * C++ counterpart of .NET ICollection<T>.IsReadOnly.
     */
    [[nodiscard]] virtual bool getIsReadOnlyProperty() const = 0;

    /**
     * @brief Adds an item to the collection.
     *
     * C++ counterpart of .NET ICollection<T>.Add(T).
     * @param item The object to add.
     */
    virtual void Add(const T& item) = 0;

    /**
     * @brief Removes all items from the collection.
     *
     * C++ counterpart of .NET ICollection<T>.Clear().
     */
    virtual void Clear() = 0;

    /**
     * @brief Determines whether the collection contains a specific value.
     *
     * C++ counterpart of .NET ICollection<T>.Contains(T).
     * @param item The object to locate.
     * @return true if item is found; otherwise false.
     */
    [[nodiscard]] virtual bool Contains(const T& item) const = 0;

    /**
     * @brief Removes the first occurrence of a specific object from the collection.
     *
     * C++ counterpart of .NET ICollection<T>.Remove(T).
     * @param item The object to remove.
     * @return true if item was removed; false if not found.
     */
    virtual bool Remove(const T& item) = 0;
};

} // namespace System::Collections::Generic
