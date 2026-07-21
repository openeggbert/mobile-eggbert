// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/IComparer.hpp"

namespace System::Collections {

using SharpRuntime::intcs;

/**
 * @brief Supports structural comparison of objects within a collection.
 *
 * C++ counterpart of .NET System.Collections.IStructuralComparable.
 */
class IStructuralComparable {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IStructuralComparable() = default;

    /**
     * @brief Compares this object with another using the specified comparer.
     *
     * C++ counterpart of .NET IStructuralComparable.CompareTo(object, IComparer).
     * @param other   The object to compare with the current instance.
     * @param comparer The comparer to use.
     * @return Negative, zero, or positive depending on relative order.
     */
    [[nodiscard]] virtual intcs CompareTo(const void* other, const IComparer& comparer) const = 0;
};

} // namespace System::Collections
