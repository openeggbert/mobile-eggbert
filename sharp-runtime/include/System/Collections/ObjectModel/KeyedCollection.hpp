// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <unordered_map>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"
#include "System/Collections/ObjectModel/Collection.hpp"

namespace System::Collections::ObjectModel {

using SharpRuntime::intcs;

/**
 * @brief Provides the abstract base class for a collection whose keys are embedded in the values.
 *
 * C++ counterpart of .NET System.Collections.ObjectModel.KeyedCollection<TKey,TItem>.
 * Subclasses must implement GetKeyForItem() to extract the key from each stored item.
 * Provides key-based lookup backed by an internal key-to-index map.
 *
 * Deviation from .NET: this port does not support an injectable IEqualityComparer<TKey>
 * (TKey's own operator== / std::hash is always used) — consistent with every other
 * generic collection in this codebase (Dictionary<K,V>, HashSet<T>, SortedDictionary<K,V>, ...).
 *
 * @tparam TKey  The type of keys in the collection.
 * @tparam TItem The type of items in the collection.
 */
template<typename TKey, typename TItem>
class KeyedCollection : public Collection<TItem> {
    std::unordered_map<TKey, intcs> keyIndex_;

protected:
    /**
     * @brief Extracts the key from the given item.
     *
     * C++ counterpart of .NET KeyedCollection<TKey,TItem>.GetKeyForItem(TItem).
     * Subclasses must implement this method to define the key for each item.
     * @param item The item from which to extract the key.
     * @return The key for the item.
     */
    virtual TKey GetKeyForItem(const TItem& item) const = 0;

    /**
     * @brief Notifies the collection that the key of an already-stored item is about to change.
     *
     * C++ counterpart of .NET KeyedCollection<TKey,TItem>.ChangeItemKey(TItem, TKey).
     * Must be called with @p item still holding its current (old) key — i.e. before the
     * caller actually mutates the field that GetKeyForItem() reads — so the internal index
     * can be moved from the old key to @p newKey atomically.
     * @param item   The item whose key is changing; must already be present in the collection.
     * @param newKey The new key the item is about to take on.
     * @throws System::ArgumentException if @p item is not present in the collection, or if
     *         @p newKey is already used by a different item.
     */
    void ChangeItemKey(const TItem& item, const TKey& newKey) {
        auto it = std::find(this->items_.begin(), this->items_.end(), item);
        if (it == this->items_.end())
            throw System::ArgumentException("The specified item does not exist in this KeyedCollection.");

        TKey oldKey = GetKeyForItem(*it);
        if (!(oldKey == newKey)) {
            if (keyIndex_.count(newKey))
                throw System::ArgumentException("An item with the same key has already been added.");
            intcs index = static_cast<intcs>(it - this->items_.begin());
            keyIndex_.erase(oldKey);
            keyIndex_[newKey] = index;
        }
    }

    /**
     * @brief Inserts @p item at @p index and updates the internal key index.
     *
     * C++ counterpart of .NET KeyedCollection<TKey,TItem>.InsertItem(int, TItem).
     * @param index The zero-based index at which to insert.
     * @param item  The element to insert.
     * @throws System::ArgumentException if an item with the same key already exists.
     * @throws System::ArgumentOutOfRangeException if @p index is not within [0, Count].
     */
    void InsertItem(intcs index, const TItem& item) override {
        TKey key = GetKeyForItem(item);
        if (keyIndex_.count(key))
            throw System::ArgumentException("An item with the same key has already been added.");

        Collection<TItem>::InsertItem(index, item);
        rebuildIndex();
    }

    /**
     * @brief Removes the item at @p index and updates the internal key index.
     *
     * C++ counterpart of .NET KeyedCollection<TKey,TItem>.RemoveItem(int).
     * @param index The zero-based index of the element to remove.
     */
    void RemoveItem(intcs index) override {
        Collection<TItem>::RemoveItem(index);
        rebuildIndex();
    }

    /**
     * @brief Replaces the item at @p index and updates the internal key index.
     *
     * C++ counterpart of .NET KeyedCollection<TKey,TItem>.SetItem(int, TItem).
     * @param index The zero-based index of the element to replace.
     * @param item  The new value.
     * @throws System::ArgumentException if @p item's key collides with a different existing item.
     * @throws System::ArgumentOutOfRangeException if @p index is out of range.
     */
    void SetItem(intcs index, const TItem& item) override {
        if (index < 0 || index >= static_cast<intcs>(this->items_.size()))
            throw System::ArgumentOutOfRangeException("index", "Index was out of range. Must be non-negative and less than the size of the collection.");

        TKey newKey = GetKeyForItem(item);
        TKey oldKey = GetKeyForItem(this->items_[static_cast<size_t>(index)]);
        if (!(oldKey == newKey) && keyIndex_.count(newKey))
            throw System::ArgumentException("An item with the same key has already been added.");

        Collection<TItem>::SetItem(index, item);
        rebuildIndex();
    }

    /**
     * @brief Clears all items and the internal key index.
     *
     * C++ counterpart of .NET KeyedCollection<TKey,TItem>.ClearItems().
     */
    void ClearItems() override {
        keyIndex_.clear();
        Collection<TItem>::ClearItems();
    }

public:
    /** @brief Default-constructs an empty KeyedCollection. */
    KeyedCollection() = default;

    /**
     * @brief Determines whether the collection contains an item with the specified key.
     *
     * C++ counterpart of .NET KeyedCollection<TKey,TItem>.Contains(TKey).
     * @param key The key to locate.
     * @return true if an item with the key is found; otherwise false.
     */
    [[nodiscard]] bool Contains(const TKey& key) const { return keyIndex_.count(key) > 0; }
    using Collection<TItem>::Contains;

    /**
     * @brief Gets the item with the specified key if it exists.
     *
     * C++ counterpart of .NET KeyedCollection<TKey,TItem>.TryGetValue(TKey, out TItem).
     * @param key  The key to locate.
     * @param item Receives the item if found.
     * @return true if the item was found; otherwise false.
     */
    bool TryGetValue(const TKey& key, TItem& item) const {
        auto it = keyIndex_.find(key);
        if (it == keyIndex_.end()) return false;
        item = this->items_[static_cast<size_t>(it->second)];
        return true;
    }

    /**
     * @brief Removes the item with the specified key from the collection.
     *
     * C++ counterpart of .NET KeyedCollection<TKey,TItem>.Remove(TKey).
     * @param key The key of the item to remove.
     * @return true if the item was found and removed; otherwise false.
     */
    bool Remove(const TKey& key) {
        auto it = keyIndex_.find(key);
        if (it == keyIndex_.end()) return false;
        this->RemoveAt(it->second);
        return true;
    }
    using Collection<TItem>::Remove;

    /**
     * @brief Returns a const reference to the item with the specified key.
     *
     * C++ counterpart of .NET KeyedCollection<TKey,TItem>.Item[TKey] getter.
     * @param key The key of the item to retrieve.
     * @return A const reference to the item.
     * @throws System::Collections::Generic::KeyNotFoundException if the key is not found.
     */
    [[nodiscard]] const TItem& operator[](const TKey& key) const {
        auto it = keyIndex_.find(key);
        if (it == keyIndex_.end())
            throw System::Collections::Generic::KeyNotFoundException();
        return this->items_[static_cast<size_t>(it->second)];
    }

    /**
     * @brief Returns a reference to the item with the specified key.
     *
     * C++ extension beyond .NET's get-only key indexer, provided for convenience; mutating
     * the key field through the returned reference without calling ChangeItemKey() first
     * will leave the internal index stale.
     * @param key The key of the item to retrieve.
     * @return A reference to the item.
     * @throws System::Collections::Generic::KeyNotFoundException if the key is not found.
     */
    TItem& operator[](const TKey& key) {
        auto it = keyIndex_.find(key);
        if (it == keyIndex_.end())
            throw System::Collections::Generic::KeyNotFoundException();
        return this->items_[static_cast<size_t>(it->second)];
    }

    // Note: the inherited index-based operator[](intcs) returns a plain T& (see
    // Collection<T>::operator[]'s doc comment), so `keyedCollection[i] = newItem;` writes
    // directly into storage WITHOUT going through SetItem() -- the internal keyIndex_ is left
    // stale after such an assignment. Use Remove()+Add() (or Insert()) instead when the key
    // index must stay in sync; those paths do run SetItem/InsertItem/RemoveItem.
    using Collection<TItem>::operator[];

private:
    void rebuildIndex() {
        keyIndex_.clear();
        for (intcs i = 0; i < static_cast<intcs>(this->items_.size()); ++i)
            keyIndex_[GetKeyForItem(this->items_[i])] = i;
    }
};

} // namespace System::Collections::ObjectModel
