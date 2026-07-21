// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <cstdint>
#include <cstddef>
#include <limits>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/OverflowException.hpp"

namespace System
{
    using SharpRuntime::intcs;
    using SharpRuntime::longcs;

    /**
     * @brief A platform-specific type used to represent a pointer or a handle.
     *
     * C++ counterpart of .NET System.IntPtr.
     * Holds a signed integer large enough to hold a pointer on the target platform.
     */
    struct IntPtr
    {
        /** @brief The underlying signed pointer-sized integer. */
        intptr_t value = 0;

        /** @brief Default constructor — initialises value to 0. */
        IntPtr() = default;

        IntPtr(intptr_t v) : value(v) {}           // NOLINT(google-explicit-constructor)
        IntPtr(void* p) : value(reinterpret_cast<intptr_t>(p)) {}  // NOLINT(google-explicit-constructor)

        /** @brief The IntPtr whose underlying value is zero. */
        static const IntPtr Zero;

        /** @brief The largest representable IntPtr value. C++ counterpart of .NET IntPtr.MaxValue. */
        static const IntPtr MaxValue;
        /** @brief The smallest representable IntPtr value. C++ counterpart of .NET IntPtr.MinValue. */
        static const IntPtr MinValue;

        /** @brief The size, in bytes, of an IntPtr on the current platform. C++ counterpart of .NET IntPtr.Size. */
        static constexpr intcs Size = static_cast<intcs>(sizeof(intptr_t));

        /** @brief Returns true if both IntPtr instances hold the same value. */
        [[nodiscard]] bool operator==(const IntPtr& other) const { return value == other.value; }
        /** @brief Returns true if the two IntPtr instances hold different values. */
        [[nodiscard]] bool operator!=(const IntPtr& other) const { return value != other.value; }

        /** @brief Explicit conversion to the underlying intptr_t integer. */
        [[nodiscard]] explicit operator intptr_t() const { return value; }
        /** @brief Explicit conversion to a raw void pointer. */
        [[nodiscard]] explicit operator void*() const { return reinterpret_cast<void*>(value); }

        /** @brief Returns true if the underlying value is zero. */
        [[nodiscard]] bool IsZero() const { return value == 0; }

        /**
         * @brief Converts the value of this instance to a 32-bit signed integer.
         *
         * C++ counterpart of .NET IntPtr.ToInt32().
         * @throws System::OverflowException if the value does not fit in a 32-bit signed integer
         *         (only possible when @c intptr_t is 64-bit on this platform).
         */
        [[nodiscard]] intcs ToInt32() const {
            if (value > std::numeric_limits<int32_t>::max() || value < std::numeric_limits<int32_t>::min())
                throw System::OverflowException("Arithmetic operation resulted in an overflow.");
            return static_cast<intcs>(value);
        }

        /** @brief Converts the value of this instance to a 64-bit signed integer. C++ counterpart of .NET IntPtr.ToInt64(). */
        [[nodiscard]] longcs ToInt64() const { return static_cast<longcs>(value); }

        /** @brief Converts the value of this instance to a raw pointer. C++ counterpart of .NET IntPtr.ToPointer(). */
        [[nodiscard]] void* ToPointer() const { return reinterpret_cast<void*>(value); }

        /**
         * @brief Compares this instance to another IntPtr value.
         *
         * C++ counterpart of .NET IntPtr.CompareTo(nint).
         * @return -1 if less than @p other; 0 if equal; 1 if greater than @p other.
         */
        [[nodiscard]] intcs CompareTo(const IntPtr& other) const noexcept {
            return (value < other.value) ? -1 : (value > other.value) ? 1 : 0;
        }

        /** @brief Determines whether the specified IntPtr is equal to this instance. C++ counterpart of .NET IntPtr.Equals(nint). */
        [[nodiscard]] bool Equals(const IntPtr& other) const noexcept { return value == other.value; }

        /** @brief Returns the hash code for this instance. C++ counterpart of .NET IntPtr.GetHashCode(). */
        [[nodiscard]] intcs GetHashCode() const noexcept {
            return static_cast<intcs>(static_cast<int64_t>(value) ^ (static_cast<uint64_t>(value) >> 32));
        }

        /** @brief Converts the value of this instance to its decimal string representation. C++ counterpart of .NET IntPtr.ToString(). */
        [[nodiscard]] std::string ToString() const { return std::to_string(static_cast<intmax_t>(value)); }

        /**
         * @brief Adds an offset to the value of a pointer.
         *
         * C++ counterpart of .NET IntPtr.Add(nint, int) / operator+(nint, int).
         */
        [[nodiscard]] static IntPtr Add(const IntPtr& pointer, intcs offset) {
            return IntPtr(static_cast<intptr_t>(pointer.value + offset));
        }

        /**
         * @brief Subtracts an offset from the value of a pointer.
         *
         * C++ counterpart of .NET IntPtr.Subtract(nint, int) / operator-(nint, int).
         */
        [[nodiscard]] static IntPtr Subtract(const IntPtr& pointer, intcs offset) {
            return IntPtr(static_cast<intptr_t>(pointer.value - offset));
        }

        /** @brief Adds an offset to the value of a pointer. C++ counterpart of .NET IntPtr operator+(nint, int). */
        friend IntPtr operator+(const IntPtr& pointer, intcs offset) { return Add(pointer, offset); }
        /** @brief Subtracts an offset from the value of a pointer. C++ counterpart of .NET IntPtr operator-(nint, int). */
        friend IntPtr operator-(const IntPtr& pointer, intcs offset) { return Subtract(pointer, offset); }
    };

    inline const IntPtr IntPtr::Zero{intptr_t(0)};
    inline const IntPtr IntPtr::MaxValue{std::numeric_limits<intptr_t>::max()};
    inline const IntPtr IntPtr::MinValue{std::numeric_limits<intptr_t>::min()};
}
