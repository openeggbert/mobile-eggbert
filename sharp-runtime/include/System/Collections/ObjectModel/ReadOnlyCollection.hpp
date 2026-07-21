// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <stdexcept>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/Collections/Generic/IList.hpp"

namespace System::Collections::ObjectModel {

using SharpRuntime::intcs;

/**
 * @brief Provides a read-only wrapper around a generic list.
 *
 * C++ counterpart of .NET System.Collections.ObjectModel.ReadOnlyCollection<T>. Backed by a
 * shared_ptr<vector<T>>; mutation methods throw NotSupportedException, matching .NET.
 *
 * @tparam T The type of elements in the collection.
 */
template<typename T>
class ReadOnlyCollection : public Generic::IList<T> {
private:
    std::shared_ptr<std::vector<T>> items_;

    class Enumerator : public Generic::IEnumerator<T> {
        std::shared_ptr<std::vector<T>> items_;
        intcs index_ = -1;
    public:
        explicit Enumerator(std::shared_ptr<std::vector<T>> items) : items_(std::move(items)) {}
        bool MoveNext() override { return ++index_ < static_cast<intcs>(items_->size()); }
        void Reset() override { index_ = -1; }
        [[nodiscard]] const T& Current() const override {
            return (*items_)[static_cast<size_t>(index_)];
        }
    };

public:
    /** @brief Default-constructs an empty ReadOnlyCollection. */
    ReadOnlyCollection() : items_(std::make_shared<std::vector<T>>()) {}

    /**
     * @brief Constructs a ReadOnlyCollection wrapping the given shared vector.
     *
     * C++ counterpart of .NET ReadOnlyCollection<T>(IList<T>), which stores a reference to
     * the caller's list rather than copying it -- the wrapper is a live *view*: mutating the
     * original list through a kept reference to it is visible through the wrapper too
     * (verified against ReadOnlyCollection.cs: `this.list = list;`, a plain reference
     * assignment). This overload achieves the same by taking a shared_ptr directly, matching
     * the convention already established by the sibling ReadOnlySet/ReadOnlyDictionary
     * fixes: pass the *same* shared_ptr the caller holds to get a genuinely live view.
     * @param source A shared pointer to the underlying vector.
     * @throws System::ArgumentNullException if @p source is null.
     */
    explicit ReadOnlyCollection(std::shared_ptr<std::vector<T>> source) : items_(std::move(source)) {
        if (!items_) throw System::ArgumentNullException("source");
    }

    /**
     * @brief Constructs a ReadOnlyCollection wrapping a COPY of @p source.
     *
     * Convenience overload for a plain (non-shared_ptr) vector. Unlike the shared_ptr
     * overload above, this does NOT produce a live view -- @p source is copied, so later
     * mutations to it are not reflected here. Use the shared_ptr overload when a live view
     * is needed and the source is already shared_ptr-backed.
     * @param source The vector to copy into the collection.
     */
    explicit ReadOnlyCollection(const std::vector<T>& source) : items_(std::make_shared<std::vector<T>>(source)) {}

    /**
     * @brief Constructs a ReadOnlyCollection by moving @p source into new backing storage.
     *
     * Convenience overload; like the copying overload, this does not wrap a caller-held
     * shared_ptr, so it isn't a live view of anything else (the source is consumed/moved-from).
     * @param source The vector to move into the collection.
     */
    explicit ReadOnlyCollection(std::vector<T>&& source) : items_(std::make_shared<std::vector<T>>(std::move(source))) {}

    /** @brief Virtual destructor for safe polymorphic destruction. */
    ~ReadOnlyCollection() override = default;

    /**
     * @brief Gets the number of elements in the collection.
     *
     * C++ counterpart of .NET ReadOnlyCollection<T>.Count.
     * @return The number of elements.
     */
    [[nodiscard]] intcs getCountProperty() const override {
        return static_cast<intcs>(items_->size());
    }

    /**
     * @brief Gets a value indicating whether the collection is read-only.
     *
     * C++ counterpart of .NET ICollection<T>.IsReadOnly.
     * @return Always true — ReadOnlyCollection<T> does not allow modifications.
     */
    [[nodiscard]] bool getIsReadOnlyProperty() const override { return true; }

    /**
     * @brief Returns a const reference to the element at the specified index.
     *
     * C++ counterpart of .NET ReadOnlyCollection<T>.Item[int] getter.
     * @param index The zero-based index.
     * @return A const reference to the element.
     * @throws std::out_of_range if index is out of range.
     */
    [[nodiscard]] const T& operator[](intcs index) const override {
        if (index < 0 || index >= static_cast<intcs>(items_->size()))
            throw System::ArgumentOutOfRangeException("index", "Index was out of range. Must be non-negative and less than the size of the collection.");
        return (*items_)[static_cast<size_t>(index)];
    }

    /**
     * @brief Throws because the collection is read-only.
     * @throws System::NotSupportedException always.
     */
    T& operator[](intcs index) override {
        (void)index;
        throw System::NotSupportedException("Collection is read-only.");
    }

    /**
     * @brief Returns the zero-based index of the first occurrence of @p item, or -1 if not found.
     *
     * C++ counterpart of .NET ReadOnlyCollection<T>.IndexOf(T).
     * @param item The element to locate.
     * @return The zero-based index, or -1 if not found.
     */
    [[nodiscard]] intcs IndexOf(const T& item) const override {
        for (intcs i = 0; i < static_cast<intcs>(items_->size()); ++i)
            if ((*items_)[static_cast<size_t>(i)] == item) return i;
        return -1;
    }

    /**
     * @brief Determines whether the collection contains the specified element.
     *
     * C++ counterpart of .NET ReadOnlyCollection<T>.Contains(T).
     * @param item The element to locate.
     * @return true if found; otherwise false.
     */
    [[nodiscard]] bool Contains(const T& item) const override {
        return IndexOf(item) >= 0;
    }

    /**
     * @brief Copies all elements to @p destination starting at @p index.
     *
     * C++ counterpart of .NET ReadOnlyCollection<T>.CopyTo(T[], int).
     * @param destination The target vector to copy elements into.
     * @param index       The zero-based starting index in @p destination.
     * @throws std::out_of_range if destination is too small.
     */
    void CopyTo(std::vector<T>& destination, intcs index) const {
        if (index < 0)
            throw System::ArgumentOutOfRangeException("index", "Non-negative number required.");
        // Same overflow-prone-check fix as Collection<T>::CopyTo -- see its comment. A
        // subtraction-based check matches List<T>.CopyTo's own `array.Length - arrayIndex <
        // Count` and cannot overflow here since index and destination.size() are both already
        // known non-negative at this point.
        if (static_cast<intcs>(destination.size()) - index < getCountProperty())
            throw System::ArgumentException("Destination array is not long enough to copy all the items in the collection.");
        for (intcs i = 0; i < getCountProperty(); ++i)
            destination[static_cast<size_t>(index + i)] = (*items_)[static_cast<size_t>(i)];
    }

    /**
     * @brief Throws because the collection is read-only.
     * @throws System::NotSupportedException always.
     */
    void Add(const T&) override {
        throw System::NotSupportedException("Collection is read-only.");
    }

    /**
     * @brief Throws because the collection is read-only.
     * @throws System::NotSupportedException always.
     */
    void Clear() override {
        throw System::NotSupportedException("Collection is read-only.");
    }

    /**
     * @brief Throws because the collection is read-only.
     * @throws System::NotSupportedException always.
     */
    bool Remove(const T&) override {
        throw System::NotSupportedException("Collection is read-only.");
    }

    /**
     * @brief Throws because the collection is read-only.
     * @throws System::NotSupportedException always.
     */
    void Insert(intcs, const T&) override {
        throw System::NotSupportedException("Collection is read-only.");
    }

    /**
     * @brief Throws because the collection is read-only.
     * @throws System::NotSupportedException always.
     */
    void RemoveAt(intcs) override {
        throw System::NotSupportedException("Collection is read-only.");
    }

    /**
     * @brief Returns a new enumerator that iterates through the collection.
     *
     * C++ counterpart of .NET ReadOnlyCollection<T>.GetEnumerator().
     * @return A heap-allocated IEnumerator<T>; caller takes ownership.
     */
    Generic::IEnumerator<T>* GetEnumerator() override {
        return new Enumerator(items_);
    }

    /** @brief Returns an iterator to the beginning of the collection (STL interop). */
    auto begin()       { return items_->begin(); }
    /** @brief Returns an iterator past the end of the collection (STL interop). */
    auto end()         { return items_->end(); }
    /** @brief Returns a const iterator to the beginning of the collection (STL interop). */
    [[nodiscard]] auto begin() const { return items_->cbegin(); }
    /** @brief Returns a const iterator past the end of the collection (STL interop). */
    [[nodiscard]] auto end()   const { return items_->cend(); }
};

} // namespace System::Collections::ObjectModel
