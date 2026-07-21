// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Collections/Generic/Comparer.hpp"

namespace System::Collections::Generic {

/**
 * @brief A general-purpose comparer that uses operator< for ordering.
 *
 * C++ counterpart of .NET System.Collections.Generic.ObjectComparer<T>.
 * Extends Comparer<T>; used as the fallback comparer when no IComparable<T>-specific
 * comparer is available. Delegates to operator< (less-than).
 *
 * @tparam T The type of objects to compare; must support operator<.
 */
template<typename T>
class ObjectComparer : public Comparer<T> {
public:
    /** @brief Constructs an ObjectComparer using operator< for ordering. */
    ObjectComparer() = default;

    /**
     * @brief Compares two objects of type T using operator<.
     *
     * C++ counterpart of .NET ObjectComparer<T>.Compare(T, T).
     * @param x The first object to compare.
     * @param y The second object to compare.
     * @return Negative if x < y, zero if x == y, positive if x > y.
     */
    [[nodiscard]] intcs Compare(const T& x, const T& y) const override {
        if (x < y) return -1;
        if (y < x) return  1;
        return 0;
    }
};

} // namespace System::Collections::Generic
