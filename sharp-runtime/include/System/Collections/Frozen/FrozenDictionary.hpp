// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <functional>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"

namespace System::Collections::Frozen {

/**
 * @brief Provides an immutable, read-only dictionary optimized for fast lookup and enumeration.
 *
 * C++ counterpart of .NET System.Collections.Frozen.FrozenDictionary<TKey, TValue>.
 * Once created via Create() or ToFrozenDictionary(), the dictionary cannot be modified.
 * Backed by std::unordered_map for O(1) average-case lookups.
 * Ideal for data loaded once at startup and read frequently at runtime.
 *
 * @tparam TKey   The type of the keys.
 * @tparam TValue The type of the values.
 */
template<typename TKey, typename TValue>
class FrozenDictionary {
    std::unordered_map<TKey, TValue> map_;

    explicit FrozenDictionary(std::unordered_map<TKey, TValue> m) : map_(std::move(m)) {}

public:
    using value_type    = std::pair<const TKey, TValue>;
    using const_iterator = typename std::unordered_map<TKey, TValue>::const_iterator;

    /**
     * @brief Gets an empty FrozenDictionary singleton.
     *
     * C++ counterpart of .NET FrozenDictionary<TKey,TValue>.Empty.
     */
    static const FrozenDictionary& getEmptyProperty() {
        static FrozenDictionary instance{std::unordered_map<TKey, TValue>{}};
        return instance;
    }

    /**
     * @brief Creates a FrozenDictionary from a vector of key/value pairs.
     *
     * C++ counterpart of .NET FrozenDictionary.Create(ReadOnlySpan<KeyValuePair<TKey,TValue>>).
     * If the same key appears multiple times, the last value takes precedence.
     * @param source Key/value pairs to populate the dictionary.
     * @return A new immutable FrozenDictionary.
     */
    static FrozenDictionary Create(const std::vector<std::pair<TKey, TValue>>& source) {
        std::unordered_map<TKey, TValue> m;
        m.reserve(source.size());
        for (const auto& kv : source) m[kv.first] = kv.second;
        return FrozenDictionary(std::move(m));
    }

    /**
     * @brief Creates a FrozenDictionary from an existing unordered_map.
     *
     * @param source The map whose entries are copied into the frozen dictionary.
     * @return A new immutable FrozenDictionary.
     */
    static FrozenDictionary CreateFromMap(const std::unordered_map<TKey, TValue>& source) {
        return FrozenDictionary(std::unordered_map<TKey, TValue>(source));
    }

    /**
     * @brief Gets the number of key/value pairs in the dictionary.
     *
     * C++ counterpart of .NET FrozenDictionary<TKey,TValue>.Count.
     */
    [[nodiscard]] SharpRuntime::intcs getCountProperty() const {
        return static_cast<SharpRuntime::intcs>(map_.size());
    }

    /**
     * @brief Gets a vector containing the keys of the dictionary.
     *
     * C++ counterpart of .NET FrozenDictionary<TKey,TValue>.Keys.
     * Order matches the associated values returned by getValuesProperty().
     */
    [[nodiscard]] std::vector<TKey> getKeysProperty() const {
        std::vector<TKey> keys;
        keys.reserve(map_.size());
        for (const auto& kv : map_) keys.push_back(kv.first);
        return keys;
    }

    /**
     * @brief Gets a vector containing the values of the dictionary.
     *
     * C++ counterpart of .NET FrozenDictionary<TKey,TValue>.Values.
     * Order matches the associated keys returned by getKeysProperty().
     */
    [[nodiscard]] std::vector<TValue> getValuesProperty() const {
        std::vector<TValue> vals;
        vals.reserve(map_.size());
        for (const auto& kv : map_) vals.push_back(kv.second);
        return vals;
    }

    /**
     * @brief Gets the value associated with the specified key.
     *
     * C++ counterpart of .NET FrozenDictionary<TKey,TValue> indexer getter (this[TKey]).
     * @param key The key whose value to get.
     * @return A const reference to the value.
     * @throws System::Collections::Generic::KeyNotFoundException if the key is not found.
     */
    [[nodiscard]] const TValue& getItem(const TKey& key) const {
        auto it = map_.find(key);
        if (it == map_.end()) throw System::Collections::Generic::KeyNotFoundException("The given key was not present in the dictionary.");
        return it->second;
    }

    /**
     * @brief Determines whether the dictionary contains the specified key.
     *
     * C++ counterpart of .NET FrozenDictionary<TKey,TValue>.ContainsKey(TKey).
     * @param key The key to locate.
     * @return true if found; otherwise false.
     */
    [[nodiscard]] bool ContainsKey(const TKey& key) const {
        return map_.count(key) != 0;
    }

    /**
     * @brief Attempts to get the value associated with the specified key.
     *
     * C++ counterpart of .NET FrozenDictionary<TKey,TValue>.TryGetValue(TKey, out TValue).
     * @param key   The key to search for.
     * @param value Receives the value if found.
     * @return true if the key was found; otherwise false.
     */
    bool TryGetValue(const TKey& key, TValue& value) const {
        auto it = map_.find(key);
        if (it == map_.end()) return false;
        value = it->second;
        return true;
    }

    /**
     * @brief Copies all key/value pairs to the destination vector starting at the given index.
     *
     * C++ counterpart of .NET FrozenDictionary<TKey,TValue>.CopyTo(KeyValuePair<TKey,TValue>[], int).
     * @param destination Destination vector; must already have room for index + Count elements.
     * @param index       Zero-based index at which copying begins.
     * @throws System::ArgumentOutOfRangeException if @p index is negative.
     * @throws System::ArgumentException if @p destination is not large enough.
     */
    void CopyTo(std::vector<std::pair<TKey, TValue>>& destination, SharpRuntime::intcs index) const {
        if (index < 0) throw System::ArgumentOutOfRangeException("destinationIndex");
        if (static_cast<std::size_t>(index) + map_.size() > destination.size())
            throw System::ArgumentException("Destination too small for the number of elements in the collection.", "destination");
        std::size_t i = static_cast<std::size_t>(index);
        for (const auto& kv : map_) {
            destination[i++] = {kv.first, kv.second};
        }
    }

    /** @brief Returns an iterator to the beginning of the dictionary (for range-based for). */
    [[nodiscard]] const_iterator begin() const { return map_.begin(); }

    /** @brief Returns an iterator past the end of the dictionary (for range-based for). */
    [[nodiscard]] const_iterator end()   const { return map_.end();   }
};

/**
 * @brief Creates a FrozenDictionary from an iterable range of key/value pairs.
 *
 * C++ counterpart of .NET IEnumerable<KeyValuePair<TKey,TValue>>.ToFrozenDictionary().
 * If the same key appears multiple times, the last value takes precedence.
 * @param source A vector of key/value pairs.
 * @return A new immutable FrozenDictionary.
 */
template<typename TKey, typename TValue>
FrozenDictionary<TKey, TValue> ToFrozenDictionary(
    const std::vector<std::pair<TKey, TValue>>& source)
{
    return FrozenDictionary<TKey, TValue>::Create(source);
}

} // namespace System::Collections::Frozen
