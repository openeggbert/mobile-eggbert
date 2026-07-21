// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Collections {

using SharpRuntime::intcs;

/**
 * @brief Exposes a method that compares two non-generic objects.
 *
 * C++ counterpart of .NET System.Collections.IComparer.
 * Because C++ has no unified object root, objects are typed as const void*.
 */
class IComparer {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IComparer() = default;

    /**
     * @brief Compares two objects and returns a value indicating their relative order.
     *
     * C++ counterpart of .NET IComparer.Compare(object, object).
     * @param x The first object to compare.
     * @param y The second object to compare.
     * @return A negative integer if x < y, zero if x == y, or a positive integer if x > y.
     */
    [[nodiscard]] virtual intcs Compare(const void* x, const void* y) const = 0;
};

} // namespace System::Collections
