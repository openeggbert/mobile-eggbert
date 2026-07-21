// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::Collections::Specialized {

using SharpRuntime::intcs;

namespace detail {

/** @brief Ordinal case-insensitive hash for NameValueCollection keys (matches .NET's default comparer). */
struct NameValueCollectionKeyHash {
    [[nodiscard]] size_t operator()(const std::string& key) const {
        std::string lower = key;
        std::ranges::transform(lower, lower.begin(),
                                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return std::hash<std::string>{}(lower);
    }
};

/** @brief Ordinal case-insensitive equality for NameValueCollection keys. */
struct NameValueCollectionKeyEqual {
    [[nodiscard]] bool operator()(const std::string& a, const std::string& b) const {
        return a.size() == b.size() &&
               std::ranges::equal(a, b, [](unsigned char c1, unsigned char c2) {
                   return std::tolower(c1) == std::tolower(c2);
               });
    }
};

} // namespace detail

/**
 * @brief A collection that associates string keys with one or more string values.
 *
 * C++ counterpart of .NET System.Collections.Specialized.NameValueCollection.
 * Multiple values for the same key are stored and returned comma-joined by Get().
 * Insertion order of keys is preserved. Keys are compared ordinal case-insensitively,
 * matching .NET's default (CaseInsensitiveComparer) behavior; AllKeys/Keys retain the
 * original casing of the first insertion.
 */
class NameValueCollection {
    std::vector<std::string> keys_;
    std::unordered_map<std::string, std::vector<std::string>,
                        detail::NameValueCollectionKeyHash, detail::NameValueCollectionKeyEqual> map_;

    static std::string joinComma(const std::vector<std::string>& v) {
        std::string out;
        for (size_t i = 0; i < v.size(); ++i) {
            if (i) out += ',';
            out += v[i];
        }
        return out;
    }

    // .NET's index-based accessors (Get(int)/GetValues(int)/GetKey(int)/this[int]) delegate
    // to NameObjectCollectionBase.BaseGet/BaseGetKey, which index an internal ArrayList --
    // whose indexer throws ArgumentOutOfRangeException for an out-of-range index, not a
    // silent empty/default result.
    void checkIndex(intcs index) const {
        if (index < 0 || static_cast<size_t>(index) >= keys_.size())
            throw System::ArgumentOutOfRangeException("index");
    }

public:
    /**
     * @brief Default-constructs an empty NameValueCollection.
     *
     * C++ counterpart of .NET NameValueCollection().
     */
    NameValueCollection() = default;

    /**
     * @brief Constructs an empty NameValueCollection with an initial capacity hint.
     *
     * C++ counterpart of .NET NameValueCollection(int).
     * @param capacity Initial capacity hint; ignored in this implementation.
     */
    explicit NameValueCollection(int /*capacity*/) {}

    /**
     * @brief Copy-constructs a NameValueCollection from @p col.
     *
     * C++ counterpart of .NET NameValueCollection(NameValueCollection).
     * @param col The collection to copy entries from.
     */
    NameValueCollection(const NameValueCollection& col) {
        keys_ = col.keys_;
        map_  = col.map_;
    }

    /**
     * @brief Constructs a NameValueCollection with a capacity hint pre-populated from @p col.
     *
     * C++ counterpart of .NET NameValueCollection(int, NameValueCollection).
     * @param capacity Initial capacity hint; ignored in this implementation.
     * @param col      The collection to copy entries from.
     */
    NameValueCollection(int /*capacity*/, const NameValueCollection& col) {
        keys_ = col.keys_;
        map_  = col.map_;
    }

    /**
     * @brief Gets the number of unique keys in the collection.
     *
     * C++ counterpart of .NET NameValueCollection.Count.
     * @return The number of distinct keys.
     */
    [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(keys_.size()); }

    /**
     * @brief Returns a vector of all keys in insertion order.
     *
     * C++ counterpart of .NET NameValueCollection.AllKeys.
     * @return A const reference to the ordered key list.
     */
    [[nodiscard]] const std::vector<std::string>& AllKeys() const { return keys_; }

    /**
     * @brief Returns true if the collection contains at least one key.
     *
     * C++ counterpart of .NET NameValueCollection.HasKeys().
     * @return true if Count > 0; otherwise false.
     */
    [[nodiscard]] bool HasKeys() const { return !keys_.empty(); }

    /**
     * @brief Adds a name/value pair to the collection.
     *
     * C++ counterpart of .NET NameValueCollection.Add(string, string).
     * Multiple values can be associated with the same @p name.
     * @param name  The key to add or extend.
     * @param value The value to associate with @p name.
     */
    void Add(const std::string& name, const std::string& value) {
        if (!map_.count(name)) keys_.push_back(name);
        map_[name].push_back(value);
    }

    /**
     * @brief Copies all entries from @p c into this collection.
     *
     * C++ counterpart of .NET NameValueCollection.Add(NameValueCollection).
     * @param c The collection whose entries to add.
     */
    void Add(const NameValueCollection& c) {
        for (const auto& key : c.keys_)
            for (const auto& val : c.map_.at(key))
                Add(key, val);
    }

    /**
     * @brief Sets @p name to a single value, replacing any existing values.
     *
     * C++ counterpart of .NET NameValueCollection.Set(string, string).
     * @param name  The key to set.
     * @param value The single value to associate with @p name.
     */
    void Set(const std::string& name, const std::string& value) {
        if (!map_.count(name)) keys_.push_back(name);
        map_[name] = {value};
    }

    /**
     * @brief Removes all values for the given key.
     *
     * C++ counterpart of .NET NameValueCollection.Remove(string).
     * @param name The key to remove.
     */
    void Remove(const std::string& name) {
        map_.erase(name);
        detail::NameValueCollectionKeyEqual equal;
        keys_.erase(std::remove_if(keys_.begin(), keys_.end(),
                                    [&](const std::string& k) { return equal(k, name); }),
                    keys_.end());
    }

    /**
     * @brief Removes all entries from the collection.
     *
     * C++ counterpart of .NET NameValueCollection.Clear().
     */
    void Clear() { keys_.clear(); map_.clear(); }

    /**
     * @brief Returns all values for @p name as a comma-joined string, or "" if not found.
     *
     * C++ counterpart of .NET NameValueCollection.Get(string).
     * @param name The key whose values to retrieve.
     * @return A comma-joined string of values, or "" if not found.
     */
    [[nodiscard]] std::string Get(const std::string& name) const {
        auto it = map_.find(name);
        if (it == map_.end() || it->second.empty()) return "";
        return joinComma(it->second);
    }

    /**
     * @brief Returns all values for the key at @p index as a comma-joined string.
     *
     * C++ counterpart of .NET NameValueCollection.Get(int), which delegates to
     * NameObjectCollectionBase.BaseGet(int) and, through it, an ArrayList indexer that
     * throws for an out-of-range index -- verified against NameValueCollection.cs and
     * NameObjectCollectionBase.cs. This previously returned "" instead.
     * @param index The zero-based index of the key.
     * @return A comma-joined string of values.
     * @throws System::ArgumentOutOfRangeException if @p index is negative or >= Count.
     */
    [[nodiscard]] std::string Get(intcs index) const {
        checkIndex(index);
        return Get(keys_[static_cast<size_t>(index)]);
    }

    /**
     * @brief Returns all values associated with @p name.
     *
     * C++ counterpart of .NET NameValueCollection.GetValues(string).
     * @param name The key whose values to retrieve.
     * @return A vector of values, or an empty vector if not found.
     */
    [[nodiscard]] std::vector<std::string> GetValues(const std::string& name) const {
        auto it = map_.find(name);
        if (it == map_.end()) return {};
        return it->second;
    }

    /**
     * @brief Returns all values associated with the key at @p index.
     *
     * C++ counterpart of .NET NameValueCollection.GetValues(int).
     * @param index The zero-based index of the key.
     * @return A vector of values.
     * @throws System::ArgumentOutOfRangeException if @p index is negative or >= Count.
     */
    [[nodiscard]] std::vector<std::string> GetValues(intcs index) const {
        checkIndex(index);
        return GetValues(keys_[static_cast<size_t>(index)]);
    }

    /**
     * @brief Returns the key at the given zero-based index.
     *
     * C++ counterpart of .NET NameValueCollection.GetKey(int).
     * @param index The zero-based index of the key.
     * @return A const reference to the key string.
     * @throws System::ArgumentOutOfRangeException if @p index is negative or >= Count.
     */
    [[nodiscard]] const std::string& GetKey(intcs index) const {
        checkIndex(index);
        return keys_[static_cast<size_t>(index)];
    }

    /**
     * @brief Returns all values for @p name as a comma-joined string, or "" if not found.
     *
     * C++ counterpart of .NET NameValueCollection.Item[string] getter.
     * @param name The key whose values to retrieve.
     * @return A comma-joined string of values, or "".
     */
    [[nodiscard]] std::string operator[](const std::string& name) const { return Get(name); }

    /**
     * @brief Returns all values for the key at @p index as a comma-joined string.
     *
     * C++ counterpart of .NET NameValueCollection.Item[int] getter.
     * @param index The zero-based index of the key.
     * @return A comma-joined string of values.
     * @throws System::ArgumentOutOfRangeException if @p index is negative or >= Count.
     */
    [[nodiscard]] std::string operator[](intcs index) const { return Get(index); }
};

} // namespace System::Collections::Specialized
