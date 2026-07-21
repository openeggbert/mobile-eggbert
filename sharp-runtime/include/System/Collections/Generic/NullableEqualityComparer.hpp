// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <functional>
#include "System/Collections/Generic/EqualityComparer.hpp"

namespace System::Collections::Generic {

/**
 * @brief Provides equality comparison for nullable value types (std::optional<T>).
 *
 * C++ counterpart of .NET System.Collections.Generic.NullableEqualityComparer<T>.
 * Extends EqualityComparer<std::optional<T>>: two null values are equal;
 * a null and a non-null value are not equal; non-null values are compared using operator==.
 * GetHashCode returns 0 for null, or std::hash<T> for non-null values.
 *
 * @tparam T The underlying value type (must support operator== and std::hash<T>).
 */
template<typename T>
class NullableEqualityComparer : public EqualityComparer<std::optional<T>> {
public:
    /** @brief Constructs a NullableEqualityComparer using the default equality for T. */
    NullableEqualityComparer() = default;

    /**
     * @brief Determines whether two nullable values are equal.
     *
     * C++ counterpart of .NET NullableEqualityComparer<T>.Equals(T?, T?).
     * Two nullopt values are equal; a nullopt and a value are not equal.
     * @param x The first nullable value.
     * @param y The second nullable value.
     * @return true if both are null, or both have equal values; otherwise false.
     */
    [[nodiscard]] bool Equals(const std::optional<T>& x, const std::optional<T>& y) const override {
        if (!x.has_value() && !y.has_value()) return true;
        if (!x.has_value() || !y.has_value()) return false;
        return *x == *y;
    }

    /**
     * @brief Returns a hash code for the specified nullable value.
     *
     * C++ counterpart of .NET NullableEqualityComparer<T>.GetHashCode(T?).
     * Returns 0 for null; for non-null values, returns std::hash<T> of the underlying value.
     * @param obj The nullable value for which to compute the hash code.
     * @return A hash code, or 0 if @p obj is null.
     */
    [[nodiscard]] intcs GetHashCode(const std::optional<T>& obj) const override {
        if (!obj.has_value()) return 0;
        return static_cast<intcs>(std::hash<T>{}(*obj));
    }
};

} // namespace System::Collections::Generic
