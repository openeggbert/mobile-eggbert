// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <any>
#include <string>

namespace System::Collections {

namespace detail {

/**
 * @brief Best-effort std::any -> std::string conversion for DictionaryEntry::ToString().
 *
 * C++ has no runtime-polymorphic ToString() the way .NET's object does, so std::any
 * can't generically stringify its held value. Recognizes the common primitive types
 * used as dictionary keys/values in this codebase; anything else falls back to a
 * bracketed type name — a disclosed C++ limitation, not a silent bug.
 */
inline std::string dictionaryEntryValueToString(const std::any& value) {
    if (!value.has_value()) return "";
    if (const auto* p = std::any_cast<std::string>(&value))    return *p;
    if (const auto* p = std::any_cast<const char*>(&value))    return *p;
    if (const auto* p = std::any_cast<bool>(&value))           return *p ? "True" : "False";
    if (const auto* p = std::any_cast<int>(&value))            return std::to_string(*p);
    if (const auto* p = std::any_cast<long>(&value))           return std::to_string(*p);
    if (const auto* p = std::any_cast<long long>(&value))      return std::to_string(*p);
    if (const auto* p = std::any_cast<unsigned int>(&value))   return std::to_string(*p);
    if (const auto* p = std::any_cast<unsigned long>(&value))  return std::to_string(*p);
    if (const auto* p = std::any_cast<unsigned long long>(&value)) return std::to_string(*p);
    if (const auto* p = std::any_cast<double>(&value))         return std::to_string(*p);
    if (const auto* p = std::any_cast<float>(&value))          return std::to_string(*p);
    if (const auto* p = std::any_cast<char>(&value))           return std::string(1, *p);
    return "{" + std::string(value.type().name()) + "}";
}

} // namespace detail

/**
 * @brief Defines a dictionary key/value pair that can be set or retrieved.
 *
 * C++ counterpart of .NET System.Collections.DictionaryEntry.
 * Used by IDictionary and IDictionaryEnumerator to represent a non-generic entry.
 */
struct DictionaryEntry {
private:
    std::any key_;
    std::any value_;

public:
    /**
     * @brief Default-constructs a DictionaryEntry with empty key and value.
     *
     * C++ counterpart of the implicit default constructor.
     */
    DictionaryEntry() = default;

    /**
     * @brief Constructs a DictionaryEntry with the given key and value.
     *
     * C++ counterpart of .NET DictionaryEntry(object key, object? value).
     * @param key   The key of the entry.
     * @param value The value of the entry.
     */
    DictionaryEntry(std::any key, std::any value)
        : key_(std::move(key)), value_(std::move(value)) {}

    /**
     * @brief Gets the key of the entry.
     *
     * C++ counterpart of .NET DictionaryEntry.Key getter.
     * @return The key stored in this entry.
     */
    [[nodiscard]] const std::any& getKeyProperty() const { return key_; }

    /**
     * @brief Sets the key of the entry.
     *
     * C++ counterpart of .NET DictionaryEntry.Key setter.
     * @param key The new key value.
     */
    void setKeyProperty(std::any key) { key_ = std::move(key); }

    /**
     * @brief Gets the value of the entry.
     *
     * C++ counterpart of .NET DictionaryEntry.Value getter.
     * @return The value stored in this entry.
     */
    [[nodiscard]] const std::any& getValueProperty() const { return value_; }

    /**
     * @brief Sets the value of the entry.
     *
     * C++ counterpart of .NET DictionaryEntry.Value setter.
     * @param value The new value.
     */
    void setValueProperty(std::any value) { value_ = std::move(value); }

    /**
     * @brief Deconstructs the entry into its key and value components.
     *
     * C++ counterpart of .NET DictionaryEntry.Deconstruct(out object, out object?).
     * @param key   Receives the key.
     * @param value Receives the value.
     */
    void Deconstruct(std::any& key, std::any& value) const {
        key   = key_;
        value = value_;
    }

    /**
     * @brief Returns a string representation of the key/value pair.
     *
     * C++ counterpart of .NET DictionaryEntry.ToString(), which delegates to
     * KeyValuePair.PairToString and formats as "[key, value]" using each value's
     * own string representation (not its type name).
     * @return String in the form "[key, value]".
     */
    [[nodiscard]] std::string ToString() const {
        return "[" + detail::dictionaryEntryValueToString(key_) + ", "
                    + detail::dictionaryEntryValueToString(value_) + "]";
    }
};

} // namespace System::Collections
