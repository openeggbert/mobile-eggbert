// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Collections {

using SharpRuntime::intcs;

/**
 * @brief Defines methods to support comparison of non-generic objects for equality.
 *
 * C++ counterpart of .NET System.Collections.IEqualityComparer.
 * Because C++ has no unified object root, element values are typed as const void*.
 */
class IEqualityComparer {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IEqualityComparer() = default;

    /**
     * @brief Determines whether the two specified objects are equal.
     *
     * C++ counterpart of .NET IEqualityComparer.Equals(object, object).
     * @param x The first object to compare.
     * @param y The second object to compare.
     * @return true if the specified objects are equal; otherwise false.
     */
    [[nodiscard]] virtual bool Equals(const void* x, const void* y) const = 0;

    /**
     * @brief Returns a hash code for the specified object.
     *
     * C++ counterpart of .NET IEqualityComparer.GetHashCode(object).
     * @param obj The object for which a hash code is to be returned.
     * @return A hash code for @p obj.
     */
    [[nodiscard]] virtual intcs GetHashCode(const void* obj) const = 0;
};

} // namespace System::Collections
