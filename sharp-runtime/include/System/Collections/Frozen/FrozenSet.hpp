// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::Collections::Frozen {

/**
 * @brief Provides an immutable, read-only set optimized for fast lookup and enumeration.
 *
 * C++ counterpart of .NET System.Collections.Frozen.FrozenSet<T>.
 * Once created via Create() or ToFrozenSet(), the set cannot be modified.
 * Backed by std::unordered_set for O(1) average-case lookups.
 * Ideal for data loaded once at startup and read frequently at runtime.
 *
 * @tparam T The type of the elements.
 */
template<typename T>
class FrozenSet {
    std::unordered_set<T> set_;

    explicit FrozenSet(std::unordered_set<T> s) : set_(std::move(s)) {}

public:
    using const_iterator = typename std::unordered_set<T>::const_iterator;

    /**
     * @brief Gets an empty FrozenSet singleton.
     *
     * C++ counterpart of .NET FrozenSet<T>.Empty.
     */
    static const FrozenSet& getEmptyProperty() {
        static FrozenSet instance{std::unordered_set<T>{}};
        return instance;
    }

    /**
     * @brief Creates a FrozenSet from a vector of elements.
     *
     * C++ counterpart of .NET FrozenSet.Create(ReadOnlySpan<T>).
     * Duplicate elements are silently ignored.
     * @param source Elements to populate the set.
     * @return A new immutable FrozenSet.
     */
    static FrozenSet Create(const std::vector<T>& source) {
        std::unordered_set<T> s;
        s.reserve(source.size());
        for (const auto& item : source) s.insert(item);
        return FrozenSet(std::move(s));
    }

    /**
     * @brief Creates a FrozenSet from an existing unordered_set.
     *
     * @param source The set whose elements are copied into the frozen set.
     * @return A new immutable FrozenSet.
     */
    static FrozenSet CreateFromSet(const std::unordered_set<T>& source) {
        return FrozenSet(std::unordered_set<T>(source));
    }

    /**
     * @brief Gets the number of elements in the set.
     *
     * C++ counterpart of .NET FrozenSet<T>.Count.
     */
    [[nodiscard]] SharpRuntime::intcs getCountProperty() const {
        return static_cast<SharpRuntime::intcs>(set_.size());
    }

    /**
     * @brief Gets a vector of all items in the set.
     *
     * C++ counterpart of .NET FrozenSet<T>.Items.
     */
    [[nodiscard]] std::vector<T> getItemsProperty() const {
        return std::vector<T>(set_.begin(), set_.end());
    }

    /**
     * @brief Determines whether the set contains the specified element.
     *
     * C++ counterpart of .NET FrozenSet<T>.Contains(T).
     * @param item The element to locate.
     * @return true if found; otherwise false.
     */
    [[nodiscard]] bool Contains(const T& item) const {
        return set_.count(item) != 0;
    }

    /**
     * @brief Attempts to get the actual stored value equal to the given value.
     *
     * C++ counterpart of .NET FrozenSet<T>.TryGetValue(T, out T).
     * Useful when the equality comparer distinguishes objects that compare equal.
     * @param equalValue  The value to search for.
     * @param actualValue Receives the stored element if found.
     * @return true if an equal element was found; otherwise false.
     */
    bool TryGetValue(const T& equalValue, T& actualValue) const {
        auto it = set_.find(equalValue);
        if (it == set_.end()) return false;
        actualValue = *it;
        return true;
    }

    /**
     * @brief Copies all elements to the destination vector starting at the given index.
     *
     * C++ counterpart of .NET FrozenSet<T>.CopyTo(T[], int).
     * @param destination Destination vector; must already have room for index + Count elements.
     * @param index       Zero-based index at which copying begins.
     * @throws System::ArgumentOutOfRangeException if @p index is negative.
     * @throws System::ArgumentException if @p destination is not large enough.
     */
    void CopyTo(std::vector<T>& destination, SharpRuntime::intcs index) const {
        if (index < 0) throw System::ArgumentOutOfRangeException("destinationIndex");
        if (static_cast<std::size_t>(index) + set_.size() > destination.size())
            throw System::ArgumentException("Destination too small for the number of elements in the collection.", "destination");
        std::size_t i = static_cast<std::size_t>(index);
        for (const auto& item : set_) {
            destination[i++] = item;
        }
    }

    /** @brief Returns an iterator to the beginning of the set (for range-based for). */
    [[nodiscard]] const_iterator begin() const { return set_.begin(); }

    /** @brief Returns an iterator past the end of the set (for range-based for). */
    [[nodiscard]] const_iterator end()   const { return set_.end();   }
};

/**
 * @brief Creates a FrozenSet from a vector of elements.
 *
 * C++ counterpart of .NET IEnumerable<T>.ToFrozenSet().
 * Duplicate elements are silently ignored.
 * @param source Elements to populate the set.
 * @return A new immutable FrozenSet.
 */
template<typename T>
FrozenSet<T> ToFrozenSet(const std::vector<T>& source) {
    return FrozenSet<T>::Create(source);
}

} // namespace System::Collections::Frozen
