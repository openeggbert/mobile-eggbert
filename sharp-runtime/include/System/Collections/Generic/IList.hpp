// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/Generic/ICollection.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Represents a generic collection of objects that can be individually
 *        accessed by index.
 *
 * C++ counterpart of .NET System.Collections.Generic.IList<T>.
 * Extends ICollection<T> with index-based access, insertion, and removal.
 *
 * @tparam T The type of elements in the list.
 */
template<typename T>
class IList : public ICollection<T> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    ~IList() override = default;

    /**
     * @brief Gets the element at the specified index (const).
     *
     * C++ counterpart of .NET IList<T>.Item[int] getter.
     * @param index The zero-based index of the element to get.
     * @return A const reference to the element at the specified index.
     */
    [[nodiscard]] virtual const T& operator[](intcs index) const = 0;

    /**
     * @brief Gets or sets the element at the specified index (mutable).
     *
     * C++ counterpart of .NET IList<T>.Item[int] setter.
     * @param index The zero-based index of the element to get or set.
     * @return A reference to the element at the specified index.
     */
    virtual T& operator[](intcs index) = 0;

    /**
     * @brief Determines the index of a specific item in the list.
     *
     * C++ counterpart of .NET IList<T>.IndexOf(T).
     * @param item The object to locate in the list.
     * @return The index of the item if found; otherwise -1.
     */
    [[nodiscard]] virtual intcs IndexOf(const T& item) const = 0;

    /**
     * @brief Inserts an item at the specified index.
     *
     * C++ counterpart of .NET IList<T>.Insert(int, T).
     * @param index The zero-based index at which to insert the item.
     * @param item  The object to insert.
     */
    virtual void Insert(intcs index, const T& item) = 0;

    /**
     * @brief Removes the element at the specified index.
     *
     * C++ counterpart of .NET IList<T>.RemoveAt(int).
     * @param index The zero-based index of the element to remove.
     */
    virtual void RemoveAt(intcs index) = 0;
};

} // namespace System::Collections::Generic
