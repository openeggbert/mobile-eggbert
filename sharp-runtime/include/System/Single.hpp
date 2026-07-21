// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <array>
#include <bit>
#include <cctype>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArithmeticException.hpp"
#include "System/FormatException.hpp"

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

    /** @brief Largest finite positive float (~3.4028235E+38). C++ counterpart of .NET Single.MaxValue. */
    static constexpr float MaxValue         =  std::numeric_limits<float>::max();

    /** @brief Largest finite negative float (~-3.4028235E+38). C++ counterpart of .NET Single.MinValue. */
    static constexpr float MinValue         = -std::numeric_limits<float>::max();

    /** @brief Smallest positive float greater than zero (1E-45, denormalized). C++ counterpart of .NET Single.Epsilon. */
    static constexpr float Epsilon          =  std::numeric_limits<float>::denorm_min();

    /** @brief Not-a-Number (quiet NaN). C++ counterpart of .NET Single.NaN. */
    static constexpr float NaN              =  std::numeric_limits<float>::quiet_NaN();

    /** @brief Positive infinity. C++ counterpart of .NET Single.PositiveInfinity. */
    static constexpr float PositiveInfinity =  std::numeric_limits<float>::infinity();

    /** @brief Negative infinity. C++ counterpart of .NET Single.NegativeInfinity. */
    static constexpr float NegativeInfinity = -std::numeric_limits<float>::infinity();

    /** @brief Negative zero (-0.0f). C++ counterpart of .NET Single.NegativeZero. */
    static constexpr float NegativeZero     = -0.0f;

    /** @brief Euler's number (~2.7182818). C++ counterpart of .NET Single.E. */
    static constexpr float E   = 2.7182817f;

    /** @brief The ratio of a circle's circumference to its diameter (~3.1415927). C++ counterpart of .NET Single.Pi. */
    static constexpr float Pi  = 3.1415927f;

    /** @brief The ratio of a circle's circumference to its radius (~6.2831855). C++ counterpart of .NET Single.Tau. */
    static constexpr float Tau = 6.2831855f;

    // -------------------------------------------------------------------------
    // Classification predicates
    // -------------------------------------------------------------------------

    /** @brief Returns true if @p f is Not-a-Number. C++ counterpart of .NET Single.IsNaN(float). */
    [[nodiscard]] static bool IsNaN(float f) noexcept { return std::isnan(f); }

    /** @brief Returns true if @p f is positive or negative infinity. C++ counterpart of .NET Single.IsInfinity(float). */
    [[nodiscard]] static bool IsInfinity(float f) noexcept { return std::isinf(f); }

    /** @brief Returns true if @p f is positive infinity. C++ counterpart of .NET Single.IsPositiveInfinity(float). */
    [[nodiscard]] static bool IsPositiveInfinity(float f) noexcept { return std::isinf(f) && f > 0.0f; }

    /** @brief Returns true if @p f is negative infinity. C++ counterpart of .NET Single.IsNegativeInfinity(float). */
    [[nodiscard]] static bool IsNegativeInfinity(float f) noexcept { return std::isinf(f) && f < 0.0f; }

    /** @brief Returns true if @p f is a finite number (not infinity or NaN). C++ counterpart of .NET Single.IsFinite(float). */
    [[nodiscard]] static bool IsFinite(float f) noexcept { return std::isfinite(f); }

    /** @brief Returns true if @p f is a normalized (not denormalized) finite number. C++ counterpart of .NET Single.IsNormal(float). */
    [[nodiscard]] static bool IsNormal(float f) noexcept { return std::isnormal(f); }

    /** @brief Returns true if @p f is a subnormal (denormalized) number. C++ counterpart of .NET Single.IsSubnormal(float). */
    [[nodiscard]] static bool IsSubnormal(float f) noexcept { return std::fpclassify(f) == FP_SUBNORMAL; }

    /** @brief Returns true if @p value is negative (including negative zero and negative infinity). C++ counterpart of .NET Single.IsNegative(float). */
    [[nodiscard]] static bool IsNegative(float value) noexcept { return std::signbit(value); }

    /** @brief Returns true if @p value is positive (not NaN, not negative). C++ counterpart of .NET Single.IsPositive(float). */
    [[nodiscard]] static bool IsPositive(float value) noexcept { return !std::signbit(value) && !std::isnan(value); }

    /** @brief Returns true if @p value is an integer value (no fractional part). C++ counterpart of .NET Single.IsInteger(float). */
    [[nodiscard]] static bool IsInteger(float value) noexcept {
        return std::isfinite(value) && value == std::trunc(value);
    }

    /** @brief Returns true if @p value is an even integer. C++ counterpart of .NET Single.IsEvenInteger(float). */
    [[nodiscard]] static bool IsEvenInteger(float value) noexcept {
        return IsInteger(value) && std::fmod(std::abs(value), 2.0f) == 0.0f;
    }

    /** @brief Returns true if @p value is an odd integer. C++ counterpart of .NET Single.IsOddInteger(float). */
    [[nodiscard]] static bool IsOddInteger(float value) noexcept {
        return IsInteger(value) && std::fmod(std::abs(value), 2.0f) == 1.0f;
    }

    /** @brief Returns true if @p value is not NaN (finite or infinite). C++ counterpart of .NET Single.IsRealNumber(float). */
    [[nodiscard]] static bool IsRealNumber(float value) noexcept { return !std::isnan(value); }

    /**
     * @brief Returns true if @p value is a power of two. C++ counterpart of .NET Single.IsPow2(float).
     * Returns false for zero, negative values, NaN, and infinity.
     */
    [[nodiscard]] static bool IsPow2(float value) noexcept {
        if (value <= 0.0f || !std::isfinite(value)) return false;
        uint32_t bits;
        static_assert(sizeof(float) == sizeof(uint32_t));
        __builtin_memcpy(&bits, &value, sizeof(bits));
        constexpr uint32_t TrailingMask = 0x007F'FFFFu;
        return (bits & TrailingMask) == 0;
    }

    // -------------------------------------------------------------------------
    // Arithmetic
    // -------------------------------------------------------------------------

    /** @brief Returns the absolute value of @p value. C++ counterpart of .NET Single.Abs(float). */
    [[nodiscard]] static float Abs(float value) noexcept { return std::abs(value); }

    /**
     * @brief Returns -1, 0, or 1 depending on the sign of @p value.
     * C++ counterpart of .NET Single.Sign(float).
     * @throws System::ArithmeticException if @p value is NaN, matching .NET (all
     *         relational comparisons against NaN are false, so a naive comparison
     *         chain would silently return 0 instead).
     */
    [[nodiscard]] static intcs Sign(float value) {
        if (value < 0.0f) return -1;
        if (value > 0.0f) return  1;
        if (value == 0.0f) return 0;
        throw ArithmeticException("Function does not accept floating point Not-a-Number values.");
    }

    /**
     * @brief Clamps @p value to [@p min, @p max]. C++ counterpart of .NET Single.Clamp(float,float,float).
     * @throws System::ArgumentException if @p min is greater than @p max.
     */
    [[nodiscard]] static float Clamp(float value, float min, float max) {
        if (min > max) throw System::ArgumentException("min cannot be greater than max.");
        return Min(Max(value, min), max);
    }

    /**
     * @brief Returns the larger of @p x and @p y.
     *
     * C++ counterpart of .NET Single.Max(float,float) (matches IEEE 754-2019
     * `maximum`). Unlike std::fmax (which returns the non-NaN argument when one
     * side is NaN), this propagates NaN if either argument is NaN, and treats
     * +0 as greater than -0, matching .NET's actual semantics exactly.
     */
    [[nodiscard]] static float Max(float x, float y) noexcept {
        if (x != y) {
            if (!std::isnan(x)) return y < x ? x : y;
            return x;
        }
        return std::signbit(y) ? x : y;
    }

    /**
     * @brief Returns the smaller of @p x and @p y.
     *
     * C++ counterpart of .NET Single.Min(float,float) (matches IEEE 754-2019
     * `minimum`). Unlike std::fmin, this propagates NaN if either argument is
     * NaN, and treats +0 as greater than -0, matching .NET's actual semantics.
     */
    [[nodiscard]] static float Min(float x, float y) noexcept {
        if (x != y) {
            if (!std::isnan(x)) return x < y ? x : y;
            return x;
        }
        return std::signbit(x) ? x : y;
    }

    /**
     * @brief Returns the value with the larger magnitude; if equal magnitude, returns the positive one.
     * C++ counterpart of .NET Single.MaxMagnitude(float,float).
     */
    [[nodiscard]] static float MaxMagnitude(float x, float y) noexcept {
        float ax = std::abs(x), ay = std::abs(y);
        if (ax > ay || std::isnan(ax)) return x;
        if (ax == ay) return std::signbit(x) ? y : x;
        return y;
    }

    /**
     * @brief Returns the value with the smaller magnitude; if equal magnitude, returns the negative one.
     * C++ counterpart of .NET Single.MinMagnitude(float,float).
     */
    [[nodiscard]] static float MinMagnitude(float x, float y) noexcept {
        float ax = std::abs(x), ay = std::abs(y);
        if (ax < ay || std::isnan(ax)) return x;
        if (ax == ay) return std::signbit(x) ? x : y;
        return y;
    }

    /**
     * @brief Returns a value with the magnitude of @p value and the sign of @p sign.
     * C++ counterpart of .NET Single.CopySign(float,float).
     */
    [[nodiscard]] static float CopySign(float value, float sign) noexcept {
        return std::copysign(value, sign);
    }

    // -------------------------------------------------------------------------
    // Rounding
    // -------------------------------------------------------------------------

    /** @brief Returns the smallest integral value greater than or equal to @p x. C++ counterpart of .NET Single.Ceiling(float). */
    [[nodiscard]] static float Ceiling(float x) noexcept { return std::ceil(x); }

    /** @brief Returns the largest integral value less than or equal to @p x. C++ counterpart of .NET Single.Floor(float). */
    [[nodiscard]] static float Floor(float x) noexcept { return std::floor(x); }

    /** @brief Returns the integral part (discards fractional part). C++ counterpart of .NET Single.Truncate(float). */
    [[nodiscard]] static float Truncate(float x) noexcept { return std::trunc(x); }

    /** @brief Rounds @p x to the nearest integer (ties to even). C++ counterpart of .NET Single.Round(float). */
    [[nodiscard]] static float Round(float x) noexcept { return std::nearbyint(x); }

    /** @brief Rounds @p x to @p digits decimal places (ties to even). C++ counterpart of .NET Single.Round(float,int). */
    [[nodiscard]] static float Round(float x, intcs digits) noexcept {
        float factor = std::pow(10.0f, static_cast<float>(digits));
        return std::nearbyint(x * factor) / factor;
    }

    // -------------------------------------------------------------------------
    // Exponential / logarithmic
    // -------------------------------------------------------------------------

    /** @brief Returns e raised to the power @p x. C++ counterpart of .NET Single.Exp(float). */
    [[nodiscard]] static float Exp(float x) noexcept { return std::exp(x); }

    /** @brief Returns 2 raised to the power @p x. C++ counterpart of .NET Single.Exp2(float). */
    [[nodiscard]] static float Exp2(float x) noexcept { return std::exp2(x); }

    /** @brief Returns 10 raised to the power @p x. C++ counterpart of .NET Single.Exp10(float). */
    [[nodiscard]] static float Exp10(float x) noexcept { return std::pow(10.0f, x); }

    /** @brief Returns the natural logarithm of @p x. C++ counterpart of .NET Single.Log(float). */
    [[nodiscard]] static float Log(float x) noexcept { return std::log(x); }

    /** @brief Returns the logarithm of @p x in base @p newBase. C++ counterpart of .NET Single.Log(float,float). */
    [[nodiscard]] static float Log(float x, float newBase) noexcept { return std::log(x) / std::log(newBase); }

    /** @brief Returns the base-2 logarithm of @p x. C++ counterpart of .NET Single.Log2(float). */
    [[nodiscard]] static float Log2(float x) noexcept { return std::log2(x); }

    /** @brief Returns the base-10 logarithm of @p x. C++ counterpart of .NET Single.Log10(float). */
    [[nodiscard]] static float Log10(float x) noexcept { return std::log10(x); }

    /** @brief Returns @p x raised to the power @p y. C++ counterpart of .NET Single.Pow(float,float). */
    [[nodiscard]] static float Pow(float x, float y) noexcept { return std::pow(x, y); }

    // -------------------------------------------------------------------------
    // Root / reciprocal
    // -------------------------------------------------------------------------

    /** @brief Returns the square root of @p x. C++ counterpart of .NET Single.Sqrt(float). */
    [[nodiscard]] static float Sqrt(float x) noexcept { return std::sqrt(x); }

    /** @brief Returns the cube root of @p x. C++ counterpart of .NET Single.Cbrt(float). */
    [[nodiscard]] static float Cbrt(float x) noexcept { return std::cbrt(x); }

private:
    [[nodiscard]] static float RootNPositive(float x, intcs n) noexcept {
        if (std::isfinite(x)) {
            if (x != 0.0f) {
                if (x > 0.0f || (n % 2 != 0)) {
                    float result = static_cast<float>(std::pow(static_cast<double>(std::abs(x)), 1.0 / n));
                    return std::copysign(result, x);
                }
                return NaN;
            }
            return (n % 2 == 0) ? 0.0f : std::copysign(0.0f, x);
        }
        if (std::isnan(x)) return NaN;
        if (x > 0.0f) return PositiveInfinity;
        return (n % 2 != 0) ? NegativeInfinity : NaN;
    }

    [[nodiscard]] static float RootNNegative(float x, intcs n) noexcept {
        if (std::isfinite(x)) {
            if (x != 0.0f) {
                if (x > 0.0f || (n % 2 != 0)) {
                    float result = static_cast<float>(std::pow(static_cast<double>(std::abs(x)), 1.0 / n));
                    return std::copysign(result, x);
                }
                return NaN;
            }
            return (n % 2 == 0) ? PositiveInfinity : std::copysign(PositiveInfinity, x);
        }
        if (std::isnan(x)) return NaN;
        if (x > 0.0f) return 0.0f;
        return (n % 2 != 0) ? NegativeZero : NaN;
    }

public:
    /**
     * @brief Returns the n-th root of @p x. C++ counterpart of .NET Single.RootN(float,int).
     */
    [[nodiscard]] static float RootN(float x, intcs n) noexcept {
        if (n > 0) {
            if (n == 2) return x != 0.0f ? Sqrt(x) : 0.0f;
            if (n == 3) return Cbrt(x);
            return RootNPositive(x, n);
        }
        if (n < 0) return RootNNegative(x, n);
        return NaN;
    }

    /** @brief Returns an estimate of 1/x. C++ counterpart of .NET Single.ReciprocalEstimate(float). */
    [[nodiscard]] static float ReciprocalEstimate(float x) noexcept { return 1.0f / x; }

    /** @brief Returns an estimate of 1/sqrt(x). C++ counterpart of .NET Single.ReciprocalSqrtEstimate(float). */
    [[nodiscard]] static float ReciprocalSqrtEstimate(float x) noexcept { return 1.0f / std::sqrt(x); }

    // -------------------------------------------------------------------------
    // Trigonometric
    // -------------------------------------------------------------------------

    /** @brief Returns the sine of @p x (in radians). C++ counterpart of .NET Single.Sin(float). */
    [[nodiscard]] static float Sin(float x) noexcept { return std::sin(x); }

    /** @brief Returns the cosine of @p x (in radians). C++ counterpart of .NET Single.Cos(float). */
    [[nodiscard]] static float Cos(float x) noexcept { return std::cos(x); }

    /** @brief Returns the tangent of @p x (in radians). C++ counterpart of .NET Single.Tan(float). */
    [[nodiscard]] static float Tan(float x) noexcept { return std::tan(x); }

    /** @brief Returns the arcsine of @p x. C++ counterpart of .NET Single.Asin(float). */
    [[nodiscard]] static float Asin(float x) noexcept { return std::asin(x); }

    /** @brief Returns the arccosine of @p x. C++ counterpart of .NET Single.Acos(float). */
    [[nodiscard]] static float Acos(float x) noexcept { return std::acos(x); }

    /** @brief Returns the arctangent of @p x. C++ counterpart of .NET Single.Atan(float). */
    [[nodiscard]] static float Atan(float x) noexcept { return std::atan(x); }

    /** @brief Returns the angle (in radians) whose tangent is @p y/@p x. C++ counterpart of .NET Single.Atan2(float,float). */
    [[nodiscard]] static float Atan2(float y, float x) noexcept { return std::atan2(y, x); }

    /** @brief Returns the sine of @p x * Pi. C++ counterpart of .NET Single.SinPi(float). */
    [[nodiscard]] static float SinPi(float x) noexcept { return std::sin(x * Pi); }

    /** @brief Returns the cosine of @p x * Pi. C++ counterpart of .NET Single.CosPi(float). */
    [[nodiscard]] static float CosPi(float x) noexcept { return std::cos(x * Pi); }

    /** @brief Returns the tangent of @p x * Pi. C++ counterpart of .NET Single.TanPi(float). */
    [[nodiscard]] static float TanPi(float x) noexcept { return std::tan(x * Pi); }

    /** @brief Returns the arccosine of @p x divided by Pi. C++ counterpart of .NET Single.AcosPi(float). */
    [[nodiscard]] static float AcosPi(float x) noexcept { return std::acos(x) / Pi; }

    /** @brief Returns the arcsine of @p x divided by Pi. C++ counterpart of .NET Single.AsinPi(float). */
    [[nodiscard]] static float AsinPi(float x) noexcept { return std::asin(x) / Pi; }

    /** @brief Returns the arctangent of @p x divided by Pi. C++ counterpart of .NET Single.AtanPi(float). */
    [[nodiscard]] static float AtanPi(float x) noexcept { return std::atan(x) / Pi; }

    /** @brief Returns the angle (in turns) whose tangent is @p y/@p x. C++ counterpart of .NET Single.Atan2Pi(float,float). */
    [[nodiscard]] static float Atan2Pi(float y, float x) noexcept { return std::atan2(y, x) / Pi; }

    /** @brief Pair returned by SinCos. */
    struct SinCosResult { float Sin; float Cos; };

    /**
     * @brief Computes sine and cosine of @p x simultaneously.
     * C++ counterpart of .NET Single.SinCos(float).
     */
    [[nodiscard]] static SinCosResult SinCos(float x) noexcept {
        return { std::sin(x), std::cos(x) };
    }

    /** @brief Pair returned by SinCosPi. */
    struct SinCosPiResult { float SinPi; float CosPi; };

    /**
     * @brief Computes sine and cosine of @p x * Pi simultaneously.
     * C++ counterpart of .NET Single.SinCosPi(float).
     */
    [[nodiscard]] static SinCosPiResult SinCosPi(float x) noexcept {
        return { std::sin(x * Pi), std::cos(x * Pi) };
    }

    // -------------------------------------------------------------------------
    // Hyperbolic
    // -------------------------------------------------------------------------

    /** @brief Returns the hyperbolic sine of @p x. C++ counterpart of .NET Single.Sinh(float). */
    [[nodiscard]] static float Sinh(float x) noexcept { return std::sinh(x); }

    /** @brief Returns the hyperbolic cosine of @p x. C++ counterpart of .NET Single.Cosh(float). */
    [[nodiscard]] static float Cosh(float x) noexcept { return std::cosh(x); }

    /** @brief Returns the hyperbolic tangent of @p x. C++ counterpart of .NET Single.Tanh(float). */
    [[nodiscard]] static float Tanh(float x) noexcept { return std::tanh(x); }

    /** @brief Returns the inverse hyperbolic sine of @p x. C++ counterpart of .NET Single.Asinh(float). */
    [[nodiscard]] static float Asinh(float x) noexcept { return std::asinh(x); }

    /** @brief Returns the inverse hyperbolic cosine of @p x. C++ counterpart of .NET Single.Acosh(float). */
    [[nodiscard]] static float Acosh(float x) noexcept { return std::acosh(x); }

    /** @brief Returns the inverse hyperbolic tangent of @p x. C++ counterpart of .NET Single.Atanh(float). */
    [[nodiscard]] static float Atanh(float x) noexcept { return std::atanh(x); }

    // -------------------------------------------------------------------------
    // IEEE 754 utilities
    // -------------------------------------------------------------------------

    /**
     * @brief Returns the hypotenuse of a right triangle with legs @p x and @p y.
     * C++ counterpart of .NET Single.Hypot(float,float).
     */
    [[nodiscard]] static float Hypot(float x, float y) noexcept { return std::hypot(x, y); }

    /**
     * @brief Returns (left * right) + addend computed with a single rounding.
     * C++ counterpart of .NET Single.FusedMultiplyAdd(float,float,float).
     */
    [[nodiscard]] static float FusedMultiplyAdd(float left, float right, float addend) noexcept {
        return std::fma(left, right, addend);
    }

    /**
     * @brief Returns the IEEE 754 remainder of @p left / @p right.
     * C++ counterpart of .NET Single.Ieee754Remainder(float,float).
     */
    [[nodiscard]] static float Ieee754Remainder(float left, float right) noexcept {
        return std::remainder(left, right);
    }

    /**
     * @brief Returns the next representable float value after @p x in the direction of negative infinity.
     * C++ counterpart of .NET Single.BitDecrement(float).
     */
    [[nodiscard]] static float BitDecrement(float x) noexcept { return std::nextafter(x, -std::numeric_limits<float>::infinity()); }

    /**
     * @brief Returns the next representable float value after @p x in the direction of positive infinity.
     * C++ counterpart of .NET Single.BitIncrement(float).
     */
    [[nodiscard]] static float BitIncrement(float x) noexcept { return std::nextafter(x, std::numeric_limits<float>::infinity()); }

    /**
     * @brief Returns the base-2 integer exponent of @p x.
     * C++ counterpart of .NET Single.ILogB(float).
     */
    [[nodiscard]] static intcs ILogB(float x) noexcept { return static_cast<intcs>(std::ilogb(x)); }

    /**
     * @brief Returns x * 2^n. C++ counterpart of .NET Single.ScaleB(float,int).
     */
    [[nodiscard]] static float ScaleB(float x, intcs n) noexcept { return std::ldexp(x, n); }

    // -------------------------------------------------------------------------
    // Angle conversion
    // -------------------------------------------------------------------------

    /** @brief Converts @p degrees to radians. C++ counterpart of .NET Single.DegreesToRadians(float). */
    [[nodiscard]] static float DegreesToRadians(float degrees) noexcept { return degrees * (Pi / 180.0f); }

    /** @brief Converts @p radians to degrees. C++ counterpart of .NET Single.RadiansToDegrees(float). */
    [[nodiscard]] static float RadiansToDegrees(float radians) noexcept { return radians * (180.0f / Pi); }

    // -------------------------------------------------------------------------
    // Comparison
    // -------------------------------------------------------------------------

    /** @brief Returns negative/zero/positive when @p a is less than/equal to/greater than @p b. C++ counterpart of .NET Single.CompareTo(float). */
    [[nodiscard]] static intcs CompareTo(float a, float b) noexcept {
        if (std::isnan(a) && std::isnan(b)) return 0;
        if (std::isnan(a)) return -1;
        if (std::isnan(b)) return  1;
        return static_cast<intcs>((a > b) - (a < b));
    }

    /**
     * @brief Returns true if @p a equals @p b. C++ counterpart of .NET Single.Equals(float).
     * Unlike @c operator==, two NaN values are considered equal to each other here.
     */
    [[nodiscard]] static bool Equals(float a, float b) noexcept {
        return a == b || (std::isnan(a) && std::isnan(b));
    }

    /**
     * @brief Returns a hash code for @p value. C++ counterpart of .NET Single.GetHashCode().
     * All NaN bit patterns and both signed zeros hash identically, so that values
     * considered Equals() (see above) always produce the same hash code.
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

private:
    static bool equalsIgnoreCaseAscii(const std::string& s, const char* token) noexcept {
        size_t n = std::char_traits<char>::length(token);
        if (s.size() != n) return false;
        for (size_t i = 0; i < n; ++i) {
            if (std::tolower(static_cast<unsigned char>(s[i])) != std::tolower(static_cast<unsigned char>(token[i])))
                return false;
        }
        return true;
    }

    // Verified against Number.Parsing.cs's TryParseFloat (same finding as System::Double,
    // ticket 245): real .NET recognizes exactly (case-insensitively) "Infinity"/"+Infinity"/
    // "-Infinity" and "NaN"/"+NaN"/"-NaN" as special tokens -- nothing else, including
    // abbreviations like "inf" or "nan(...)". Prior to this fix, this port only special-cased
    // the exact-cased strings "NaN"/"Infinity"/"-Infinity" and fell through to std::from_chars
    // for everything else -- but from_chars's floating-point grammar itself recognizes
    // "inf"/"infinity"/"nan"/"nan(n-char-seq)" case-insensitively regardless of chars_format (a
    // C++ standard requirement, not an implementation quirk), so inputs like "inf", "-inf",
    // "INF", or "nan(123)" were silently ACCEPTED here even though real .NET's float.Parse
    // throws FormatException for all of them.
    static bool tryParseCore(const std::string& s, float& result) noexcept {
        if (equalsIgnoreCaseAscii(s, "NaN") || equalsIgnoreCaseAscii(s, "+NaN") || equalsIgnoreCaseAscii(s, "-NaN")) {
            result = std::numeric_limits<float>::quiet_NaN();
            return true;
        }
        if (equalsIgnoreCaseAscii(s, "Infinity") || equalsIgnoreCaseAscii(s, "+Infinity")) {
            result = std::numeric_limits<float>::infinity();
            return true;
        }
        if (equalsIgnoreCaseAscii(s, "-Infinity")) {
            result = -std::numeric_limits<float>::infinity();
            return true;
        }
        auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), result);
        if (ec != std::errc{} || ptr != s.data() + s.size() || !std::isfinite(result)) {
            result = 0.0f;
            return false;
        }
        return true;
    }

public:
    // -------------------------------------------------------------------------
    // Parse / ToString
    // -------------------------------------------------------------------------

    /**
     * @brief Converts the string representation of a number to its float equivalent.
     * C++ counterpart of .NET Single.Parse(string).
     * @throws System::FormatException if the string is not a valid floating-point literal.
     */
    [[nodiscard]] static float Parse(const std::string& s) {
        float result{};
        if (!tryParseCore(s, result))
            throw System::FormatException("Input string was not in a correct format.");
        return result;
    }

    /**
     * @brief Tries to convert a string to a float without throwing.
     * C++ counterpart of .NET Single.TryParse(string, out float).
     */
    static bool TryParse(const std::string& s, float& result) noexcept {
        return tryParseCore(s, result);
    }

    /**
     * @brief Converts the float @p value to its string representation.
     * C++ counterpart of .NET Single.ToString().
     */
    [[nodiscard]] static std::string ToString(float value) {
        if (std::isnan(value)) return "NaN";
        if (std::isinf(value)) return value > 0 ? "Infinity" : "-Infinity";
        std::array<char, 64> buf;
        auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        if (ec == std::errc{}) return std::string(buf.data(), ptr);
        return std::to_string(value);
    }

    /** @brief Converts @p value to a string using a format specifier ("F2", "E3", "G", "R", "N2"). C++ counterpart of .NET Single.ToString(string). */
    [[nodiscard]] static std::string ToString(float value, const std::string& format) {
        if (format.empty()) return ToString(value);
        if (std::isnan(value)) return "NaN";
        if (std::isinf(value)) return value > 0 ? "Infinity" : "-Infinity";
        char type = format[0];
        int prec = format.size() > 1 ? std::stoi(format.substr(1)) : -1;
        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        if (type == 'F' || type == 'f') {
            oss << std::fixed << std::setprecision(prec >= 0 ? prec : 2) << value;
            return oss.str();
        }
        if (type == 'E' || type == 'e') {
            if (type == 'E') oss << std::uppercase;
            oss << std::scientific << std::setprecision(prec >= 0 ? prec : 6) << value;
            return oss.str();
        }
        if (type == 'G' || type == 'g') {
            if (prec > 0) oss << std::setprecision(prec);
            oss << value;
            return oss.str();
        }
        if (type == 'R' || type == 'r') return ToString(value);
        if (type == 'N' || type == 'n') {
            oss << std::fixed << std::setprecision(prec >= 0 ? prec : 2) << value;
            return oss.str();
        }
        return ToString(value);
    }
};

} // namespace System
