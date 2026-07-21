// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <stdexcept>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Generic/IList.hpp"

namespace System::Collections::ObjectModel {

using SharpRuntime::intcs;

/**
 * @brief Provides the base class for a generic collection.
 *
 * C++ counterpart of .NET System.Collections.ObjectModel.Collection<T>.
 * Backed by std::vector<T>; exposes virtual hook methods (InsertItem, RemoveItem,
 * ClearItems, SetItem) that derived classes can override to intercept changes.
 *
 * @tparam T The type of elements in the collection.
 */
template<typename T>
class Collection : public Generic::IList<T> {
private:
    class Enumerator : public Generic::IEnumerator<T> {
        const std::vector<T>& items_;
        intcs index_ = -1;
    public:
        explicit Enumerator(const std::vector<T>& items) : items_(items) {}
        bool MoveNext() override { return ++index_ < static_cast<intcs>(items_.size()); }
        void Reset() override { index_ = -1; }
        [[nodiscard]] const T& Current() const override { return items_[static_cast<size_t>(index_)]; }
    };

    void requireIndexInRange(intcs index) const {
        if (index < 0 || index >= static_cast<intcs>(items_.size()))
            throw System::ArgumentOutOfRangeException("index", "Index was out of range. Must be non-negative and less than the size of the collection.");
    }

protected:
    /** @brief The underlying storage for collection items. */
    std::vector<T> items_;

    /**
     * @brief Inserts @p item at @p index into items_; override to intercept insertion.
     *
     * C++ counterpart of .NET Collection<T>.InsertItem(int, T).
     * @param index The zero-based index at which to insert.
     * @param item  The element to insert.
     * @throws System::ArgumentOutOfRangeException if @p index is not within [0, Count].
     */
    virtual void InsertItem(intcs index, const T& item) {
        if (index < 0 || index > static_cast<intcs>(items_.size()))
            throw System::ArgumentOutOfRangeException("index", "Index must be within the bounds of the List.");
        items_.insert(items_.begin() + index, item);
    }

    /**
     * @brief Removes the item at @p index from items_; override to intercept removal.
     *
     * C++ counterpart of .NET Collection<T>.RemoveItem(int).
     * @param index The zero-based index of the element to remove.
     * @throws System::ArgumentOutOfRangeException if @p index is out of range.
     */
    virtual void RemoveItem(intcs index) {
        requireIndexInRange(index);
        items_.erase(items_.begin() + index);
    }

    /**
     * @brief Removes all items from items_; override to intercept clear.
     *
     * C++ counterpart of .NET Collection<T>.ClearItems().
     */
    virtual void ClearItems() {
        items_.clear();
    }

    /**
     * @brief Replaces the item at @p index; override to intercept replacement.
     *
     * C++ counterpart of .NET Collection<T>.SetItem(int, T).
     * @param index The zero-based index of the element to replace.
     * @param item  The new value.
     * @throws System::ArgumentOutOfRangeException if @p index is out of range.
     */
    virtual void SetItem(intcs index, const T& item) {
        requireIndexInRange(index);
        items_[static_cast<size_t>(index)] = item;
    }

public:
    /** @brief Default-constructs an empty Collection. */
    Collection() = default;
    /** @brief Virtual destructor for safe polymorphic destruction. */
    ~Collection() override = default;

    /**
     * @brief Gets the number of elements in the collection.
     *
     * C++ counterpart of .NET Collection<T>.Count.
     * @return The number of elements.
     */
    [[nodiscard]] intcs getCountProperty() const override {
        return static_cast<intcs>(items_.size());
    }

    /**
     * @brief Gets a value indicating whether the collection is read-only.
     *
     * C++ counterpart of .NET ICollection<T>.IsReadOnly.
     * @return Always false — Collection<T> allows modifications.
     */
    [[nodiscard]] bool getIsReadOnlyProperty() const override { return false; }

    /**
     * @brief Adds @p item to the end of the collection.
     *
     * C++ counterpart of .NET Collection<T>.Add(T).
     * @param item The element to add.
     */
    void Add(const T& item) override {
        InsertItem(static_cast<intcs>(items_.size()), item);
    }

    /**
     * @brief Removes all elements from the collection.
     *
     * C++ counterpart of .NET Collection<T>.Clear().
     */
    void Clear() override { ClearItems(); }

    /**
     * @brief Determines whether the collection contains the specified element.
     *
     * C++ counterpart of .NET Collection<T>.Contains(T).
     * @param item The element to locate.
     * @return true if found; otherwise false.
     */
    [[nodiscard]] bool Contains(const T& item) const override {
        return std::find(items_.begin(), items_.end(), item) != items_.end();
    }

    /**
     * @brief Copies all elements to @p destination starting at @p index.
     *
     * C++ counterpart of .NET Collection<T>.CopyTo(T[], int).
     * @param destination The target vector to copy elements into.
     * @param index       The zero-based starting index in @p destination.
     */
    void CopyTo(std::vector<T>& destination, intcs index) const {
        if (index < 0)
            throw System::ArgumentOutOfRangeException("index", "Non-negative number required.");
        // Verified against List<T>.CopyTo's own bounds check (`array.Length - arrayIndex <
        // Count`): a subtraction-based check, not addition. `index + getCountProperty() >
        // destination.size()` is signed-integer-overflow UB for a large index (confirmed via
        // UBSan) -- and worse than UB in the abstract: the wraparound can make the check
        // evaluate false when it should be true, silently skipping the bounds throw and letting
        // the loop below write out of bounds into destination. index is already known
        // non-negative here, and destination.size() cast to intcs is likewise non-negative, so
        // the subtraction cannot itself overflow.
        if (static_cast<intcs>(destination.size()) - index < getCountProperty())
            throw System::ArgumentException("Destination array is not long enough to copy all the items in the collection.");
        for (intcs i = 0; i < getCountProperty(); ++i)
            destination[static_cast<size_t>(index + i)] = items_[static_cast<size_t>(i)];
    }

    /**
     * @brief Removes the first occurrence of @p item from the collection.
     *
     * C++ counterpart of .NET Collection<T>.Remove(T).
     * @param item The element to remove.
     * @return true if the element was found and removed; otherwise false.
     */
    bool Remove(const T& item) override {
        auto it = std::find(items_.begin(), items_.end(), item);
        if (it == items_.end()) return false;
        RemoveItem(static_cast<intcs>(it - items_.begin()));
        return true;
    }

    /**
     * @brief Returns a const reference to the element at the specified index.
     *
     * C++ counterpart of .NET Collection<T>.Item[int] getter.
     * @param index The zero-based index.
     * @return A const reference to the element.
     * @throws System::ArgumentOutOfRangeException if index is out of range.
     */
    [[nodiscard]] const T& operator[](intcs index) const override {
        requireIndexInRange(index);
        return items_[static_cast<size_t>(index)];
    }

    /**
     * @brief Returns a reference to the element at the specified index.
     *
     * C++ counterpart of .NET Collection<T>.Item[int] setter.
     * @note .NET's indexer setter (`this[index] = value`) dispatches through the virtual
     *       `SetItem(index, value)` hook, so a derived class overriding `SetItem` (e.g. to keep
     *       an auxiliary index in sync, as `KeyedCollection<TKey,TItem>` does) sees every
     *       assignment. A C++ reference has no equivalent interception point: code that writes
     *       `collection[index] = value` assigns directly into the backing storage and never
     *       calls `SetItem`, silently skipping that hook. This is an inherent C++-reference-vs-
     *       C#-property gap, not a bug to fix locally -- documented rather than "solved" with an
     *       assignment-intercepting proxy type, which would be a much larger, riskier API change
     *       (affecting every existing caller of this operator) than a single audit ticket
     *       warrants. Callers of a `SetItem`-overriding derived class that need the hook to run
     *       must call a dedicated mutator instead of the plain indexer (e.g.
     *       `KeyedCollection::SetItem` isn't public, but `Remove`+`Add`/`Insert` do run the
     *       hooks).
     * @param index The zero-based index.
     * @return A reference to the element.
     * @throws System::ArgumentOutOfRangeException if index is out of range.
     */
    T& operator[](intcs index) override {
        requireIndexInRange(index);
        return items_[static_cast<size_t>(index)];
    }

    /**
     * @brief Returns the zero-based index of the first occurrence of @p item, or -1 if not found.
     *
     * C++ counterpart of .NET Collection<T>.IndexOf(T).
     * @param item The element to locate.
     * @return The zero-based index, or -1 if not found.
     */
    [[nodiscard]] intcs IndexOf(const T& item) const override {
        auto it = std::find(items_.begin(), items_.end(), item);
        if (it == items_.end()) return -1;
        return static_cast<intcs>(it - items_.begin());
    }

    /**
     * @brief Inserts @p item at the specified index.
     *
     * C++ counterpart of .NET Collection<T>.Insert(int, T).
     * @param index The zero-based index at which to insert.
     * @param item  The element to insert.
     */
    void Insert(intcs index, const T& item) override { InsertItem(index, item); }

    /**
     * @brief Removes the element at the specified index.
     *
     * C++ counterpart of .NET Collection<T>.RemoveAt(int).
     * @param index The zero-based index of the element to remove.
     */
    void RemoveAt(intcs index) override { RemoveItem(index); }

    /**
     * @brief Returns a new enumerator that iterates through the collection.
     *
     * C++ counterpart of .NET Collection<T>.GetEnumerator().
     * @return A heap-allocated IEnumerator<T>; caller takes ownership.
     */
    Generic::IEnumerator<T>* GetEnumerator() override {
        return new Enumerator(items_);
    }

    /** @brief Returns an iterator to the beginning of the collection (STL interop). */
    auto begin()       { return items_.begin(); }
    /** @brief Returns an iterator past the end of the collection (STL interop). */
    auto end()         { return items_.end(); }
    /** @brief Returns a const iterator to the beginning of the collection (STL interop). */
    [[nodiscard]] auto begin() const { return items_.cbegin(); }
    /** @brief Returns a const iterator past the end of the collection (STL interop). */
    [[nodiscard]] auto end()   const { return items_.cend(); }
};

} // namespace System::Collections::ObjectModel
