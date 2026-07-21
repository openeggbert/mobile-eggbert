// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <any>
#include <stdexcept>
#include <vector>
#include "System/Collections/IList.hpp"
#include "System/Collections/IComparer.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Collections {

using SharpRuntime::bytecs;
using SharpRuntime::intcs;

namespace detail {

/**
 * @brief Best-effort std::any value equality for ArrayList's typed Contains/IndexOf/
 * LastIndexOf overloads.
 *
 * Real .NET compares elements via Object.Equals, which works for any boxed value because
 * every .NET type carries a virtual Equals; std::any has no equivalent built-in equality
 * operator. Common primitive/string types are compared by actual value here. Types outside
 * that known set report as never-equal rather than matching purely on type() -- an earlier
 * version of this code compared only item.type() == value.type(), which caused false-positive
 * matches (e.g. searching a list of ints [1,2,3] for the value 0 would "find" index 0, since
 * only the type was compared).
 */
inline bool arrayListValueEquals(const std::any& a, const std::any& b) {
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
    // Unknown type with no recognized equality operator: never match, rather than a
    // false-positive type-only match.
    return false;
}

} // namespace detail

/**
 * @brief Implements the IList interface using an array whose size is dynamically increased.
 *
 * C++ counterpart of .NET System.Collections.ArrayList.
 * Elements are stored as std::any to approximate .NET's object storage.
 * IComparer-based methods pass const std::any* pointers to the comparer.
 */
class ArrayList : public IList {
    // Verified against ArrayList.cs: every structural mutation (Add/Insert/Remove/Clear/
    // Reverse/Sort/SetRange/the indexer setter) bumps _version, and GetEnumerator()'s
    // enumerator throws InvalidOperationException on the next MoveNext()/Reset()/getCurrentProperty()
    // call if the list was mutated since the enumerator was created -- .NET's fail-fast
    // enumeration contract. Not bumped by non-mutating operations (BinarySearch, IndexOf,
    // Contains, ToArray, Clone, etc.) or by TrimToSize() (capacity-only, matching .NET).
    intcs version_ = 0;

public:
    // -----------------------------------------------------------------------
    // Constructors
    // -----------------------------------------------------------------------

    /** @brief Constructs an empty ArrayList with default capacity. */
    ArrayList() = default;

    /**
     * @brief Constructs an ArrayList with the specified initial capacity.
     * @param capacity Initial reserved capacity.
     * @throws System::ArgumentOutOfRangeException if @p capacity is negative.
     */
    explicit ArrayList(intcs capacity) {
        System::ArgumentOutOfRangeException::ThrowIfNegative(capacity, "capacity");
        _items.reserve(static_cast<size_t>(capacity));
    }

    /**
     * @brief Constructs an ArrayList by copying all elements from @p c via GetEnumerator().
     *
     * C++ counterpart of .NET ArrayList(ICollection).
     * Each element is stored as void* (the raw pointer returned by IEnumerator::getCurrentProperty()).
     */
    explicit ArrayList(ICollection& c) {
        IEnumerator* e = c.GetEnumerator();
        if (e) {
            while (e->MoveNext())
                _items.emplace_back(e->getCurrentProperty());
            delete e;
        }
    }

    // -----------------------------------------------------------------------
    // Properties
    // -----------------------------------------------------------------------

    /**
     * @brief Returns the number of elements in the list.
     *
     * C++ counterpart of .NET ArrayList.Count.
     */
    [[nodiscard]] intcs getCountProperty() const override { return static_cast<intcs>(_items.size()); }

    /**
     * @brief Copies the elements of the list into the given buffer starting at @p index.
     *
     * C++ counterpart of .NET ArrayList.CopyTo(Array, int).
     * @param array Pointer to a std::any[] destination buffer.
     * @param index Zero-based index at which copying begins.
     */
    void CopyTo(void* array, intcs index) override {
        auto* dest = static_cast<std::any*>(array);
        for (size_t i = 0; i < _items.size(); ++i)
            dest[static_cast<size_t>(index) + i] = _items[i];
    }

    /**
     * @brief Returns the currently allocated capacity of the internal storage.
     *
     * C++ counterpart of .NET ArrayList.Capacity.
     */
    [[nodiscard]] intcs getCapacityProperty() const { return static_cast<intcs>(_items.capacity()); }

    /**
     * @brief Sets the capacity, reserving at least @p value elements in the internal storage.
     *
     * C++ counterpart of .NET ArrayList.Capacity setter.
     * @throws System::ArgumentOutOfRangeException if @p value is less than getCountProperty().
     * @note Unlike real .NET, this can only grow capacity, never shrink it (std::vector::reserve()
     *       is a no-op for a smaller value) -- use TrimToSize() to shrink to the current count.
     */
    void setCapacityProperty(intcs value) {
        if (value < static_cast<intcs>(_items.size()))
            throw System::ArgumentOutOfRangeException("value", "capacity was less than the current size.");
        _items.reserve(static_cast<size_t>(value));
    }

    /**
     * @brief Returns false; ArrayList is never read-only.
     *
     * C++ counterpart of .NET ArrayList.IsReadOnly.
     */
    [[nodiscard]] bool getIsReadOnlyProperty() const override { return false; }

    /**
     * @brief Returns false; ArrayList is never fixed-size.
     *
     * C++ counterpart of .NET ArrayList.IsFixedSize.
     */
    [[nodiscard]] bool getIsFixedSizeProperty() const override { return false; }

    /**
     * @brief Returns false; ArrayList is not thread-safe.
     *
     * C++ counterpart of .NET ArrayList.IsSynchronized.
     */
    [[nodiscard]] bool getIsSynchronizedProperty() const override { return false; }

    /**
     * @brief Gets an object that can be used to synchronize access to the ArrayList.
     *
     * C++ counterpart of .NET ArrayList.SyncRoot.
     */
    [[nodiscard]] const void* getSyncRootProperty() const override { return this; }

    // -----------------------------------------------------------------------
    // Element access
    // -----------------------------------------------------------------------

    /**
     * @brief Returns a pointer to the element at the given index.
     *
     * C++ counterpart of .NET IList indexer getter (this[int index]).
     * @param index Zero-based index.
     * @return Pointer to the std::any element at @p index.
     */
    [[nodiscard]] void* getItem(intcs index) const override {
        return const_cast<std::any*>(&_items.at(static_cast<size_t>(index)));
    }

    /**
     * @brief Sets the element at the given index.
     *
     * C++ counterpart of .NET IList indexer setter (this[int index] = value).
     * @param index Zero-based index.
     * @param value Pointer to a std::any to store; if null, stores an empty std::any.
     */
    void setItem(intcs index, void* value) override {
        _items.at(static_cast<size_t>(index)) = value ? *static_cast<std::any*>(value) : std::any{};
        ++version_;
    }

    /**
     * @brief Returns a reference to the element at the given index.
     *
     * @note Unlike setItem()/the .NET indexer setter, assigning through the reference this
     * returns does not bump the fail-fast version counter (there is no way to intercept a
     * plain C++ reference assignment) -- a known, narrow gap versus real .NET, where every
     * indexer-setter assignment invalidates in-progress enumerators. Prefer setItem() when
     * fail-fast enumeration correctness matters.
     * @param index Zero-based index.
     */
    std::any& operator[](intcs index) { return _items.at(static_cast<size_t>(index)); }

    /**
     * @brief Returns a const reference to the element at the given index.
     * @param index Zero-based index.
     */
    const std::any& operator[](intcs index) const { return _items.at(static_cast<size_t>(index)); }

    // -----------------------------------------------------------------------
    // Add
    // -----------------------------------------------------------------------

    /**
     * @brief Adds a raw pointer value to the end of the list and returns its new index.
     *
     * C++ counterpart of .NET ArrayList.Add(object).
     */
    SharpRuntime::intcs Add(void* value) override {
        _items.emplace_back(value);
        ++version_;
        return static_cast<SharpRuntime::intcs>(_items.size()) - 1;
    }

    /**
     * @brief Adds a typed value to the end of the list and returns its new index.
     * @param value Value to add.
     * @return Zero-based index at which the value was inserted.
     */
    intcs Add(const std::any& value) {
        _items.push_back(value);
        ++version_;
        return static_cast<intcs>(_items.size()) - 1;
    }

    /**
     * @brief Appends all elements from @p c to the end of the list.
     *
     * C++ counterpart of .NET ArrayList.AddRange(ICollection).
     */
    void AddRange(const std::vector<std::any>& c) {
        if (c.empty()) return;
        _items.insert(_items.end(), c.begin(), c.end());
        ++version_;
    }

    // -----------------------------------------------------------------------
    // Clear / Remove
    // -----------------------------------------------------------------------

    /**
     * @brief Removes all elements from the list.
     *
     * C++ counterpart of .NET ArrayList.Clear().
     */
    void Clear() override { _items.clear(); ++version_; }

    /**
     * @brief Removes the first occurrence of the raw pointer from the list.
     *
     * C++ counterpart of .NET ArrayList.Remove(object).
     */
    void Remove(void* value) override {
        intcs idx = IndexOf(value);
        if (idx >= 0) RemoveAt(idx);
    }

    /**
     * @brief Removes the element at the specified index.
     *
     * C++ counterpart of .NET ArrayList.RemoveAt(int).
     * @param index Zero-based index of the element to remove.
     */
    void RemoveAt(SharpRuntime::intcs index) override {
        if (index < 0 || index >= static_cast<intcs>(_items.size())) throw System::ArgumentOutOfRangeException("index");
        _items.erase(_items.begin() + index);
        ++version_;
    }

    /**
     * @brief Removes @p count elements starting at @p index.
     *
     * C++ counterpart of .NET ArrayList.RemoveRange(int, int).
     * @throws System::ArgumentOutOfRangeException if @p index or @p count is negative.
     * @throws System::ArgumentException if [@p index, @p index+@p count) is out of bounds.
     */
    void RemoveRange(intcs index, intcs count) {
        requireValidRange(index, count);
        if (count <= 0) return;
        _items.erase(_items.begin() + index, _items.begin() + index + count);
        ++version_;
    }

    // -----------------------------------------------------------------------
    // Contains / IndexOf / LastIndexOf
    // -----------------------------------------------------------------------

    /**
     * @brief Returns true if the list contains a raw pointer equal to @p value.
     *
     * C++ counterpart of .NET ArrayList.Contains(object) for void* elements.
     */
    bool Contains(void* value) const override {
        for (const auto& item : _items)
            if (std::any_cast<void*>(&item) && std::any_cast<void*>(item) == value) return true;
        return false;
    }

    /**
     * @brief Returns true if the list contains an element whose type matches @p value.
     */
    bool Contains(const std::any& value) const {
        for (const auto& item : _items)
            if (detail::arrayListValueEquals(item, value)) return true;
        return false;
    }

    /**
     * @brief Returns the zero-based index of the first occurrence of the raw pointer, or -1.
     *
     * C++ counterpart of .NET ArrayList.IndexOf(object).
     */
    SharpRuntime::intcs IndexOf(void* value) const override {
        for (intcs i = 0; i < static_cast<intcs>(_items.size()); ++i)
            if (std::any_cast<void*>(&_items[i]) && std::any_cast<void*>(_items[i]) == value) return i;
        return -1;
    }

    /**
     * @brief Returns the zero-based index of the first type-matching element, or -1.
     *
     * C++ counterpart of .NET ArrayList.IndexOf(object).
     */
    intcs IndexOf(const std::any& value) const {
        for (intcs i = 0; i < static_cast<intcs>(_items.size()); ++i)
            if (detail::arrayListValueEquals(_items[i], value)) return i;
        return -1;
    }

    /**
     * @brief Returns the first type-matching element at or after @p startIndex, or -1.
     *
     * C++ counterpart of .NET ArrayList.IndexOf(object, int).
     * @throws System::ArgumentOutOfRangeException if @p startIndex is greater than getCountProperty().
     */
    intcs IndexOf(const std::any& value, intcs startIndex) const {
        intcs size = static_cast<intcs>(_items.size());
        if (startIndex > size)
            throw System::ArgumentOutOfRangeException("startIndex", "Index was out of range. Must be non-negative and less than the size of the collection.");
        for (intcs i = startIndex; i < size; ++i)
            if (detail::arrayListValueEquals(_items[i], value)) return i;
        return -1;
    }

    /**
     * @brief Returns the first type-matching element in [@p startIndex, @p startIndex+@p count), or -1.
     *
     * C++ counterpart of .NET ArrayList.IndexOf(object, int, int).
     * @throws System::ArgumentOutOfRangeException if @p startIndex is greater than getCountProperty(),
     *         or if @p count is negative or the range extends past the end of the list.
     */
    intcs IndexOf(const std::any& value, intcs startIndex, intcs count) const {
        intcs size = static_cast<intcs>(_items.size());
        if (startIndex > size)
            throw System::ArgumentOutOfRangeException("startIndex", "Index was out of range. Must be non-negative and less than the size of the collection.");
        if (count < 0 || startIndex > size - count)
            throw System::ArgumentOutOfRangeException("count", "Count must be positive and count must refer to a location within the string/array/collection.");
        intcs end = startIndex + count;
        for (intcs i = startIndex; i < end && i < size; ++i)
            if (detail::arrayListValueEquals(_items[i], value)) return i;
        return -1;
    }

    /**
     * @brief Returns the zero-based index of the last type-matching element, or -1.
     *
     * C++ counterpart of .NET ArrayList.LastIndexOf(object).
     */
    intcs LastIndexOf(const std::any& value) const {
        return LastIndexOf(value, static_cast<intcs>(_items.size()) - 1, static_cast<intcs>(_items.size()));
    }

    /**
     * @brief Searches backward from @p startIndex for the last type-matching element, or -1.
     *
     * C++ counterpart of .NET ArrayList.LastIndexOf(object, int).
     * @throws System::ArgumentOutOfRangeException if @p startIndex is greater than or equal to
     *         getCountProperty().
     */
    intcs LastIndexOf(const std::any& value, intcs startIndex) const {
        if (startIndex >= static_cast<intcs>(_items.size()))
            throw System::ArgumentOutOfRangeException("startIndex", "Index was out of range. Must be non-negative and less than the size of the collection.");
        return LastIndexOf(value, startIndex, startIndex + 1);
    }

    /**
     * @brief Searches backward in [@p startIndex-@p count+1, @p startIndex] for the last type-matching element.
     *
     * C++ counterpart of .NET ArrayList.LastIndexOf(object, int, int).
     * @throws System::ArgumentOutOfRangeException if @p startIndex or @p count is negative on a
     *         non-empty list, or if the range they describe is out of bounds. A negative
     *         @p startIndex on an empty list quietly returns -1, matching .NET's own special case.
     */
    intcs LastIndexOf(const std::any& value, intcs startIndex, intcs count) const {
        intcs size = static_cast<intcs>(_items.size());
        if (size != 0) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(startIndex, "startIndex");
            System::ArgumentOutOfRangeException::ThrowIfNegative(count, "count");
        }
        if (size == 0) return -1;
        if (startIndex >= size || count > startIndex + 1)
            throw System::ArgumentOutOfRangeException(startIndex >= size ? "startIndex" : "count",
                "Index was out of range. Must be non-negative and less than the size of the collection.");
        intcs lo = startIndex - count + 1;
        for (intcs i = startIndex; i >= lo; --i)
            if (detail::arrayListValueEquals(_items[static_cast<size_t>(i)], value)) return i;
        return -1;
    }

    // -----------------------------------------------------------------------
    // Insert
    // -----------------------------------------------------------------------

    /**
     * @brief Inserts a raw pointer at the specified index.
     *
     * C++ counterpart of .NET ArrayList.Insert(int, object).
     * @throws System::ArgumentOutOfRangeException if @p index is negative or greater than
     *         getCountProperty() (insertion at the end, i.e. index == count, is legal).
     */
    void Insert(SharpRuntime::intcs index, void* value) override {
        requireInsertIndexInRange(index);
        _items.insert(_items.begin() + index, value);
        ++version_;
    }

    /**
     * @brief Inserts a typed value at the specified index.
     * @throws System::ArgumentOutOfRangeException if @p index is negative or greater than
     *         getCountProperty().
     */
    void Insert(intcs index, const std::any& value) {
        requireInsertIndexInRange(index);
        _items.insert(_items.begin() + index, value);
        ++version_;
    }

    /**
     * @brief Inserts all elements of @p c starting at the specified index.
     *
     * C++ counterpart of .NET ArrayList.InsertRange(int, ICollection).
     * @throws System::ArgumentOutOfRangeException if @p index is negative or greater than
     *         getCountProperty().
     */
    void InsertRange(intcs index, const std::vector<std::any>& c) {
        requireInsertIndexInRange(index);
        if (c.empty()) return;
        _items.insert(_items.begin() + index, c.begin(), c.end());
        ++version_;
    }

    // -----------------------------------------------------------------------
    // Reverse / SetRange / GetRange
    // -----------------------------------------------------------------------

    /**
     * @brief Reverses the order of all elements in the list.
     *
     * C++ counterpart of .NET ArrayList.Reverse().
     */
    void Reverse() { std::reverse(_items.begin(), _items.end()); ++version_; }

    /**
     * @brief Reverses the order of @p count elements starting at @p index.
     *
     * C++ counterpart of .NET ArrayList.Reverse(int, int).
     * @throws System::ArgumentOutOfRangeException if @p index or @p count is negative.
     * @throws System::ArgumentException if [@p index, @p index+@p count) is out of bounds.
     */
    void Reverse(intcs index, intcs count) {
        requireValidRange(index, count);
        std::reverse(_items.begin() + index, _items.begin() + index + count);
        ++version_;
    }

    /**
     * @brief Copies elements from @p c over the list starting at @p index.
     *
     * C++ counterpart of .NET ArrayList.SetRange(int, ICollection).
     * @throws System::ArgumentOutOfRangeException if @p index is negative or the range
     *         [@p index, @p index+@p c.size()) doesn't fit within the current list.
     */
    void SetRange(intcs index, const std::vector<std::any>& c) {
        intcs count = static_cast<intcs>(c.size());
        if (index < 0 || index > static_cast<intcs>(_items.size()) - count)
            throw System::ArgumentOutOfRangeException("index", "Index was out of range. Must be non-negative and less than the size of the collection.");
        if (c.empty()) return;
        for (size_t i = 0; i < c.size(); ++i)
            _items[static_cast<size_t>(index) + i] = c[i];
        ++version_;
    }

    /**
     * @brief Returns a new ArrayList containing @p count elements starting at @p index.
     *
     * C++ counterpart of .NET ArrayList.GetRange(int, int).
     * @throws System::ArgumentOutOfRangeException if @p index or @p count is negative.
     * @throws System::ArgumentException if [@p index, @p index+@p count) is out of bounds.
     */
    [[nodiscard]] ArrayList GetRange(intcs index, intcs count) const {
        requireValidRange(index, count);
        ArrayList result;
        result._items.insert(result._items.end(),
                             _items.begin() + index,
                             _items.begin() + index + count);
        return result;
    }

    // -----------------------------------------------------------------------
    // Sort
    // -----------------------------------------------------------------------

    /**
     * @brief Sorts all elements using the specified comparer.
     *
     * C++ counterpart of .NET ArrayList.Sort(IComparer).
     * The comparer receives const void* pointing to std::any elements.
     */
    void Sort(const IComparer& comparer) {
        std::stable_sort(_items.begin(), _items.end(),
            [&comparer](const std::any& a, const std::any& b) {
                return comparer.Compare(&a, &b) < 0;
            });
        ++version_;
    }

    /**
     * @brief Sorts @p count elements starting at @p index using the specified comparer.
     *
     * C++ counterpart of .NET ArrayList.Sort(int, int, IComparer).
     * @throws System::ArgumentOutOfRangeException if @p index or @p count is negative.
     * @throws System::ArgumentException if [@p index, @p index+@p count) is out of bounds.
     */
    void Sort(intcs index, intcs count, const IComparer& comparer) {
        requireValidRange(index, count);
        std::stable_sort(_items.begin() + index, _items.begin() + index + count,
            [&comparer](const std::any& a, const std::any& b) {
                return comparer.Compare(&a, &b) < 0;
            });
        ++version_;
    }

    // -----------------------------------------------------------------------
    // BinarySearch
    // -----------------------------------------------------------------------

    /**
     * @brief Binary-searches the entire list for @p value using the specified comparer.
     *
     * C++ counterpart of .NET ArrayList.BinarySearch(object, IComparer).
     * @return Index of the found element, or bitwise complement of insertion point if not found.
     */
    [[nodiscard]] intcs BinarySearch(const std::any& value, const IComparer& comparer) const {
        return BinarySearch(0, static_cast<intcs>(_items.size()), value, comparer);
    }

    /**
     * @brief Binary-searches [@p index, @p index+@p count) for @p value using @p comparer.
     *
     * C++ counterpart of .NET ArrayList.BinarySearch(int, int, object, IComparer).
     * @throws System::ArgumentOutOfRangeException if @p index or @p count is negative.
     * @throws System::ArgumentException if [@p index, @p index+@p count) is out of bounds.
     * @return Index of the found element, or bitwise complement of insertion point if not found.
     */
    [[nodiscard]] intcs BinarySearch(intcs index, intcs count, const std::any& value, const IComparer& comparer) const {
        requireValidRange(index, count);
        intcs lo = index, hi = index + count - 1;
        while (lo <= hi) {
            intcs mid = lo + (hi - lo) / 2;
            int cmp = comparer.Compare(&_items[static_cast<size_t>(mid)], &value);
            if (cmp == 0) return mid;
            if (cmp < 0) lo = mid + 1;
            else         hi = mid - 1;
        }
        return ~lo;
    }

    // -----------------------------------------------------------------------
    // Clone / ToArray / TrimToSize
    // -----------------------------------------------------------------------

    /**
     * @brief Returns a shallow copy of this ArrayList.
     *
     * C++ counterpart of .NET ArrayList.Clone().
     */
    [[nodiscard]] ArrayList Clone() const {
        ArrayList copy;
        copy._items = _items;
        return copy;
    }

    /**
     * @brief Returns a copy of the internal items as a std::vector.
     *
     * C++ counterpart of .NET ArrayList.ToArray().
     */
    std::vector<std::any> ToArray() const { return _items; }

    /**
     * @brief Releases any excess capacity, shrinking internal storage to fit the current count.
     *
     * C++ counterpart of .NET ArrayList.TrimToSize().
     */
    void TrimToSize() { _items.shrink_to_fit(); }

    // -----------------------------------------------------------------------
    // Static factories
    // -----------------------------------------------------------------------

    /**
     * @brief Returns an ArrayList with @p count copies of @p value.
     *
     * C++ counterpart of .NET ArrayList.Repeat(object, int).
     * @throws System::ArgumentOutOfRangeException if @p count is negative.
     */
    [[nodiscard]] static ArrayList Repeat(const std::any& value, intcs count) {
        System::ArgumentOutOfRangeException::ThrowIfNegative(count, "count");
        ArrayList result(count);
        for (intcs i = 0; i < count; ++i) result.Add(value);
        return result;
    }

    // -----------------------------------------------------------------------
    // Misc
    // -----------------------------------------------------------------------

    /** @brief Returns a const reference to the underlying item vector. */
    [[nodiscard]] const std::vector<std::any>& getItems() const { return _items; }

    /**
     * @brief Returns an enumerator that iterates through the full list.
     *
     * C++ counterpart of .NET ArrayList.GetEnumerator(). The enumerator throws
     * InvalidOperationException on MoveNext()/Reset()/getCurrentProperty() if the list is
     * structurally modified while enumeration is in progress, matching .NET's fail-fast
     * contract (see the Enumerator class doc-comment for the one known gap: mutation via
     * the non-const operator[] reference isn't detected).
     */
    IEnumerator* GetEnumerator() override { return new Enumerator(this, 0, static_cast<intcs>(_items.size())); }

    /**
     * @brief Returns an enumerator that iterates through @p count elements starting at @p index.
     *
     * C++ counterpart of .NET ArrayList.GetEnumerator(int, int).
     * @throws System::ArgumentOutOfRangeException if @p index or @p count is negative.
     * @throws System::ArgumentException if [@p index, @p index+@p count) is out of bounds.
     */
    IEnumerator* GetEnumerator(intcs index, intcs count) {
        if (index < 0) throw System::ArgumentOutOfRangeException("index");
        if (count < 0) throw System::ArgumentOutOfRangeException("count");
        if (static_cast<size_t>(index) + static_cast<size_t>(count) > _items.size())
            throw System::ArgumentException("Invalid index/count: range exceeds the list's bounds.");
        return new Enumerator(this, index, index + count);
    }

private:
    std::vector<std::any> _items;

    // Matches .NET ArrayList's range-validation contract (used by BinarySearch(index,count,...)/
    // GetRange/RemoveRange/Reverse(index,count)/Sort(index,count,...), each of which calls this
    // exact 3-check sequence in ArrayList.cs): negative index/count -> ArgumentOutOfRangeException;
    // range extending past the end -> ArgumentException. Previously entirely absent from all of
    // these methods, so an out-of-range index/count reached std::vector iterator arithmetic
    // (UB, not a clean exception) instead.
    void requireValidRange(intcs index, intcs count) const {
        if (index < 0)
            throw System::ArgumentOutOfRangeException("index", "Non-negative number required.");
        if (count < 0)
            throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
        if (static_cast<intcs>(_items.size()) - index < count)
            throw System::ArgumentException("Offset and length were out of bounds for the array, or count is greater than the number of elements from index to the end of the source collection.");
    }

    // Matches .NET ArrayList's Insert/InsertRange validation: unlike element-access indices,
    // index == count (inserting at the end) is explicitly legal.
    void requireInsertIndexInRange(intcs index) const {
        if (index < 0 || index > static_cast<intcs>(_items.size()))
            throw System::ArgumentOutOfRangeException("index", "Index was out of range. Must be non-negative and less than the size of the collection.");
    }

    /**
     * @brief Enumerator over an ArrayList (or a sub-range of one), matching the fail-fast
     * versioning pattern established by Queue::Enumerator in this codebase.
     */
    class Enumerator : public IEnumerator {
        const ArrayList* list_;
        intcs version_;
        intcs start_;
        intcs end_;     // exclusive
        intcs index_;   // one before start_ / past-end after MoveNext() returns false
        bool started_ = false;

    public:
        Enumerator(const ArrayList* list, intcs start, intcs end)
            : list_(list), version_(list->version_), start_(start), end_(end), index_(start - 1) {}

        bool MoveNext() override {
            if (version_ != list_->version_) throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            if (index_ + 1 >= end_) { started_ = true; index_ = end_; return false; }
            ++index_;
            started_ = true;
            return true;
        }

        void Reset() override {
            if (version_ != list_->version_) throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            index_ = start_ - 1;
            started_ = false;
        }

        [[nodiscard]] void* getCurrentProperty() const override {
            if (!started_ || index_ < start_ || index_ >= end_)
                throw System::InvalidOperationException("Enumeration has either not started or has already finished.");
            return const_cast<std::any*>(&list_->_items[static_cast<size_t>(index_)]);
        }
    };
};

} // namespace System::Collections
