// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::Collections::Specialized {

using SharpRuntime::intcs;

/**
 * @brief A strongly-typed collection of strings.
 *
 * C++ counterpart of .NET System.Collections.Specialized.StringCollection.
 * Backed by std::vector<std::string>; preserves insertion order.
 */
class StringCollection {
    std::vector<std::string> data_;

public:
    /** @brief Default-constructs an empty StringCollection. */
    StringCollection() = default;

    /**
     * @brief Gets the number of strings in the collection.
     *
     * C++ counterpart of .NET StringCollection.Count.
     * @return The number of elements.
     */
    [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(data_.size()); }

    /**
     * @brief Gets a value indicating whether the collection is read-only.
     *
     * C++ counterpart of .NET StringCollection.IsReadOnly.
     * @return Always false — this collection allows modifications.
     */
    [[nodiscard]] bool getIsReadOnlyProperty() const { return false; }

    /**
     * @brief Gets a value indicating whether access to the collection is thread-safe.
     *
     * C++ counterpart of .NET StringCollection.IsSynchronized.
     * @return Always false — this implementation is not thread-safe.
     */
    [[nodiscard]] bool getIsSynchronizedProperty() const { return false; }

    /**
     * @brief Gets an object that can be used to synchronize access to the collection.
     *
     * C++ counterpart of .NET StringCollection.SyncRoot.
     * @return A pointer to this instance used as a synchronization token.
     */
    [[nodiscard]] const void* getSyncRootProperty() const { return this; }

    /**
     * @brief Returns a const reference to the string at the given index.
     *
     * C++ counterpart of .NET StringCollection.Item[int] getter.
     * @param index The zero-based index.
     * @return A const reference to the string.
     * @throws System::ArgumentOutOfRangeException if @p index is out of range.
     */
    [[nodiscard]] const std::string& operator[](intcs index) const {
        if (index < 0 || index >= static_cast<intcs>(data_.size()))
            throw System::ArgumentOutOfRangeException("index");
        return data_[static_cast<size_t>(index)];
    }

    /**
     * @brief Returns a reference to the string at the given index.
     *
     * C++ counterpart of .NET StringCollection.Item[int] setter.
     * @param index The zero-based index.
     * @return A reference to the string.
     * @throws System::ArgumentOutOfRangeException if @p index is out of range.
     */
    std::string& operator[](intcs index) {
        if (index < 0 || index >= static_cast<intcs>(data_.size()))
            throw System::ArgumentOutOfRangeException("index");
        return data_[static_cast<size_t>(index)];
    }

    /**
     * @brief Adds a string to the end of the collection.
     *
     * C++ counterpart of .NET StringCollection.Add(string).
     * @param value The string to add.
     * @return The zero-based index at which @p value was added.
     */
    intcs Add(const std::string& value) {
        data_.push_back(value);
        return static_cast<intcs>(data_.size()) - 1;
    }

    /**
     * @brief Appends all strings in @p values to the collection.
     *
     * C++ counterpart of .NET StringCollection.AddRange(string[]).
     * @param values The strings to append.
     */
    void AddRange(const std::vector<std::string>& values) {
        for (const auto& v : values) data_.push_back(v);
    }

    /**
     * @brief Inserts a string at the specified index.
     *
     * C++ counterpart of .NET StringCollection.Insert(int, string).
     * @param index The zero-based index at which to insert; may equal Count to append.
     * @param value The string to insert.
     * @throws System::ArgumentOutOfRangeException if @p index is negative or greater than Count.
     */
    void Insert(intcs index, const std::string& value) {
        if (index < 0 || index > static_cast<intcs>(data_.size()))
            throw System::ArgumentOutOfRangeException("index");
        data_.insert(data_.begin() + index, value);
    }

    /**
     * @brief Removes the first occurrence of the given string.
     *
     * C++ counterpart of .NET StringCollection.Remove(string).
     * @param value The string to remove.
     */
    void Remove(const std::string& value) {
        auto it = std::find(data_.begin(), data_.end(), value);
        if (it != data_.end()) data_.erase(it);
    }

    /**
     * @brief Removes the string at the specified index.
     *
     * C++ counterpart of .NET StringCollection.RemoveAt(int).
     * @param index The zero-based index of the element to remove.
     * @throws System::ArgumentOutOfRangeException if @p index is out of range.
     */
    void RemoveAt(intcs index) {
        if (index < 0 || index >= static_cast<intcs>(data_.size()))
            throw System::ArgumentOutOfRangeException("index");
        data_.erase(data_.begin() + index);
    }

    /**
     * @brief Removes all strings from the collection.
     *
     * C++ counterpart of .NET StringCollection.Clear().
     */
    void Clear() { data_.clear(); }

    /**
     * @brief Determines whether the collection contains the given string.
     *
     * C++ counterpart of .NET StringCollection.Contains(string).
     * @param value The string to locate.
     * @return true if @p value is found; otherwise false.
     */
    [[nodiscard]] bool Contains(const std::string& value) const {
        return std::find(data_.begin(), data_.end(), value) != data_.end();
    }

    /**
     * @brief Returns the zero-based index of the first occurrence of @p value, or -1 if not found.
     *
     * C++ counterpart of .NET StringCollection.IndexOf(string).
     * @param value The string to locate.
     * @return The zero-based index, or -1 if not found.
     */
    [[nodiscard]] intcs IndexOf(const std::string& value) const {
        auto it = std::find(data_.begin(), data_.end(), value);
        return it == data_.end() ? -1 : static_cast<intcs>(it - data_.begin());
    }

    /**
     * @brief Copies all elements to @p dest starting at the given index.
     *
     * C++ counterpart of .NET StringCollection.CopyTo(string[], int).
     * @param dest  The destination vector.
     * @param index The zero-based starting index in @p dest.
     * @throws System::ArgumentOutOfRangeException if @p index is negative.
     * @throws System::ArgumentException if @p dest does not have enough room starting at @p index.
     */
    void CopyTo(std::vector<std::string>& dest, intcs index) const {
        if (index < 0)
            throw System::ArgumentOutOfRangeException("index");
        if (static_cast<intcs>(dest.size()) - index < static_cast<intcs>(data_.size()))
            throw System::ArgumentException("Destination array was not long enough.", "dest");
        for (size_t i = 0; i < data_.size(); ++i)
            dest[static_cast<size_t>(index) + i] = data_[i];
    }

    /** @brief Returns a const iterator to the beginning of the collection (STL interop). */
    auto begin() const { return data_.begin(); }
    /** @brief Returns a const iterator past the end of the collection (STL interop). */
    auto end()   const { return data_.end(); }
    /** @brief Returns an iterator to the beginning of the collection (STL interop). */
    auto begin()       { return data_.begin(); }
    /** @brief Returns an iterator past the end of the collection (STL interop). */
    auto end()         { return data_.end(); }
};

} // namespace System::Collections::Specialized
