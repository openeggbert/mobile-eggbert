// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Defines methods to support the comparison of objects for equality.
 *
 * C++ counterpart of .NET System.Collections.Generic.IEqualityComparer<T>.
 * Implement this interface to provide custom equality semantics for use in
 * hash-based collections such as Dictionary and HashSet.
 *
 * @tparam T The type of objects to compare.
 */
template<typename T>
class IEqualityComparer {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IEqualityComparer() = default;

    /**
     * @brief Determines whether two objects of type T are equal.
     *
     * C++ counterpart of .NET IEqualityComparer<T>.Equals(T, T).
     * @param x The first object to compare.
     * @param y The second object to compare.
     * @return true if the specified objects are considered equal; otherwise false.
     */
    [[nodiscard]] virtual bool Equals(const T& x, const T& y) const = 0;

    /**
     * @brief Returns a hash code for the specified object.
     *
     * C++ counterpart of .NET IEqualityComparer<T>.GetHashCode(T).
     * Objects that compare equal must return the same hash code.
     * @param obj The object for which to get a hash code.
     * @return A hash code for the specified object.
     */
    [[nodiscard]] virtual intcs GetHashCode(const T& obj) const = 0;
};

} // namespace System::Collections::Generic
