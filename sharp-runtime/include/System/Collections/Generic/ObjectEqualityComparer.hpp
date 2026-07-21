// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/Collections/Generic/EqualityComparer.hpp"

namespace System::Collections::Generic {

/**
 * @brief A general-purpose equality comparer that uses operator== and std::hash<T>.
 *
 * C++ counterpart of .NET System.Collections.Generic.ObjectEqualityComparer<T>.
 * Extends EqualityComparer<T>; used as the fallback equality comparer when no
 * IEquatable<T>-specific comparer is available. Delegates to operator== for equality
 * and std::hash<T> for hash codes.
 *
 * @tparam T The type of objects to compare; must support operator== and std::hash<T>.
 */
template<typename T>
class ObjectEqualityComparer : public EqualityComparer<T> {
public:
    /** @brief Constructs an ObjectEqualityComparer using operator== and std::hash<T>. */
    ObjectEqualityComparer() = default;

    /**
     * @brief Determines whether two objects of type T are equal using operator==.
     *
     * C++ counterpart of .NET ObjectEqualityComparer<T>.Equals(T, T).
     * @param x The first object to compare.
     * @param y The second object to compare.
     * @return true if x == y; otherwise false.
     */
    [[nodiscard]] bool Equals(const T& x, const T& y) const override {
        return x == y;
    }

    /**
     * @brief Returns a hash code for the specified object using std::hash<T>.
     *
     * C++ counterpart of .NET ObjectEqualityComparer<T>.GetHashCode(T).
     * @param obj The object for which to compute the hash code.
     * @return A hash code for @p obj.
     */
    [[nodiscard]] intcs GetHashCode(const T& obj) const override {
        return static_cast<intcs>(std::hash<T>{}(obj));
    }
};

} // namespace System::Collections::Generic
