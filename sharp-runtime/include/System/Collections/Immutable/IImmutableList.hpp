// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "System/Collections/Generic/IReadOnlyList.hpp"
#include "System/Collections/Generic/IEqualityComparer.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Collections::Immutable {

using System::Collections::Generic::IReadOnlyList;
using System::Collections::Generic::IEqualityComparer;
using SharpRuntime::intcs;

/**
 * @brief Represents an immutable ordered collection of elements.
 *
 * C++ counterpart of .NET System.Collections.Immutable.IImmutableList<T>.
 * All mutating operations return a new instance; the original is unchanged.
 *
 * @tparam T The type of elements in the list.
 */
template<typename T>
class IImmutableList : public IReadOnlyList<T> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    ~IImmutableList() override = default;

    /**
     * @brief Returns an empty immutable list.
     *
     * C++ counterpart of .NET IImmutableList<T>.Clear().
     * @return A shared_ptr to a new empty IImmutableList.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableList<T>> Clear() const = 0;

    /**
     * @brief Returns a new list with the element appended at the end.
     *
     * C++ counterpart of .NET IImmutableList<T>.Add(T).
     * @param value The element to append.
     * @return A shared_ptr to a new IImmutableList with the element added.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableList<T>> Add(const T& value) const = 0;

    /**
     * @brief Returns a new list with the elements from items appended at the end.
     *
     * C++ counterpart of .NET IImmutableList<T>.AddRange(IEnumerable<T>).
     * @param items The elements to append.
     * @return A shared_ptr to a new IImmutableList with the elements added.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableList<T>>
        AddRange(const std::vector<T>& items) const = 0;

    /**
     * @brief Returns a new list with the element inserted at the given index.
     *
     * C++ counterpart of .NET IImmutableList<T>.Insert(int, T).
     * @param index   The zero-based index at which to insert.
     * @param element The element to insert.
     * @return A shared_ptr to a new IImmutableList with the element inserted.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableList<T>>
        Insert(intcs index, const T& element) const = 0;

    /**
     * @brief Returns a new list with the elements inserted at the given index.
     *
     * C++ counterpart of .NET IImmutableList<T>.InsertRange(int, IEnumerable<T>).
     * @param index The zero-based index at which to insert.
     * @param items The elements to insert.
     * @return A shared_ptr to a new IImmutableList with the elements inserted.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableList<T>>
        InsertRange(intcs index, const std::vector<T>& items) const = 0;

    /**
     * @brief Returns a new list with the first matching element removed.
     *
     * C++ counterpart of .NET IImmutableList<T>.Remove(T, IEqualityComparer<T>?).
     * @param value            The element to remove.
     * @param equalityComparer Optional comparer; null uses default equality.
     * @return A shared_ptr to a new IImmutableList with the element removed.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableList<T>>
        Remove(const T& value, IEqualityComparer<T>* equalityComparer = nullptr) const = 0;

    /**
     * @brief Returns a new list with all elements matching the predicate removed.
     *
     * C++ counterpart of .NET IImmutableList<T>.RemoveAll(Predicate<T>).
     * @param match The predicate that identifies elements to remove.
     * @return A shared_ptr to a new IImmutableList with matching elements removed.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableList<T>>
        RemoveAll(std::function<bool(const T&)> match) const = 0;

    /**
     * @brief Returns a new list with all matching elements in items removed.
     *
     * C++ counterpart of .NET IImmutableList<T>.RemoveRange(IEnumerable<T>, IEqualityComparer<T>?).
     * @param items            The elements to remove.
     * @param equalityComparer Optional comparer; null uses default equality.
     * @return A shared_ptr to a new IImmutableList with the elements removed.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableList<T>>
        RemoveRange(const std::vector<T>& items,
                    IEqualityComparer<T>* equalityComparer = nullptr) const = 0;

    /**
     * @brief Returns a new list with @p count elements removed starting at @p index.
     *
     * C++ counterpart of .NET IImmutableList<T>.RemoveRange(int, int).
     * @param index The zero-based starting index.
     * @param count The number of elements to remove.
     * @return A shared_ptr to a new IImmutableList with the range removed.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableList<T>>
        RemoveRange(intcs index, intcs count) const = 0;

    /**
     * @brief Returns a new list with the element at the specified index removed.
     *
     * C++ counterpart of .NET IImmutableList<T>.RemoveAt(int).
     * @param index The zero-based index of the element to remove.
     * @return A shared_ptr to a new IImmutableList with the element removed.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableList<T>> RemoveAt(intcs index) const = 0;

    /**
     * @brief Returns a new list with the element at the specified index replaced.
     *
     * C++ counterpart of .NET IImmutableList<T>.SetItem(int, T).
     * @param index The zero-based index of the element to replace.
     * @param value The new value.
     * @return A shared_ptr to a new IImmutableList with the replacement applied.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableList<T>>
        SetItem(intcs index, const T& value) const = 0;

    /**
     * @brief Returns a new list with the first occurrence of oldValue replaced by newValue.
     *
     * C++ counterpart of .NET IImmutableList<T>.Replace(T, T, IEqualityComparer<T>?).
     * @param oldValue         The value to replace.
     * @param newValue         The replacement value.
     * @param equalityComparer Optional comparer; null uses default equality.
     * @return A shared_ptr to a new IImmutableList with the replacement applied.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableList<T>>
        Replace(const T& oldValue, const T& newValue,
                IEqualityComparer<T>* equalityComparer = nullptr) const = 0;

    /**
     * @brief Searches for the specified item and returns its zero-based index.
     *
     * C++ counterpart of .NET IImmutableList<T>.IndexOf(T, int, int, IEqualityComparer<T>?).
     * @param item             The item to locate.
     * @param index            The starting search index.
     * @param count            The number of elements to search.
     * @param equalityComparer Optional comparer; null uses default equality.
     * @return The zero-based index of the item, or -1 if not found.
     */
    [[nodiscard]] virtual intcs IndexOf(const T& item, intcs index, intcs count,
                                        IEqualityComparer<T>* equalityComparer = nullptr) const = 0;

    /**
     * @brief Searches backwards for the specified item and returns its zero-based index.
     *
     * C++ counterpart of .NET IImmutableList<T>.LastIndexOf(T, int, int, IEqualityComparer<T>?).
     * @param item             The item to locate.
     * @param index            The starting search index (from the end).
     * @param count            The number of elements to search.
     * @param equalityComparer Optional comparer; null uses default equality.
     * @return The zero-based index of the last occurrence, or -1 if not found.
     */
    [[nodiscard]] virtual intcs LastIndexOf(const T& item, intcs index, intcs count,
                                            IEqualityComparer<T>* equalityComparer = nullptr) const = 0;
};

} // namespace System::Collections::Immutable
