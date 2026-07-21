// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/Collections/Generic/IEqualityComparer.hpp"

namespace System::Collections::Generic {

/**
 * @brief An equality comparer that compares objects by reference (pointer identity).
 *
 * C++ counterpart of .NET System.Collections.Generic.ReferenceEqualityComparer.
 * Two pointers are considered equal only if they point to the same object in memory,
 * regardless of the value pointed to.
 *
 * Use Instance() to obtain the singleton.
 *
 * @tparam T The pointed-to type; the comparer operates on T* pointers.
 */
template<typename T>
class ReferenceEqualityComparer : public IEqualityComparer<T*> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~ReferenceEqualityComparer() = default;

    /**
     * @brief Determines whether two pointers refer to the same object.
     *
     * C++ counterpart of .NET ReferenceEqualityComparer.Equals(object, object).
     * @param x The first pointer.
     * @param y The second pointer.
     * @return true if x and y point to the same object (x == y); otherwise false.
     */
    [[nodiscard]] bool Equals(T* const& x, T* const& y) const override {
        return x == y;
    }

    /**
     * @brief Returns a hash code based on the pointer address of @p obj.
     *
     * C++ counterpart of .NET ReferenceEqualityComparer.GetHashCode(object).
     * @param obj The pointer whose address is used as the hash input.
     * @return A hash code derived from the pointer value.
     */
    [[nodiscard]] intcs GetHashCode(T* const& obj) const override {
        return static_cast<intcs>(std::hash<T*>{}(obj));
    }

    /**
     * @brief Returns the singleton ReferenceEqualityComparer<T> instance.
     *
     * C++ counterpart of .NET ReferenceEqualityComparer.Instance.
     * @return A reference to the shared singleton.
     */
    static ReferenceEqualityComparer<T>& Instance() {
        static ReferenceEqualityComparer<T> inst;
        return inst;
    }
};

} // namespace System::Collections::Generic
