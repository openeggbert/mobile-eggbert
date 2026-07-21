// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/IComparer.hpp"
#include "System/Collections/IEqualityComparer.hpp"
#include "System/Collections/IStructuralComparable.hpp"
#include "System/Collections/IStructuralEquatable.hpp"

namespace System::Collections {

using SharpRuntime::intcs;

/**
 * @brief Provides objects for performing a structural comparison of two collection objects.
 *
 * C++ counterpart of .NET System.Collections.StructuralComparisons.
 * Provides singleton instances of IComparer and IEqualityComparer that delegate
 * to IStructuralComparable and IStructuralEquatable when the objects implement them.
 */
class StructuralComparisons {
public:
    StructuralComparisons() = delete;

    /**
     * @brief Gets an IComparer object that performs structural comparison.
     *
     * C++ counterpart of .NET StructuralComparisons.StructuralComparer.
     *
     * @warning Unlike .NET's real StructuralComparer.Compare(object,object) (StructuralComparisons.cs),
     * which safely tests `x as IStructuralComparable` and falls back to
     * `Comparer<object>.Default.Compare` when the cast fails, this C++ port has no way to
     * perform that safe runtime check: IComparer::Compare takes `const void*` (there is no
     * common polymorphic object root in C++ to safely query), so the returned comparer's
     * Compare() unconditionally `static_cast`s its non-null arguments to
     * `const IStructuralComparable*`. Calling Compare() with an @p x that does not actually
     * point to an IStructuralComparable instance is undefined behavior, not a safe fallback —
     * only pass pointers to types that genuinely derive from IStructuralComparable.
     */
    static const IComparer& getStructuralComparerProperty();

    /**
     * @brief Gets an IEqualityComparer object that performs structural equality comparison.
     *
     * C++ counterpart of .NET StructuralComparisons.StructuralEqualityComparer.
     *
     * @warning Same caveat as getStructuralComparerProperty(): .NET's real
     * StructuralEqualityComparer safely tests `x as IStructuralEquatable` and falls back to
     * `x.Equals(y)`/`obj.GetHashCode()`; this port has no way to perform that check from a
     * bare `const void*` and unconditionally `static_cast`s to `const IStructuralEquatable*`.
     * Calling Equals()/GetHashCode() with an argument that is not actually an
     * IStructuralEquatable instance is undefined behavior — only pass pointers to types that
     * genuinely derive from IStructuralEquatable.
     */
    static const IEqualityComparer& getStructuralEqualityComparerProperty();
};

// -----------------------------------------------------------------------
// Implementation detail — private comparers defined in this header
// -----------------------------------------------------------------------

namespace detail {

class StructuralComparerImpl final : public IComparer {
public:
    /**
     * @brief Compares two objects structurally.
     *
     * Delegates to IStructuralComparable::CompareTo. @p x must actually point to an
     * IStructuralComparable instance (or be null) — see the @warning on
     * StructuralComparisons::getStructuralComparerProperty().
     * @param x Left operand (pointer to an IStructuralComparable instance, or null).
     * @param y Right operand (pointer to the object).
     * @return Negative, zero, or positive.
     */
    [[nodiscard]] intcs Compare(const void* x, const void* y) const override {
        if (x == y) return 0;
        if (!x)     return -1;
        if (!y)     return  1;
        const auto* sc = static_cast<const IStructuralComparable*>(x);
        return sc->CompareTo(y, *this);
    }
};

class StructuralEqualityComparerImpl final : public IEqualityComparer {
public:
    /**
     * @brief Determines structural equality of two objects.
     *
     * Delegates to IStructuralEquatable::Equals. @p x must actually point to an
     * IStructuralEquatable instance (or be null) — see the @warning on
     * StructuralComparisons::getStructuralEqualityComparerProperty().
     * @param x Left operand (pointer to an IStructuralEquatable instance, or null).
     * @param y Right operand.
     * @return true if structurally equal.
     */
    [[nodiscard]] bool Equals(const void* x, const void* y) const override {
        if (x == y) return true;
        if (!x || !y) return false;
        const auto* se = static_cast<const IStructuralEquatable*>(x);
        return se->Equals(y, *this);
    }

    /**
     * @brief Returns a structural hash code for an object.
     *
     * Delegates to IStructuralEquatable::GetHashCode. @p obj must actually point to an
     * IStructuralEquatable instance (or be null).
     * @param obj The object to hash (pointer to an IStructuralEquatable instance, or null).
     * @return Hash code.
     */
    [[nodiscard]] intcs GetHashCode(const void* obj) const override {
        if (!obj) return 0;
        const auto* se = static_cast<const IStructuralEquatable*>(obj);
        return se->GetHashCode(*this);
    }
};

} // namespace detail

inline const IComparer& StructuralComparisons::getStructuralComparerProperty() {
    static detail::StructuralComparerImpl instance;
    return instance;
}

inline const IEqualityComparer& StructuralComparisons::getStructuralEqualityComparerProperty() {
    static detail::StructuralEqualityComparerImpl instance;
    return instance;
}

} // namespace System::Collections
