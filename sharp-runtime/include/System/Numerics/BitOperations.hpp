// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <bit>
#include <cstdint>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Numerics {

using SharpRuntime::intcs;
using SharpRuntime::longcs;
using SharpRuntime::uintcs;
using SharpRuntime::ulongcs;

/**
 * Utility class providing static methods for common bit-manipulation operations.
 *
 * All methods map directly to C++20 <bit> intrinsics (std::popcount, std::countl_zero,
 * std::rotl, std::rotr, std::bit_width) for maximum performance.
 */
class BitOperations {
public:
    BitOperations() = delete;

    /** @return True if @p value is an exact power of two (and non-zero). */
    static bool IsPow2(uintcs value) noexcept { return value != 0 && (value & (value - 1)) == 0; }
    /** @return True if @p value is an exact power of two (and non-zero). */
    static bool IsPow2(ulongcs value) noexcept { return value != 0 && (value & (value - 1)) == 0; }
    /** @return True if @p value is a positive exact power of two. */
    static bool IsPow2(intcs value)  noexcept { return value > 0  && (value & (value - 1)) == 0; }
    /** @return True if @p value is a positive exact power of two. */
    static bool IsPow2(longcs value)  noexcept { return value > 0  && (value & (value - 1)) == 0; }

    /** @return The smallest power of two that is >= @p value; returns 0 if @p value is 0 or the result overflows. */
    static uintcs RoundUpToPowerOf2(uintcs value) noexcept {
        --value;
        value |= value >> 1; value |= value >> 2; value |= value >> 4;
        value |= value >> 8; value |= value >> 16;
        return value + 1;
    }
    /** @return The smallest power of two that is >= @p value; returns 0 if @p value is 0 or the result overflows. */
    static ulongcs RoundUpToPowerOf2(ulongcs value) noexcept {
        --value;
        value |= value >> 1;  value |= value >> 2;  value |= value >> 4;
        value |= value >> 8;  value |= value >> 16; value |= value >> 32;
        return value + 1;
    }

    /** @return The number of leading zero bits in @p value (0 to 32; 32 if value == 0). */
    static intcs LeadingZeroCount(uintcs value) noexcept {
        return static_cast<intcs>(std::countl_zero(value));
    }
    /** @return The number of leading zero bits in @p value (0 to 64; 64 if value == 0). */
    static intcs LeadingZeroCount(ulongcs value) noexcept {
        return static_cast<intcs>(std::countl_zero(value));
    }

    /** @return Floor of log₂(value); returns 0 for value <= 1. */
    static intcs Log2(uintcs value) noexcept {
        return value <= 1 ? 0 : static_cast<intcs>(std::bit_width(value)) - 1;
    }
    /** @return Floor of log₂(value); returns 0 for value <= 1. */
    static intcs Log2(ulongcs value) noexcept {
        return value <= 1 ? 0 : static_cast<intcs>(std::bit_width(value)) - 1;
    }

    /** @return The number of set bits (1-bits) in @p value. */
    static intcs PopCount(uintcs value) noexcept { return static_cast<intcs>(std::popcount(value)); }
    /** @return The number of set bits (1-bits) in @p value. */
    static intcs PopCount(ulongcs value) noexcept { return static_cast<intcs>(std::popcount(value)); }

    /** @return The number of trailing zero bits in @p value (0 to 32; 32 if value == 0). */
    static intcs TrailingZeroCount(uintcs value) noexcept {
        return static_cast<intcs>(std::countr_zero(value));
    }
    /** @return The number of trailing zero bits in @p value (0 to 64; 64 if value == 0). */
    static intcs TrailingZeroCount(ulongcs value) noexcept {
        return static_cast<intcs>(std::countr_zero(value));
    }
    /** @return The number of trailing zero bits in @p value (treats as unsigned). */
    static intcs TrailingZeroCount(intcs value) noexcept {
        return TrailingZeroCount(static_cast<uintcs>(value));
    }

    /** @return @p value rotated left by @p offset bits (wraps around at 32 bits). */
    static uintcs RotateLeft(uintcs value, intcs offset) noexcept {
        return std::rotl(value, offset);
    }
    /** @return @p value rotated left by @p offset bits (wraps around at 64 bits). */
    static ulongcs RotateLeft(ulongcs value, intcs offset) noexcept {
        return std::rotl(value, offset);
    }
    /** @return @p value rotated right by @p offset bits (wraps around at 32 bits). */
    static uintcs RotateRight(uintcs value, intcs offset) noexcept {
        return std::rotr(value, offset);
    }
    /** @return @p value rotated right by @p offset bits (wraps around at 64 bits). */
    static ulongcs RotateRight(ulongcs value, intcs offset) noexcept {
        return std::rotr(value, offset);
    }

    /** @return @p value with its bits in reversed order (bit 0 ↔ bit 31). */
    static uintcs ReverseBits(uintcs value) noexcept {
        value = ((value >> 1) & 0x55555555u) | ((value & 0x55555555u) << 1);
        value = ((value >> 2) & 0x33333333u) | ((value & 0x33333333u) << 2);
        value = ((value >> 4) & 0x0F0F0F0Fu) | ((value & 0x0F0F0F0Fu) << 4);
        value = ((value >> 8) & 0x00FF00FFu) | ((value & 0x00FF00FFu) << 8);
        return (value >> 16) | (value << 16);
    }
};

} // namespace System::Numerics
