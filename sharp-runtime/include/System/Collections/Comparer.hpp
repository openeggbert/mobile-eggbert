// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstring>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/IComparer.hpp"

namespace System::Collections {

using SharpRuntime::intcs;

/**
 * @brief Compares two objects for equivalence using pointer ordering.
 *
 * C++ counterpart of .NET System.Collections.Comparer.
 * Because C++ has no runtime type system, objects are compared by pointer value.
 * Callers that need value-based comparison should use a typed comparer.
 *
 * @warning This is a permanent architectural limitation, not a bug to be fixed locally: real
 * .NET Comparer.Compare calls IComparable.CompareTo on the actual runtime type via `x as
 * IComparable`, but C++ has no common object root, so this non-generic, `const void*`-typed
 * comparer cannot safely determine or invoke the pointee's real comparison logic. Same root
 * cause as System::Collections::ListDictionaryInternal and
 * System::Collections::StructuralComparisons.
 */
class Comparer : public IComparer {
public:
    /**
     * @brief Compares two objects and returns a value indicating their relative order.
     *
     * C++ counterpart of .NET Comparer.Compare(object, object).
     * Null is less than any non-null value; equal pointers return 0.
     * @param x The first object to compare.
     * @param y The second object to compare.
     * @return -1, 0, or 1.
     */
    [[nodiscard]] intcs Compare(const void* x, const void* y) const override {
        if (x == y) return 0;
        if (!x) return -1;
        if (!y) return  1;
        return x < y ? -1 : 1;
    }

    /**
     * @brief Returns the default Comparer instance (current-culture equivalent).
     *
     * C++ counterpart of .NET Comparer.Default.
     */
    static const Comparer& Default() {
        static Comparer instance;
        return instance;
    }

    /**
     * @brief Returns a Comparer that performs invariant-culture-equivalent comparisons.
     *
     * C++ counterpart of .NET Comparer.DefaultInvariant.
     * In this C++ implementation, both Default and DefaultInvariant use pointer ordering.
     */
    static const Comparer& DefaultInvariant() {
        static Comparer instance;
        return instance;
    }
};

} // namespace System::Collections
