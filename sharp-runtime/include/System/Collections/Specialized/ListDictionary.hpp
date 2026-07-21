// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <any>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"

namespace System::Collections::Specialized {

using SharpRuntime::intcs;

/**
 * @brief A simple ordered key-value dictionary backed by a linked-list style vector of pairs.
 *
 * C++ counterpart of .NET System.Collections.Specialized.ListDictionary.
 * Optimized for small collections; uses a linear scan for key lookup.
 * Keys and values are typed as std::string and std::any respectively.
 */
class ListDictionary {
    std::vector<std::pair<std::string, std::any>> data_;

    int findIndex(const std::string& key) const {
        for (int i = 0; i < static_cast<int>(data_.size()); ++i)
            if (data_[static_cast<size_t>(i)].first == key) return i;
        return -1;
    }

public:
    /**
     * @brief Default-constructs an empty ListDictionary.
     *
     * C++ counterpart of .NET ListDictionary().
     */
    ListDictionary() = default;

    /**
     * @brief Constructs an empty ListDictionary with an optional comparer.
     *
     * C++ counterpart of .NET ListDictionary(IComparer).
     * The comparer parameter is accepted for API compatibility but ignored.
     */
    explicit ListDictionary(std::nullptr_t /*comparer*/) {}

    /**
     * @brief Gets the number of key/value pairs in the dictionary.
     *
     * C++ counterpart of .NET ListDictionary.Count.
     * @return The number of entries.
     */
    [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(data_.size()); }

    /**
     * @brief Gets a value indicating whether the dictionary contains no elements.
     *
     * C++ counterpart of checking Count == 0 on .NET ListDictionary.
     * @return true if the dictionary is empty; otherwise false.
     */
    [[nodiscard]] bool getIsEmptyProperty() const { return data_.empty(); }

    /**
     * @brief Gets a value indicating whether the dictionary has a fixed size.
     *
     * C++ counterpart of .NET ListDictionary.IsFixedSize.
     * @return Always false — this dictionary can grow.
     */
    [[nodiscard]] bool getIsFixedSizeProperty() const { return false; }

    /**
     * @brief Gets a value indicating whether the dictionary is read-only.
     *
     * C++ counterpart of .NET ListDictionary.IsReadOnly.
     * @return Always false — this dictionary allows modifications.
     */
    [[nodiscard]] bool getIsReadOnlyProperty() const { return false; }

    /**
     * @brief Gets a value indicating whether access to the dictionary is thread-safe.
     *
     * C++ counterpart of .NET ListDictionary.IsSynchronized.
     * @return Always false — this implementation is not thread-safe.
     */
    [[nodiscard]] bool getIsSynchronizedProperty() const { return false; }

    /**
     * @brief Gets an object that can be used to synchronize access to the dictionary.
     *
     * C++ counterpart of .NET ListDictionary.SyncRoot.
     * @return A pointer to this instance used as a synchronization token.
     */
    [[nodiscard]] const void* getSyncRootProperty() const { return this; }

    /**
     * @brief Gets a vector of all keys in insertion order.
     *
     * C++ counterpart of .NET ListDictionary.Keys.
     * @return A vector containing all string keys.
     */
    [[nodiscard]] std::vector<std::string> getKeysProperty() const {
        std::vector<std::string> keys;
        keys.reserve(data_.size());
        for (const auto& p : data_) keys.push_back(p.first);
        return keys;
    }

    /**
     * @brief Gets a vector of all values in insertion order.
     *
     * C++ counterpart of .NET ListDictionary.Values.
     * @return A vector containing all values as std::any.
     */
    [[nodiscard]] std::vector<std::any> getValuesProperty() const {
        std::vector<std::any> vals;
        vals.reserve(data_.size());
        for (const auto& p : data_) vals.push_back(p.second);
        return vals;
    }

    /**
     * @brief Adds a key/value pair to the dictionary.
     *
     * C++ counterpart of .NET ListDictionary.Add(object, object).
     * @param key   The key to add.
     * @param value The value to associate with @p key.
     * @throws System::ArgumentException if @p key already exists.
     */
    void Add(const std::string& key, const std::any& value) {
        if (findIndex(key) >= 0)
            throw System::ArgumentException("An item with the same key has already been added.");
        data_.emplace_back(key, value);
    }

    /**
     * @brief Sets the value for the given key, inserting it if absent.
     *
     * C++ counterpart of .NET ListDictionary.Item[object] setter.
     * @param key   The key to set.
     * @param value The value to associate with @p key.
     */
    void set(const std::string& key, const std::any& value) {
        int idx = findIndex(key);
        if (idx >= 0) data_[static_cast<size_t>(idx)].second = value;
        else          data_.emplace_back(key, value);
    }

    /**
     * @brief Removes the entry with the given key.
     *
     * C++ counterpart of .NET ListDictionary.Remove(object).
     * Does nothing if the key is not found.
     * @param key The key of the entry to remove.
     */
    void Remove(const std::string& key) {
        int idx = findIndex(key);
        if (idx >= 0) data_.erase(data_.begin() + idx);
    }

    /**
     * @brief Removes all entries from the dictionary.
     *
     * C++ counterpart of .NET ListDictionary.Clear().
     */
    void Clear() { data_.clear(); }

    /**
     * @brief Determines whether the dictionary contains the given key.
     *
     * C++ counterpart of .NET ListDictionary.Contains(object).
     * @param key The key to locate.
     * @return true if an entry with @p key exists; otherwise false.
     */
    [[nodiscard]] bool Contains(const std::string& key) const { return findIndex(key) >= 0; }

    /**
     * @brief Returns the value for the given key, or an empty std::any if not found.
     *
     * C++ counterpart of .NET ListDictionary.Item[object] getter. Serves both const and
     * non-const instances -- there is deliberately no non-const overload. A prior non-const
     * `std::any&`-returning overload phantom-inserted an empty entry for a missing key even
     * on what looked like a read (`auto v = d["missing"]` silently grew Count and made
     * `Contains("missing")` become true) and returned a reference into the backing
     * `std::vector` that dangled after any later insertion reallocated it -- the same
     * reference-escape bug class fixed in ConcurrentDictionary (commit 3605260). Real .NET's
     * getter (ListDictionary.cs) never mutates on a miss; only the setter (`set()` below)
     * inserts. Use `set(key, value)` for `dict[key] = value`-style writes.
     * @param key The key whose value to retrieve.
     * @return The associated value, or an empty std::any if @p key is not present.
     */
    [[nodiscard]] std::any operator[](const std::string& key) const {
        int idx = findIndex(key);
        return (idx >= 0) ? data_[static_cast<size_t>(idx)].second : std::any{};
    }

    /** @brief Returns a const iterator to the beginning of the dictionary (STL interop). */
    auto begin() const { return data_.begin(); }
    /** @brief Returns a const iterator past the end of the dictionary (STL interop). */
    auto end()   const { return data_.end(); }
};

} // namespace System::Collections::Specialized
