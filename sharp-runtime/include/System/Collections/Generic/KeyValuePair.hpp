// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <sstream>
#include <string>
#include <utility>

namespace System::Collections::Generic {

/**
 * @brief Defines a key/value pair that can be set or retrieved.
 *
 * C++ counterpart of .NET System.Collections.Generic.KeyValuePair<TKey,TValue>.
 * Used as the element type of dictionary enumerators and read-only dictionary interfaces.
 *
 * @tparam TKey   The type of the key.
 * @tparam TValue The type of the value.
 */
template<typename TKey, typename TValue>
struct KeyValuePair {
    /** @brief The key component of the pair. */
    TKey Key{};
    /** @brief The value component of the pair. */
    TValue Value{};

    /** @brief Value-initializes a KeyValuePair (primitives zeroed, objects default-constructed). */
    KeyValuePair() = default;

    /**
     * @brief Constructs a KeyValuePair with copies of the given key and value.
     * @param key   The key.
     * @param value The value.
     */
    KeyValuePair(const TKey& key, const TValue& value) : Key(key), Value(value) {}

    /**
     * @brief Constructs a KeyValuePair by moving the given key and value.
     * @param key   The key (moved).
     * @param value The value (moved).
     */
    KeyValuePair(TKey&& key, TValue&& value) : Key(std::move(key)), Value(std::move(value)) {}

    /**
     * @brief Returns true if both key and value compare equal to those of the other pair.
     * @param other The pair to compare to.
     * @return true if Key == other.Key and Value == other.Value; otherwise false.
     */
    bool operator==(const KeyValuePair& other) const {
        return Key == other.Key && Value == other.Value;
    }

    /**
     * @brief Returns true if either key or value differs from the other pair.
     * @param other The pair to compare to.
     * @return true if the pairs are not equal; otherwise false.
     */
    bool operator!=(const KeyValuePair& other) const { return !(*this == other); }

    /**
     * @brief Returns a string representation of the pair in the form "[key, value]".
     *
     * C++ counterpart of .NET KeyValuePair<TKey,TValue>.ToString(). Requires TKey and
     * TValue to support operator<<(std::ostream&).
     */
    [[nodiscard]] std::string ToString() const {
        std::ostringstream ss;
        ss << "[" << Key << ", " << Value << "]";
        return ss.str();
    }
};

/**
 * @brief Creates a KeyValuePair<TKey,TValue> with types inferred from the arguments.
 *
 * C++ counterpart of .NET's static (non-generic) KeyValuePair.Create<TKey,TValue>(TKey, TValue)
 * helper class. A free function, not a KeyValuePair<TKey,TValue> static member, since C++ does
 * not allow a class template and a non-template class to share the same name in one namespace
 * the way .NET's generic KeyValuePair<TKey,TValue> and non-generic KeyValuePair class do.
 */
template<typename TKey, typename TValue>
[[nodiscard]] KeyValuePair<TKey, TValue> Create(const TKey& key, const TValue& value) {
    return KeyValuePair<TKey, TValue>(key, value);
}

} // namespace System::Collections::Generic
