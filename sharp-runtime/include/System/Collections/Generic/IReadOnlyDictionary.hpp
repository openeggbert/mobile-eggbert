// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Collections/Generic/IReadOnlyCollection.hpp"
#include "System/Collections/Generic/IEnumerable.hpp"
#include "System/Collections/Generic/KeyValuePair.hpp"

namespace System::Collections::Generic {

/**
 * @brief Represents a generic read-only collection of key/value pairs.
 *
 * C++ counterpart of .NET System.Collections.Generic.IReadOnlyDictionary<TKey,TValue>.
 * Extends IReadOnlyCollection<KeyValuePair<TKey,TValue>> with key-based lookup,
 * key/value sequence access, and a typed indexer.
 *
 * @tparam TKey   The type of keys in the read-only dictionary.
 * @tparam TValue The type of values in the read-only dictionary.
 */
template<typename TKey, typename TValue>
class IReadOnlyDictionary : public IReadOnlyCollection<KeyValuePair<TKey, TValue>> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IReadOnlyDictionary() = default;

    /**
     * @brief Gets the element that has the specified key in the read-only dictionary.
     *
     * C++ counterpart of .NET IReadOnlyDictionary<TKey,TValue>.Item[TKey] getter.
     * @param key The key to locate.
     * @return A const reference to the value for the specified key.
     */
    [[nodiscard]] virtual const TValue& operator[](const TKey& key) const = 0;

    /**
     * @brief Gets an enumerable collection that contains the keys in the dictionary.
     *
     * C++ counterpart of .NET IReadOnlyDictionary<TKey,TValue>.Keys.
     * @return A pointer to an IEnumerable<TKey> of the dictionary's keys.
     */
    [[nodiscard]] virtual IEnumerable<TKey>* getKeysProperty() const = 0;

    /**
     * @brief Gets an enumerable collection that contains the values in the dictionary.
     *
     * C++ counterpart of .NET IReadOnlyDictionary<TKey,TValue>.Values.
     * @return A pointer to an IEnumerable<TValue> of the dictionary's values.
     */
    [[nodiscard]] virtual IEnumerable<TValue>* getValuesProperty() const = 0;

    /**
     * @brief Determines whether the read-only dictionary contains an element
     *        that has the specified key.
     *
     * C++ counterpart of .NET IReadOnlyDictionary<TKey,TValue>.ContainsKey(TKey).
     * @param key The key to locate.
     * @return true if the dictionary contains the key; otherwise false.
     */
    [[nodiscard]] virtual bool ContainsKey(const TKey& key) const = 0;

    /**
     * @brief Gets the value that is associated with the specified key.
     *
     * C++ counterpart of .NET IReadOnlyDictionary<TKey,TValue>.TryGetValue(TKey, out TValue).
     * @param key   The key to locate.
     * @param value Receives the value if the key is found.
     * @return true if the key was found; otherwise false.
     */
    [[nodiscard]] virtual bool TryGetValue(const TKey& key, TValue& value) const = 0;
};

} // namespace System::Collections::Generic
