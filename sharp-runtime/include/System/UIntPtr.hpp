// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <functional>
#include <limits>
#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    using SharpRuntime::intcs;

    /**
     * @brief A platform-specific type used to represent a pointer or a handle (unsigned).
     *
     * C++ counterpart of .NET System.UIntPtr.
     * Holds an unsigned integer large enough to hold a pointer on the target platform.
     */
    struct UIntPtr {
        /** @brief The underlying unsigned pointer-sized integer. */
        uintptr_t value = 0;

        /** @brief Default constructor — initialises value to 0. */
        UIntPtr() = default;
        UIntPtr(uintptr_t v) : value(v) {}       // NOLINT(google-explicit-constructor)
        UIntPtr(void* p) : value(reinterpret_cast<uintptr_t>(p)) {} // NOLINT(google-explicit-constructor)

        /** @brief The UIntPtr whose underlying value is zero. */
        static const UIntPtr Zero;

        /** @brief The largest possible value of a UIntPtr. C++ counterpart of .NET UIntPtr.MaxValue. */
        static const UIntPtr MaxValue;

        /**
         * @brief The size, in bytes, of a UIntPtr on the current platform (4 or 8).
         * C++ counterpart of .NET UIntPtr.Size.
         */
        static constexpr intcs Size = static_cast<intcs>(sizeof(uintptr_t));

        /** @brief Returns true if both UIntPtr instances hold the same value. */
        [[nodiscard]] bool operator==(const UIntPtr& o) const { return value == o.value; }
        /** @brief Returns true if the two UIntPtr instances hold different values. */
        [[nodiscard]] bool operator!=(const UIntPtr& o) const { return value != o.value; }

        /** @brief Explicit conversion to the underlying uintptr_t integer. */
        [[nodiscard]] explicit operator uintptr_t() const { return value; }
        /** @brief Explicit conversion to a raw void pointer. */
        [[nodiscard]] explicit operator void*() const { return reinterpret_cast<void*>(value); }

        /** @brief Returns true if the underlying value is zero. */
        [[nodiscard]] bool IsZero() const { return value == 0; }

        /** @brief Returns a raw void pointer view of this value. C++ counterpart of .NET UIntPtr.ToPointer(). */
        [[nodiscard]] void* ToPointer() const { return reinterpret_cast<void*>(value); }

        /** @brief Compares this value to @p other. C++ counterpart of .NET UIntPtr.CompareTo(UIntPtr). */
        [[nodiscard]] intcs CompareTo(const UIntPtr& other) const {
            return (value < other.value) ? -1 : (value > other.value) ? 1 : 0;
        }

        /** @brief Returns true if this value equals @p other. C++ counterpart of .NET UIntPtr.Equals(UIntPtr). */
        [[nodiscard]] bool Equals(const UIntPtr& other) const { return value == other.value; }

        /** @brief Returns a hash code for this value. C++ counterpart of .NET UIntPtr.GetHashCode(). */
        [[nodiscard]] intcs GetHashCode() const {
            return static_cast<intcs>(std::hash<uintptr_t>{}(value));
        }

        /** @brief Returns the decimal string representation of this value. C++ counterpart of .NET UIntPtr.ToString(). */
        [[nodiscard]] std::string ToString() const { return std::to_string(value); }

        /** @brief Returns 0 if @p value is zero; otherwise 1. C++ counterpart of .NET UIntPtr.Sign(UIntPtr). */
        [[nodiscard]] static intcs Sign(const UIntPtr& v) { return v.value == 0 ? 0 : 1; }
    };

    inline const UIntPtr UIntPtr::Zero{uintptr_t(0)};
    inline const UIntPtr UIntPtr::MaxValue{std::numeric_limits<uintptr_t>::max()};

} // namespace System
