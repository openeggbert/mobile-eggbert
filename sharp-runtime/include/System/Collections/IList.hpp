// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/ICollection.hpp"

namespace System::Collections {

/**
 * @brief Represents a non-generic collection of objects that can be individually
 * accessed by index.
 *
 * C++ counterpart of .NET System.Collections.IList.
 * Because C++ has no unified object root, element values are typed as void*.
 * The Add() method mirrors .NET and returns the index at which the item was inserted.
 */
class IList : public ICollection {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IList() = default;

    /**
     * @brief Gets the element at the specified index.
     *
     * C++ counterpart of .NET IList indexer getter (this[int index]).
     * @param index Zero-based index of the element to get.
     * @return Pointer to the element at @p index.
     */
    [[nodiscard]] virtual void* getItem(SharpRuntime::intcs index) const = 0;

    /**
     * @brief Sets the element at the specified index.
     *
     * C++ counterpart of .NET IList indexer setter (this[int index] = value).
     * @param index Zero-based index of the element to set.
     * @param value The new element value.
     */
    virtual void setItem(SharpRuntime::intcs index, void* value) = 0;

    /**
     * @brief Gets a value indicating whether the list is read-only.
     *
     * C++ counterpart of .NET IList.IsReadOnly.
     */
    [[nodiscard]] virtual bool getIsReadOnlyProperty() const { return false; }

    /**
     * @brief Gets a value indicating whether the list has a fixed size.
     *
     * C++ counterpart of .NET IList.IsFixedSize.
     */
    [[nodiscard]] virtual bool getIsFixedSizeProperty() const { return false; }

    /**
     * @brief Adds an item to the list.
     *
     * C++ counterpart of .NET IList.Add(object?).
     * Returns the position at which the new element was inserted, or −1 if not inserted.
     * @param value The item to add.
     * @return The index at which @p value was inserted.
     */
    virtual SharpRuntime::intcs Add(void* value) = 0;

    /**
     * @brief Determines whether the list contains a specific value.
     *
     * C++ counterpart of .NET IList.Contains(object?).
     * @param value The item to locate.
     * @return true if @p value is found; otherwise false.
     */
    [[nodiscard]] virtual bool Contains(void* value) const = 0;

    /**
     * @brief Removes all items from the list.
     *
     * C++ counterpart of .NET IList.Clear().
     */
    virtual void Clear() = 0;

    /**
     * @brief Determines the index of a specific item in the list.
     *
     * C++ counterpart of .NET IList.IndexOf(object?).
     * @param value The item to locate.
     * @return The index of @p value if found; otherwise −1.
     */
    [[nodiscard]] virtual SharpRuntime::intcs IndexOf(void* value) const = 0;

    /**
     * @brief Inserts an item into the list at the specified index.
     *
     * C++ counterpart of .NET IList.Insert(int, object?).
     * @param index The zero-based index at which @p value should be inserted.
     * @param value The item to insert.
     */
    virtual void Insert(SharpRuntime::intcs index, void* value) = 0;

    /**
     * @brief Removes the first occurrence of a specific item from the list.
     *
     * C++ counterpart of .NET IList.Remove(object?).
     * @param value The item to remove.
     */
    virtual void Remove(void* value) = 0;

    /**
     * @brief Removes the item at the specified index.
     *
     * C++ counterpart of .NET IList.RemoveAt(int).
     * @param index The zero-based index of the item to remove.
     */
    virtual void RemoveAt(SharpRuntime::intcs index) = 0;
};

} // namespace System::Collections
