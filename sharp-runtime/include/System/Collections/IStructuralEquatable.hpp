// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/IEqualityComparer.hpp"

namespace System::Collections {

using SharpRuntime::intcs;

/**
 * @brief Defines methods to support structural equality comparison of objects within a collection.
 *
 * C++ counterpart of .NET System.Collections.IStructuralEquatable.
 */
class IStructuralEquatable {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IStructuralEquatable() = default;

    /**
     * @brief Determines whether an object is structurally equal to the current instance.
     *
     * C++ counterpart of .NET IStructuralEquatable.Equals(object, IEqualityComparer).
     * @param other    The object to compare with the current instance.
     * @param comparer The comparer to use.
     * @return true if the objects are considered equal; otherwise false.
     */
    [[nodiscard]] virtual bool Equals(const void* other, const IEqualityComparer& comparer) const = 0;

    /**
     * @brief Returns a hash code for the current instance using the specified comparer.
     *
     * C++ counterpart of .NET IStructuralEquatable.GetHashCode(IEqualityComparer).
     * @param comparer The comparer to use.
     * @return A hash code for the current instance.
     */
    [[nodiscard]] virtual intcs GetHashCode(const IEqualityComparer& comparer) const = 0;
};

} // namespace System::Collections
