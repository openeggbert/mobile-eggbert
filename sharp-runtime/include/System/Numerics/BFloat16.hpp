// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <array>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <string>

namespace System::Numerics {

/**
 * Brain Float 16 — a 16-bit floating-point type using the same bit layout as
 * the upper 16 bits of a 32-bit IEEE 754 float (1 sign, 8 exponent, 7 mantissa bits).
 * 
 * BFloat16 has the same dynamic range as float32 but less precision than float16.
 * Arithmetic is performed by promoting to float, computing, then truncating back.
 */
class BFloat16 {
    uint16_t bits_;

    static uint16_t fromFloat(float f) {
        uint32_t u;
        std::memcpy(&u, &f, 4);
        return static_cast<uint16_t>(u >> 16);
    }
    static float toFloat(uint16_t bits) {
        uint32_t u = static_cast<uint32_t>(bits) << 16;
        float f;
        std::memcpy(&f, &u, 4);
        return f;
    }

public:
    /** Constructs BFloat16 with value 0. */
    constexpr BFloat16() : bits_(0) {}
    /** Constructs BFloat16 from raw 16-bit pattern (no conversion). */
    constexpr explicit BFloat16(uint16_t rawBits) : bits_(rawBits) {}
    /** Converts a float to BFloat16 by truncating the lower 16 mantissa bits. */
    explicit BFloat16(float v) : bits_(fromFloat(v)) {}

    /** @return BFloat16 representation of 0. */
    static BFloat16 Zero()             { return BFloat16(uint16_t(0)); }
    /** @return BFloat16 representation of 1. */
    static BFloat16 One()              { return BFloat16(uint16_t(0x3F80u)); }
    /** @return BFloat16 representation of -1. */
    static BFloat16 NegativeOne()      { return BFloat16(uint16_t(0xBF80u)); }
    /** @return The largest finite BFloat16 value (~3.39×10³⁸). */
    static BFloat16 MaxValue()         { return BFloat16(uint16_t(0x7F7Fu)); }
    /** @return The most negative finite BFloat16 value (~−3.39×10³⁸). */
    static BFloat16 MinValue()         { return BFloat16(uint16_t(0xFF7Fu)); }
    /** @return The smallest positive BFloat16 value greater than zero (~9.18×10⁻⁴¹). */
    static BFloat16 Epsilon()          { return BFloat16(uint16_t(0x0001u)); }
    /** @return A BFloat16 Not-a-Number (NaN) value. */
    static BFloat16 NaN()              { return BFloat16(uint16_t(0xFFC0u)); }
    /** @return Positive infinity. */
    static BFloat16 PositiveInfinity() { return BFloat16(uint16_t(0x7F80u)); }
    /** @return Negative infinity. */
    static BFloat16 NegativeInfinity() { return BFloat16(uint16_t(0xFF80u)); }

    /** @return The raw 16-bit bit pattern. */
    [[nodiscard]] uint16_t getBitsProperty() const { return bits_; }

    /** Converts to float (lossless — BFloat16 is a subset of float32). */
    explicit operator float()  const { return toFloat(bits_); }
    /** Converts to double. */
    explicit operator double() const { return static_cast<double>(toFloat(bits_)); }

    /** Addition via float promotion. */
    BFloat16 operator+(const BFloat16& o) const { return BFloat16(toFloat(bits_) + toFloat(o.bits_)); }
    /** Subtraction via float promotion. */
    BFloat16 operator-(const BFloat16& o) const { return BFloat16(toFloat(bits_) - toFloat(o.bits_)); }
    /** Multiplication via float promotion. */
    BFloat16 operator*(const BFloat16& o) const { return BFloat16(toFloat(bits_) * toFloat(o.bits_)); }
    /** Division via float promotion. */
    BFloat16 operator/(const BFloat16& o) const { return BFloat16(toFloat(bits_) / toFloat(o.bits_)); }
    /** Unary negation (flips the sign bit). */
    BFloat16 operator-()                  const { return BFloat16(static_cast<uint16_t>(bits_ ^ 0x8000u)); }

    /** IEEE 754 equality via float promotion (NaN != NaN; +0 == -0). */
    bool operator==(const BFloat16& o) const { return toFloat(bits_) == toFloat(o.bits_); }
    /** IEEE 754 inequality via float promotion (NaN != NaN; +0 == -0). */
    bool operator!=(const BFloat16& o) const { return toFloat(bits_) != toFloat(o.bits_); }
    /** Less-than comparison via float promotion. */
    bool operator< (const BFloat16& o) const { return toFloat(bits_) <  toFloat(o.bits_); }
    /** Less-than-or-equal comparison via float promotion. */
    bool operator<=(const BFloat16& o) const { return toFloat(bits_) <= toFloat(o.bits_); }
    /** Greater-than comparison via float promotion. */
    bool operator> (const BFloat16& o) const { return toFloat(bits_) >  toFloat(o.bits_); }
    /** Greater-than-or-equal comparison via float promotion. */
    bool operator>=(const BFloat16& o) const { return toFloat(bits_) >= toFloat(o.bits_); }

    /** @return True if @p v is a NaN value. */
    static bool IsNaN(BFloat16 v)              { return (v.bits_ & 0x7FFFu) > 0x7F80u; }
    /** @return True if @p v is positive or negative infinity. */
    static bool IsInfinity(BFloat16 v)         { return (v.bits_ & 0x7FFFu) == 0x7F80u; }
    /** @return True if @p v is positive infinity. */
    static bool IsPositiveInfinity(BFloat16 v) { return v.bits_ == 0x7F80u; }
    /** @return True if @p v is negative infinity. */
    static bool IsNegativeInfinity(BFloat16 v) { return v.bits_ == 0xFF80u; }

    /** @return The value formatted as a decimal string using the invariant locale. */
    [[nodiscard]] std::string ToString() const {
        float v = toFloat(bits_);
        std::array<char, 32> buf;
        auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), v);
        return ec == std::errc{} ? std::string(buf.data(), ptr) : std::to_string(v);
    }
};

} // namespace System::Numerics
