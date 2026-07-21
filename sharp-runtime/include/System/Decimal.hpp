// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>

#include "System/MidpointRounding.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

#if defined(_MSC_VER)
#  error "Decimal requires unsigned __int128 (GCC/Clang only). MSVC is not supported for this type."
#endif

namespace System {

    using SharpRuntime::intcs;
    using SharpRuntime::longcs;
    using SharpRuntime::uintcs;
    using SharpRuntime::ulongcs;
    using SharpRuntime::bytecs;
    using SharpRuntime::sbytecs;
    using SharpRuntime::shortcs;
    using SharpRuntime::ushortcs;

/**
 * @brief 128-bit fixed-point decimal type matching .NET System.Decimal.
 *
 * Representation: value = (−1)^sign × mantissa / 10^scale,
 * where mantissa is a 96-bit unsigned integer and scale is 0–28.
 * This matches the .NET range: ±79,228,162,514,264,337,593,543,950,335.
 *
 * C++ counterpart of .NET System.Decimal.
 *
 * @note POSIX/GCC/Clang only — uses unsigned __int128 which MSVC does not support.
 * @note Deviations from .NET, consistent with project-wide out-of-scope decisions:
 *   - GetBits(decimal) returns its four 32-bit words via out-parameters instead of
 *     an int[4], since System::Array does not exist in sharp-runtime.
 *   - Culture-aware ToString/Parse (IFormatProvider, NumberStyles), span- and
 *     UTF-8-based Parse/TryParse/TryFormat overloads are not provided.
 *   - Generic-math CreateChecked/CreateSaturating/CreateTruncating/ConvertToInteger
 *     and reflection-based IConvertible/GetTypeCode are out of scope.
 */
class Decimal {
    using u128 = unsigned __int128;
    static constexpr u128 MAX_MANTISSA = (u128(1) << 96) - 1;

    u128    mantissa_ = 0;
    uint8_t scale_    = 0;
    bool    negative_ = false;

    struct uint192 { uint64_t lo = 0, mid = 0, hi = 0; };

    static uint192  mul96x96(u128 a, u128 b);
    static uint32_t div192by10(uint192& v);
    static bool     fits96(const uint192& v);
    static u128     to128(const uint192& v);
    static void     alignScales(u128& m1, uint8_t& s1, u128& m2, uint8_t& s2);
    static std::string u128str(u128 v);
    static double   u128tod(u128 v);

    void fitMantissa();
    void normalize();

    /** @brief Private constructor from raw internal parts. */
    Decimal(u128 m, uint8_t s, bool n);

public:
    // -----------------------------------------------------------------------
    // Constructors
    // -----------------------------------------------------------------------

    /** @brief Initializes a new instance of Decimal to zero. */
    Decimal() = default;

    /**
     * @brief Constructs a Decimal from a signed 32-bit integer.
     *
     * C++ counterpart of .NET Decimal(int).
     */
    /* implicit */ Decimal(intcs v);

    /**
     * @brief Constructs a Decimal from a signed 64-bit integer.
     *
     * C++ counterpart of .NET Decimal(long).
     */
    /* implicit */ Decimal(long long v);

    /**
     * @brief Constructs a Decimal from a signed long (platform-dependent width).
     */
    explicit Decimal(long v);

    /**
     * @brief Constructs a Decimal from a double-precision floating-point number.
     *
     * C++ counterpart of .NET Decimal(double).
     * Mirrors .NET behaviour: Decimal(0.1) may differ from Parse("0.1").
     * @throws System::OverflowException if @p v is NaN, Infinity, or too large.
     */
    explicit Decimal(double v);

    /**
     * @brief Constructs a Decimal from a single-precision floating-point number.
     *
     * C++ counterpart of .NET Decimal(float). Implemented by widening to double.
     * @throws System::OverflowException if @p v is NaN, Infinity, or too large.
     */
    explicit Decimal(float v);

    /**
     * @brief Constructs a Decimal from an unsigned 32-bit integer.
     *
     * C++ counterpart of .NET Decimal(uint).
     */
    /* implicit */ Decimal(uintcs v);

    /**
     * @brief Constructs a Decimal from an unsigned 64-bit integer.
     *
     * C++ counterpart of .NET Decimal(ulong).
     */
    /* implicit */ Decimal(ulongcs v);

    /**
     * @brief Constructs a Decimal from its raw 96-bit mantissa (as three 32-bit
     * words), sign, and scale.
     *
     * C++ counterpart of .NET Decimal(int, int, int, bool, byte).
     * @param lo         The low 32 bits of the 96-bit mantissa.
     * @param mid        The middle 32 bits of the 96-bit mantissa.
     * @param hi         The high 32 bits of the 96-bit mantissa.
     * @param isNegative true to represent a negative value; otherwise false.
     * @param scale      A power of 10 ranging from 0 to 28.
     * @throws System::ArgumentOutOfRangeException if @p scale is greater than 28.
     */
    Decimal(intcs lo, intcs mid, intcs hi, bool isNegative, bytecs scale);

    // -----------------------------------------------------------------------
    // Manifest constants
    // -----------------------------------------------------------------------

    /** @brief Represents the number zero (0). */
    static const Decimal Zero;

    /** @brief Represents the number one (1). */
    static const Decimal One;

    /** @brief Represents the number negative one (−1). */
    static const Decimal MinusOne;

    /** @brief Represents the largest possible value of a Decimal (+79,228,162,514,264,337,593,543,950,335). */
    static const Decimal MaxValue;

    /** @brief Represents the smallest possible value of a Decimal (−79,228,162,514,264,337,593,543,950,335). */
    static const Decimal MinValue;

    // -----------------------------------------------------------------------
    // Properties
    // -----------------------------------------------------------------------

    /**
     * @brief Gets the number of decimal places (0–28) in this instance.
     *
     * C++ counterpart of .NET Decimal.Scale.
     * @return The scale (number of digits to the right of the decimal point).
     */
    [[nodiscard]] uint8_t getScaleProperty() const noexcept { return scale_; }

    /**
     * @brief Gets a value indicating whether this instance is negative.
     *
     * C++ counterpart of checking the sign bit of a .NET Decimal.
     * @return true if the value is negative; otherwise false.
     */
    [[nodiscard]] bool getIsNegativeProperty() const noexcept { return negative_; }

    // -----------------------------------------------------------------------
    // Instance comparison / equality
    // -----------------------------------------------------------------------

    /**
     * @brief Compares this instance to another Decimal value.
     *
     * C++ counterpart of .NET Decimal.CompareTo(decimal).
     * @return A negative integer, zero, or a positive integer as this value is
     *         less than, equal to, or greater than @p other.
     */
    [[nodiscard]] intcs CompareTo(const Decimal& other) const
    {
        if (*this == other) return 0;
        return (*this < other) ? -1 : 1;
    }

    /**
     * @brief Determines whether this instance and @p other represent the same value.
     *
     * C++ counterpart of .NET Decimal.Equals(decimal).
     * @param other The Decimal to compare with this instance.
     * @return true if they are equal; otherwise false.
     */
    [[nodiscard]] bool Equals(const Decimal& other) const { return *this == other; }

    /**
     * @brief Returns a hash code for this Decimal value.
     *
     * C++ counterpart of .NET Decimal.GetHashCode().
     * Equal Decimal values (regardless of scale) produce the same hash code.
     * @return A 32-bit hash code.
     */
    [[nodiscard]] intcs GetHashCode() const noexcept
    {
        // Normalise: values that compare equal must have equal hashes, so
        // strip trailing zeros before hashing (same as .NET's behaviour).
        u128 m = mantissa_;
        uint8_t s = scale_;
        while (s > 0 && m % 10 == 0) { m /= 10; --s; }
        bool neg = negative_ && m != 0;
        size_t h = std::hash<uint64_t>{}(uint64_t(m));
        h ^= std::hash<uint64_t>{}(uint64_t(m >> 64)) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<uint8_t>{}(s)   + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<bool>{}(neg)    + 0x9e3779b9 + (h << 6) + (h >> 2);
        return static_cast<intcs>(h & 0x7fffffff);
    }

    // -----------------------------------------------------------------------
    // Conversions to primitive types
    // -----------------------------------------------------------------------

    /** @brief Converts this Decimal to a double-precision floating-point number. */
    [[nodiscard]] double  ToDouble() const;

    /** @brief Converts this Decimal to a single-precision floating-point number. */
    [[nodiscard]] float   ToSingle() const;

    /**
     * @brief Converts this Decimal to a 32-bit signed integer, truncating any fractional part.
     *
     * C++ counterpart of .NET Decimal.ToInt32(decimal).
     * @throws System::OverflowException if the value is outside the Int32 range.
     */
    [[nodiscard]] intcs  ToInt32() const;

    /**
     * @brief Converts this Decimal to a 64-bit signed integer, truncating any fractional part.
     *
     * C++ counterpart of .NET Decimal.ToInt64(decimal).
     * @throws System::OverflowException if the value is outside the Int64 range.
     */
    [[nodiscard]] longcs ToInt64() const;

    /**
     * @brief Converts this Decimal to a 32-bit unsigned integer, truncating any fractional part.
     *
     * C++ counterpart of .NET Decimal.ToUInt32(decimal).
     * @throws System::OverflowException if the value is negative or exceeds uint32 range.
     */
    [[nodiscard]] uintcs ToUInt32() const;

    /**
     * @brief Converts this Decimal to a 64-bit unsigned integer, truncating any fractional part.
     *
     * C++ counterpart of .NET Decimal.ToUInt64(decimal).
     * @throws System::OverflowException if the value is negative or exceeds uint64 range.
     */
    [[nodiscard]] ulongcs ToUInt64() const;

    /**
     * @brief Converts a Decimal to an unsigned byte, truncating any fractional part.
     *
     * C++ counterpart of .NET Decimal.ToByte(decimal).
     * @throws System::OverflowException if the value is outside the byte range.
     */
    [[nodiscard]] static bytecs ToByte(const Decimal& value);

    /**
     * @brief Converts a Decimal to a signed byte, truncating any fractional part.
     *
     * C++ counterpart of .NET Decimal.ToSByte(decimal).
     * @throws System::OverflowException if the value is outside the sbyte range.
     */
    [[nodiscard]] static sbytecs ToSByte(const Decimal& value);

    /**
     * @brief Converts a Decimal to a 16-bit signed integer, truncating any fractional part.
     *
     * C++ counterpart of .NET Decimal.ToInt16(decimal).
     * @throws System::OverflowException if the value is outside the Int16 range.
     */
    [[nodiscard]] static shortcs ToInt16(const Decimal& value);

    /**
     * @brief Converts a Decimal to a 16-bit unsigned integer, truncating any fractional part.
     *
     * C++ counterpart of .NET Decimal.ToUInt16(decimal).
     * @throws System::OverflowException if the value is outside the UInt16 range.
     */
    [[nodiscard]] static ushortcs ToUInt16(const Decimal& value);

    /** @brief Converts a Decimal to a double-precision floating-point number. C++ counterpart of .NET Decimal.ToDouble(decimal). */
    [[nodiscard]] static double  ToDouble(const Decimal& d) { return d.ToDouble(); }
    /** @brief Converts a Decimal to a single-precision floating-point number. C++ counterpart of .NET Decimal.ToSingle(decimal). */
    [[nodiscard]] static float   ToSingle(const Decimal& d) { return d.ToSingle(); }
    /** @brief Converts a Decimal to a 32-bit signed integer. C++ counterpart of .NET Decimal.ToInt32(decimal). */
    [[nodiscard]] static intcs   ToInt32(const Decimal& d)  { return d.ToInt32(); }
    /** @brief Converts a Decimal to a 64-bit signed integer. C++ counterpart of .NET Decimal.ToInt64(decimal). */
    [[nodiscard]] static longcs  ToInt64(const Decimal& d)  { return d.ToInt64(); }
    /** @brief Converts a Decimal to a 32-bit unsigned integer. C++ counterpart of .NET Decimal.ToUInt32(decimal). */
    [[nodiscard]] static uintcs  ToUInt32(const Decimal& d) { return d.ToUInt32(); }
    /** @brief Converts a Decimal to a 64-bit unsigned integer. C++ counterpart of .NET Decimal.ToUInt64(decimal). */
    [[nodiscard]] static ulongcs ToUInt64(const Decimal& d) { return d.ToUInt64(); }

    /**
     * @brief Decomposes this Decimal into its four constituent 32-bit words.
     *
     * C++ counterpart of .NET Decimal.GetBits(decimal). Returns the components via
     * out-parameters instead of an int[4], since System::Array does not exist.
     * @param d     The Decimal value to decompose.
     * @param lo    Receives the low 32 bits of the 96-bit mantissa.
     * @param mid   Receives the middle 32 bits of the 96-bit mantissa.
     * @param hi    Receives the high 32 bits of the 96-bit mantissa.
     * @param flags Receives the sign (bit 31) and scale (bits 16–23) word.
     */
    static void GetBits(const Decimal& d, intcs& lo, intcs& mid, intcs& hi, intcs& flags);

    // -----------------------------------------------------------------------
    // Parse / ToString
    // -----------------------------------------------------------------------

    /**
     * @brief Attempts to convert a string to a Decimal.
     *
     * C++ counterpart of .NET Decimal.TryParse(string, out decimal).
     * Accepts an optional leading sign and a single decimal separator ('.' or ',').
     * @param s      Input string.
     * @param result Receives the parsed value on success.
     * @return true on success; false if the string is malformed or overflows.
     */
    static bool TryParse(const std::string& s, Decimal& result);

    /**
     * @brief Converts the string representation of a number to its Decimal equivalent.
     *
     * C++ counterpart of .NET Decimal.Parse(string).
     * @throws System::FormatException if the string is malformed or overflows.
     */
    static Decimal Parse(const std::string& s);

    /**
     * @brief Converts the value of this instance to its equivalent string representation.
     *
     * C++ counterpart of .NET Decimal.ToString().
     * Integer values have no decimal point; fractional values include exactly
     * scale digits after the decimal point.
     */
    [[nodiscard]] std::string ToString() const;

    // -----------------------------------------------------------------------
    // Arithmetic operators
    // -----------------------------------------------------------------------

    /** @brief Adds two Decimal values. */
    Decimal operator+(const Decimal& o) const;
    /** @brief Subtracts one Decimal from another. */
    Decimal operator-(const Decimal& o) const;
    /** @brief Multiplies two Decimal values. */
    Decimal operator*(const Decimal& o) const;
    /** @brief Divides one Decimal by another. @throws System::DivideByZeroException on division by zero. */
    Decimal operator/(const Decimal& o) const;
    /** @brief Returns the remainder of dividing two Decimal values. */
    Decimal operator%(const Decimal& o) const;
    /** @brief Returns the negation of a Decimal value. */
    Decimal operator-() const;
    /** @brief Returns the value of the operand (unary plus). C++ counterpart of .NET's unary operator+(decimal). */
    Decimal operator+() const { return *this; }

    Decimal& operator+=(const Decimal& o);
    Decimal& operator-=(const Decimal& o);
    Decimal& operator*=(const Decimal& o);
    Decimal& operator/=(const Decimal& o);

    /** @brief Pre-increments this value by one. C++ counterpart of .NET's operator++(decimal). */
    Decimal& operator++() { *this = *this + One; return *this; }
    /** @brief Post-increments this value by one. C++ counterpart of .NET's operator++(decimal). */
    Decimal operator++(int) { Decimal tmp = *this; *this = *this + One; return tmp; }
    /** @brief Pre-decrements this value by one. C++ counterpart of .NET's operator--(decimal). */
    Decimal& operator--() { *this = *this - One; return *this; }
    /** @brief Post-decrements this value by one. C++ counterpart of .NET's operator--(decimal). */
    Decimal operator--(int) { Decimal tmp = *this; *this = *this - One; return tmp; }

    /** @brief Explicit conversion to unsigned byte. C++ counterpart of .NET's explicit operator byte(decimal). */
    explicit operator bytecs()   const { return ToByte(*this); }
    /** @brief Explicit conversion to signed byte. C++ counterpart of .NET's explicit operator sbyte(decimal). */
    explicit operator sbytecs()  const { return ToSByte(*this); }
    /** @brief Explicit conversion to a 16-bit signed integer. C++ counterpart of .NET's explicit operator short(decimal). */
    explicit operator shortcs()  const { return ToInt16(*this); }
    /** @brief Explicit conversion to a 16-bit unsigned integer. C++ counterpart of .NET's explicit operator ushort(decimal). */
    explicit operator ushortcs() const { return ToUInt16(*this); }
    /** @brief Explicit conversion to a 32-bit signed integer. C++ counterpart of .NET's explicit operator int(decimal). */
    explicit operator intcs()    const { return ToInt32(); }
    /** @brief Explicit conversion to a 32-bit unsigned integer. C++ counterpart of .NET's explicit operator uint(decimal). */
    explicit operator uintcs()   const { return ToUInt32(); }
    /** @brief Explicit conversion to a 64-bit signed integer. C++ counterpart of .NET's explicit operator long(decimal). */
    explicit operator longcs()   const { return ToInt64(); }
    /** @brief Explicit conversion to a 64-bit unsigned integer. C++ counterpart of .NET's explicit operator ulong(decimal). */
    explicit operator ulongcs()  const { return ToUInt64(); }
    /** @brief Explicit conversion to single precision. C++ counterpart of .NET's explicit operator float(decimal). */
    explicit operator float()    const { return ToSingle(); }
    /** @brief Explicit conversion to double precision. C++ counterpart of .NET's explicit operator double(decimal). */
    explicit operator double()   const { return ToDouble(); }

    // -----------------------------------------------------------------------
    // Comparison operators
    // -----------------------------------------------------------------------

    bool operator==(const Decimal& o) const;
    bool operator!=(const Decimal& o) const;
    bool operator< (const Decimal& o) const;
    bool operator<=(const Decimal& o) const;
    bool operator> (const Decimal& o) const;
    bool operator>=(const Decimal& o) const;

    // -----------------------------------------------------------------------
    // Static named arithmetic helpers (mirror .NET Decimal.Add etc.)
    // -----------------------------------------------------------------------

    /**
     * @brief Adds two Decimal values.
     *
     * C++ counterpart of .NET Decimal.Add(decimal, decimal).
     */
    static Decimal Add(const Decimal& d1, const Decimal& d2)      { return d1 + d2; }

    /**
     * @brief Subtracts one Decimal value from another.
     *
     * C++ counterpart of .NET Decimal.Subtract(decimal, decimal).
     */
    static Decimal Subtract(const Decimal& d1, const Decimal& d2) { return d1 - d2; }

    /**
     * @brief Multiplies two Decimal values.
     *
     * C++ counterpart of .NET Decimal.Multiply(decimal, decimal).
     */
    static Decimal Multiply(const Decimal& d1, const Decimal& d2) { return d1 * d2; }

    /**
     * @brief Divides one Decimal value by another.
     *
     * C++ counterpart of .NET Decimal.Divide(decimal, decimal).
     * @throws System::DivideByZeroException on division by zero.
     */
    static Decimal Divide(const Decimal& d1, const Decimal& d2)   { return d1 / d2; }

    /**
     * @brief Computes the remainder after dividing one Decimal by another.
     *
     * C++ counterpart of .NET Decimal.Remainder(decimal, decimal).
     */
    static Decimal Remainder(const Decimal& d1, const Decimal& d2){ return d1 % d2; }

    /**
     * @brief Returns the result of multiplying the specified Decimal value by negative one.
     *
     * C++ counterpart of .NET Decimal.Negate(decimal).
     */
    static Decimal Negate(const Decimal& d)                       { return -d; }

    // -----------------------------------------------------------------------
    // Static comparison helper
    // -----------------------------------------------------------------------

    /**
     * @brief Compares two Decimal values.
     *
     * C++ counterpart of .NET Decimal.Compare(decimal, decimal).
     * @return A negative integer, zero, or a positive integer as @p d1 is
     *         less than, equal to, or greater than @p d2.
     */
    static intcs Compare(const Decimal& d1, const Decimal& d2)
    {
        return d1.CompareTo(d2);
    }

    // -----------------------------------------------------------------------
    // Math helpers
    // -----------------------------------------------------------------------

    /**
     * @brief Returns the absolute value of a Decimal number.
     *
     * C++ counterpart of .NET Decimal.Abs(decimal).
     */
    static Decimal Abs(const Decimal& d);

    /**
     * @brief Returns the smallest integral value greater than or equal to @p d.
     *
     * C++ counterpart of .NET Decimal.Ceiling(decimal).
     */
    static Decimal Ceiling(const Decimal& d);

    /**
     * @brief Returns the largest integral value less than or equal to @p d.
     *
     * C++ counterpart of .NET Decimal.Floor(decimal).
     */
    static Decimal Floor(const Decimal& d);

    /**
     * @brief Returns the integral digits of @p d, discarding any fractional digits.
     *
     * C++ counterpart of .NET Decimal.Truncate(decimal).
     */
    static Decimal Truncate(const Decimal& d);

    /**
     * @brief Rounds @p d to the nearest integer, using banker's rounding (round half to even).
     *
     * C++ counterpart of .NET Decimal.Round(decimal).
     */
    static Decimal Round(const Decimal& d) { return Round(d, 0, MidpointRounding::ToEven); }

    /**
     * @brief Rounds @p d to @p decimals decimal places, using banker's rounding
     * (round half to even) — matches .NET's default MidpointRounding.ToEven.
     *
     * C++ counterpart of .NET Decimal.Round(decimal, int).
     * @param d        Value to round.
     * @param decimals Number of decimal places (0–28).
     * @throws System::ArgumentOutOfRangeException if @p decimals is outside 0–28.
     */
    static Decimal Round(const Decimal& d, intcs decimals) { return Round(d, decimals, MidpointRounding::ToEven); }

    /**
     * @brief Rounds @p d to the nearest integer, using the specified rounding convention.
     *
     * C++ counterpart of .NET Decimal.Round(decimal, MidpointRounding).
     */
    static Decimal Round(const Decimal& d, MidpointRounding mode) { return Round(d, 0, mode); }

    /**
     * @brief Rounds @p d to @p decimals decimal places, using the specified rounding convention.
     *
     * C++ counterpart of .NET Decimal.Round(decimal, int, MidpointRounding).
     * @param d        Value to round.
     * @param decimals Number of decimal places (0–28).
     * @param mode     The rounding convention to use for midpoint values.
     * @throws System::ArgumentOutOfRangeException if @p decimals is outside 0–28.
     */
    static Decimal Round(const Decimal& d, intcs decimals, MidpointRounding mode);

    /**
     * @brief Returns the larger of two Decimal values.
     *
     * C++ counterpart of .NET Math.Max / INumber.Max for Decimal.
     */
    static Decimal Max(const Decimal& d1, const Decimal& d2)
    {
        return (d1 >= d2) ? d1 : d2;
    }

    /**
     * @brief Returns the smaller of two Decimal values.
     *
     * C++ counterpart of .NET Math.Min / INumber.Min for Decimal.
     */
    static Decimal Min(const Decimal& d1, const Decimal& d2)
    {
        return (d1 <= d2) ? d1 : d2;
    }

    /**
     * @brief Returns the value with the larger absolute value (ties favour the non-negative value).
     *
     * C++ counterpart of .NET Decimal.MaxMagnitude(decimal, decimal) / INumberBase.MaxMagnitude.
     */
    static Decimal MaxMagnitude(const Decimal& x, const Decimal& y)
    {
        Decimal ax = Abs(x), ay = Abs(y);
        if (ax > ay) return x;
        if (ax == ay) return x.negative_ ? y : x;
        return y;
    }

    /**
     * @brief Returns the value with the smaller absolute value (ties favour the negative value).
     *
     * C++ counterpart of .NET Decimal.MinMagnitude(decimal, decimal) / INumberBase.MinMagnitude.
     */
    static Decimal MinMagnitude(const Decimal& x, const Decimal& y)
    {
        Decimal ax = Abs(x), ay = Abs(y);
        if (ax < ay) return x;
        if (ax == ay) return x.negative_ ? x : y;
        return y;
    }

    /**
     * @brief Returns a value clamped to the inclusive range [min, max].
     *
     * C++ counterpart of .NET Math.Clamp for Decimal.
     * @param value Value to clamp.
     * @param min   Minimum bound.
     * @param max   Maximum bound.
     */
    static Decimal Clamp(const Decimal& value, const Decimal& min, const Decimal& max)
    {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    /**
     * @brief Returns an integer that indicates the sign of a Decimal number.
     *
     * C++ counterpart of .NET Decimal.Sign / Math.Sign(decimal).
     * @return −1 if negative, 0 if zero, 1 if positive.
     */
    static intcs Sign(const Decimal& d)
    {
        if (d == Zero) return 0;
        return d.negative_ ? -1 : 1;
    }

    /**
     * @brief Returns a value with the magnitude of @p value and the sign of @p sign.
     *
     * C++ counterpart of .NET Decimal.CopySign(decimal, decimal) / INumber.CopySign.
     */
    static Decimal CopySign(const Decimal& value, const Decimal& sign)
    {
        return Decimal(value.mantissa_, value.scale_, sign.negative_ && value.mantissa_ != 0);
    }

    /**
     * @brief Determines if @p value represents an integral value.
     *
     * C++ counterpart of .NET Decimal.IsInteger(decimal) / INumberBase.IsInteger.
     */
    static bool IsInteger(const Decimal& value) { return value == Truncate(value); }

    /**
     * @brief Determines if @p value is negative.
     *
     * C++ counterpart of .NET Decimal.IsNegative(decimal) / INumberBase.IsNegative.
     */
    static bool IsNegative(const Decimal& value) noexcept { return value.negative_; }

    /**
     * @brief Determines if @p value is positive (including positive zero).
     *
     * C++ counterpart of .NET Decimal.IsPositive(decimal) / INumberBase.IsPositive.
     */
    static bool IsPositive(const Decimal& value) noexcept { return !value.negative_; }

    /**
     * @brief Determines if @p value is an even integral value.
     *
     * C++ counterpart of .NET Decimal.IsEvenInteger(decimal) / INumberBase.IsEvenInteger.
     */
    static bool IsEvenInteger(const Decimal& value)
    {
        Decimal t = Truncate(value);
        return value == t && (uint64_t(t.mantissa_) & 1) == 0;
    }

    /**
     * @brief Determines if @p value is an odd integral value.
     *
     * C++ counterpart of .NET Decimal.IsOddInteger(decimal) / INumberBase.IsOddInteger.
     */
    static bool IsOddInteger(const Decimal& value)
    {
        Decimal t = Truncate(value);
        return value == t && (uint64_t(t.mantissa_) & 1) != 0;
    }

    /**
     * @brief Determines if @p value is in its canonical representation (no unnecessary trailing zeros).
     *
     * C++ counterpart of .NET Decimal.IsCanonical(decimal) / INumberBase.IsCanonical.
     */
    static bool IsCanonical(const Decimal& value)
    {
        if (value.scale_ == 0) return true;
        return (value.mantissa_ % 10) != 0;
    }

    // -----------------------------------------------------------------------
    // OLE Automation Currency interop
    // -----------------------------------------------------------------------

    /**
     * @brief Converts an OLE Automation Currency value to a Decimal.
     *
     * C++ counterpart of .NET Decimal.FromOACurrency(long). OA Currency stores
     * monetary values as a 64-bit integer where 1 unit = 10000 (i.e. four
     * implicit decimal places).
     * @param cy OA Currency value.
     * @return The equivalent Decimal value (cy / 10000).
     */
    static Decimal FromOACurrency(long long cy)
    {
        return Decimal(cy) / Decimal(10000LL);
    }

    /**
     * @brief Converts this Decimal to an OLE Automation Currency value.
     *
     * C++ counterpart of .NET Decimal.ToOACurrency(decimal). Multiplies by
     * 10000 and truncates to a 64-bit integer.
     * @return OA Currency representation of this value.
     */
    [[nodiscard]] long long ToOACurrency() const
    {
        Decimal scaled = *this * Decimal(10000LL);
        return Truncate(scaled).ToInt64();
    }

    /**
     * @brief Converts a Decimal to an OLE Automation Currency value (static form).
     *
     * C++ counterpart of .NET Decimal.ToOACurrency(decimal).
     * @param d The Decimal value to convert.
     * @return OA Currency representation.
     */
    static long long ToOACurrency(const Decimal& d)
    {
        return d.ToOACurrency();
    }
};

} // namespace System
