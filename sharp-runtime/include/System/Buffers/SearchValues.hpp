// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <initializer_list>
#include <unordered_set>
#include <vector>
#include "System/Span.hpp"
#include "System/ReadOnlySpan.hpp"

namespace System::Buffers {

/**
 * @brief Provides an immutable, read-only set of values optimized for searching.
 *
 * C++ counterpart of .NET System.Buffers.SearchValues&lt;T&gt;.
 * Instances are created via the SearchValues factory (see Create overloads below).
 * This implementation uses an unordered_set for O(1) Contains() lookups.
 *
 * @tparam T The type of the values to search for. Must be equality-comparable.
 */
template<typename T>
class SearchValues {
    std::unordered_set<T> values_;

public:
    /** @brief Constructs a SearchValues from an initializer list of values. */
    SearchValues(std::initializer_list<T> values) : values_(values) {}

    /** @brief Constructs a SearchValues from a vector of values. */
    explicit SearchValues(const std::vector<T>& values)
        : values_(values.begin(), values.end()) {}

    /**
     * @brief Searches for the specified value and returns true if found.
     *
     * C++ counterpart of .NET SearchValues&lt;T&gt;.Contains(T).
     * @param value The value to search for.
     * @return true if @p value is in the set; otherwise false.
     */
    [[nodiscard]] bool Contains(const T& value) const {
        return values_.count(value) > 0;
    }

    /**
     * @brief Returns a copy of all values in this SearchValues instance.
     *
     * C++ counterpart of the internal .NET SearchValues&lt;T&gt;.GetValues().
     */
    [[nodiscard]] std::vector<T> GetValues() const {
        return std::vector<T>(values_.begin(), values_.end());
    }
};

/**
 * @brief Factory class for creating SearchValues instances.
 *
 * C++ counterpart of .NET System.Buffers.SearchValues (the static factory class).
 */
class SearchValuesFactory {
public:
    SearchValuesFactory() = delete;

    /**
     * @brief Creates an optimized SearchValues&lt;uint8_t&gt; from the given values.
     *
     * C++ counterpart of .NET SearchValues.Create(ReadOnlySpan&lt;byte&gt;).
     */
    [[nodiscard]] static SearchValues<uint8_t> Create(
        std::initializer_list<uint8_t> values)
    {
        return SearchValues<uint8_t>(values);
    }

    /**
     * @brief Creates an optimized SearchValues&lt;char&gt; from the given values.
     *
     * C++ counterpart of .NET SearchValues.Create(ReadOnlySpan&lt;char&gt;).
     */
    [[nodiscard]] static SearchValues<char> Create(
        std::initializer_list<char> values)
    {
        return SearchValues<char>(values);
    }

    /**
     * @brief Creates a SearchValues&lt;T&gt; from the given vector.
     *
     * Generic overload for any equality-comparable type T.
     */
    template<typename T>
    [[nodiscard]] static SearchValues<T> Create(const std::vector<T>& values) {
        return SearchValues<T>(values);
    }
};

} // namespace System::Buffers
