// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/Generic/IComparer.hpp"
#include "System/Collections/Generic/EqualityComparer.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

/**
 * @brief Provides a base class for implementations of IComparer<T>.
 *
 * C++ counterpart of .NET System.Collections.Generic.Comparer<T>.
 * Derive from this class and override Compare to implement a custom ordering.
 *
 * @tparam T The type of objects to compare.
 */
template<typename T>
class Comparer : public IComparer<T> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~Comparer() = default;

    /**
     * @brief When overridden in a derived class, compares two objects and returns an ordering integer.
     *
     * C++ counterpart of .NET Comparer<T>.Compare(T, T).
     * @param x The first object to compare.
     * @param y The second object to compare.
     * @return Negative if x < y, zero if x == y, positive if x > y.
     */
    [[nodiscard]] virtual intcs Compare(const T& x, const T& y) const override = 0;

    /**
     * @brief Gets the default comparer for type T, using operator<.
     *
     * C++ counterpart of .NET Comparer<T>.Default.
     * @return A singleton comparer that orders elements using operator<.
     */
    static const Comparer<T>& Default() {
        static struct DefaultComparer : Comparer<T> {
            [[nodiscard]] intcs Compare(const T& x, const T& y) const override {
                if (x < y) return -1;
                if (y < x) return  1;
                return 0;
            }
        } instance;
        return instance;
    }

    /**
     * @brief Creates a comparer from a comparison function.
     *
     * C++ counterpart of .NET Comparer<T>.Create(Comparison<T>).
     * @param comparison A function that compares two T values and returns an ordering integer.
     * @return A new heap-allocated Comparer wrapping the function (caller owns the pointer).
     */
    static Comparer<T>* Create(std::function<intcs(const T&, const T&)> comparison) {
        struct LambdaComparer : Comparer<T> {
            std::function<intcs(const T&, const T&)> fn_;
            explicit LambdaComparer(std::function<intcs(const T&, const T&)> f) : fn_(std::move(f)) {}
            [[nodiscard]] intcs Compare(const T& x, const T& y) const override { return fn_(x, y); }
        };
        return new LambdaComparer(std::move(comparison));
    }
};

} // namespace System::Collections::Generic
