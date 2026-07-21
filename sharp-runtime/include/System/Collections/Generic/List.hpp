// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Collections/Generic/IList.hpp"
#include "System/Collections/ObjectModel/ReadOnlyCollection.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Represents a strongly typed list of objects that can be accessed by index.
 *
 * C++ counterpart of .NET System.Collections.Generic.List<T>.
 * Backed by std::vector<T>; provides O(1) amortized Add, O(1) indexed access,
 * and full IList<T> compliance.
 *
 * @note GetEnumerator()'s Enumerator detects structural modification (Add/Remove/Clear/Insert/
 * etc.) during iteration via a version counter, matching .NET's InvalidOperationException
 * fail-fast contract (see ArrayList's Enumerator in this codebase for the same established
 * pattern). One narrow, documented gap remains: real .NET's index-assignment setter
 * (`list[i] = value`) also bumps `_version`, but this port's `operator[]` returns a plain `T&`
 * for C++ ergonomics -- there is no hook point to intercept a subsequent `list[i] = value;`
 * assignment through that reference, so value-only index writes do NOT trigger fail-fast
 * detection here. Widening `operator[]` into a proxy-object return (as done for Dictionary's
 * indexer to fix a different bug, ticket 1712) would be a much larger, riskier change to the
 * single most call-site-heavy method in this codebase; deferred rather than rushed. Directly
 * using begin()/end() STL iterators still follows plain std::vector<T> invalidation rules, not
 * .NET's version-checked contract -- only the GetEnumerator()-returned Enumerator is fail-fast.
 *
 * @tparam T The type of elements in the list.
 */
template<typename T>
class List : public IList<T> {
    private:
        std::vector<T> items_;
        intcs version_ = 0;

        class Enumerator : public IEnumerator<T>
        {
            const List<T>* list_;
            intcs version_;
            intcs index_ = -1;
        public:
            explicit Enumerator(const List<T>* list) : list_(list), version_(list->version_) {}
            bool MoveNext() override {
                if (version_ != list_->version_)
                    throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
                return ++index_ < static_cast<intcs>(list_->items_.size());
            }
            void Reset() override {
                if (version_ != list_->version_)
                    throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
                index_ = -1;
            }
            [[nodiscard]] const T& Current() const override { return list_->items_[static_cast<size_t>(index_)]; }
        };

        void requireIndexInRange(intcs index) const {
            if (index < 0 || index >= static_cast<intcs>(items_.size()))
                throw System::ArgumentOutOfRangeException("index", "Index was out of range. Must be non-negative and less than the size of the collection.");
        }

        // Matches .NET's List<T> range-validation contract (used by GetRange/RemoveRange/
        // Reverse(index,count)): negative index/count -> ArgumentOutOfRangeException;
        // range extending past the end -> ArgumentException.
        void requireValidRange(intcs index, intcs count) const {
            if (index < 0)
                throw System::ArgumentOutOfRangeException("index", "Non-negative number required.");
            if (count < 0)
                throw System::ArgumentOutOfRangeException("count", "Non-negative number required.");
            if (static_cast<intcs>(items_.size()) - index < count)
                throw System::ArgumentException("Offset and length were out of bounds for the array, or count is greater than the number of elements from index to the end of the source collection.");
        }

    public:
        List() = default;

        explicit List(const std::vector<T>& source) : items_(source) {}

        [[nodiscard]] intcs getCountProperty() const override
        {
            return static_cast<intcs>(items_.size());
        }

        /** @brief Returns false; List&lt;T&gt; is never read-only. */
        [[nodiscard]] bool getIsReadOnlyProperty() const override { return false; }

        /** @brief Appends @p item to the end of the list. */
        void Add(const T& item) override { items_.push_back(item); ++version_; }

        /** @brief Removes all elements from the list. */
        void Clear() override { items_.clear(); ++version_; }

        [[nodiscard]] bool Contains(const T& item) const override
        {
            return std::find(items_.begin(), items_.end(), item) != items_.end();
        }

        bool Remove(const T& item) override
        {
            auto it = std::find(items_.begin(), items_.end(), item);
            if (it == items_.end()) return false;
            items_.erase(it);
            ++version_;
            return true;
        }

        [[nodiscard]] const T& operator[](intcs index) const override
        {
            requireIndexInRange(index);
            return items_[static_cast<std::size_t>(index)];
        }

        T& operator[](intcs index) override
        {
            requireIndexInRange(index);
            return items_[static_cast<std::size_t>(index)];
        }

        [[nodiscard]] intcs IndexOf(const T& item) const override
        {
            auto it = std::find(items_.begin(), items_.end(), item);
            if (it == items_.end()) return -1;
            return static_cast<intcs>(it - items_.begin());
        }

        /**
         * @brief Copies the entire list to a compatible one-dimensional array, starting at
         * @p arrayIndex.
         *
         * C++ counterpart of .NET List<T>.CopyTo(T[], int). @p array must point to a buffer of
         * at least @p arrayIndex + Count elements; @p arrayLength is the caller-supplied total
         * capacity of that buffer, used to validate the copy fits (real .NET can infer array
         * length from the CLR array object itself; a raw C++ pointer carries no length, so this
         * port requires the caller to state it explicitly).
         * @throws System::ArgumentNullException if @p array is null.
         * @throws System::ArgumentOutOfRangeException if @p arrayIndex is negative.
         * @throws System::ArgumentException if @p arrayLength - @p arrayIndex is less than
         *         Count (the destination is too small to hold the copy).
         */
        void CopyTo(T* array, intcs arrayLength, intcs arrayIndex = 0) const
        {
            if (array == nullptr)
                throw System::ArgumentNullException("array");
            if (arrayIndex < 0)
                throw System::ArgumentOutOfRangeException("arrayIndex", "Non-negative number required.");
            if (arrayLength - arrayIndex < static_cast<intcs>(items_.size()))
                throw System::ArgumentException("Destination array was not long enough. Check the destination index, length, and the array's lower bounds.");
            std::copy(items_.begin(), items_.end(), array + arrayIndex);
        }

        /**
         * @brief Copies the entire list to a compatible destination @p std::vector, starting at
         * @p arrayIndex.
         *
         * C++ counterpart of .NET List<T>.CopyTo(T[]) / CopyTo(int, T[], int, int) collapsed
         * into a single convenience overload for this codebase's std::vector-based idiom.
         * @throws System::ArgumentOutOfRangeException if @p arrayIndex is negative.
         * @throws System::ArgumentException if @p destination is too small to hold the copy
         *         starting at @p arrayIndex.
         */
        void CopyTo(std::vector<T>& destination, intcs arrayIndex = 0) const
        {
            if (arrayIndex < 0)
                throw System::ArgumentOutOfRangeException("arrayIndex", "Non-negative number required.");
            if (static_cast<intcs>(destination.size()) - arrayIndex < static_cast<intcs>(items_.size()))
                throw System::ArgumentException("Destination array was not long enough. Check the destination index, length, and the array's lower bounds.");
            std::copy(items_.begin(), items_.end(), destination.begin() + arrayIndex);
        }

        void Insert(intcs index, const T& item) override
        {
            if (index < 0 || index > static_cast<intcs>(items_.size()))
                throw System::ArgumentOutOfRangeException("index", "Index must be within the bounds of the List.");
            items_.insert(items_.begin() + index, item);
            ++version_;
        }

        void RemoveAt(intcs index) override
        {
            requireIndexInRange(index);
            items_.erase(items_.begin() + index);
            ++version_;
        }

        IEnumerator<T>* GetEnumerator() override
        {
            return new Enumerator(this);
        }

        /**
         * @brief Returns the underlying std::vector for STL interop.
         *
         * Not part of the .NET API; provided for direct interop with std::vector<T>.
         * @return A const reference to the internal storage.
         */
        [[nodiscard]] const std::vector<T>& ToVector() const { return items_; }
        /** @brief Returns the underlying std::vector for STL interop (mutable). */
        [[nodiscard]] std::vector<T>& ToVector() { return items_; }

        /** @brief STL-interop mutable begin iterator (not part of the .NET API). */
        auto begin() { return items_.begin(); }
        /** @brief STL-interop mutable end iterator (not part of the .NET API). */
        auto end()   { return items_.end(); }
        /** @brief STL-interop const begin iterator (not part of the .NET API). */
        [[nodiscard]] auto begin() const { return items_.cbegin(); }
        /** @brief STL-interop const end iterator (not part of the .NET API). */
        [[nodiscard]] auto end()   const { return items_.cend(); }

        /**
         * @brief Appends the elements of @p collection to the end of the list.
         *
         * C++ counterpart of .NET List<T>.AddRange(IEnumerable<T>).
         * @param collection The elements to add.
         */
        void AddRange(const std::vector<T>& collection) {
            if (!collection.empty()) {
                items_.insert(items_.end(), collection.begin(), collection.end());
                ++version_;
            }
        }

        /**
         * @brief Appends all elements of another List<T> to the end of this list.
         * @param other The list whose elements to append.
         */
        void AddRange(const List<T>& other) { AddRange(other.items_); }

        /**
         * @brief Inserts the elements of @p collection into the list at @p index.
         *
         * C++ counterpart of .NET List<T>.InsertRange(int, IEnumerable<T>).
         * @param index      The zero-based index at which insertion begins.
         * @param collection The elements to insert.
         */
        void InsertRange(intcs index, const std::vector<T>& collection) {
            if (index < 0 || index > static_cast<intcs>(items_.size()))
                throw System::ArgumentOutOfRangeException("index", "Index must be within the bounds of the List.");
            if (!collection.empty()) {
                items_.insert(items_.begin() + index, collection.begin(), collection.end());
                ++version_;
            }
        }

        /**
         * @brief Creates a shallow copy of a range of elements.
         *
         * C++ counterpart of .NET List<T>.GetRange(int, int).
         * @param index The zero-based index at which the range starts.
         * @param count The number of elements in the range.
         * @return A new List<T> containing the specified range.
         */
        [[nodiscard]] List<T> GetRange(intcs index, intcs count) const {
            requireValidRange(index, count);
            return List<T>(std::vector<T>(items_.begin() + index, items_.begin() + index + count));
        }

        /**
         * @brief Copies the elements to a new array.
         *
         * C++ counterpart of .NET List<T>.ToArray().
         * @return A std::vector<T> containing copies of the list's elements.
         */
        [[nodiscard]] std::vector<T> ToArray() const { return items_; }

        /**
         * @brief Sorts the elements in the list using the default comparer.
         *
         * C++ counterpart of .NET List<T>.Sort().
         */
        void Sort() {
            std::sort(items_.begin(), items_.end());
            ++version_;
        }

        /**
         * @brief Sorts the elements using a comparison delegate.
         *
         * C++ counterpart of .NET List<T>.Sort(Comparison<T>).
         * @param comparison A function returning negative/zero/positive.
         */
        void Sort(std::function<intcs(const T&, const T&)> comparison) {
            std::sort(items_.begin(), items_.end(),
                      [&](const T& a, const T& b) { return comparison(a, b) < 0; });
            ++version_;
        }

        /**
         * @brief Reverses the order of all elements in the list.
         *
         * C++ counterpart of .NET List<T>.Reverse().
         */
        void Reverse() {
            std::reverse(items_.begin(), items_.end());
            ++version_;
        }

        /**
         * @brief Searches for an element matching the predicate and returns it.
         *
         * C++ counterpart of .NET List<T>.Find(Predicate<T>).
         * @param predicate The condition to test each element against.
         * @return The first matching element, or default T{} if none found.
         */
        [[nodiscard]] T Find(std::function<bool(const T&)> predicate) const {
            for (const auto& item : items_)
                if (predicate(item)) return item;
            return T{};
        }

        /**
         * @brief Retrieves all elements that match the conditions defined by the predicate.
         *
         * C++ counterpart of .NET List<T>.FindAll(Predicate<T>).
         * @param predicate The condition to test each element against.
         * @return A new List<T> of all matching elements.
         */
        [[nodiscard]] List<T> FindAll(std::function<bool(const T&)> predicate) const {
            List<T> result;
            for (const auto& item : items_)
                if (predicate(item)) result.Add(item);
            return result;
        }

        /**
         * @brief Searches for an element matching the predicate and returns the index of its
         *        first occurrence, or -1 if not found.
         *
         * C++ counterpart of .NET List<T>.FindIndex(Predicate<T>).
         * @param predicate The condition to test each element against.
         * @return The index of the first matching element, or -1.
         */
        [[nodiscard]] intcs FindIndex(std::function<bool(const T&)> predicate) const {
            for (intcs i = 0; i < static_cast<intcs>(items_.size()); ++i)
                if (predicate(items_[static_cast<size_t>(i)])) return i;
            return -1;
        }

        /**
         * @brief Searches for an element matching the predicate and returns the index of its
         *        last occurrence, or -1 if not found.
         *
         * C++ counterpart of .NET List<T>.FindLastIndex(Predicate<T>).
         * @param predicate The condition to test each element against.
         * @return The index of the last matching element, or -1.
         */
        [[nodiscard]] intcs FindLastIndex(std::function<bool(const T&)> predicate) const {
            for (intcs i = static_cast<intcs>(items_.size()) - 1; i >= 0; --i)
                if (predicate(items_[static_cast<size_t>(i)])) return i;
            return -1;
        }

        /**
         * @brief Removes all elements matching the predicate.
         *
         * C++ counterpart of .NET List<T>.RemoveAll(Predicate<T>).
         * @param predicate The condition to test each element against.
         * @return The number of elements removed.
         */
        intcs RemoveAll(std::function<bool(const T&)> predicate) {
            auto it = std::remove_if(items_.begin(), items_.end(), predicate);
            intcs count = static_cast<intcs>(items_.end() - it);
            items_.erase(it, items_.end());
            if (count > 0) ++version_;
            return count;
        }

        /**
         * @brief Performs the specified action on each element of the list.
         *
         * C++ counterpart of .NET List<T>.ForEach(Action<T>).
         * @param action The action to perform on each element.
         */
        void ForEach(std::function<void(const T&)> action) const {
            for (const auto& item : items_) action(item);
        }

        /**
         * @brief Determines whether the list contains elements matching the predicate.
         *
         * C++ counterpart of .NET List<T>.Exists(Predicate<T>).
         * @param predicate The condition to test each element against.
         * @return true if any element matches; otherwise false.
         */
        [[nodiscard]] bool Exists(std::function<bool(const T&)> predicate) const {
            for (const auto& item : items_)
                if (predicate(item)) return true;
            return false;
        }

        /**
         * @brief Determines whether every element in the list matches the predicate.
         *
         * C++ counterpart of .NET List<T>.TrueForAll(Predicate<T>).
         * @param predicate The condition to test each element against.
         * @return true if every element matches, or the list is empty; otherwise false.
         */
        [[nodiscard]] bool TrueForAll(std::function<bool(const T&)> predicate) const {
            for (const auto& item : items_)
                if (!predicate(item)) return false;
            return true;
        }

        /**
         * @brief Searches a sorted list for a value using the default comparer.
         *
         * C++ counterpart of .NET List<T>.BinarySearch(T).
         * @param item The value to search for.
         * @return The zero-based index if found; the bitwise complement of the
         *         insertion point if not found.
         */
        [[nodiscard]] intcs BinarySearch(const T& item) const {
            auto it = std::lower_bound(items_.begin(), items_.end(), item);
            if (it != items_.end() && *it == item)
                return static_cast<intcs>(it - items_.begin());
            return ~static_cast<intcs>(it - items_.begin());
        }

        /**
         * @brief Searches for the specified object starting at @p startIndex.
         *
         * C++ counterpart of .NET List<T>.IndexOf(T, int).
         * @param item       The value to locate.
         * @param startIndex The zero-based index to start searching at.
         * @return The index of the first occurrence, or -1 if not found.
         * @throws System::ArgumentOutOfRangeException if @p startIndex is negative or greater than Count.
         */
        [[nodiscard]] intcs IndexOf(const T& item, intcs startIndex) const {
            if (startIndex < 0 || startIndex > static_cast<intcs>(items_.size()))
                throw System::ArgumentOutOfRangeException("startIndex");
            for (intcs i = startIndex; i < static_cast<intcs>(items_.size()); ++i)
                if (items_[static_cast<size_t>(i)] == item) return i;
            return -1;
        }

        /**
         * @brief Searches for the specified object in [@p startIndex, @p startIndex+@p count).
         *
         * C++ counterpart of .NET List<T>.IndexOf(T, int, int).
         * @param item       The value to locate.
         * @param startIndex The zero-based index to start searching at.
         * @param count      The number of elements to search.
         * @return The index of the first occurrence, or -1 if not found.
         * @throws System::ArgumentOutOfRangeException if @p startIndex is negative or greater
         *         than Count, or if @p count is negative or the range extends past the end.
         */
        [[nodiscard]] intcs IndexOf(const T& item, intcs startIndex, intcs count) const {
            intcs size = static_cast<intcs>(items_.size());
            if (startIndex < 0 || startIndex > size)
                throw System::ArgumentOutOfRangeException("startIndex");
            if (count < 0 || startIndex > size - count)
                throw System::ArgumentOutOfRangeException("count");
            intcs end = startIndex + count;
            for (intcs i = startIndex; i < end; ++i)
                if (items_[static_cast<size_t>(i)] == item) return i;
            return -1;
        }

        /**
         * @brief Searches for the last occurrence of @p item in the entire list.
         *
         * C++ counterpart of .NET List<T>.LastIndexOf(T).
         * @param item The value to locate.
         * @return The index of the last occurrence, or -1 if not found.
         */
        [[nodiscard]] intcs LastIndexOf(const T& item) const {
            for (intcs i = static_cast<intcs>(items_.size()) - 1; i >= 0; --i)
                if (items_[static_cast<size_t>(i)] == item) return i;
            return -1;
        }

        /**
         * @brief Searches for the last occurrence of @p item at or before @p startIndex.
         *
         * C++ counterpart of .NET List<T>.LastIndexOf(T, int).
         * @param item       The value to locate.
         * @param startIndex The zero-based index to start searching backward from.
         * @return The index of the last occurrence, or -1 if not found.
         * @throws System::ArgumentOutOfRangeException if @p startIndex is negative or >= Count.
         */
        [[nodiscard]] intcs LastIndexOf(const T& item, intcs startIndex) const {
            if (startIndex < 0 || startIndex >= static_cast<intcs>(items_.size()))
                throw System::ArgumentOutOfRangeException("startIndex");
            for (intcs i = startIndex; i >= 0; --i)
                if (items_[static_cast<size_t>(i)] == item) return i;
            return -1;
        }

        /**
         * @brief Searches backward from @p startIndex for the last occurrence of @p item,
         *        examining @p count elements.
         *
         * C++ counterpart of .NET List<T>.LastIndexOf(T, int, int).
         * @param item       The value to locate.
         * @param startIndex The zero-based index to start searching backward from.
         * @param count      The number of elements to search.
         * @return The index of the last occurrence, or -1 if not found.
         * @throws System::ArgumentOutOfRangeException if @p startIndex or @p count is negative
         *         on a non-empty list, if @p startIndex is out of range, or if @p count extends
         *         past the start of the list. A negative @p startIndex on an empty list quietly
         *         returns -1, matching .NET's own special case.
         */
        [[nodiscard]] intcs LastIndexOf(const T& item, intcs startIndex, intcs count) const {
            intcs size = static_cast<intcs>(items_.size());
            if (size != 0) {
                if (startIndex < 0) throw System::ArgumentOutOfRangeException("startIndex");
                if (count < 0) throw System::ArgumentOutOfRangeException("count");
            }
            if (size == 0) return -1;
            if (startIndex >= size) throw System::ArgumentOutOfRangeException("startIndex");
            if (count > startIndex + 1) throw System::ArgumentOutOfRangeException("count");
            intcs lo = startIndex - count + 1;
            for (intcs i = startIndex; i >= lo; --i)
                if (items_[static_cast<size_t>(i)] == item) return i;
            return -1;
        }

        /**
         * @brief Gets the total number of elements the internal storage can hold without resizing.
         *
         * C++ counterpart of .NET List<T>.Capacity.
         * @return The current capacity.
         */
        [[nodiscard]] intcs getCapacityProperty() const {
            return static_cast<intcs>(items_.capacity());
        }

        /**
         * @brief Ensures that the list can hold at least @p capacity elements.
         *
         * C++ counterpart of .NET List<T>.EnsureCapacity(int), which returns the resulting
         * capacity (`_items.Length` after the call) rather than void -- added to match that
         * signature; ported code may capture the return value.
         * @param capacity The minimum capacity to ensure.
         * @return The resulting capacity.
         * @throws System::ArgumentOutOfRangeException if @p capacity is negative.
         */
        intcs EnsureCapacity(intcs capacity) {
            if (capacity < 0)
                throw System::ArgumentOutOfRangeException("capacity");
            if (capacity > static_cast<intcs>(items_.capacity()))
                items_.reserve(static_cast<std::size_t>(capacity));
            return static_cast<intcs>(items_.capacity());
        }

        /**
         * @brief Sets the capacity to the actual number of elements, reducing memory overhead.
         *
         * C++ counterpart of .NET List<T>.TrimExcess().
         */
        void TrimExcess() { items_.shrink_to_fit(); }

        /**
         * @brief Converts the elements in the list to another type and returns a new list.
         *
         * C++ counterpart of .NET List<T>.ConvertAll<TOutput>(Converter<T,TOutput>).
         * @tparam TOutput The type of the elements of the target list.
         * @param converter A function that converts each element to @p TOutput.
         * @return A new List<TOutput> containing the converted elements.
         */
        template<typename TOutput>
        [[nodiscard]] List<TOutput> ConvertAll(std::function<TOutput(const T&)> converter) const {
            List<TOutput> result;
            result.EnsureCapacity(static_cast<intcs>(items_.size()));
            for (const auto& item : items_) result.Add(converter(item));
            return result;
        }

        /**
         * @brief Returns a read-only wrapper around this list.
         *
         * C++ counterpart of .NET List<T>.AsReadOnly().
         *
         * @warning Real .NET's AsReadOnly() returns a live *view*: mutations to this List
         * afterward are visible through the returned wrapper (ReadOnlyCollection.cs stores a
         * plain reference to the source list, not a copy). This port's List<T> holds its
         * elements in a plain (non-shared_ptr) std::vector<T>, so achieving that exact live-
         * view semantic here would require changing List<T>'s internal storage to
         * shared_ptr-backed -- a broad change to a heavily-used core type, out of scope per
         * CLAUDE.md rule #10. This returns a ReadOnlyCollection<T> that copies this list's
         * current elements instead; it will NOT reflect later mutations to this List.
         * @return A ReadOnlyCollection<T> containing a snapshot copy of this list's elements.
         */
        [[nodiscard]] System::Collections::ObjectModel::ReadOnlyCollection<T> AsReadOnly() const {
            return System::Collections::ObjectModel::ReadOnlyCollection<T>(items_);
        }

        /**
         * @brief Searches for the last element matching the predicate.
         *
         * C++ counterpart of .NET List<T>.FindLast(Predicate<T>).
         * @param predicate The condition to test each element against.
         * @return The last matching element, or default T{} if none found.
         */
        [[nodiscard]] T FindLast(std::function<bool(const T&)> predicate) const {
            for (intcs i = static_cast<intcs>(items_.size()) - 1; i >= 0; --i)
                if (predicate(items_[static_cast<size_t>(i)])) return items_[static_cast<size_t>(i)];
            return T{};
        }

        /**
         * @brief Removes a range of elements from the list.
         *
         * C++ counterpart of .NET List<T>.RemoveRange(int, int).
         * @param index The zero-based starting index of the range to remove.
         * @param count The number of elements to remove.
         */
        void RemoveRange(intcs index, intcs count) {
            requireValidRange(index, count);
            if (count > 0) {
                items_.erase(items_.begin() + index, items_.begin() + index + count);
                ++version_;
            }
        }

        /**
         * @brief Reverses the order of the elements in the specified range.
         *
         * C++ counterpart of .NET List<T>.Reverse(int, int).
         * @param index The zero-based starting index of the range to reverse.
         * @param count The number of elements in the range to reverse.
         */
        void Reverse(intcs index, intcs count) {
            requireValidRange(index, count);
            std::reverse(items_.begin() + index, items_.begin() + index + count);
            ++version_;
        }
};

} // namespace System::Collections::Generic
