// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <unordered_set>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentNullException.hpp"

namespace System::Collections::ObjectModel {

/**
 * @brief Provides a read-only wrapper around a generic set.
 *
 * C++ counterpart of .NET System.Collections.ObjectModel.ReadOnlySet<T>.
 * Wraps a shared_ptr<unordered_set<T>> and exposes read-only set operations.
 * Mutation methods are not provided.
 *
 * @tparam T The type of elements in the set.
 */
template<typename T>
class ReadOnlySet {
    std::shared_ptr<std::unordered_set<T>> set_;

public:
    /**
     * @brief Constructs a ReadOnlySet wrapping the given shared set.
     *
     * C++ counterpart of .NET ReadOnlySet<T>(ISet<T>).
     * @param set A shared pointer to the underlying unordered_set.
     * @throws System::ArgumentNullException if @p set is null.
     */
    explicit ReadOnlySet(std::shared_ptr<std::unordered_set<T>> set)
        : set_(std::move(set)) {
        if (!set_)
            throw System::ArgumentNullException("set");
    }

    /**
     * @brief Gets an empty ReadOnlySet instance.
     *
     * C++ counterpart of .NET ReadOnlySet<T>.Empty.
     * @return A shared, permanently-empty instance.
     */
    static ReadOnlySet<T> Empty() {
        static ReadOnlySet<T> empty(std::make_shared<std::unordered_set<T>>());
        return empty;
    }

    /**
     * @brief Gets the number of elements in the set.
     *
     * C++ counterpart of .NET ReadOnlySet<T>.Count.
     * @return The number of elements.
     */
    [[nodiscard]] SharpRuntime::intcs getCountProperty() const {
        return static_cast<SharpRuntime::intcs>(set_->size());
    }

    /**
     * @brief Gets a value indicating whether the set contains no elements.
     *
     * C++ counterpart of checking Count == 0 on .NET ReadOnlySet<T>.
     * @return true if the set is empty; otherwise false.
     */
    [[nodiscard]] bool getIsEmptyProperty() const { return set_->empty(); }

    /**
     * @brief Determines whether the set contains the specified element.
     *
     * C++ counterpart of .NET ReadOnlySet<T>.Contains(T).
     * @param item The element to locate.
     * @return true if @p item is found; otherwise false.
     */
    [[nodiscard]] bool Contains(const T& item) const { return set_->count(item) > 0; }

    /**
     * @brief Determines whether this set is a subset of @p other.
     *
     * C++ counterpart of .NET ReadOnlySet<T>.IsSubsetOf(IEnumerable<T>).
     * @param other The set to compare against.
     * @return true if every element of this set is in @p other.
     */
    [[nodiscard]] bool IsSubsetOf(const ReadOnlySet<T>& other) const {
        for (const T& item : *set_)
            if (!other.Contains(item)) return false;
        return true;
    }

    /**
     * @brief Determines whether this set is a superset of @p other.
     *
     * C++ counterpart of .NET ReadOnlySet<T>.IsSupersetOf(IEnumerable<T>).
     * @param other The set to compare against.
     * @return true if every element of @p other is in this set.
     */
    [[nodiscard]] bool IsSupersetOf(const ReadOnlySet<T>& other) const {
        return other.IsSubsetOf(*this);
    }

    /**
     * @brief Determines whether this set is a proper subset of @p other.
     *
     * C++ counterpart of .NET ReadOnlySet<T>.IsProperSubsetOf(IEnumerable<T>).
     * A proper subset is a subset that is not equal to @p other.
     * @param other The set to compare against.
     * @return true if this is a proper subset of @p other.
     */
    [[nodiscard]] bool IsProperSubsetOf(const ReadOnlySet<T>& other) const {
        return IsSubsetOf(other) && getCountProperty() < other.getCountProperty();
    }

    /**
     * @brief Determines whether this set is a proper superset of @p other.
     *
     * C++ counterpart of .NET ReadOnlySet<T>.IsProperSupersetOf(IEnumerable<T>).
     * A proper superset is a superset that is not equal to @p other.
     * @param other The set to compare against.
     * @return true if this is a proper superset of @p other.
     */
    [[nodiscard]] bool IsProperSupersetOf(const ReadOnlySet<T>& other) const {
        return IsSupersetOf(other) && getCountProperty() > other.getCountProperty();
    }

    /**
     * @brief Determines whether this set and @p other share at least one element.
     *
     * C++ counterpart of .NET ReadOnlySet<T>.Overlaps(IEnumerable<T>).
     * @param other The set to compare against.
     * @return true if at least one element is common; otherwise false.
     */
    [[nodiscard]] bool Overlaps(const ReadOnlySet<T>& other) const {
        for (const T& item : *set_)
            if (other.Contains(item)) return true;
        return false;
    }

    /**
     * @brief Determines whether this set and @p other contain the same elements.
     *
     * C++ counterpart of .NET ReadOnlySet<T>.SetEquals(IEnumerable<T>).
     * @param other The set to compare against.
     * @return true if both sets have the same elements.
     */
    [[nodiscard]] bool SetEquals(const ReadOnlySet<T>& other) const {
        return getCountProperty() == other.getCountProperty() && IsSubsetOf(other);
    }

    /** @brief Returns a const iterator to the beginning of the set (STL interop). */
    auto begin() const { return set_->begin(); }
    /** @brief Returns a const iterator past the end of the set (STL interop). */
    auto end()   const { return set_->end(); }
};

} // namespace System::Collections::ObjectModel
