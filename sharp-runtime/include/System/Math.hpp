// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ArithmeticException.hpp"
#include "System/DivideByZeroException.hpp"
#include "System/MidpointRounding.hpp"
#include "System/OverflowException.hpp"

namespace System
{
    using SharpRuntime::intcs;
    using SharpRuntime::longcs;
    using SharpRuntime::shortcs;
    using SharpRuntime::sbytecs;
    using SharpRuntime::bytecs;
    using SharpRuntime::uintcs;
    using SharpRuntime::ulongcs;
    using SharpRuntime::ushortcs;

    /**
     * @brief Provides constants and static methods for trigonometric,
     * logarithmic, and other common mathematical functions.
     *
     * C++ counterpart of .NET System.Math.
     */
    class Math
    {
    public:
        Math() = delete;
        ~Math() = delete;

        /** @brief The base of natural logarithms, e ≈ 2.718. */
        static constexpr double E   = 2.71828'18284'59045'2354;
        /** @brief The ratio of a circle's circumference to its diameter, π ≈ 3.14159. */
        static constexpr double PI  = 3.14159'26535'89793'23846;
        /** @brief Two times PI (τ = 2π), a full circle in radians. */
        static constexpr double Tau = 6.28318'53071'79586'47692;

        /**
         * @brief Returns the sine of the specified angle.
         *
         * The angle is specified in radians.
         *
         * @param value Angle in radians.
         * @return Sine of the angle.
         *
         */
        [[nodiscard]] static double Sin(double value);

        /**
         * @brief Returns the cosine of the specified angle.
         *
         * The angle is specified in radians.
         *
         * @param value Angle in radians.
         * @return Cosine of the angle.
         *
         */
        [[nodiscard]] static double Cos(double value);

        /**
         * @brief Returns the tangent of the specified angle.
         *
         * The angle is specified in radians.
         *
         * @param value Angle in radians.
         * @return Tangent of the angle.
         *
         */
        [[nodiscard]] static double Tan(double value);

        /**
         * @brief Returns the square root of a specified number.
         *
         * @param value Input value.
         * @return Square root of the input value.
         *
         */
        [[nodiscard]] static double Sqrt(double value);

        /**
         * @brief Returns the absolute value of a double-precision number.
         *
         * @param value Input value.
         * @return Absolute value.
         *
         */
        [[nodiscard]] static double Abs(double value);

        /**
         * @brief Returns the absolute value of a 32-bit signed integer.
         *
         * @param value Input value.
         * @return Absolute value.
         * @throws System::OverflowException if @p value is MinValue (its magnitude does not fit in a 32-bit signed integer).
         */
        [[nodiscard]] static intcs Abs(intcs value);

        /**
         * @brief Returns the smaller of two 32-bit signed integers.
         *
         * @param a First value.
         * @param b Second value.
         * @return Smaller value.
         *
         */
        [[nodiscard]] static intcs Min(intcs a, intcs b);

        /**
         * @brief Returns the smaller of two double-precision numbers.
         *
         * @param a First value.
         * @param b Second value.
         * @return Smaller value.
         *
         */
        [[nodiscard]] static double Min(double a, double b);

        /**
         * @brief Returns the larger of two 32-bit signed integers.
         *
         * @param a First value.
         * @param b Second value.
         * @return Larger value.
         *
         */
        [[nodiscard]] static intcs Max(intcs a, intcs b);

        /**
         * @brief Returns the larger of two double-precision numbers.
         *
         * @param a First value.
         * @param b Second value.
         * @return Larger value.
         *
         */
        [[nodiscard]] static double Max(double a, double b);

        /**
         * @brief Restricts an integer value to the specified range.
         *
         * @param value Value to clamp.
         * @param min Minimum allowed value.
         * @param max Maximum allowed value.
         * @return Clamped value.
         * @throws System::ArgumentException if @p min is greater than @p max.
         */
        [[nodiscard]] static intcs Clamp(intcs value, intcs min, intcs max);

        /**
         * @brief Restricts a double value to the specified range.
         *
         * @param value Value to clamp.
         * @param min Minimum allowed value.
         * @param max Maximum allowed value.
         * @return Clamped value.
         * @throws System::ArgumentException if @p min is greater than @p max.
         */
        [[nodiscard]] static double Clamp(double value, double min, double max);

        /**
         * @brief Returns the absolute value of a 64-bit signed integer.
         * @throws System::OverflowException if @p value is MinValue (its magnitude does not fit in a 64-bit signed integer).
         */
        [[nodiscard]] static longcs Abs(longcs value);
        /** Returns the smaller of two 64-bit signed integers. */
        [[nodiscard]] static longcs Min(longcs a, longcs b);
        /** Returns the larger of two 64-bit signed integers. */
        [[nodiscard]] static longcs Max(longcs a, longcs b);
        /**
         * @brief Clamps a 64-bit signed integer to [@p min, @p max].
         * @throws System::ArgumentException if @p min is greater than @p max.
         */
        [[nodiscard]] static longcs Clamp(longcs value, longcs min, longcs max);

        /** Returns the absolute value of a single-precision float. */
        [[nodiscard]] static float Abs(float value)              { return std::abs(value); }
        /**
         * @brief Returns the smaller of two single-precision floats.
         *
         * Matches the IEEE 754:2019 `minimum` function: propagates NaN regardless of which
         * argument it's in, and treats +0 as greater than -0 -- unlike a naive `a < b ? a : b`
         * (which found the identical bug fixed in this session's double overload: e.g.
         * `5.0f < NaN ? ... : ...` is always false, so NaN only propagates when it's the first
         * argument, not both).
         */
        [[nodiscard]] static float Min(float val1, float val2) {
            if (val1 != val2) {
                if (!std::isnan(val1)) return val1 < val2 ? val1 : val2;
                return val1;
            }
            return std::signbit(val1) ? val1 : val2;
        }
        /** @brief Returns the larger of two single-precision floats (see Min()'s doc-comment). */
        [[nodiscard]] static float Max(float val1, float val2) {
            if (val1 != val2) {
                if (!std::isnan(val1)) return val2 < val1 ? val1 : val2;
                return val1;
            }
            return std::signbit(val2) ? val1 : val2;
        }
        /**
         * @brief Clamps a single-precision float to [@p min, @p max].
         * @throws System::ArgumentException if @p min is greater than @p max.
         */
        [[nodiscard]] static float Clamp(float value, float min, float max) {
            if (min > max) throw System::ArgumentException("min cannot be greater than max.");
            return value < min ? min : (value > max ? max : value);
        }

        /**
         * @brief Rounds a double-precision floating-point value to the nearest integer.
         *
         * @param value Input value.
         * @return Rounded value.
         *
         */
        [[nodiscard]] static double Round(double value);

        /**
         * @brief Returns the largest integer less than or equal to the specified number.
         *
         * @param value Input value.
         * @return Floor of the input value.
         *
         */
        [[nodiscard]] static double Floor(double value);

        /**
         * @brief Returns the smallest integer greater than or equal to the specified number.
         *
         * @param value Input value.
         * @return Ceiling of the input value.
         *
         */
        [[nodiscard]] static double Ceiling(double value);

        /**
         * @brief Returns a specified number raised to the specified power.
         *
         * @param x Base value.
         * @param y Exponent value.
         * @return x raised to the power y.
         *
         */
        [[nodiscard]] static double Pow(double x, double y);

        /** @brief Returns the natural (base-e) logarithm of @p d. */
        [[nodiscard]] static double Log(double d);
        /** @brief Returns the logarithm of @p a in the specified @p newBase. */
        [[nodiscard]] static double Log(double a, double newBase);
        /** @brief Returns the base-2 logarithm of @p x. */
        [[nodiscard]] static double Log2(double x);
        /** @brief Returns the base-10 logarithm of @p d. */
        [[nodiscard]] static double Log10(double d);
        /** @brief Returns e raised to the power @p d. */
        [[nodiscard]] static double Exp(double d);

        /** @brief Returns the angle whose sine is @p d (in radians). */
        [[nodiscard]] static double Asin(double d);
        /** @brief Returns the angle whose cosine is @p d (in radians). */
        [[nodiscard]] static double Acos(double d);
        /** @brief Returns the angle whose tangent is @p d (in radians). */
        [[nodiscard]] static double Atan(double d);
        /** @brief Returns the angle (in radians) whose tangent is the quotient of @p y and @p x. */
        [[nodiscard]] static double Atan2(double y, double x);

        /** @brief Returns the hyperbolic sine of @p value. */
        [[nodiscard]] static double Sinh(double value);
        /** @brief Returns the hyperbolic cosine of @p value. */
        [[nodiscard]] static double Cosh(double value);
        /** @brief Returns the hyperbolic tangent of @p value. */
        [[nodiscard]] static double Tanh(double value);

        /** @brief Returns an integer indicating the sign of a 32-bit integer (-1, 0, or 1). */
        [[nodiscard]] static intcs Sign(intcs value);
        /**
         * @brief Returns an integer indicating the sign of a double (-1, 0, or 1).
         * @throws ArithmeticException if @p value is NaN.
         */
        [[nodiscard]] static intcs Sign(double value);
        /** @brief Returns an integer indicating the sign of a 64-bit integer (-1, 0, or 1). */
        [[nodiscard]] static intcs Sign(longcs value);
        /**
         * @brief Returns an integer indicating the sign of a single-precision float (-1, 0, or 1).
         * @throws ArithmeticException if @p value is NaN.
         */
        [[nodiscard]] static intcs Sign(float value);

        /** @brief Returns the integral part of @p d (discards the fractional part). */
        [[nodiscard]] static double Truncate(double d);

        /** @brief Returns the IEEE 754 remainder of @p x divided by @p y. */
        [[nodiscard]] static double IEEERemainder(double x, double y);

        /**
         * @brief Divides @p a by @p b and stores the remainder in @p result; returns the quotient.
         * @throws System::DivideByZeroException if @p b is zero. Unlike real .NET (where the CLR's
         * `div`/`rem` instructions raise this automatically), plain C++ integer division by zero is
         * undefined behavior, so this must be checked explicitly to get an equivalent, catchable
         * failure instead of a crash.
         */
        static intcs DivRem(intcs a, intcs b, intcs& result);
        /**
         * @brief Divides @p a by @p b (64-bit) and stores the remainder in @p result; returns the quotient.
         * @throws System::DivideByZeroException if @p b is zero.
         */
        static longcs DivRem(longcs a, longcs b, longcs& result);

        /** @brief Returns the value with the greater magnitude; if equal magnitudes, returns the positive one. */
        [[nodiscard]] static double MaxMagnitude(double x, double y);
        /** @brief Returns the value with the lesser magnitude; if equal magnitudes, returns the negative one. */
        [[nodiscard]] static double MinMagnitude(double x, double y);

        /** @brief Returns the 64-bit product of two 32-bit integers. */
        [[nodiscard]] static longcs BigMul(intcs a, intcs b);

        /** @brief Returns @p x multiplied by 2 raised to the power @p n (scalbn). */
        [[nodiscard]] static double ScaleB(double x, intcs n);

        /** @brief Returns the cube root of @p x. */
        [[nodiscard]] static double Cbrt(double x);

        /** @brief Returns the angle whose hyperbolic cosine is @p d. */
        [[nodiscard]] static double Acosh(double d);

        /** @brief Returns the angle whose hyperbolic sine is @p d. */
        [[nodiscard]] static double Asinh(double d);

        /** @brief Returns the angle whose hyperbolic tangent is @p d. */
        [[nodiscard]] static double Atanh(double d);

        /**
         * @brief Rounds @p value to @p digits decimal places using banker's rounding (MidpointRounding::ToEven).
         * @throws System::ArgumentOutOfRangeException if @p digits is outside [0, 15].
         */
        [[nodiscard]] static double Round(double value, intcs digits);

        /** @brief Returns a value with the magnitude of @p x and the sign of @p y. */
        [[nodiscard]] static double CopySign(double x, double y);

        /** @brief Returns the smallest value greater than @p x. */
        [[nodiscard]] static double BitIncrement(double x);

        /** @brief Returns the largest value less than @p x. */
        [[nodiscard]] static double BitDecrement(double x);

        /** @brief Returns @p x × @p y + @p z computed as a single fused operation. */
        [[nodiscard]] static double FusedMultiplyAdd(double x, double y, double z);

        // ------------------------------------------------------------------
        // short / sbyte / byte / uint / ulong / ushort overloads
        // ------------------------------------------------------------------

        /**
         * @brief Returns the absolute value of a 16-bit signed integer.
         * @throws System::OverflowException if @p value is MinValue (its magnitude does not fit in a 16-bit signed integer).
         */
        [[nodiscard]] static shortcs Abs(shortcs value) {
            if (value == std::numeric_limits<shortcs>::min())
                throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
            return value < 0 ? static_cast<shortcs>(-value) : value;
        }
        /**
         * @brief Returns the absolute value of a signed byte.
         * @throws System::OverflowException if @p value is MinValue (its magnitude does not fit in a signed byte).
         */
        [[nodiscard]] static sbytecs Abs(sbytecs value) {
            if (value == std::numeric_limits<sbytecs>::min())
                throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
            return value < 0 ? static_cast<sbytecs>(-value) : value;
        }

        /** @brief Returns the smaller of two 16-bit signed integers. */
        [[nodiscard]] static shortcs  Min(shortcs a,  shortcs b)  { return a < b ? a : b; }
        /** @brief Returns the larger of two 16-bit signed integers. */
        [[nodiscard]] static shortcs  Max(shortcs a,  shortcs b)  { return a > b ? a : b; }
        /**
         * @brief Clamps a 16-bit signed integer to [@p min, @p max].
         * @throws System::ArgumentException if @p mn is greater than @p mx.
         */
        [[nodiscard]] static shortcs  Clamp(shortcs v, shortcs mn, shortcs mx)  {
            if (mn > mx) throw System::ArgumentException("min cannot be greater than max.");
            return v < mn ? mn : v > mx ? mx : v;
        }

        /** @brief Returns the smaller of two signed bytes. */
        [[nodiscard]] static sbytecs Min(sbytecs a, sbytecs b)  { return a < b ? a : b; }
        /** @brief Returns the larger of two signed bytes. */
        [[nodiscard]] static sbytecs Max(sbytecs a, sbytecs b)  { return a > b ? a : b; }
        /**
         * @brief Clamps a signed byte to [@p min, @p max].
         * @throws System::ArgumentException if @p mn is greater than @p mx.
         */
        [[nodiscard]] static sbytecs Clamp(sbytecs v, sbytecs mn, sbytecs mx) {
            if (mn > mx) throw System::ArgumentException("min cannot be greater than max.");
            return v < mn ? mn : v > mx ? mx : v;
        }

        /** @brief Returns the smaller of two unsigned bytes. */
        [[nodiscard]] static bytecs  Min(bytecs a,  bytecs b)   { return a < b ? a : b; }
        /** @brief Returns the larger of two unsigned bytes. */
        [[nodiscard]] static bytecs  Max(bytecs a,  bytecs b)   { return a > b ? a : b; }
        /**
         * @brief Clamps an unsigned byte to [@p min, @p max].
         * @throws System::ArgumentException if @p mn is greater than @p mx.
         */
        [[nodiscard]] static bytecs  Clamp(bytecs v, bytecs mn, bytecs mx)  {
            if (mn > mx) throw System::ArgumentException("min cannot be greater than max.");
            return v < mn ? mn : v > mx ? mx : v;
        }

        /** @brief Returns the smaller of two 32-bit unsigned integers. */
        [[nodiscard]] static uintcs  Min(uintcs a,  uintcs b)   { return a < b ? a : b; }
        /** @brief Returns the larger of two 32-bit unsigned integers. */
        [[nodiscard]] static uintcs  Max(uintcs a,  uintcs b)   { return a > b ? a : b; }
        /**
         * @brief Clamps a 32-bit unsigned integer to [@p min, @p max].
         * @throws System::ArgumentException if @p mn is greater than @p mx.
         */
        [[nodiscard]] static uintcs  Clamp(uintcs v, uintcs mn, uintcs mx)  {
            if (mn > mx) throw System::ArgumentException("min cannot be greater than max.");
            return v < mn ? mn : v > mx ? mx : v;
        }

        /** @brief Returns the smaller of two 64-bit unsigned integers. */
        [[nodiscard]] static ulongcs Min(ulongcs a, ulongcs b)  { return a < b ? a : b; }
        /** @brief Returns the larger of two 64-bit unsigned integers. */
        [[nodiscard]] static ulongcs Max(ulongcs a, ulongcs b)  { return a > b ? a : b; }
        /**
         * @brief Clamps a 64-bit unsigned integer to [@p min, @p max].
         * @throws System::ArgumentException if @p mn is greater than @p mx.
         */
        [[nodiscard]] static ulongcs Clamp(ulongcs v, ulongcs mn, ulongcs mx) {
            if (mn > mx) throw System::ArgumentException("min cannot be greater than max.");
            return v < mn ? mn : v > mx ? mx : v;
        }

        /** @brief Returns the smaller of two 16-bit unsigned integers. */
        [[nodiscard]] static ushortcs Min(ushortcs a, ushortcs b)  { return a < b ? a : b; }
        /** @brief Returns the larger of two 16-bit unsigned integers. */
        [[nodiscard]] static ushortcs Max(ushortcs a, ushortcs b)  { return a > b ? a : b; }
        /**
         * @brief Clamps a 16-bit unsigned integer to [@p min, @p max].
         * @throws System::ArgumentException if @p mn is greater than @p mx.
         */
        [[nodiscard]] static ushortcs Clamp(ushortcs v, ushortcs mn, ushortcs mx) {
            if (mn > mx) throw System::ArgumentException("min cannot be greater than max.");
            return v < mn ? mn : v > mx ? mx : v;
        }

        // ------------------------------------------------------------------
        // ILogB / BigMul(long,long,long&) / DivRem pair overloads
        // ------------------------------------------------------------------

        /** @brief Returns the base-2 integer logarithm of @p x (std::ilogb). */
        [[nodiscard]] static intcs ILogB(double x) { return static_cast<intcs>(std::ilogb(x)); }

#if !defined(_MSC_VER)
        /**
         * @brief Multiplies two 64-bit signed integers and returns the full 128-bit product.
         *
         * The high 64 bits of the product are stored in @p high.
         * C++ counterpart of .NET Math.BigMul(long, long, out long).
         *
         * MSVC-unsupported: relies on the GCC/Clang `__int128` extension (see CLAUDE.md's
         * documented, permanent __int128-dependency policy -- the same reasoning as
         * System::Int128/UInt128). Guarded out here rather than left to fail with a raw
         * "__int128 is not a recognized built-in type" compiler error, matching the pattern
         * already established in System::Buffers::Binary::BinaryPrimitives for its
         * Int128/UInt128 overloads.
         */
        static longcs BigMul(longcs a, longcs b, longcs& high) {
            __int128 product = static_cast<__int128>(a) * static_cast<__int128>(b);
            high = static_cast<longcs>(product >> 64);
            return static_cast<longcs>(product);
        }
#endif // !defined(_MSC_VER)

        /**
         * @brief Divides @p a by @p b and returns {quotient, remainder} as a pair.
         * @throws System::DivideByZeroException if @p b is zero (see the out-param DivRem() overload).
         * @throws System::OverflowException if @p a is intcs's MinValue and @p b is -1 (see the
         *         out-param DivRem() overload's comment -- same real hardware-trap hazard,
         *         confirmed via a standalone repro).
         */
        [[nodiscard]] static std::pair<intcs, intcs> DivRem(intcs a, intcs b) {
            if (b == 0) throw System::DivideByZeroException();
            if (a == std::numeric_limits<intcs>::min() && b == -1)
                throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
            return { a / b, a % b };
        }
        /**
         * @brief Divides @p a by @p b (64-bit) and returns {quotient, remainder} as a pair.
         * @throws System::DivideByZeroException if @p b is zero.
         * @throws System::OverflowException if @p a is longcs's MinValue and @p b is -1.
         */
        [[nodiscard]] static std::pair<longcs, longcs> DivRem(longcs a, longcs b) {
            if (b == 0) throw System::DivideByZeroException();
            if (a == std::numeric_limits<longcs>::min() && b == -1)
                throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
            return { a / b, a % b };
        }

        // ------------------------------------------------------------------
        // MidpointRounding overloads for Round
        // ------------------------------------------------------------------

        /**
         * @brief Rounds @p value to the nearest integer, ties to even, independent of the
         * ambient floating-point rounding-mode register.
         *
         * Deliberately does NOT use std::nearbyint/std::rint -- both follow the CURRENT
         * fesetround() mode rather than always implementing ties-to-even, so under a non-default
         * rounding mode (e.g. FE_UPWARD, which some SIMD/audio libraries set process-wide)
         * nearbyint(2.5) silently returns 3.0 instead of 2.0. std::floor/std::fmod are, by
         * contrast, specified with fixed rounding behavior independent of fesetround, so this
         * implementation is immune regardless of what a third-party library elsewhere in the
         * same process has set the rounding mode to.
         */
        static double roundToEvenImpl(double value) {
            double floorVal = std::floor(value);
            double diff = value - floorVal;
            if (diff < 0.5) return floorVal;
            if (diff > 0.5) return floorVal + 1.0;
            // Exact tie: round to the even neighbor.
            return (std::fmod(floorVal, 2.0) == 0.0) ? floorVal : floorVal + 1.0;
        }

        /** @brief Rounds @p value to the nearest integer using the specified rounding convention. */
        [[nodiscard]] static double Round(double value, MidpointRounding mode) {
            switch (mode) {
                case MidpointRounding::ToEven:             return roundToEvenImpl(value);
                case MidpointRounding::AwayFromZero:       return std::round(value);
                case MidpointRounding::ToZero:             return std::trunc(value);
                case MidpointRounding::ToNegativeInfinity: return std::floor(value);
                case MidpointRounding::ToPositiveInfinity: return std::ceil(value);
                default:                                   return roundToEvenImpl(value);
            }
        }

        /**
         * @brief Rounds @p value to @p digits decimal places using the specified rounding convention.
         *
         * C++ counterpart of .NET Math.Round(double, int, MidpointRounding). Matches .NET: values
         * whose magnitude is already >= 1e16 have no fractional part representable in a double and
         * are returned unchanged (also avoids precision loss from multiplying huge values by a
         * power of ten); the power-of-ten factor comes from a fixed lookup table rather than
         * std::pow, since digits is bounded and exact powers of ten avoid std::pow's rounding error.
         * @throws System::ArgumentOutOfRangeException if @p digits is outside [0, 15].
         */
        [[nodiscard]] static double Round(double value, intcs digits, MidpointRounding mode) {
            static constexpr double kPow10[16] = {
                1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7,
                1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
            };
            if (digits < 0 || digits > 15) {
                throw System::ArgumentOutOfRangeException("digits", "digits must be between 0 and 15, inclusive.");
            }
            if (Abs(value) < 1e16) {
                double power10 = kPow10[digits];
                value = Round(value * power10, mode) / power10;
            }
            return value;
        }

        // ------------------------------------------------------------------
        // SinCos
        // ------------------------------------------------------------------

        /** Holds the result of SinCos — the sine and cosine of an angle computed simultaneously. */
        struct SinCosResult { double Sin; double Cos; };

        /** @brief Computes the sine and cosine of @p x simultaneously. Returns a SinCosResult with Sin and Cos fields. */
        [[nodiscard]] static SinCosResult SinCos(double x) {
            return { std::sin(x), std::cos(x) };
        }

        // ------------------------------------------------------------------
        // ReciprocalSqrtEstimate
        // ------------------------------------------------------------------

        /** @brief Returns an estimate of the reciprocal square root of @p d (1/sqrt(d)). */
        [[nodiscard]] static double ReciprocalSqrtEstimate(double d) { return 1.0 / std::sqrt(d); }
    };
}