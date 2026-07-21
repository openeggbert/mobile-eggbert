// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <utility>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/Generic/ICollection.hpp"
#include "System/Collections/Generic/IEnumerable.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Represents a generic collection of key/value pairs.
 *
 * C++ counterpart of .NET System.Collections.Generic.IDictionary<TKey,TValue>.
 * Extends IEnumerable<pair<TKey,TValue>> with indexed access, key lookup,
 * add, remove, and clear operations.
 *
 * @tparam TKey   The type of keys in the dictionary.
 * @tparam TValue The type of values in the dictionary.
 */
template<typename TKey, typename TValue>
class IDictionary : public IEnumerable<std::pair<TKey, TValue>> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IDictionary() = default;

    /**
     * @brief Gets the number of key/value pairs in the dictionary.
     *
     * C++ counterpart of .NET IDictionary<TKey,TValue>.Count.
     */
    [[nodiscard]] virtual intcs getCountProperty() const = 0;

    /**
     * @brief Gets or sets the element with the specified key (mutable).
     *
     * C++ counterpart of .NET IDictionary<TKey,TValue> indexer setter.
     * @param key The key of the element to get or set.
     * @return A reference to the value for the key.
     */
    virtual TValue& operator[](const TKey& key) = 0;

    /**
     * @brief Gets the element with the specified key (const).
     *
     * C++ counterpart of .NET IDictionary<TKey,TValue> indexer getter.
     * @param key The key of the element to get.
     * @return A const reference to the value for the key.
     */
    [[nodiscard]] virtual const TValue& operator[](const TKey& key) const = 0;

    /**
     * @brief Determines whether the dictionary contains an element with the specified key.
     *
     * C++ counterpart of .NET IDictionary<TKey,TValue>.ContainsKey(TKey).
     * @param key The key to locate.
     * @return true if the key is found; otherwise false.
     */
    [[nodiscard]] virtual bool ContainsKey(const TKey& key) const = 0;

    /**
     * @brief Adds an element with the specified key and value to the dictionary.
     *
     * C++ counterpart of .NET IDictionary<TKey,TValue>.Add(TKey, TValue).
     * @param key   The key of the element to add.
     * @param value The value of the element to add.
     */
    virtual void Add(const TKey& key, const TValue& value) = 0;

    /**
     * @brief Removes the element with the specified key from the dictionary.
     *
     * C++ counterpart of .NET IDictionary<TKey,TValue>.Remove(TKey).
     * @param key The key of the element to remove.
     * @return true if the element was found and removed; otherwise false.
     */
    virtual bool Remove(const TKey& key) = 0;

    /**
     * @brief Gets the value associated with the specified key.
     *
     * C++ counterpart of .NET IDictionary<TKey,TValue>.TryGetValue(TKey, out TValue).
     * @param key      The key whose value to get.
     * @param outValue Receives the value if the key is found.
     * @return true if the key was found; otherwise false.
     */
    virtual bool TryGetValue(const TKey& key, TValue& outValue) const = 0;

    /**
     * @brief Removes all key/value pairs from the dictionary.
     *
     * C++ counterpart of .NET IDictionary<TKey,TValue>.Clear().
     */
    virtual void Clear() = 0;
};

} // namespace System::Collections::Generic
