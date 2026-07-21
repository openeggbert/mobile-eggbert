// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <algorithm>
#include <cstring>
#include <functional>
#include <limits>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System {

    using SharpRuntime::intcs;

    /**
     * @brief Provides static methods for creating, manipulating, searching, and sorting arrays.
     *
     * Partial C++ counterpart of .NET System.Array.
     * All methods operate on std::vector<T> rather than raw C-style arrays.
     */
    class Array {
    public:
        Array() = delete;

        /**
         * @brief Gets the maximum number of elements supported by the array.
         *
         * C++ counterpart of .NET Array.MaxLength.
         */
        [[nodiscard]] static constexpr intcs MaxLengthProperty() noexcept {
            return std::numeric_limits<intcs>::max();
        }

        /** @brief Sorts all elements of @p array using the default less-than comparator. */
        template<typename T>
        static void Sort(std::vector<T>& array) {
            std::sort(array.begin(), array.end());
        }

        /**
         * @brief Sorts all elements of @p array using @p comparison.
         * @param comparison Function returning negative/zero/positive for a&lt;b / a==b / a&gt;b.
         */
        template<typename T>
        static void Sort(std::vector<T>& array, std::function<int(const T&, const T&)> comparison) {
            std::sort(array.begin(), array.end(), [&](const T& a, const T& b) {
                return comparison(a, b) < 0;
            });
        }

        /**
         * @brief Sorts @p length elements starting at @p index using the default comparator.
         * @throws System::ArgumentOutOfRangeException if @p index or @p length is negative.
         * @throws System::ArgumentException if [@p index, @p index+@p length) is out of bounds.
         */
        template<typename T>
        static void Sort(std::vector<T>& array, intcs index, intcs length) {
            requireValidRange(static_cast<intcs>(array.size()), index, length);
            std::sort(array.begin() + index, array.begin() + index + length);
        }

        /**
         * @brief Sorts @p length elements starting at @p index using @p comparison.
         * @param comparison Function returning negative/zero/positive for a&lt;b / a==b / a&gt;b.
         * @throws System::ArgumentOutOfRangeException if @p index or @p length is negative.
         * @throws System::ArgumentException if [@p index, @p index+@p length) is out of bounds.
         */
        template<typename T>
        static void Sort(std::vector<T>& array, intcs index, intcs length,
                         std::function<int(const T&, const T&)> comparison) {
            requireValidRange(static_cast<intcs>(array.size()), index, length);
            std::sort(array.begin() + index, array.begin() + index + length,
                      [&](const T& a, const T& b) { return comparison(a, b) < 0; });
        }

        /**
         * @brief Copies @p length elements from the start of @p src into the start of @p dst.
         *
         * C++ counterpart of .NET Array.Copy(Array, Array, int).
         * @param dst Destination vector (must already be sized to fit @p length elements).
         * @throws System::ArgumentOutOfRangeException if @p length is negative.
         * @throws System::ArgumentException if @p length exceeds either vector's size.
         */
        template<typename T>
        static void Copy(const std::vector<T>& src, std::vector<T>& dst, intcs length) {
            requireValidRange(static_cast<intcs>(src.size()), 0, length);
            requireValidRange(static_cast<intcs>(dst.size()), 0, length);
            for (intcs i = 0; i < length; ++i)
                dst[static_cast<size_t>(i)] = src[static_cast<size_t>(i)];
        }

        /**
         * @brief Copies @p length elements from @p src[@p srcIndex] into @p dst[@p dstIndex].
         * @param dst Destination vector (must already be sized to fit the copied range).
         * @throws System::ArgumentOutOfRangeException if @p srcIndex, @p dstIndex, or @p length is negative.
         * @throws System::ArgumentException if either copied range is out of bounds.
         */
        template<typename T>
        static void Copy(const std::vector<T>& src, intcs srcIndex,
                         std::vector<T>& dst, intcs dstIndex, intcs length) {
            requireValidRange(static_cast<intcs>(src.size()), srcIndex, length);
            requireValidRange(static_cast<intcs>(dst.size()), dstIndex, length);
            for (intcs i = 0; i < length; ++i)
                dst[dstIndex + i] = src[srcIndex + i];
        }

        /**
         * @brief Copies @p length elements from raw C-array @p src into @p dst using memcpy.
         *
         * @note Unlike the std::vector<T> overloads above, this cannot validate bounds -- a raw
         * pointer carries no length information for either side, matching the same structural
         * limitation as other raw-pointer buffer methods across this codebase (e.g.
         * ArrayList::CopyTo(void*, int)). The caller is responsible for ensuring both buffers
         * are large enough.
         */
        template<typename T>
        static void Copy(const T* src, intcs srcIndex, T* dst, intcs dstIndex, intcs length) {
            std::memcpy(dst + dstIndex, src + srcIndex, static_cast<size_t>(length) * sizeof(T));
        }

        /**
         * @brief Copies elements like Copy but signals that the copy must succeed atomically
         * (no partial copy on type mismatch). In C++ there is no runtime type check, so
         * this delegates directly to Copy.
         */
        template<typename T>
        static void ConstrainedCopy(const std::vector<T>& src, intcs srcIndex,
                                    std::vector<T>& dst, intcs dstIndex, intcs length) {
            Copy(src, srcIndex, dst, dstIndex, length);
        }

        /**
         * @brief Resizes @p array to @p newSize, preserving existing elements and default-initializing new ones.
         * @throws System::ArgumentOutOfRangeException if @p newSize is negative.
         */
        template<typename T>
        static void Resize(std::vector<T>& array, intcs newSize) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(newSize, "newSize");
            array.resize(static_cast<size_t>(newSize));
        }

        /** @brief Returns the zero-based index of the first element equal to @p value, or -1 if not found. */
        template<typename T>
        static intcs IndexOf(const std::vector<T>& array, const T& value) {
            for (intcs i = 0; i < static_cast<intcs>(array.size()); ++i)
                if (array[i] == value) return i;
            return -1;
        }

        /**
         * @brief Returns the first index of @p value starting at @p startIndex, or -1.
         * @throws System::ArgumentOutOfRangeException if @p startIndex is negative or greater than the array's size.
         */
        template<typename T>
        static intcs IndexOf(const std::vector<T>& array, const T& value, intcs startIndex) {
            requireValidStartIndex(static_cast<intcs>(array.size()), startIndex);
            for (intcs i = startIndex; i < static_cast<intcs>(array.size()); ++i)
                if (array[static_cast<size_t>(i)] == value) return i;
            return -1;
        }

        /**
         * @brief Returns the first index of @p value in [@p startIndex, @p startIndex+@p count), or -1.
         * @throws System::ArgumentOutOfRangeException if @p startIndex or @p count is negative,
         *         @p startIndex exceeds the array's size, or the range extends past the end.
         */
        template<typename T>
        static intcs IndexOf(const std::vector<T>& array, const T& value,
                             intcs startIndex, intcs count) {
            requireValidRange(static_cast<intcs>(array.size()), startIndex, count);
            intcs end = startIndex + count;
            for (intcs i = startIndex; i < end; ++i)
                if (array[static_cast<size_t>(i)] == value) return i;
            return -1;
        }

        /** @brief Reverses the order of all elements in @p array in place. */
        template<typename T>
        static void Reverse(std::vector<T>& array) {
            std::reverse(array.begin(), array.end());
        }

        /**
         * @brief Reverses @p length elements of @p array starting at @p index.
         * @throws System::ArgumentOutOfRangeException if @p index or @p length is negative.
         * @throws System::ArgumentException if [@p index, @p index+@p length) is out of bounds.
         */
        template<typename T>
        static void Reverse(std::vector<T>& array, intcs index, intcs length) {
            requireValidRange(static_cast<intcs>(array.size()), index, length);
            std::reverse(array.begin() + index, array.begin() + index + length);
        }

        /** @brief Resets all elements of @p array to the default value of T. */
        template<typename T>
        static void Clear(std::vector<T>& array) {
            std::fill(array.begin(), array.end(), T{});
        }

        /**
         * @brief Resets @p length elements starting at @p index to the default value of T.
         * @throws System::ArgumentOutOfRangeException if @p index or @p length is negative.
         * @throws System::ArgumentException if [@p index, @p index+@p length) is out of bounds.
         */
        template<typename T>
        static void Clear(std::vector<T>& array, intcs index, intcs length) {
            requireValidRange(static_cast<intcs>(array.size()), index, length);
            for (intcs i = index; i < index + length; ++i)
                array[static_cast<size_t>(i)] = T{};
        }

        /**
         * @brief Searches a sorted @p array for @p value using binary search.
         * @return Zero-based index if found; bitwise complement of the insertion point otherwise.
         */
        template<typename T>
        static intcs BinarySearch(const std::vector<T>& array, const T& value) {
            intcs lo = 0, hi = static_cast<intcs>(array.size()) - 1;
            while (lo <= hi) {
                intcs mid = lo + (hi - lo) / 2;
                if (array[static_cast<size_t>(mid)] == value) return mid;
                if (array[static_cast<size_t>(mid)] < value)  lo = mid + 1;
                else                                           hi = mid - 1;
            }
            return ~lo;
        }

        /**
         * @brief Searches a sorted sub-range of @p array for @p value using binary search.
         * @param index  First index of the range to search.
         * @param length Number of elements in the range.
         * @return Zero-based index if found; bitwise complement of the insertion point otherwise.
         * @throws System::ArgumentOutOfRangeException if @p index or @p length is negative.
         * @throws System::ArgumentException if [@p index, @p index+@p length) is out of bounds.
         */
        template<typename T>
        static intcs BinarySearch(const std::vector<T>& array, intcs index, intcs length,
                                   const T& value) {
            requireValidRange(static_cast<intcs>(array.size()), index, length);
            intcs lo = index, hi = index + length - 1;
            while (lo <= hi) {
                intcs mid = lo + (hi - lo) / 2;
                if (array[static_cast<size_t>(mid)] == value) return mid;
                if (array[static_cast<size_t>(mid)] < value)  lo = mid + 1;
                else                                           hi = mid - 1;
            }
            return ~lo;
        }

        /**
         * @brief Searches a sorted @p array for @p value using a custom comparer.
         * @param comparison Returns negative/zero/positive for less/equal/greater.
         * @return Zero-based index if found; bitwise complement of the insertion point otherwise.
         */
        template<typename T>
        static intcs BinarySearch(const std::vector<T>& array, const T& value,
                                   std::function<int(const T&, const T&)> comparison) {
            intcs lo = 0, hi = static_cast<intcs>(array.size()) - 1;
            while (lo <= hi) {
                intcs mid = lo + (hi - lo) / 2;
                int cmp = comparison(array[static_cast<size_t>(mid)], value);
                if (cmp == 0) return mid;
                if (cmp < 0)  lo = mid + 1;
                else          hi = mid - 1;
            }
            return ~lo;
        }

        /**
         * @brief Searches a sorted sub-range of @p array for @p value using a custom comparer.
         * @param index      First index of the range to search.
         * @param length     Number of elements in the range.
         * @param comparison Returns negative/zero/positive for less/equal/greater.
         * @return Zero-based index if found; bitwise complement of the insertion point otherwise.
         * @throws System::ArgumentOutOfRangeException if @p index or @p length is negative.
         * @throws System::ArgumentException if [@p index, @p index+@p length) is out of bounds.
         */
        template<typename T>
        static intcs BinarySearch(const std::vector<T>& array, intcs index, intcs length,
                                   const T& value,
                                   std::function<int(const T&, const T&)> comparison) {
            requireValidRange(static_cast<intcs>(array.size()), index, length);
            intcs lo = index, hi = index + length - 1;
            while (lo <= hi) {
                intcs mid = lo + (hi - lo) / 2;
                int cmp = comparison(array[static_cast<size_t>(mid)], value);
                if (cmp == 0) return mid;
                if (cmp < 0)  lo = mid + 1;
                else          hi = mid - 1;
            }
            return ~lo;
        }

        /** @brief Sets every element in @p array to @p value. */
        template<typename T>
        static void Fill(std::vector<T>& array, const T& value) {
            std::fill(array.begin(), array.end(), value);
        }

        /**
         * @brief Sets @p count elements starting at @p startIndex to @p value.
         * @throws System::ArgumentOutOfRangeException if @p startIndex or @p count is negative.
         * @throws System::ArgumentException if [@p startIndex, @p startIndex+@p count) is out of bounds.
         */
        template<typename T>
        static void Fill(std::vector<T>& array, const T& value, intcs startIndex, intcs count) {
            requireValidRange(static_cast<intcs>(array.size()), startIndex, count);
            std::fill(array.begin() + startIndex, array.begin() + startIndex + count, value);
        }

        /** @brief Returns an empty vector of type T (equivalent of .NET Array.Empty&lt;T&gt;()). */
        template<typename T>
        [[nodiscard]] static std::vector<T> Empty() { return {}; }

        /** @brief Converts every element using @p converter and returns the results as a new vector. */
        template<typename T, typename TOutput>
        [[nodiscard]] static std::vector<TOutput> ConvertAll(
                const std::vector<T>& array,
                std::function<TOutput(const T&)> converter) {
            std::vector<TOutput> result;
            result.reserve(array.size());
            for (const auto& item : array) result.push_back(converter(item));
            return result;
        }

        /** @brief Returns true if any element of @p array satisfies @p predicate. */
        template<typename T>
        [[nodiscard]] static bool Exists(const std::vector<T>& array, std::function<bool(const T&)> predicate) {
            for (const auto& item : array)
                if (predicate(item)) return true;
            return false;
        }

        /** @brief Returns the first element satisfying @p predicate, or default T{} if none found. */
        template<typename T>
        [[nodiscard]] static T Find(const std::vector<T>& array, std::function<bool(const T&)> predicate) {
            for (const auto& item : array)
                if (predicate(item)) return item;
            return T{};
        }

        /** @brief Returns the last element satisfying @p predicate, or default T{} if none found. */
        template<typename T>
        [[nodiscard]] static T FindLast(const std::vector<T>& array, std::function<bool(const T&)> predicate) {
            for (intcs i = static_cast<intcs>(array.size()) - 1; i >= 0; --i)
                if (predicate(array[static_cast<size_t>(i)])) return array[static_cast<size_t>(i)];
            return T{};
        }

        /** @brief Returns a new vector of all elements satisfying @p predicate. */
        template<typename T>
        [[nodiscard]] static std::vector<T> FindAll(
                const std::vector<T>& array,
                std::function<bool(const T&)> predicate) {
            std::vector<T> result;
            for (const auto& item : array)
                if (predicate(item)) result.push_back(item);
            return result;
        }

        /** @brief Returns the index of the first element satisfying @p predicate, or -1 if none. */
        template<typename T>
        [[nodiscard]] static intcs FindIndex(
                const std::vector<T>& array,
                std::function<bool(const T&)> predicate) {
            for (intcs i = 0; i < static_cast<intcs>(array.size()); ++i)
                if (predicate(array[static_cast<size_t>(i)])) return i;
            return -1;
        }

        /**
         * @brief Searches [@p startIndex, end) for the first element satisfying @p predicate.
         * @throws System::ArgumentOutOfRangeException if @p startIndex is negative or greater than the array's size.
         */
        template<typename T>
        [[nodiscard]] static intcs FindIndex(
                const std::vector<T>& array, intcs startIndex,
                std::function<bool(const T&)> predicate) {
            requireValidStartIndex(static_cast<intcs>(array.size()), startIndex);
            for (intcs i = startIndex; i < static_cast<intcs>(array.size()); ++i)
                if (predicate(array[static_cast<size_t>(i)])) return i;
            return -1;
        }

        /**
         * @brief Searches [@p startIndex, @p startIndex+@p count) for the first element satisfying @p predicate.
         * @throws System::ArgumentOutOfRangeException if @p startIndex or @p count is negative,
         *         @p startIndex exceeds the array's size, or the range extends past the end.
         */
        template<typename T>
        [[nodiscard]] static intcs FindIndex(
                const std::vector<T>& array, intcs startIndex, intcs count,
                std::function<bool(const T&)> predicate) {
            requireValidRange(static_cast<intcs>(array.size()), startIndex, count);
            intcs end = startIndex + count;
            for (intcs i = startIndex; i < end; ++i)
                if (predicate(array[static_cast<size_t>(i)])) return i;
            return -1;
        }

        /** @brief Returns the index of the last element satisfying @p predicate, or -1 if none. */
        template<typename T>
        [[nodiscard]] static intcs FindLastIndex(
                const std::vector<T>& array,
                std::function<bool(const T&)> predicate) {
            for (intcs i = static_cast<intcs>(array.size()) - 1; i >= 0; --i)
                if (predicate(array[static_cast<size_t>(i)])) return i;
            return -1;
        }

        /**
         * @brief Searches backward from @p startIndex for the last element satisfying @p predicate.
         * @throws System::ArgumentOutOfRangeException if @p startIndex is out of range (see
         *         requireValidBackwardRange's doc-comment for the empty-array special case).
         */
        template<typename T>
        [[nodiscard]] static intcs FindLastIndex(
                const std::vector<T>& array, intcs startIndex,
                std::function<bool(const T&)> predicate) {
            intcs size = static_cast<intcs>(array.size());
            requireValidBackwardRange(size, startIndex, size == 0 ? 0 : startIndex + 1);
            for (intcs i = startIndex; i >= 0; --i)
                if (predicate(array[static_cast<size_t>(i)])) return i;
            return -1;
        }

        /**
         * @brief Searches backward in [@p startIndex-@p count+1, @p startIndex] for an element satisfying @p predicate.
         * @throws System::ArgumentOutOfRangeException if @p startIndex or @p count is out of
         *         range (see requireValidBackwardRange's doc-comment for the empty-array special case).
         */
        template<typename T>
        [[nodiscard]] static intcs FindLastIndex(
                const std::vector<T>& array, intcs startIndex, intcs count,
                std::function<bool(const T&)> predicate) {
            requireValidBackwardRange(static_cast<intcs>(array.size()), startIndex, count);
            intcs endIdx = startIndex - count + 1;
            for (intcs i = startIndex; i >= endIdx && i >= 0; --i)
                if (predicate(array[static_cast<size_t>(i)])) return i;
            return -1;
        }

        /** @brief Applies @p action to every element of @p array. */
        template<typename T>
        static void ForEach(const std::vector<T>& array, std::function<void(const T&)> action) {
            for (const auto& item : array) action(item);
        }

        /** @brief Returns true if all elements satisfy @p predicate (vacuously true for empty arrays). */
        template<typename T>
        [[nodiscard]] static bool TrueForAll(const std::vector<T>& array, std::function<bool(const T&)> predicate) {
            for (const auto& item : array)
                if (!predicate(item)) return false;
            return true;
        }

        /** @brief Returns the zero-based index of the last occurrence of @p value in @p array, or -1. */
        template<typename T>
        [[nodiscard]] static intcs LastIndexOf(const std::vector<T>& array, const T& value) {
            for (intcs i = static_cast<intcs>(array.size()) - 1; i >= 0; --i)
                if (array[static_cast<size_t>(i)] == value) return i;
            return -1;
        }

        /**
         * @brief Searches backward from @p startIndex for the last occurrence of @p value, or -1.
         * @throws System::ArgumentOutOfRangeException if @p startIndex is out of range (see
         *         requireValidBackwardRange's doc-comment for the empty-array special case).
         */
        template<typename T>
        [[nodiscard]] static intcs LastIndexOf(const std::vector<T>& array, const T& value,
                                               intcs startIndex) {
            intcs size = static_cast<intcs>(array.size());
            requireValidBackwardRange(size, startIndex, size == 0 ? 0 : startIndex + 1);
            for (intcs i = startIndex; i >= 0; --i)
                if (array[static_cast<size_t>(i)] == value) return i;
            return -1;
        }

        /**
         * @brief Searches backward in [@p startIndex-@p count+1, @p startIndex] for @p value, or -1.
         * @throws System::ArgumentOutOfRangeException if @p startIndex or @p count is out of
         *         range (see requireValidBackwardRange's doc-comment for the empty-array special case).
         */
        template<typename T>
        [[nodiscard]] static intcs LastIndexOf(const std::vector<T>& array, const T& value,
                                               intcs startIndex, intcs count) {
            requireValidBackwardRange(static_cast<intcs>(array.size()), startIndex, count);
            intcs endIdx = startIndex - count + 1;
            for (intcs i = startIndex; i >= endIdx && i >= 0; --i)
                if (array[static_cast<size_t>(i)] == value) return i;
            return -1;
        }

        /**
         * @brief Returns a const reference to @p array as a read-only view.
         * C++ counterpart of .NET Array.AsReadOnly&lt;T&gt;().
         */
        template<typename T>
        [[nodiscard]] static const std::vector<T>& AsReadOnly(const std::vector<T>& array) {
            return array;
        }

    private:
        // Every index/count-taking method above previously did zero validation before raw
        // std::vector iterator/index arithmetic -- a negative index in particular cast to
        // size_t (e.g. via `array[static_cast<size_t>(i)]` with i==-1) becomes a huge positive
        // offset, a genuine out-of-bounds access, not just a wrong-answer bug. Matches real
        // .NET Array's own (uint)index > (uint)size / (uint)length > (uint)(size-index)
        // validation pattern exactly (see e.g. Array.IndexOf<T>(T[],T,int,int) in Array.cs) --
        // unsigned comparison + subtraction instead of signed addition, so large index/length
        // can't integer-overflow past the check either (same fix already applied this session
        // to Span<T>::Slice, ticket 265, and its many siblings, ticket 1487).
        static void requireValidRange(intcs size, intcs index, intcs length) {
            if (static_cast<SharpRuntime::uintcs>(index) > static_cast<SharpRuntime::uintcs>(size))
                throw System::ArgumentOutOfRangeException("index", "Index was out of range. Must be non-negative and less than the size of the collection.");
            if (static_cast<SharpRuntime::uintcs>(length) > static_cast<SharpRuntime::uintcs>(size - index))
                throw System::ArgumentOutOfRangeException("length", "Count must be positive and count must refer to a location within the string/array/collection.");
        }

        // For the single-startIndex forward-search overloads (search from startIndex to the
        // end): startIndex == size is legal (an immediately-exhausted search), matching real
        // .NET Array.IndexOf<T>(T[],T,int)'s delegation to the 4-arg overload with
        // count = Length - startIndex (which is 0, not an error, when startIndex == Length).
        static void requireValidStartIndex(intcs size, intcs startIndex) {
            if (static_cast<SharpRuntime::uintcs>(startIndex) > static_cast<SharpRuntime::uintcs>(size))
                throw System::ArgumentOutOfRangeException("startIndex", "Index was out of range. Must be non-negative and less than the size of the collection.");
        }

        // For the backward-search (LastIndexOf/FindLastIndex) overloads. Matches real .NET
        // Array.LastIndexOf<T>(T[],T,int,int)'s exact validation, including its documented
        // empty-array special case: for a zero-length array, startIndex of -1 or 0 is accepted
        // (for "compatibility reasons", per Array.cs's own comment) and count must be exactly 0.
        static void requireValidBackwardRange(intcs size, intcs startIndex, intcs count) {
            if (size == 0) {
                if (startIndex != -1 && startIndex != 0)
                    throw System::ArgumentOutOfRangeException("startIndex", "Index was out of range. Must be non-negative and less than the size of the collection.");
                if (count != 0)
                    throw System::ArgumentOutOfRangeException("count", "Count must be positive and count must refer to a location within the string/array/collection.");
                return;
            }
            if (static_cast<SharpRuntime::uintcs>(startIndex) >= static_cast<SharpRuntime::uintcs>(size))
                throw System::ArgumentOutOfRangeException("startIndex", "Index was out of range. Must be non-negative and less than the size of the collection.");
            if (count < 0 || count > startIndex + 1)
                throw System::ArgumentOutOfRangeException("count", "Count must be positive and count must refer to a location within the string/array/collection.");
        }
    };

} // namespace System
