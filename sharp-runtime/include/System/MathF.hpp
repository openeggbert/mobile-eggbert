// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cmath>
#include <limits>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ArithmeticException.hpp"
#include "System/MidpointRounding.hpp"

namespace System {

    using SharpRuntime::intcs;

    /**
     * @brief Provides constants and static methods for trigonometric, logarithmic,
     * and other common mathematical functions for single-precision floating-point numbers.
     *
     * C++ counterpart of .NET System.MathF.
     * All members are static; instantiation is not allowed.
     */
    class MathF {
    public:
        /** @brief Deleted constructor — all members are static. */
        MathF() = delete;

        /** @brief Euler's number e. */
        static constexpr float E   = 2.71828182845904523536f;
        /** @brief The ratio of a circle's circumference to its diameter. */
        static constexpr float PI  = 3.14159265358979323846f;
        /** @brief Tau (2*PI), the ratio of circumference to radius. */
        static constexpr float Tau = 6.28318530717958647692f;

        /** @brief Returns the absolute value of @p x. */
        static float Abs(float x)                  { return std::abs(x); }
        /** @brief Returns the smallest integral value greater than or equal to @p x. */
        static float Ceiling(float x)              { return std::ceil(x); }
        /** @brief Returns the largest integral value less than or equal to @p x. */
        static float Floor(float x)                { return std::floor(x); }
        /**
         * @brief Returns @p x rounded to the nearest integer.
         *
         * C++ counterpart of .NET MathF.Round(float). Uses round-to-even (banker's
         * rounding) for midpoint values, matching .NET's default - NOT std::round,
         * which always rounds halfway cases away from zero.
         */
        static float Round(float x)                { return std::nearbyintf(x); }
        /** @brief Returns the integral part of @p x, discarding the fractional digits. */
        static float Truncate(float x)             { return std::trunc(x); }
        /** @brief Returns the square root of @p x. */
        static float Sqrt(float x)                 { return std::sqrt(x); }
        /** @brief Returns the cube root of @p x. */
        static float Cbrt(float x)                 { return std::cbrt(x); }
        /** @brief Returns e raised to the power @p x. */
        static float Exp(float x)                  { return std::exp(x); }
        /** @brief Returns the natural (base-e) logarithm of @p x. */
        static float Log(float x)                  { return std::log(x); }

        /**
         * @brief Returns the logarithm of @p x in the specified base @p y.
         *
         * C++ counterpart of .NET MathF.Log(float, float). Matches .NET's explicit
         * IEEE 754 special cases rather than a naive log(x)/log(y), which would
         * disagree at these edges (e.g. y == 1 naively gives +-infinity or NaN
         * depending on x, but .NET always returns NaN).
         * @param x Value whose logarithm is to be found.
         * @param y Logarithm base.
         * @return log_y(x).
         */
        static float Log(float x, float y) {
            if (std::isnan(x)) return x;
            if (std::isnan(y)) return y;
            if (y == 1.0f) return std::numeric_limits<float>::quiet_NaN();
            if (x != 1.0f && (y == 0.0f || y == std::numeric_limits<float>::infinity()))
                return std::numeric_limits<float>::quiet_NaN();
            return std::log(x) / std::log(y);
        }

        /** @brief Returns the base-2 logarithm of @p x. */
        static float Log2(float x)                 { return std::log2(x); }
        /** @brief Returns the base-10 logarithm of @p x. */
        static float Log10(float x)                { return std::log10(x); }

        /**
         * @brief Returns @p x raised to the power @p y.
         * @param x Base.
         * @param y Exponent.
         */
        static float Pow(float x, float y)         { return std::pow(x, y); }

        /** @brief Returns the sine of angle @p x (in radians). */
        static float Sin(float x)                  { return std::sin(x); }
        /** @brief Returns the cosine of angle @p x (in radians). */
        static float Cos(float x)                  { return std::cos(x); }
        /** @brief Returns the tangent of angle @p x (in radians). */
        static float Tan(float x)                  { return std::tan(x); }
        /** @brief Returns the arc sine of @p x in radians. */
        static float Asin(float x)                 { return std::asin(x); }
        /** @brief Returns the arc cosine of @p x in radians. */
        static float Acos(float x)                 { return std::acos(x); }
        /** @brief Returns the arc tangent of @p x in radians. */
        static float Atan(float x)                 { return std::atan(x); }

        /**
         * @brief Returns the angle whose tangent is @p y / @p x.
         * @param y Numerator (y-coordinate).
         * @param x Denominator (x-coordinate).
         * @return Angle in radians in [-PI, PI].
         */
        static float Atan2(float y, float x)       { return std::atan2(y, x); }

        /** @brief Returns the hyperbolic sine of @p x. */
        static float Sinh(float x)                 { return std::sinh(x); }
        /** @brief Returns the hyperbolic cosine of @p x. */
        static float Cosh(float x)                 { return std::cosh(x); }
        /** @brief Returns the hyperbolic tangent of @p x. */
        static float Tanh(float x)                 { return std::tanh(x); }
        /**
         * @brief Returns the larger of @p x and @p y.
         *
         * C++ counterpart of .NET MathF.Max(float, float) (matches IEEE 754-2019
         * `maximum`). Unlike std::fmax (which returns the non-NaN argument when one
         * side is NaN), this propagates NaN if either argument is NaN, and treats
         * +0 as greater than -0, matching .NET's actual semantics exactly.
         */
        static float Max(float x, float y) {
            if (x != y) {
                if (!std::isnan(x)) return y < x ? x : y;
                return x;
            }
            return std::signbit(y) ? x : y;
        }

        /**
         * @brief Returns the smaller of @p x and @p y.
         *
         * C++ counterpart of .NET MathF.Min(float, float) (matches IEEE 754-2019
         * `minimum`). Unlike std::fmin, this propagates NaN if either argument is
         * NaN, and treats +0 as greater than -0, matching .NET's actual semantics.
         */
        static float Min(float x, float y) {
            if (x != y) {
                if (!std::isnan(x)) return x < y ? x : y;
                return x;
            }
            return std::signbit(x) ? x : y;
        }

        /**
         * @brief Clamps @p v to the range [@p min, @p max].
         * @param v   Value to clamp.
         * @param min Inclusive lower bound.
         * @param max Inclusive upper bound.
         */
        static float Clamp(float v, float min, float max) {
            return v < min ? min : (v > max ? max : v);
        }

        /**
         * @brief Returns -1, 0, or 1 indicating the sign of @p x.
         *
         * C++ counterpart of .NET MathF.Sign(float) (via Math.Sign(float)).
         * @throws System::ArithmeticException if @p x is NaN, matching .NET (a naive
         *         comparison-based implementation would silently return 0 for NaN
         *         instead, since all relational comparisons against NaN are false).
         */
        static intcs Sign(float x) {
            if (x < 0.0f) return -1;
            if (x > 0.0f) return 1;
            if (x == 0.0f) return 0;
            throw ArithmeticException("Function does not accept floating point Not-a-Number values.");
        }
        /** @brief Returns true if @p x is Not-a-Number (NaN). */
        static bool IsNaN(float x)                 { return std::isnan(x); }
        /** @brief Returns true if @p x is positive or negative infinity. */
        static bool IsInfinity(float x)            { return std::isinf(x); }
        /** @brief Returns true if @p x is positive infinity. */
        static bool IsPositiveInfinity(float x)    { return x == std::numeric_limits<float>::infinity(); }
        /** @brief Returns true if @p x is negative infinity. */
        static bool IsNegativeInfinity(float x)    { return x == -std::numeric_limits<float>::infinity(); }

        /**
         * @brief Returns the IEEE 754 remainder of @p x / @p y.
         * @param x Dividend.
         * @param y Divisor.
         */
        static float IEEERemainder(float x, float y) { return std::remainder(x, y); }

        /** @brief Returns the inverse hyperbolic cosine of @p x. */
        static float Acosh(float x)                { return std::acosh(x); }
        /** @brief Returns the inverse hyperbolic sine of @p x. */
        static float Asinh(float x)                { return std::asinh(x); }
        /** @brief Returns the inverse hyperbolic tangent of @p x. */
        static float Atanh(float x)                { return std::atanh(x); }
        /** @brief Returns a value with the magnitude of @p x and the sign of @p y. */
        static float CopySign(float x, float y)    { return std::copysign(x, y); }
        /** @brief Returns the smallest float greater than @p x. */
        static float BitIncrement(float x)         { return std::nextafter(x,  std::numeric_limits<float>::infinity()); }
        /** @brief Returns the largest float less than @p x. */
        static float BitDecrement(float x)         { return std::nextafter(x, -std::numeric_limits<float>::infinity()); }
        /** @brief Returns (x * y) + z, computed with a single rounding step. */
        static float FusedMultiplyAdd(float x, float y, float z) { return std::fma(x, y, z); }

        /**
         * @brief Rounds @p x to @p digits decimal places.
         *
         * C++ counterpart of .NET MathF.Round(float, int). Uses round-to-even
         * (banker's rounding) for midpoint values, matching .NET's default
         * (`Round(x, digits, MidpointRounding.ToEven)`) - NOT away-from-zero.
         * @param x      Value to round.
         * @param digits Number of decimal places.
         * @throws System::ArgumentOutOfRangeException if @p digits is outside [0, 6].
         */
        static float Round(float x, intcs digits) {
            return Round(x, digits, MidpointRounding::ToEven);
        }

        /**
         * @brief Rounds @p x to an integer using the specified rounding convention.
         * @param x    Value to round.
         * @param mode Rounding convention.
         */
        static float Round(float x, MidpointRounding mode) {
            switch (mode) {
                case MidpointRounding::ToEven:             return std::nearbyintf(x);
                case MidpointRounding::AwayFromZero:       return std::roundf(x);
                case MidpointRounding::ToZero:             return std::truncf(x);
                case MidpointRounding::ToNegativeInfinity: return std::floorf(x);
                case MidpointRounding::ToPositiveInfinity: return std::ceilf(x);
                default:                                   return std::nearbyintf(x);
            }
        }

        /**
         * @brief Rounds @p x to @p digits decimal places using the specified rounding convention.
         *
         * C++ counterpart of .NET MathF.Round(float, int, MidpointRounding). Matches
         * real .NET's exact guards: @p digits must be in [0, 6] (MathF.cs's
         * maxRoundingDigits, distinct from Math.Round(double)'s 0-15 range, since
         * float's ~7 significant decimal digits can't usefully support more), and for
         * |x| >= 1e8 (singleRoundLimit) the value is returned completely unchanged
         * rather than attempting the multiply/round/divide dance -- which would
         * otherwise risk overflowing to infinity for x near FLT_MAX (confirmed via a
         * standalone repro before this fix: Round(3.0e38f, 6) returned `inf`).
         * @param x      Value to round.
         * @param digits Number of decimal places.
         * @param mode   Rounding convention.
         * @throws System::ArgumentOutOfRangeException if @p digits is outside [0, 6].
         */
        static float Round(float x, intcs digits, MidpointRounding mode) {
            if (digits < 0 || digits > 6)
                throw System::ArgumentOutOfRangeException("digits", "Rounding digits must be between 0 and 6, inclusive.");
            static constexpr float kPower10[] = { 1e0f, 1e1f, 1e2f, 1e3f, 1e4f, 1e5f, 1e6f };
            constexpr float kSingleRoundLimit = 1e8f;
            if (std::fabs(x) < kSingleRoundLimit) {
                float power10 = kPower10[digits];
                x = Round(x * power10, mode) / power10;
            }
            return x;
        }

        /** @brief Returns true if @p x is finite (not NaN or infinity). */
        static bool IsFinite(float x)              { return std::isfinite(x); }
        /** @brief Returns true if @p x is a normal floating-point value (not zero, subnormal, NaN, or infinite). */
        static bool IsNormal(float x)              { return std::isnormal(x); }
        /** @brief Returns true if @p x is subnormal (denormalized). */
        static bool IsSubnormal(float x)           { return std::fpclassify(x) == FP_SUBNORMAL; }
        /** @brief Returns true if @p x is negative (including -0 and -infinity). */
        static bool IsNegative(float x)            { return std::signbit(x); }
        /** @brief Returns @p x multiplied by 2 raised to the power @p n. */
        static float ScaleB(float x, intcs n)      { return std::scalbn(x, n); }
        /**
         * @brief Returns the base-2 integer exponent of @p x (i.e. floor(log2(|x|))).
         *
         * C++ counterpart of .NET MathF.ILogB(float). Real .NET always returns
         * int.MaxValue for both infinity AND NaN, and int.MinValue for zero. std::ilogb's
         * NaN sentinel (FP_ILOGBNAN) is implementation-defined and on glibc is actually
         * INT_MIN, not INT_MAX -- confirmed via a standalone repro before this fix
         * (MathF::ILogB(NaN) returned INT_MIN here, diverging from .NET). Handled
         * explicitly so the result is correct and portable rather than relying on a
         * library-specific sentinel value.
         */
        static intcs ILogB(float x) {
            if (std::isnan(x) || std::isinf(x)) return std::numeric_limits<intcs>::max();
            if (x == 0.0f) return std::numeric_limits<intcs>::min();
            return static_cast<intcs>(std::ilogb(x));
        }
        /** @brief Returns an estimate of the reciprocal of @p x (1/x). */
        static float ReciprocalEstimate(float x)   { return 1.0f / x; }
        /** @brief Returns an estimate of the reciprocal square root of @p x (1/sqrt(x)). */
        static float ReciprocalSqrtEstimate(float x) { return 1.0f / std::sqrt(x); }

        /**
         * @brief Returns the value with the greater magnitude; if equal, returns the positive one.
         * @param x First value.
         * @param y Second value.
         */
        static float MaxMagnitude(float x, float y) {
            float ax = std::fabs(x), ay = std::fabs(y);
            if (ax > ay || std::isnan(ax)) return x;
            if (ax == ay) return std::signbit(x) ? y : x;
            return y;
        }

        /**
         * @brief Returns the value with the lesser magnitude; if equal, returns the negative one.
         * @param x First value.
         * @param y Second value.
         */
        static float MinMagnitude(float x, float y) {
            float ax = std::fabs(x), ay = std::fabs(y);
            if (ax < ay || std::isnan(ax)) return x;
            if (ax == ay) return std::signbit(x) ? x : y;
            return y;
        }

        /** Holds the result of SinCos — the sine and cosine of an angle computed simultaneously. */
        struct SinCosResult { float Sin; float Cos; };

        /**
         * @brief Computes the sine and cosine of @p x simultaneously.
         * @param x Angle in radians.
         * @return A SinCosResult with Sin and Cos fields.
         */
        static SinCosResult SinCos(float x) {
            return { std::sin(x), std::cos(x) };
        }
    };

} // namespace System
