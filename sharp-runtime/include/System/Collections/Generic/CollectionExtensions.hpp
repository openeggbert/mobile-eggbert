// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <unordered_map>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Provides extension-style utility methods for generic collections.
 *
 * C++ counterpart of .NET System.Collections.Generic.CollectionExtensions.
 * All members are static; the class cannot be instantiated.
 */
class CollectionExtensions {
public:
    /** @brief Deleted constructor — all members are static. */
    CollectionExtensions() = delete;

    /**
     * @brief Returns the value for the key if present, or a default-constructed value otherwise.
     *
     * C++ counterpart of .NET CollectionExtensions.GetValueOrDefault<TKey,TValue>(IReadOnlyDictionary,TKey).
     * @param dict The source dictionary.
     * @param key  The key to look up.
     * @return The associated value, or V{} if the key is not found.
     */
    template<typename K, typename V>
    static V GetValueOrDefault(const std::unordered_map<K, V>& dict, const K& key) {
        auto it = dict.find(key);
        return it == dict.end() ? V{} : it->second;
    }

    /**
     * @brief Returns the value for the key if present, or the specified default value otherwise.
     *
     * C++ counterpart of .NET CollectionExtensions.GetValueOrDefault<TKey,TValue>(IReadOnlyDictionary,TKey,TValue).
     * @param dict         The source dictionary.
     * @param key          The key to look up.
     * @param defaultValue The value to return if the key is absent.
     * @return The associated value, or defaultValue if not found.
     */
    template<typename K, typename V>
    static V GetValueOrDefault(const std::unordered_map<K, V>& dict, const K& key, const V& defaultValue) {
        auto it = dict.find(key);
        return it == dict.end() ? defaultValue : it->second;
    }

    /**
     * @brief Adds the key/value pair only if the key is not already present.
     *
     * C++ counterpart of .NET CollectionExtensions.TryAdd<TKey,TValue>(IDictionary,TKey,TValue).
     * @param dict  The dictionary to add to.
     * @param key   The key to add.
     * @param value The value to associate with the key.
     * @return true if the pair was added; false if the key already existed.
     */
    template<typename K, typename V>
    static bool TryAdd(std::unordered_map<K, V>& dict, const K& key, const V& value) {
        return dict.emplace(key, value).second;
    }

    /**
     * @brief Removes the entry with the given key and outputs its value.
     *
     * C++ counterpart of .NET CollectionExtensions.Remove<TKey,TValue>(IDictionary,TKey,out TValue).
     * @param dict         The dictionary to remove from.
     * @param key          The key to remove.
     * @param removedValue Receives the removed value if the key was found.
     * @return true if the key was found and removed; otherwise false.
     */
    template<typename K, typename V>
    static bool Remove(std::unordered_map<K, V>& dict, const K& key, V& removedValue) {
        auto it = dict.find(key);
        if (it == dict.end()) return false;
        removedValue = std::move(it->second);
        dict.erase(it);
        return true;
    }

    /**
     * @brief Returns a const reference to the vector, treating it as read-only.
     *
     * C++ counterpart of .NET CollectionExtensions.AsReadOnly<T>(IList<T>).
     * @param list The source list.
     * @return A const reference to the same list.
     */
    template<typename T>
    static const std::vector<T>& AsReadOnly(const std::vector<T>& list) {
        return list;
    }

    /**
     * @brief Appends all elements from source to the end of list.
     *
     * C++ counterpart of .NET CollectionExtensions.AddRange<T>(List<T>, ReadOnlySpan<T>).
     * @param list   The list to append to.
     * @param source Elements to append.
     */
    template<typename T>
    static void AddRange(std::vector<T>& list, const std::vector<T>& source) {
        list.insert(list.end(), source.begin(), source.end());
    }

    /**
     * @brief Inserts all elements from source into list at the specified index.
     *
     * C++ counterpart of .NET CollectionExtensions.InsertRange<T>(List<T>, int, ReadOnlySpan<T>).
     * @param list   The list to insert into.
     * @param index  Zero-based index at which insertion begins.
     * @param source Elements to insert.
     * @throws System::ArgumentOutOfRangeException if @p index is negative or greater than the list's size.
     */
    template<typename T>
    static void InsertRange(std::vector<T>& list, intcs index, const std::vector<T>& source) {
        if (index < 0 || static_cast<std::size_t>(index) > list.size())
            throw System::ArgumentOutOfRangeException("index");
        list.insert(list.begin() + index, source.begin(), source.end());
    }
};

} // namespace System::Collections::Generic
