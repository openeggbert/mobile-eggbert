// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/InvalidOperationException.hpp"

namespace System::Linq {

    using SharpRuntime::intcs;

    /**
     * @brief Header-only LINQ-style sequence operators over std::vector<T>.
     *
     * Partial C++ counterpart of LINQ methods from System.Linq.Enumerable.
     * All operators take a std::vector<T> by value and return a new std::vector.
     *
     * Implemented: Where, Select, FirstOrDefault, First, LastOrDefault, Any, All, Count,
     * ToList, Sum, Min, Max, OrderBy, OrderByDescending, Distinct, Reverse, Skip, Take,
     * Concat, Contains. Not implemented (real .NET's Enumerable surface is 100+ methods):
     * GroupBy, Join, Zip, SelectMany, ToDictionary, ToHashSet, Average, Aggregate, ElementAt,
     * SkipWhile, TakeWhile, Except, Intersect, Union, SequenceEqual, Single/SingleOrDefault,
     * DefaultIfEmpty, Chunk, and more -- add on demand as ported code needs them, per
     * CLAUDE.md's "No LINQ" policy for code THIS project writes (this class exists only to
     * support already-ported C#/XNA call sites that use these operators).
     *
     * @note Status: PARTIAL
     */

    /** @brief Returns elements that satisfy @p predicate. */
    template<typename T>
    std::vector<T> Where(const std::vector<T>& source,
                         std::function<bool(const T&)> predicate)
    {
        std::vector<T> result;
        for (const auto& item : source)
            if (predicate(item)) result.push_back(item);
        return result;
    }

    /** @brief Projects each element using @p selector. */
    template<typename T, typename R>
    std::vector<R> Select(const std::vector<T>& source,
                          std::function<R(const T&)> selector)
    {
        std::vector<R> result;
        result.reserve(source.size());
        for (const auto& item : source)
            result.push_back(selector(item));
        return result;
    }

    /** @brief Returns the first element matching @p predicate, or default T{} if none. */
    template<typename T>
    T FirstOrDefault(const std::vector<T>& source,
                     std::function<bool(const T&)> predicate)
    {
        for (const auto& item : source)
            if (predicate(item)) return item;
        return T{};
    }

    /** @brief Returns the first element, or default T{} if the sequence is empty. */
    template<typename T>
    T FirstOrDefault(const std::vector<T>& source)
    {
        return source.empty() ? T{} : source.front();
    }

    /** @brief Returns the first element matching @p predicate; throws if none found. */
    template<typename T>
    T First(const std::vector<T>& source,
            std::function<bool(const T&)> predicate)
    {
        for (const auto& item : source)
            if (predicate(item)) return item;
        throw System::InvalidOperationException("Sequence contains no matching element.");
    }

    /** @brief Returns the first element; throws if the sequence is empty. */
    template<typename T>
    T First(const std::vector<T>& source)
    {
        if (source.empty()) throw System::InvalidOperationException("Sequence contains no elements.");
        return source.front();
    }

    /** @brief Returns the last element matching @p predicate, or default T{} if none. */
    template<typename T>
    T LastOrDefault(const std::vector<T>& source,
                    std::function<bool(const T&)> predicate)
    {
        T result{};
        bool found = false;
        for (const auto& item : source)
            if (predicate(item)) { result = item; found = true; }
        (void)found;
        return result;
    }

    /** @brief Returns the last element, or default T{} if empty. */
    template<typename T>
    T LastOrDefault(const std::vector<T>& source)
    {
        return source.empty() ? T{} : source.back();
    }

    /** @brief Returns true if any element satisfies @p predicate. */
    template<typename T>
    bool Any(const std::vector<T>& source,
             std::function<bool(const T&)> predicate)
    {
        for (const auto& item : source)
            if (predicate(item)) return true;
        return false;
    }

    /** @brief Returns true if the sequence contains at least one element. */
    template<typename T>
    bool Any(const std::vector<T>& source) { return !source.empty(); }

    /** @brief Returns true if all elements satisfy @p predicate (true for empty sequences). */
    template<typename T>
    bool All(const std::vector<T>& source,
             std::function<bool(const T&)> predicate)
    {
        for (const auto& item : source)
            if (!predicate(item)) return false;
        return true;
    }

    /** @brief Returns the number of elements satisfying @p predicate. */
    template<typename T>
    intcs Count(const std::vector<T>& source,
                std::function<bool(const T&)> predicate)
    {
        intcs n = 0;
        for (const auto& item : source)
            if (predicate(item)) ++n;
        return n;
    }

    /** @brief Returns the total number of elements. */
    template<typename T>
    intcs Count(const std::vector<T>& source)
    {
        return static_cast<intcs>(source.size());
    }

    /** @brief Returns the input sequence as a vector (identity; mirrors .NET ToList()). */
    template<typename T>
    std::vector<T> ToList(const std::vector<T>& source) { return source; }

    /** @brief Returns the sum of all elements (requires operator+). */
    template<typename T>
    T Sum(const std::vector<T>& source)
    {
        T result{};
        for (const auto& item : source) result = result + item;
        return result;
    }

    /** @brief Returns the sum of a projected value (requires operator+). */
    template<typename T, typename R>
    R Sum(const std::vector<T>& source, std::function<R(const T&)> selector)
    {
        R result{};
        for (const auto& item : source) result = result + selector(item);
        return result;
    }

    /** @brief Returns the minimum element (requires operator<). */
    template<typename T>
    T Min(const std::vector<T>& source)
    {
        if (source.empty()) throw System::InvalidOperationException("Sequence contains no elements.");
        return *std::min_element(source.begin(), source.end());
    }

    /** @brief Returns the maximum element (requires operator<). */
    template<typename T>
    T Max(const std::vector<T>& source)
    {
        if (source.empty()) throw System::InvalidOperationException("Sequence contains no elements.");
        return *std::max_element(source.begin(), source.end());
    }

    /**
     * @brief Returns elements sorted ascending by @p keySelector.
     *
     * C++ counterpart of .NET Enumerable.OrderBy. Real .NET's OrderBy is explicitly documented
     * as a stable sort ("if the keys of two elements are equal, the order of the elements is
     * preserved") -- confirmed against the reference source's own internal machinery
     * (OrderBy.cs's ImplicitlyStableOrderedIterator exists specifically to preserve this
     * guarantee). std::sort gives no such guarantee (introsort-based implementations may
     * reorder equal-key elements arbitrarily); std::stable_sort is required to match.
     */
    template<typename T, typename Key>
    std::vector<T> OrderBy(const std::vector<T>& source,
                           std::function<Key(const T&)> keySelector)
    {
        std::vector<T> result = source;
        std::stable_sort(result.begin(), result.end(),
                  [&](const T& a, const T& b) { return keySelector(a) < keySelector(b); });
        return result;
    }

    /**
     * @brief Returns elements sorted descending by @p keySelector.
     *
     * C++ counterpart of .NET Enumerable.OrderByDescending. Same stable-sort requirement as
     * OrderBy above -- see its comment for the rationale.
     */
    template<typename T, typename Key>
    std::vector<T> OrderByDescending(const std::vector<T>& source,
                                     std::function<Key(const T&)> keySelector)
    {
        std::vector<T> result = source;
        std::stable_sort(result.begin(), result.end(),
                  [&](const T& a, const T& b) { return keySelector(a) > keySelector(b); });
        return result;
    }

    /** @brief Returns distinct elements (preserves first occurrence; requires operator==). */
    template<typename T>
    std::vector<T> Distinct(const std::vector<T>& source)
    {
        std::vector<T> result;
        for (const auto& item : source) {
            bool found = false;
            for (const auto& existing : result)
                if (existing == item) { found = true; break; }
            if (!found) result.push_back(item);
        }
        return result;
    }

    /** @brief Returns the elements in reverse order. */
    template<typename T>
    std::vector<T> Reverse(const std::vector<T>& source)
    {
        std::vector<T> result(source.rbegin(), source.rend());
        return result;
    }

    /** @brief Skips the first @p count elements and returns the rest. */
    template<typename T>
    std::vector<T> Skip(const std::vector<T>& source, intcs count)
    {
        if (count <= 0) return source;
        if (static_cast<size_t>(count) >= source.size()) return {};
        return std::vector<T>(source.begin() + count, source.end());
    }

    /** @brief Returns the first @p count elements. */
    template<typename T>
    std::vector<T> Take(const std::vector<T>& source, intcs count)
    {
        if (count <= 0) return {};
        size_t n = std::min(static_cast<size_t>(count), source.size());
        return std::vector<T>(source.begin(), source.begin() + n);
    }

    /** @brief Concatenates two sequences. */
    template<typename T>
    std::vector<T> Concat(const std::vector<T>& first, const std::vector<T>& second)
    {
        std::vector<T> result = first;
        result.insert(result.end(), second.begin(), second.end());
        return result;
    }

    /** @brief Returns true if @p source contains @p value (requires operator==). */
    template<typename T>
    bool Contains(const std::vector<T>& source, const T& value)
    {
        for (const auto& item : source)
            if (item == value) return true;
        return false;
    }

} // namespace System::Linq
