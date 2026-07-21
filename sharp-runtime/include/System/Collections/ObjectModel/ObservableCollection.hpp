// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ComponentModel/INotifyPropertyChanged.hpp"
#include "System/Collections/ObjectModel/Collection.hpp"
#include "System/Collections/Specialized/NotifyCollectionChangedAction.hpp"
#include "System/Collections/Specialized/NotifyCollectionChangedEventArgs.hpp"
#include "System/Collections/Specialized/NotifyCollectionChangedEventHandler.hpp"

namespace System::Collections::ObjectModel {

using SharpRuntime::intcs;
using System::Collections::Specialized::NotifyCollectionChangedAction;
using System::Collections::Specialized::NotifyCollectionChangedEventArgs;
using System::Collections::Specialized::NotifyCollectionChangedEventHandler;

/**
 * @brief A dynamic data collection that provides notifications when items are added,
 *        removed, replaced, moved, or when the whole list is refreshed.
 *
 * C++ counterpart of .NET System.Collections.ObjectModel.ObservableCollection<T>.
 * Subscribers register via the CollectionChanged vector of handler callbacks; property-change
 * notifications for "Count" and "Item[]" are raised through the inherited INotifyPropertyChanged.
 *
 * Unlike the pre-existing implementation of this header, all mutation paths (Add, Remove, Clear,
 * Insert, RemoveAt, Move, and the protected SetItem hook) are funneled through the same four
 * protected virtual hooks (InsertItem/RemoveItem/ClearItems/SetItem) that Collection<T> already
 * calls internally — matching .NET, where ObservableCollection<T> overrides only those hooks and
 * never Add/Remove/Clear/Insert/RemoveAt themselves. Previously Insert()/RemoveAt() silently
 * bypassed CollectionChanged entirely because only the public Add/Remove/Clear were overridden.
 *
 * @tparam T The type of elements in the collection.
 */
template<typename T>
class ObservableCollection : public Collection<T>, public System::ComponentModel::INotifyPropertyChanged {
    intcs blockReentrancyCount_ = 0;

    void requireIndexInRange(intcs index) const {
        if (index < 0 || index >= static_cast<intcs>(this->items_.size()))
            throw System::ArgumentOutOfRangeException("index", "Index was out of range. Must be non-negative and less than the size of the collection.");
    }

    void OnCountPropertyChanged() { this->OnPropertyChanged("Count"); }
    void OnIndexerPropertyChanged() { this->OnPropertyChanged("Item[]"); }

public:
    /** @brief Handler type for CollectionChanged subscribers. */
    using ChangedHandler = NotifyCollectionChangedEventHandler<T>;

    /** @brief List of CollectionChanged event subscribers. */
    std::vector<ChangedHandler> CollectionChanged;

    /** @brief RAII guard returned by BlockReentrancy(); releases the guard on destruction. */
    class ReentrancyGuard {
        ObservableCollection* owner_;
    public:
        explicit ReentrancyGuard(ObservableCollection* owner) : owner_(owner) {}
        ReentrancyGuard(const ReentrancyGuard&) = delete;
        ReentrancyGuard& operator=(const ReentrancyGuard&) = delete;
        ~ReentrancyGuard() { owner_->blockReentrancyCount_--; }
    };

    /** @brief Default-constructs an empty ObservableCollection. */
    ObservableCollection() = default;

    /**
     * @brief Constructs an ObservableCollection pre-populated with the given items.
     *
     * C++ counterpart of .NET ObservableCollection<T>(IEnumerable<T>) /
     * ObservableCollection<T>(List<T>). Matches .NET semantics: the items are copied in
     * directly with no CollectionChanged/PropertyChanged notifications during construction.
     * @param items The initial items.
     */
    explicit ObservableCollection(std::vector<T> items) {
        this->items_ = std::move(items);
    }

    /**
     * @brief Moves the item at @p oldIndex to @p newIndex and fires a CollectionChanged Move event.
     *
     * C++ counterpart of .NET ObservableCollection<T>.Move(int, int).
     * @param oldIndex The zero-based index of the item to move.
     * @param newIndex The zero-based index to move the item to.
     */
    void Move(intcs oldIndex, intcs newIndex) { MoveItem(oldIndex, newIndex); }

protected:
    /**
     * @brief Moves the item at @p oldIndex to @p newIndex; override to customize move behavior.
     *
     * C++ counterpart of .NET ObservableCollection<T>.MoveItem(int, int).
     * @param oldIndex The zero-based index of the item to move.
     * @param newIndex The zero-based index to move the item to.
     * @throws System::ArgumentOutOfRangeException if either index is out of range.
     * @throws System::InvalidOperationException if called reentrantly from a CollectionChanged handler.
     */
    virtual void MoveItem(intcs oldIndex, intcs newIndex) {
        CheckReentrancy();
        requireIndexInRange(oldIndex);
        requireIndexInRange(newIndex);

        T removedItem = this->items_[static_cast<size_t>(oldIndex)];
        this->items_.erase(this->items_.begin() + oldIndex);
        this->items_.insert(this->items_.begin() + newIndex, removedItem);

        OnIndexerPropertyChanged();
        OnCollectionChanged(NotifyCollectionChangedEventArgs<T>::Move(removedItem, newIndex, oldIndex));
    }

    /**
     * @brief Clears all items and fires Count/Item[]/Reset notifications.
     *
     * C++ counterpart of .NET ObservableCollection<T>.ClearItems().
     */
    void ClearItems() override {
        CheckReentrancy();
        Collection<T>::ClearItems();
        OnCountPropertyChanged();
        OnIndexerPropertyChanged();
        OnCollectionChanged(NotifyCollectionChangedEventArgs<T>(NotifyCollectionChangedAction::Reset));
    }

    /**
     * @brief Removes the item at @p index and fires Count/Item[]/Remove notifications.
     *
     * C++ counterpart of .NET ObservableCollection<T>.RemoveItem(int).
     * @param index The zero-based index of the element to remove.
     */
    void RemoveItem(intcs index) override {
        CheckReentrancy();
        T removedItem = (*this)[index];
        Collection<T>::RemoveItem(index);

        OnCountPropertyChanged();
        OnIndexerPropertyChanged();
        OnCollectionChanged(NotifyCollectionChangedEventArgs<T>(NotifyCollectionChangedAction::Remove, removedItem, index));
    }

    /**
     * @brief Inserts @p item at @p index and fires Count/Item[]/Add notifications.
     *
     * C++ counterpart of .NET ObservableCollection<T>.InsertItem(int, T).
     * @param index The zero-based index at which to insert.
     * @param item  The element to insert.
     */
    void InsertItem(intcs index, const T& item) override {
        CheckReentrancy();
        Collection<T>::InsertItem(index, item);

        OnCountPropertyChanged();
        OnIndexerPropertyChanged();
        OnCollectionChanged(NotifyCollectionChangedEventArgs<T>(NotifyCollectionChangedAction::Add, item, index));
    }

    /**
     * @brief Replaces the item at @p index and fires an Item[]/Replace notification.
     *
     * C++ counterpart of .NET ObservableCollection<T>.SetItem(int, T).
     * @param index The zero-based index of the element to replace.
     * @param item  The new value.
     */
    void SetItem(intcs index, const T& item) override {
        CheckReentrancy();
        T originalItem = (*this)[index];
        Collection<T>::SetItem(index, item);

        OnIndexerPropertyChanged();
        OnCollectionChanged(NotifyCollectionChangedEventArgs<T>::Replace(item, originalItem, index));
    }

    /**
     * @brief Raises the CollectionChanged event, guarding reentrancy around the dispatch.
     *
     * C++ counterpart of .NET ObservableCollection<T>.OnCollectionChanged(NotifyCollectionChangedEventArgs).
     * @param args The event data describing the change.
     */
    virtual void OnCollectionChanged(const NotifyCollectionChangedEventArgs<T>& args) {
        blockReentrancyCount_++;
        try {
            for (auto& h : CollectionChanged) h(this, args);
        } catch (...) {
            blockReentrancyCount_--;
            throw;
        }
        blockReentrancyCount_--;
    }

    /**
     * @brief Disallows reentrant attempts to change this collection while it has more than
     *        one CollectionChanged subscriber; returns immediately (no exception) with a
     *        single subscriber, matching .NET's compatibility carve-out.
     *
     * C++ counterpart of .NET ObservableCollection<T>.BlockReentrancy().
     * @return A guard object; changes are re-allowed once it is destroyed.
     */
    ReentrancyGuard BlockReentrancy() {
        blockReentrancyCount_++;
        return ReentrancyGuard(this);
    }

    /**
     * @brief Throws if this collection is currently being mutated reentrantly from within a
     *        CollectionChanged handler while more than one subscriber is registered.
     *
     * C++ counterpart of .NET ObservableCollection<T>.CheckReentrancy().
     * @throws System::InvalidOperationException if a reentrant change is detected.
     */
    void CheckReentrancy() const {
        if (blockReentrancyCount_ > 0 && CollectionChanged.size() > 1)
            throw System::InvalidOperationException("Cannot change ObservableCollection during a CollectionChanged event.");
    }
};

} // namespace System::Collections::ObjectModel
