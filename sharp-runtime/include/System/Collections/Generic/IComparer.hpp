// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Defines a method that a type implements to compare two objects of the same type.
 *
 * C++ counterpart of .NET System.Collections.Generic.IComparer<T>.
 * Used by sorting algorithms and sorted collections to impose a total order on elements.
 *
 * @tparam T The type of objects to compare.
 */
template<typename T>
class IComparer {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IComparer() = default;

    /**
     * @brief Compares two objects and returns a value indicating their relative order.
     *
     * C++ counterpart of .NET IComparer<T>.Compare(T, T).
     * @param x The first object to compare.
     * @param y The second object to compare.
     * @return Negative if x < y, zero if x == y, positive if x > y.
     */
    [[nodiscard]] virtual intcs Compare(const T& x, const T& y) const = 0;
};

} // namespace System::Collections::Generic
