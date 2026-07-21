// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <vector>
#include "System/Collections/Generic/IReadOnlyDictionary.hpp"
#include "System/Collections/Generic/KeyValuePair.hpp"

namespace System::Collections::Immutable {

using System::Collections::Generic::IReadOnlyDictionary;
using System::Collections::Generic::KeyValuePair;

/**
 * @brief Represents an immutable collection of key/value pairs.
 *
 * C++ counterpart of .NET System.Collections.Immutable.IImmutableDictionary<TKey,TValue>.
 * All mutating operations return a new instance; the original is unchanged.
 *
 * @tparam TKey   The type of keys.
 * @tparam TValue The type of values.
 */
template<typename TKey, typename TValue>
class IImmutableDictionary : public IReadOnlyDictionary<TKey, TValue> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    ~IImmutableDictionary() override = default;

    /**
     * @brief Returns an empty immutable dictionary.
     *
     * C++ counterpart of .NET IImmutableDictionary<TKey,TValue>.Clear().
     * @return A shared_ptr to a new empty IImmutableDictionary.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableDictionary<TKey, TValue>> Clear() const = 0;

    /**
     * @brief Returns a new dictionary with the specified key/value pair added.
     *
     * C++ counterpart of .NET IImmutableDictionary<TKey,TValue>.Add(TKey, TValue).
     * @param key   The key to add.
     * @param value The value to add.
     * @return A shared_ptr to a new IImmutableDictionary with the pair added.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableDictionary<TKey, TValue>>
        Add(const TKey& key, const TValue& value) const = 0;

    /**
     * @brief Returns a new dictionary with the specified key/value pairs added.
     *
     * C++ counterpart of .NET IImmutableDictionary<TKey,TValue>.AddRange(IEnumerable<KeyValuePair<TKey,TValue>>).
     * @param pairs The key/value pairs to add.
     * @return A shared_ptr to a new IImmutableDictionary with the pairs added.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableDictionary<TKey, TValue>>
        AddRange(const std::vector<KeyValuePair<TKey, TValue>>& pairs) const = 0;

    /**
     * @brief Returns a new dictionary with the value at the given key replaced.
     *
     * C++ counterpart of .NET IImmutableDictionary<TKey,TValue>.SetItem(TKey, TValue).
     * @param key   The key whose value to set.
     * @param value The new value.
     * @return A shared_ptr to a new IImmutableDictionary with the key set.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableDictionary<TKey, TValue>>
        SetItem(const TKey& key, const TValue& value) const = 0;

    /**
     * @brief Returns a new dictionary with the given key/value pairs set.
     *
     * C++ counterpart of .NET IImmutableDictionary<TKey,TValue>.SetItems(IEnumerable<KeyValuePair<TKey,TValue>>).
     * @param items The key/value pairs to set.
     * @return A shared_ptr to a new IImmutableDictionary with items set.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableDictionary<TKey, TValue>>
        SetItems(const std::vector<KeyValuePair<TKey, TValue>>& items) const = 0;

    /**
     * @brief Returns a new dictionary with the specified keys removed.
     *
     * C++ counterpart of .NET IImmutableDictionary<TKey,TValue>.RemoveRange(IEnumerable<TKey>).
     * @param keys The keys to remove.
     * @return A shared_ptr to a new IImmutableDictionary with those keys removed.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableDictionary<TKey, TValue>>
        RemoveRange(const std::vector<TKey>& keys) const = 0;

    /**
     * @brief Returns a new dictionary with the specified key removed.
     *
     * C++ counterpart of .NET IImmutableDictionary<TKey,TValue>.Remove(TKey).
     * @param key The key to remove.
     * @return A shared_ptr to a new IImmutableDictionary with the key removed.
     */
    [[nodiscard]] virtual std::shared_ptr<IImmutableDictionary<TKey, TValue>>
        Remove(const TKey& key) const = 0;

    /**
     * @brief Determines whether the dictionary contains the specified key/value pair.
     *
     * C++ counterpart of .NET IImmutableDictionary<TKey,TValue>.Contains(KeyValuePair<TKey,TValue>).
     * @param pair The key/value pair to locate.
     * @return true if found; otherwise false.
     */
    [[nodiscard]] virtual bool Contains(const KeyValuePair<TKey, TValue>& pair) const = 0;

    /**
     * @brief Searches the dictionary for a given key and returns the equal key it finds, if any.
     *
     * C++ counterpart of .NET IImmutableDictionary<TKey,TValue>.TryGetKey(TKey, out TKey).
     * @param equalKey  The key to search for.
     * @param actualKey Receives the actual key from the dictionary if found.
     * @return true if a key equal to @p equalKey was found; otherwise false.
     */
    virtual bool TryGetKey(const TKey& equalKey, TKey& actualKey) const = 0;
};

} // namespace System::Collections::Immutable
