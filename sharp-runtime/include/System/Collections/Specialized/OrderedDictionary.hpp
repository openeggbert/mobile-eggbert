// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/NotSupportedException.hpp"

namespace System::Collections::Specialized {

using SharpRuntime::intcs;

/**
 * @brief A dictionary that preserves insertion order and allows access by key or integer index.
 *
 * C++ counterpart of .NET System.Collections.Specialized.OrderedDictionary.
 * Keys and values are std::string (non-generic, mirroring the .NET non-generic version).
 * Provides O(1) key lookup via an internal index map and O(1) index access via parallel vectors.
 *
 * Storage is held behind a shared_ptr so that AsReadOnly() can return a live, read-only *view*
 * of the same data (matching .NET, whose AsReadOnly() wraps the same underlying ArrayList/
 * Hashtable instances) rather than either copying the data or — as a previous version of this
 * header did — flipping the read-only flag on the original instance itself.
 */
class OrderedDictionary {
    struct Storage {
        std::vector<std::string>               keys;
        std::vector<std::string>               values;
        std::unordered_map<std::string, intcs> index;
    };

    std::shared_ptr<Storage> storage_;
    bool                     readOnly_ = false;

    OrderedDictionary(std::shared_ptr<Storage> storage, bool readOnly)
        : storage_(std::move(storage)), readOnly_(readOnly) {}

    void rebuildIndex() {
        storage_->index.clear();
        for (intcs i = 0; i < static_cast<intcs>(storage_->keys.size()); ++i)
            storage_->index[storage_->keys[static_cast<size_t>(i)]] = i;
    }

    void checkMutable() const {
        if (readOnly_)
            throw System::NotSupportedException("Collection is read-only.");
    }

public:
    /**
     * @brief Default-constructs an empty OrderedDictionary.
     *
     * C++ counterpart of .NET OrderedDictionary().
     */
    OrderedDictionary() : storage_(std::make_shared<Storage>()) {}

    /**
     * @brief Constructs an empty OrderedDictionary with an optional comparer.
     *
     * C++ counterpart of .NET OrderedDictionary(IEqualityComparer).
     * The comparer is accepted for API compatibility but ignored.
     */
    explicit OrderedDictionary(std::nullptr_t /*comparer*/) : storage_(std::make_shared<Storage>()) {}

    /**
     * @brief Constructs an empty OrderedDictionary with an initial capacity hint.
     *
     * C++ counterpart of .NET OrderedDictionary(int).
     * @param capacity Initial capacity hint; ignored in this implementation.
     */
    explicit OrderedDictionary(int /*capacity*/) : storage_(std::make_shared<Storage>()) {}

    /**
     * @brief Constructs an empty OrderedDictionary with a capacity hint and optional comparer.
     *
     * C++ counterpart of .NET OrderedDictionary(int, IEqualityComparer).
     * @param capacity  Initial capacity hint; ignored.
     * @param comparer  Equality comparer; accepted for API compatibility but ignored.
     */
    OrderedDictionary(int /*capacity*/, std::nullptr_t /*comparer*/) : storage_(std::make_shared<Storage>()) {}

    /**
     * @brief Gets the number of key/value pairs in the dictionary.
     *
     * C++ counterpart of .NET OrderedDictionary.Count.
     * @return The number of entries.
     */
    [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(storage_->keys.size()); }

    /**
     * @brief Gets a value indicating whether the dictionary is read-only.
     *
     * C++ counterpart of .NET OrderedDictionary.IsReadOnly.
     * @return true if this instance was obtained via AsReadOnly(); otherwise false.
     */
    [[nodiscard]] bool getIsReadOnlyProperty() const { return readOnly_; }

    /**
     * @brief Gets a value indicating whether the dictionary has a fixed size.
     *
     * C++ counterpart of .NET IDictionary.IsFixedSize (explicit implementation).
     * @return true for a read-only view; otherwise false (this dictionary can grow).
     */
    [[nodiscard]] bool getIsFixedSizeProperty() const { return readOnly_; }

    /**
     * @brief Gets a value indicating whether access to the dictionary is thread-safe.
     *
     * C++ counterpart of .NET ICollection.IsSynchronized (explicit implementation).
     * @return Always false — this implementation is not thread-safe.
     */
    [[nodiscard]] bool getIsSynchronizedProperty() const { return false; }

    /**
     * @brief Gets an object that can be used to synchronize access to the dictionary.
     *
     * C++ counterpart of .NET ICollection.SyncRoot (explicit implementation).
     * @return A pointer to this instance used as a synchronization token.
     */
    [[nodiscard]] const void* getSyncRootProperty() const { return this; }

    /**
     * @brief Gets a vector of all keys in insertion order.
     *
     * C++ counterpart of .NET OrderedDictionary.Keys.
     * @return A const reference to the ordered key vector.
     */
    [[nodiscard]] const std::vector<std::string>& getKeysProperty() const { return storage_->keys; }

    /**
     * @brief Gets a vector of all values in insertion order.
     *
     * C++ counterpart of .NET OrderedDictionary.Values.
     * @return A const reference to the ordered value vector.
     */
    [[nodiscard]] const std::vector<std::string>& getValuesProperty() const { return storage_->values; }

    /**
     * @brief Returns a read-only view sharing this dictionary's underlying storage.
     *
     * C++ counterpart of .NET OrderedDictionary.AsReadOnly().
     * The returned instance is a live view: further mutations to @c this are visible through it.
     * @c this itself is left fully mutable — only the returned wrapper is read-only.
     * @return A new OrderedDictionary wrapping the same storage in read-only mode.
     */
    [[nodiscard]] OrderedDictionary AsReadOnly() const { return OrderedDictionary(storage_, true); }

    /**
     * @brief Adds a key/value pair at the end of the dictionary.
     *
     * C++ counterpart of .NET OrderedDictionary.Add(object, object).
     * @param key   The key to add.
     * @param value The value to associate with @p key.
     * @throws System::NotSupportedException if the dictionary is read-only.
     * @throws System::ArgumentException if @p key already exists.
     */
    void Add(const std::string& key, const std::string& value) {
        checkMutable();
        if (storage_->index.count(key))
            throw System::ArgumentException("An item with the same key has already been added.");
        storage_->index[key] = static_cast<intcs>(storage_->keys.size());
        storage_->keys.push_back(key);
        storage_->values.push_back(value);
    }

    /**
     * @brief Inserts a key/value pair at the specified zero-based index.
     *
     * C++ counterpart of .NET OrderedDictionary.Insert(int, object, object).
     * @param index The position at which to insert; may equal Count to append.
     * @param key   The key to insert.
     * @param value The value to associate with @p key.
     * @throws System::NotSupportedException if the dictionary is read-only.
     * @throws System::ArgumentOutOfRangeException if @p index is negative or greater than Count.
     * @throws System::ArgumentException if @p key already exists.
     */
    void Insert(intcs index, const std::string& key, const std::string& value) {
        checkMutable();
        if (index < 0 || index > getCountProperty())
            throw System::ArgumentOutOfRangeException("index");
        if (storage_->index.count(key))
            throw System::ArgumentException("An item with the same key has already been added.");
        storage_->keys.insert(storage_->keys.begin() + index, key);
        storage_->values.insert(storage_->values.begin() + index, value);
        rebuildIndex();
    }

    /**
     * @brief Removes the entry with the given key.
     *
     * C++ counterpart of .NET OrderedDictionary.Remove(object).
     * Does nothing if the key is not found.
     * @param key The key to remove.
     * @throws System::NotSupportedException if the dictionary is read-only.
     */
    void Remove(const std::string& key) {
        checkMutable();
        auto it = storage_->index.find(key);
        if (it == storage_->index.end()) return;
        intcs i = it->second;
        storage_->keys.erase(storage_->keys.begin() + i);
        storage_->values.erase(storage_->values.begin() + i);
        rebuildIndex();
    }

    /**
     * @brief Removes the entry at the specified zero-based index.
     *
     * C++ counterpart of .NET OrderedDictionary.RemoveAt(int).
     * @param index The zero-based index of the entry to remove.
     * @throws System::NotSupportedException if the dictionary is read-only.
     * @throws System::ArgumentOutOfRangeException if @p index is out of range.
     */
    void RemoveAt(intcs index) {
        checkMutable();
        if (index < 0 || index >= getCountProperty())
            throw System::ArgumentOutOfRangeException("index");
        Remove(storage_->keys[static_cast<size_t>(index)]);
    }

    /**
     * @brief Removes all entries from the dictionary.
     *
     * C++ counterpart of .NET OrderedDictionary.Clear().
     * @throws System::NotSupportedException if the dictionary is read-only.
     */
    void Clear() {
        checkMutable();
        storage_->keys.clear();
        storage_->values.clear();
        storage_->index.clear();
    }

    /**
     * @brief Determines whether the dictionary contains the given key.
     *
     * C++ counterpart of .NET OrderedDictionary.Contains(object).
     * @param key The key to locate.
     * @return true if an entry with @p key exists; otherwise false.
     */
    [[nodiscard]] bool Contains(const std::string& key) const { return storage_->index.count(key) > 0; }

    /**
     * @brief Returns the value for the given key, or "" if not found.
     *
     * C++ counterpart of .NET OrderedDictionary.Item[object] getter, which returns null for a
     * missing key rather than throwing. Serves both const and non-const instances --
     * deliberately no non-const overload. A prior non-const `std::string&`-returning overload
     * phantom-inserted an empty entry for a missing key even on what looked like a read, and
     * returned a reference into `storage_->values` (a std::vector) that dangled after any
     * later insertion reallocated it -- the same reference-escape bug class fixed in
     * ConcurrentDictionary (commit 3605260). Use `set(key, value)` for
     * `dict[key] = value`-style writes.
     * @param key The key whose value to retrieve.
     * @return The associated value, or "" if @p key is not present.
     */
    [[nodiscard]] std::string operator[](const std::string& key) const {
        auto it = storage_->index.find(key);
        return (it != storage_->index.end()) ? storage_->values[static_cast<size_t>(it->second)] : std::string{};
    }

    /**
     * @brief Sets the value for the given key, inserting a new entry at the end if absent.
     *
     * C++ counterpart of .NET OrderedDictionary.Item[object] setter.
     * @param key   The key whose value to set.
     * @param value The value to associate with @p key.
     * @throws System::NotSupportedException if the dictionary is read-only.
     */
    void set(const std::string& key, const std::string& value) {
        checkMutable();
        auto it = storage_->index.find(key);
        if (it != storage_->index.end()) {
            storage_->values[static_cast<size_t>(it->second)] = value;
            return;
        }
        storage_->index[key] = static_cast<intcs>(storage_->keys.size());
        storage_->keys.push_back(key);
        storage_->values.push_back(value);
    }

    /**
     * @brief Returns the value at the given zero-based index.
     *
     * C++ counterpart of .NET OrderedDictionary.Item[int] getter.
     * @param index The zero-based index.
     * @return A const reference to the value.
     * @throws System::ArgumentOutOfRangeException if @p index is out of range.
     */
    [[nodiscard]] const std::string& GetByIndex(intcs index) const {
        if (index < 0 || index >= getCountProperty())
            throw System::ArgumentOutOfRangeException("index");
        return storage_->values[static_cast<size_t>(index)];
    }

    /**
     * @brief Sets the value at the given zero-based index, leaving its key unchanged.
     *
     * C++ counterpart of .NET OrderedDictionary.Item[int] setter.
     * @param index The zero-based index.
     * @param value The new value.
     * @throws System::NotSupportedException if the dictionary is read-only.
     * @throws System::ArgumentOutOfRangeException if @p index is out of range.
     */
    void SetByIndex(intcs index, const std::string& value) {
        checkMutable();
        if (index < 0 || index >= getCountProperty())
            throw System::ArgumentOutOfRangeException("index");
        storage_->values[static_cast<size_t>(index)] = value;
    }

    /**
     * @brief Returns the key at the given zero-based index.
     *
     * C++ counterpart of accessing the key by position in .NET OrderedDictionary.
     * @param index The zero-based index.
     * @return A const reference to the key string.
     * @throws System::ArgumentOutOfRangeException if @p index is out of range.
     */
    [[nodiscard]] const std::string& GetKey(intcs index) const {
        if (index < 0 || index >= getCountProperty())
            throw System::ArgumentOutOfRangeException("index");
        return storage_->keys[static_cast<size_t>(index)];
    }
};

} // namespace System::Collections::Specialized
