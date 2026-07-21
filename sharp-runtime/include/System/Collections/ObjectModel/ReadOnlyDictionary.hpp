// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"

namespace System::Collections::ObjectModel {

/**
 * @brief Provides a read-only wrapper around a generic dictionary.
 *
 * C++ counterpart of .NET System.Collections.ObjectModel.ReadOnlyDictionary<TKey,TValue>.
 * Wraps a shared_ptr<unordered_map<K,V>> so that the wrapped dictionary cannot be mutated
 * through this interface (mutations made directly to the wrapped map, if the caller kept
 * a reference to it, are still visible here — matching .NET's wrap-not-copy semantics).
 *
 * @tparam K The type of keys in the dictionary.
 * @tparam V The type of values in the dictionary.
 */
template<typename K, typename V>
class ReadOnlyDictionary {
    std::shared_ptr<std::unordered_map<K, V>> dict_;

public:
    /**
     * @brief Constructs a ReadOnlyDictionary wrapping the given shared dictionary.
     * @param dictionary A shared pointer to the underlying dictionary.
     * @throws System::ArgumentNullException if @p dictionary is null.
     */
    explicit ReadOnlyDictionary(std::shared_ptr<std::unordered_map<K, V>> dictionary)
        : dict_(std::move(dictionary)) {
        if (!dict_)
            throw System::ArgumentNullException("dictionary");
    }

    /**
     * @brief Gets an empty ReadOnlyDictionary instance.
     *
     * C++ counterpart of .NET ReadOnlyDictionary<TKey,TValue>.Empty.
     * @return A reference to a shared, permanently-empty instance.
     */
    static ReadOnlyDictionary<K, V>& Empty() {
        static ReadOnlyDictionary<K, V> empty(std::make_shared<std::unordered_map<K, V>>());
        return empty;
    }

    /**
     * @brief Gets the number of key/value pairs in the dictionary.
     *
     * C++ counterpart of .NET ReadOnlyDictionary<TKey,TValue>.Count.
     * @return The number of key/value pairs.
     */
    [[nodiscard]] SharpRuntime::intcs getCountProperty() const {
        return static_cast<SharpRuntime::intcs>(dict_->size());
    }

    /**
     * @brief Gets a value indicating whether the dictionary contains no elements.
     *
     * C++ counterpart of checking Count == 0 on .NET ReadOnlyDictionary<TKey,TValue>.
     * @return true if the dictionary is empty; otherwise false.
     */
    [[nodiscard]] bool getIsEmptyProperty() const {
        return dict_->empty();
    }

    /**
     * @brief Determines whether the dictionary contains the specified key.
     *
     * C++ counterpart of .NET ReadOnlyDictionary<TKey,TValue>.ContainsKey(TKey).
     * @param key The key to locate.
     * @return true if the key is found; otherwise false.
     */
    [[nodiscard]] bool ContainsKey(const K& key) const { return dict_->count(key) > 0; }

    /**
     * @brief Returns a const reference to the value associated with the given key.
     *
     * C++ counterpart of .NET ReadOnlyDictionary<TKey,TValue>.Item[TKey] getter.
     * @param key The key whose value to get.
     * @return A const reference to the value.
     * @throws System::Collections::Generic::KeyNotFoundException if the key is not found.
     */
    [[nodiscard]] const V& operator[](const K& key) const {
        auto it = dict_->find(key);
        if (it == dict_->end())
            throw System::Collections::Generic::KeyNotFoundException();
        return it->second;
    }

    /**
     * @brief Gets the value associated with the specified key.
     *
     * C++ counterpart of .NET ReadOnlyDictionary<TKey,TValue>.TryGetValue(TKey, out TValue).
     * @param key   The key to locate.
     * @param value Receives the value if found.
     * @return true if the key was found; otherwise false.
     */
    [[nodiscard]] bool TryGetValue(const K& key, V& value) const {
        auto it = dict_->find(key);
        if (it == dict_->end()) return false;
        value = it->second;
        return true;
    }

    /**
     * @brief Returns a vector of all keys in the dictionary.
     *
     * C++ counterpart of .NET ReadOnlyDictionary<TKey,TValue>.Keys.
     * @return A vector containing all keys.
     */
    [[nodiscard]] std::vector<K> getKeysProperty() const {
        std::vector<K> keys;
        keys.reserve(dict_->size());
        for (auto& p : *dict_) keys.push_back(p.first);
        return keys;
    }

    /**
     * @brief Returns a vector of all values in the dictionary.
     *
     * C++ counterpart of .NET ReadOnlyDictionary<TKey,TValue>.Values.
     * @return A vector containing all values.
     */
    [[nodiscard]] std::vector<V> getValuesProperty() const {
        std::vector<V> vals;
        vals.reserve(dict_->size());
        for (auto& p : *dict_) vals.push_back(p.second);
        return vals;
    }

    /** @brief Returns a const iterator to the beginning of the dictionary (STL interop). */
    auto begin() const { return dict_->begin(); }
    /** @brief Returns a const iterator past the end of the dictionary (STL interop). */
    auto end()   const { return dict_->end(); }

protected:
    /**
     * @brief Gets the dictionary that is wrapped by this ReadOnlyDictionary.
     *
     * C++ counterpart of .NET ReadOnlyDictionary<TKey,TValue>.Dictionary (protected).
     * @return A const reference to the wrapped map, for use by subclasses.
     */
    [[nodiscard]] const std::unordered_map<K, V>& getDictionaryProperty() const { return *dict_; }
};

} // namespace System::Collections::ObjectModel
