// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cmath>
#include <cstdint>
#include <cstring>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    using SharpRuntime::intcs;

/**
 * @brief Provides constants and static utility methods for the single-precision
 * floating-point type (C# @c float / .NET @c System.Single).
 *
 * C++ counterpart of .NET System.Single.
 * All members are static; use the native C++ @c float type for storage.
 */
class Single {
public:
    Single() = delete;

    /**
     * @brief Returns a hash code for @p value. C++ counterpart of .NET Single.GetHashCode().
     * All NaN bit patterns and both signed zeros hash identically, so that values
     * considered Equals() always produce the same hash code.
     */
    [[nodiscard]] static intcs GetHashCode(float value) noexcept {
        uint32_t bits;
        static_assert(sizeof(bits) == sizeof(value));
        __builtin_memcpy(&bits, &value, sizeof(bits));
        if (std::isnan(value) || value == 0.0f) {
            constexpr uint32_t PositiveInfinityBits = 0x7F80'0000u;
            bits &= PositiveInfinityBits;
        }
        return static_cast<intcs>(bits);
    }
};

} // namespace System
