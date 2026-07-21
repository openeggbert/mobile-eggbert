// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <any>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Collections/DictionaryEntry.hpp"
#include "System/Collections/IDictionary.hpp"
#include "System/Collections/IDictionaryEnumerator.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Collections {

using SharpRuntime::bytecs;
using SharpRuntime::intcs;

namespace detail {

/**
 * @brief Best-effort std::any value equality, shared shape with ArrayList's
 * arrayListValueEquals() (see System/Collections/ArrayList.hpp for the full rationale):
 * std::any has no built-in equality operator, so common primitive/string types are compared
 * by value and anything else reports never-equal rather than a type-only false-positive match.
 */
inline bool hashtableValueEquals(const std::any& a, const std::any& b) {
    if (a.type() != b.type()) return false;
    if (const auto* p = std::any_cast<std::string>(&a))            return *p == *std::any_cast<std::string>(&b);
    if (const auto* p = std::any_cast<const char*>(&a))            return std::string(*p) == std::string(*std::any_cast<const char*>(&b));
    if (const auto* p = std::any_cast<bool>(&a))                   return *p == *std::any_cast<bool>(&b);
    if (const auto* p = std::any_cast<int>(&a))                    return *p == *std::any_cast<int>(&b);
    if (const auto* p = std::any_cast<long>(&a))                   return *p == *std::any_cast<long>(&b);
    if (const auto* p = std::any_cast<long long>(&a))              return *p == *std::any_cast<long long>(&b);
    if (const auto* p = std::any_cast<unsigned int>(&a))           return *p == *std::any_cast<unsigned int>(&b);
    if (const auto* p = std::any_cast<unsigned long>(&a))          return *p == *std::any_cast<unsigned long>(&b);
    if (const auto* p = std::any_cast<unsigned long long>(&a))     return *p == *std::any_cast<unsigned long long>(&b);
    if (const auto* p = std::any_cast<double>(&a))                 return *p == *std::any_cast<double>(&b);
    if (const auto* p = std::any_cast<float>(&a))                  return *p == *std::any_cast<float>(&b);
    if (const auto* p = std::any_cast<char>(&a))                   return *p == *std::any_cast<char>(&b);
    if (const auto* p = std::any_cast<void*>(&a))                  return *p == *std::any_cast<void*>(&b);
    return false;
}

} // namespace detail

/**
 * @brief Represents a collection of key/value pairs organized based on the hash code of the key.
 *
 * C++ counterpart of .NET System.Collections.Hashtable.
 * Keys are stored as std::string (address-stringified for void* keys, or direct for string overloads).
 * Values are stored as std::any.
 */
class Hashtable : public IDictionary {
public:
    /** @brief Constructs an empty Hashtable. */
    Hashtable() = default;
    /** @brief Constructs a Hashtable; the capacity hint is accepted but ignored in this port. */
    explicit Hashtable(int /*capacity*/) {}

    /**
     * @brief Returns the number of key/value pairs in the table.
     * C++ counterpart of .NET Hashtable.Count.
     */
    [[nodiscard]] int getCountProperty() const override {
        return static_cast<int>(_map.size());
    }

    /**
     * @brief Copies each key/value pair into @p array as a DictionaryEntry, starting at @p index.
     *
     * C++ counterpart of .NET Hashtable.CopyTo(Array, int), matching real .NET's default
     * behavior of copying DictionaryEntry elements (the typed KeyValuePair[] fast path doesn't
     * apply here, since C++ has no equivalent runtime array-element-type check).
     * @param array Pointer to a DictionaryEntry[] destination buffer.
     * @param index Zero-based index at which copying begins.
     */
    void CopyTo(void* array, int index) override {
        auto* dest = static_cast<DictionaryEntry*>(array);
        int i = index;
        for (const auto& [k, v] : _map) dest[i++] = DictionaryEntry(k, v);
    }

    /** @brief Returns false; Hashtable is never read-only. */
    [[nodiscard]] bool getIsReadOnlyProperty()  const override { return false; }
    /** @brief Returns false; Hashtable is never fixed-size. */
    [[nodiscard]] bool getIsFixedSizeProperty() const override { return false; }
    /** @brief Returns false; Hashtable is not thread-safe. */
    [[nodiscard]] bool getIsSynchronizedProperty() const override { return false; }

    /**
     * @brief Gets the value associated with the specified key.
     * C++ counterpart of .NET Hashtable indexer getter (this[object key]).
     */
    [[nodiscard]] void* getItem(const void* key) const override {
        auto k = toKey(key);
        auto it = _map.find(k);
        if (it == _map.end()) return nullptr;
        return const_cast<std::any*>(&it->second);
    }

    /**
     * @brief Sets the value associated with the specified key.
     * C++ counterpart of .NET Hashtable indexer setter (this[object key] = value).
     */
    void setItem(const void* key, void* value) override {
        _map[toKey(key)] = value ? *static_cast<std::any*>(value) : std::any{};
        ++version_;
    }

    /**
     * @brief Gets an ICollection containing the keys of the Hashtable.
     *
     * C++ counterpart of .NET Hashtable.Keys.
     * @return nullptr. Real .NET returns a live KeyCollection view backed by the hashtable's own
     *         buckets; building an equivalent live ICollection view here would need its own
     *         ICollection subclass (Count/CopyTo/GetEnumerator all delegating back to this
     *         Hashtable) with a defined lifetime/ownership story, which is out of scope for a
     *         single audit pass. Use getKeys() for a same-instant snapshot of the string keys
     *         instead.
     */
    [[nodiscard]] ICollection* getKeysProperty() const override { return nullptr; }

    /**
     * @brief Gets an ICollection containing the values of the Hashtable.
     *
     * C++ counterpart of .NET Hashtable.Values.
     * @return nullptr — same documented gap as getKeysProperty(); use getValues() for a
     *         same-instant snapshot instead.
     */
    [[nodiscard]] ICollection* getValuesProperty() const override { return nullptr; }

    /**
     * @brief Returns true if the Hashtable contains the specified key.
     * C++ counterpart of .NET Hashtable.Contains(object).
     */
    [[nodiscard]] bool Contains(const void* key) const override {
        return _map.count(toKey(key)) > 0;
    }

    /**
     * @brief Returns true if the table contains the specified string key.
     * @param key String key to look up.
     */
    bool ContainsKey(const std::string& key) const { return _map.count(key) > 0; }

    /**
     * @brief Returns true if any stored value has the same type as @p value.
     * C++ counterpart of .NET Hashtable.ContainsValue(object?).
     */
    bool ContainsValue(const std::any& value) const {
        for (const auto& [k, v] : _map)
            if (detail::hashtableValueEquals(v, value)) return true;
        return false;
    }

    /**
     * @brief Adds an element with the specified key and value.
     * C++ counterpart of .NET Hashtable.Add(object, object?).
     * @throws System::ArgumentException if the key already exists.
     */
    void Add(const void* key, void* value) override {
        std::string k = toKey(key);
        if (_map.count(k)) throw System::ArgumentException("Item has already been added. Key in dictionary: '" + k + "'");
        _map[k] = value ? *static_cast<std::any*>(value) : std::any{};
        ++version_;
    }

    /**
     * @brief Adds a string key with a typed value.
     * @throws System::ArgumentException if the key already exists.
     */
    void Add(const std::string& key, const std::any& value) {
        if (_map.count(key)) throw System::ArgumentException("Item has already been added. Key in dictionary: '" + key + "'");
        _map[key] = value;
        ++version_;
    }

    /**
     * @brief Removes all key/value pairs from the table.
     * C++ counterpart of .NET Hashtable.Clear().
     */
    void Clear() override { _map.clear(); ++version_; }

    /**
     * @brief Removes the element with the specified key.
     * C++ counterpart of .NET Hashtable.Remove(object).
     */
    void Remove(const void* key) override { _map.erase(toKey(key)); ++version_; }

    /**
     * @brief Removes the entry with the specified string key.
     * @param key String key to remove.
     */
    void Remove(const std::string& key) { _map.erase(key); ++version_; }

    /**
     * @brief Removes the entry with the specified C-string key.
     * @param key C-string key to remove.
     */
    void Remove(const char* key) { _map.erase(key); ++version_; }

    /**
     * @brief Returns a reference to the value for the given string key, inserting a default if absent.
     *
     * @note Unlike setItem()/the .NET indexer setter, an insertion or assignment made through
     * the reference this returns does not bump the fail-fast version counter (there is no way
     * to intercept a plain C++ reference assignment or std::unordered_map's own key-insertion) —
     * the same documented, narrow gap as ArrayList::operator[] (see ArrayList.hpp). Prefer
     * setItem()/Add() when fail-fast enumeration correctness matters.
     * @param key String key.
     */
    std::any& operator[](const std::string& key) { return _map[key]; }

    /**
     * @brief Returns a const reference to the value for the given string key.
     * @throws std::out_of_range if the key is absent.
     */
    const std::any& at(const std::string& key) const { return _map.at(key); }

    /** @brief Returns a vector of all string keys in the table. */
    std::vector<std::string> getKeys() const {
        std::vector<std::string> keys;
        keys.reserve(_map.size());
        for (const auto& [k, v] : _map) keys.push_back(k);
        return keys;
    }

    /** @brief Returns a vector of all values in the table. */
    std::vector<std::any> getValues() const {
        std::vector<std::any> vals;
        vals.reserve(_map.size());
        for (const auto& [k, v] : _map) vals.push_back(v);
        return vals;
    }

    /**
     * @brief Returns a heap-allocated enumerator over the Hashtable's entries; caller takes ownership.
     *
     * C++ counterpart of .NET Hashtable.GetEnumerator(). The enumerator throws
     * InvalidOperationException on MoveNext()/Reset()/getCurrentProperty() (or Key/Value/Entry access)
     * if the Hashtable is structurally modified while enumeration is in progress, matching
     * .NET's fail-fast contract. Iteration order is unspecified (std::unordered_map bucket
     * order), matching .NET's own documented "no particular order" guarantee for Hashtable.
     */
    IDictionaryEnumerator* GetEnumerator() override { return new Enumerator(this); }

private:
    std::unordered_map<std::string, std::any> _map;
    intcs version_ = 0;

    static std::string toKey(const void* key) {
        return std::to_string(reinterpret_cast<uintptr_t>(key));
    }

    /**
     * @brief Enumerator over a Hashtable's key/value pairs, matching the fail-fast versioning
     * pattern established by Queue::Enumerator/Stack::Enumerator/ArrayList::Enumerator in this
     * codebase.
     */
    class Enumerator : public IDictionaryEnumerator {
        const Hashtable* ht_;
        intcs version_;
        std::unordered_map<std::string, std::any>::const_iterator it_;
        bool started_ = false;
        bool valid_ = false;
        mutable DictionaryEntry current_;

    public:
        explicit Enumerator(const Hashtable* ht) : ht_(ht), version_(ht->version_), it_(ht->_map.begin()) {}

        bool MoveNext() override {
            if (version_ != ht_->version_) throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            if (started_) { if (valid_) ++it_; } else { started_ = true; }
            valid_ = (it_ != ht_->_map.end());
            if (valid_) current_ = DictionaryEntry(it_->first, it_->second);
            return valid_;
        }

        void Reset() override {
            if (version_ != ht_->version_) throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            it_ = ht_->_map.begin();
            started_ = false;
            valid_ = false;
        }

        [[nodiscard]] void* getCurrentProperty() const override {
            ensureCurrent();
            return &current_;
        }

        [[nodiscard]] DictionaryEntry getEntryProperty() const override {
            ensureCurrent();
            return current_;
        }

        [[nodiscard]] const void* getKeyProperty() const override {
            ensureCurrent();
            return &it_->first;
        }

        [[nodiscard]] const void* getValueProperty() const override {
            ensureCurrent();
            return &it_->second;
        }

    private:
        void ensureCurrent() const {
            if (!started_ || !valid_)
                throw System::InvalidOperationException("Enumeration has either not started or has already finished.");
        }
    };
};

} // namespace System::Collections
