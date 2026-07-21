// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <cfenv>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

#include "System/Math.hpp"

using System::Math;

static constexpr double kEps = 1e-10;

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

TEST(MathTests, EConstant) {
    // From .NET MathTests.cs: exact bit pattern 0x4005BF0A8B145769
    EXPECT_NEAR(Math::E, 2.718281828459045, 1e-14);
}

TEST(MathTests, PIConstant) {
    // From .NET MathTests.cs: exact bit pattern 0x400921FB54442D18
    EXPECT_NEAR(Math::PI, 3.141592653589793, 1e-14);
}

// ---------------------------------------------------------------------------
// Abs
// ---------------------------------------------------------------------------

TEST(MathTests, AbsDoublePositive) {
    EXPECT_NEAR(Math::Abs(3.0), 3.0, kEps);
}

TEST(MathTests, AbsDoubleNegative) {
    EXPECT_NEAR(Math::Abs(-3.0), 3.0, kEps);
}

TEST(MathTests, AbsDoubleZero) {
    EXPECT_NEAR(Math::Abs(0.0), 0.0, kEps);
}

TEST(MathTests, AbsIntPositive) {
    EXPECT_EQ(Math::Abs(3), 3);
}

TEST(MathTests, AbsIntNegative) {
    // From .NET MathTests.cs: Assert.Equal(3, Math.Abs(-3))
    EXPECT_EQ(Math::Abs(-3), 3);
}

TEST(MathTests, AbsIntZero) {
    EXPECT_EQ(Math::Abs(0), 0);
}

TEST(MathTests, Abs_IntMinValue_Throws) {
    // .NET's Math.Abs(int) throws OverflowException for int.MinValue, since its
    // magnitude does not fit in a signed 32-bit integer - verified against Math.cs.
    EXPECT_THROW(Math::Abs(std::numeric_limits<SharpRuntime::intcs>::min()), System::OverflowException);
}

TEST(MathTests, Abs_LongMinValue_Throws) {
    EXPECT_THROW(Math::Abs(std::numeric_limits<SharpRuntime::longcs>::min()), System::OverflowException);
}

TEST(MathTests, Abs_ShortMinValue_Throws) {
    EXPECT_THROW(Math::Abs(std::numeric_limits<SharpRuntime::shortcs>::min()), System::OverflowException);
}

TEST(MathTests, Abs_SByteMinValue_Throws) {
    EXPECT_THROW(Math::Abs(std::numeric_limits<SharpRuntime::sbytecs>::min()), System::OverflowException);
}

// ---------------------------------------------------------------------------
// Min / Max (int and double)
// ---------------------------------------------------------------------------

TEST(MathTests, MinIntBasic) {
    // From .NET: Assert.Equal(-2, Math.Max(-2, 3)) — actually Min
    EXPECT_EQ(Math::Min(-2, 3), -2);
    EXPECT_EQ(Math::Min(3, -2), -2);
}

TEST(MathTests, MaxIntBasic) {
    // From .NET MathTests.cs: Assert.Equal(3, Math.Max(-2, 3))
    EXPECT_EQ(Math::Max(-2, 3), 3);
    EXPECT_EQ(Math::Max(3, -2), 3);
}

TEST(MathTests, MinMaxIntEqual) {
    EXPECT_EQ(Math::Min(5, 5), 5);
    EXPECT_EQ(Math::Max(5, 5), 5);
}

TEST(MathTests, MinDoubleBasic) {
    EXPECT_NEAR(Math::Min(-1.5, 2.5), -1.5, kEps);
}

TEST(MathTests, MaxDoubleBasic) {
    EXPECT_NEAR(Math::Max(-1.5, 2.5), 2.5, kEps);
}

// Math::Min/Max(double,double) previously delegated to std::min/std::max, which don't match
// .NET's IEEE 754:2019 minimum/maximum semantics: NaN must propagate regardless of which
// argument position it's in (std::min(5.0, NaN) == 5.0, not NaN, since NaN comparisons are
// always false and std::min's typical `b < a ? b : a` then just returns `a`), and +0 must be
// treated as strictly greater than -0.
TEST(MathTests, MinDouble_NaN_AsFirstArg_Propagates) {
    EXPECT_TRUE(std::isnan(Math::Min(std::numeric_limits<double>::quiet_NaN(), 1.0)));
}
TEST(MathTests, MinDouble_NaN_AsSecondArg_Propagates) {
    EXPECT_TRUE(std::isnan(Math::Min(1.0, std::numeric_limits<double>::quiet_NaN())));
}
TEST(MathTests, MaxDouble_NaN_AsFirstArg_Propagates) {
    EXPECT_TRUE(std::isnan(Math::Max(std::numeric_limits<double>::quiet_NaN(), 1.0)));
}
TEST(MathTests, MaxDouble_NaN_AsSecondArg_Propagates) {
    EXPECT_TRUE(std::isnan(Math::Max(1.0, std::numeric_limits<double>::quiet_NaN())));
}
TEST(MathTests, MinDouble_PositiveAndNegativeZero_ReturnsNegativeZero) {
    EXPECT_TRUE(std::signbit(Math::Min(0.0, -0.0)));
    EXPECT_TRUE(std::signbit(Math::Min(-0.0, 0.0)));
}
TEST(MathTests, MaxDouble_PositiveAndNegativeZero_ReturnsPositiveZero) {
    EXPECT_FALSE(std::signbit(Math::Max(0.0, -0.0)));
    EXPECT_FALSE(std::signbit(Math::Max(-0.0, 0.0)));
}

// ---------------------------------------------------------------------------
// Clamp
// ---------------------------------------------------------------------------

TEST(MathTests, ClampIntBelowMin) {
    EXPECT_EQ(Math::Clamp(0, 1, 10), 1);
}

TEST(MathTests, ClampIntAboveMax) {
    EXPECT_EQ(Math::Clamp(20, 1, 10), 10);
}

TEST(MathTests, ClampIntInRange) {
    EXPECT_EQ(Math::Clamp(5, 1, 10), 5);
}

TEST(MathTests, ClampDoubleBasic) {
    EXPECT_NEAR(Math::Clamp(0.5, 1.0, 3.0), 1.0, kEps);
    EXPECT_NEAR(Math::Clamp(2.0, 1.0, 3.0), 2.0, kEps);
    EXPECT_NEAR(Math::Clamp(4.0, 1.0, 3.0), 3.0, kEps);
}

TEST(MathTests, Clamp_MinGreaterThanMax_Throws) {
    // .NET's Math.Clamp throws ArgumentException if min > max, for every overload -
    // verified against Math.cs.
    EXPECT_THROW(Math::Clamp(5, 10, 0), System::ArgumentException);
    EXPECT_THROW(Math::Clamp(5.0, 10.0, 0.0), System::ArgumentException);
    EXPECT_THROW(Math::Clamp(5.0f, 10.0f, 0.0f), System::ArgumentException);
    EXPECT_THROW(Math::Clamp(static_cast<SharpRuntime::longcs>(5), static_cast<SharpRuntime::longcs>(10), static_cast<SharpRuntime::longcs>(0)), System::ArgumentException);
}

// ---------------------------------------------------------------------------
// Floor / Ceiling / Round
// ---------------------------------------------------------------------------

TEST(MathTests, FloorPositive) {
    // From .NET: Assert.Equal(1.0, Math.Floor(1.1)) and Math.Floor(1.9)
    EXPECT_NEAR(Math::Floor(1.1), 1.0, kEps);
    EXPECT_NEAR(Math::Floor(1.9), 1.0, kEps);
}

TEST(MathTests, FloorNegative) {
    // From .NET: Assert.Equal(-2.0, Math.Floor(-1.1))
    EXPECT_NEAR(Math::Floor(-1.1), -2.0, kEps);
}

TEST(MathTests, CeilingPositive) {
    // From .NET: Assert.Equal(2.0, Math.Ceiling(1.1)) and Math.Ceiling(1.9)
    EXPECT_NEAR(Math::Ceiling(1.1), 2.0, kEps);
    EXPECT_NEAR(Math::Ceiling(1.9), 2.0, kEps);
}

TEST(MathTests, CeilingNegative) {
    // From .NET: Assert.Equal(-1.0, Math.Ceiling(-1.1))
    EXPECT_NEAR(Math::Ceiling(-1.1), -1.0, kEps);
}

TEST(MathTests, Round_DefaultIsBankersRounding_ToEven) {
    // .NET's Math.Round(double) with no digits/mode defaults to MidpointRounding.ToEven
    // (banker's rounding): Round(2.5) == 2.0, not 3.0 - verified against Math.cs and
    // matches Python's round() (which uses the same IEEE 754 round-to-nearest-even).
    EXPECT_NEAR(Math::Round(2.5), 2.0, kEps);
    EXPECT_NEAR(Math::Round(3.5), 4.0, kEps);
    EXPECT_NEAR(Math::Round(2.4), 2.0, kEps);
}

TEST(MathTests, RoundNegative) {
    EXPECT_NEAR(Math::Round(-2.5), -2.0, kEps);
}

// ---------------------------------------------------------------------------
// Sqrt
// ---------------------------------------------------------------------------

TEST(MathTests, SqrtFour) {
    EXPECT_NEAR(Math::Sqrt(4.0), 2.0, kEps);
}

TEST(MathTests, SqrtTwo) {
    EXPECT_NEAR(Math::Sqrt(2.0), std::sqrt(2.0), kEps);
}

TEST(MathTests, SqrtZero) {
    EXPECT_NEAR(Math::Sqrt(0.0), 0.0, kEps);
}

// ---------------------------------------------------------------------------
// Pow
// ---------------------------------------------------------------------------

TEST(MathTests, PowTwoThree) {
    EXPECT_NEAR(Math::Pow(2.0, 3.0), 8.0, kEps);
}

TEST(MathTests, PowZeroExponent) {
    EXPECT_NEAR(Math::Pow(99.0, 0.0), 1.0, kEps);
}

TEST(MathTests, PowOneExponent) {
    EXPECT_NEAR(Math::Pow(5.0, 1.0), 5.0, kEps);
}

TEST(MathTests, PowFractional) {
    // 8^(1/3) = 2
    EXPECT_NEAR(Math::Pow(8.0, 1.0 / 3.0), 2.0, 1e-9);
}

// ---------------------------------------------------------------------------
// Trig — canonical values and identities
// ---------------------------------------------------------------------------

TEST(MathTests, SinZero) {
    EXPECT_NEAR(Math::Sin(0.0), 0.0, kEps);
}

TEST(MathTests, SinHalfPi) {
    EXPECT_NEAR(Math::Sin(Math::PI / 2.0), 1.0, kEps);
}

TEST(MathTests, CosZero) {
    EXPECT_NEAR(Math::Cos(0.0), 1.0, kEps);
}

TEST(MathTests, CosPi) {
    EXPECT_NEAR(Math::Cos(Math::PI), -1.0, kEps);
}

TEST(MathTests, TanZero) {
    EXPECT_NEAR(Math::Tan(0.0), 0.0, kEps);
}

TEST(MathTests, TanQuarterPi) {
    // tan(π/4) = 1
    EXPECT_NEAR(Math::Tan(Math::PI / 4.0), 1.0, kEps);
}

TEST(MathTests, SinSquaredPlusCosSquaredIsOne) {
    // Pythagorean identity for several angles
    for (double angle : {0.0, 0.5, 1.0, 1.5, Math::PI / 3.0}) {
        double s = Math::Sin(angle), c = Math::Cos(angle);
        EXPECT_NEAR(s * s + c * c, 1.0, kEps) << "angle=" << angle;
    }
}

// ---------------------------------------------------------------------------
// Tau constant
// ---------------------------------------------------------------------------

TEST(MathTests, TauConstant) {
    EXPECT_NEAR(Math::Tau, 2.0 * Math::PI, 1e-14);
}

// ---------------------------------------------------------------------------
// Logarithms
// ---------------------------------------------------------------------------

TEST(MathTests, LogNatural) {
    EXPECT_NEAR(Math::Log(Math::E), 1.0, kEps);
}
TEST(MathTests, LogNatural_One) {
    EXPECT_NEAR(Math::Log(1.0), 0.0, kEps);
}
TEST(MathTests, Log_WithBase) {
    EXPECT_NEAR(Math::Log(8.0, 2.0), 3.0, kEps);
}
TEST(MathTests, Log2) {
    EXPECT_NEAR(Math::Log2(8.0), 3.0, kEps);
}
TEST(MathTests, Log10) {
    EXPECT_NEAR(Math::Log10(1000.0), 3.0, kEps);
}
TEST(MathTests, Exp) {
    EXPECT_NEAR(Math::Exp(1.0), Math::E, kEps);
}
TEST(MathTests, Exp_Zero) {
    EXPECT_NEAR(Math::Exp(0.0), 1.0, kEps);
}

// ---------------------------------------------------------------------------
// Inverse trig
// ---------------------------------------------------------------------------

TEST(MathTests, Asin_One) {
    EXPECT_NEAR(Math::Asin(1.0), Math::PI / 2.0, kEps);
}
TEST(MathTests, Acos_One) {
    EXPECT_NEAR(Math::Acos(1.0), 0.0, kEps);
}
TEST(MathTests, Atan_One) {
    EXPECT_NEAR(Math::Atan(1.0), Math::PI / 4.0, kEps);
}
TEST(MathTests, Atan2_OneOne) {
    EXPECT_NEAR(Math::Atan2(1.0, 1.0), Math::PI / 4.0, kEps);
}

// ---------------------------------------------------------------------------
// Hyperbolic
// ---------------------------------------------------------------------------

TEST(MathTests, Sinh_Zero) {
    EXPECT_NEAR(Math::Sinh(0.0), 0.0, kEps);
}
TEST(MathTests, Cosh_Zero) {
    EXPECT_NEAR(Math::Cosh(0.0), 1.0, kEps);
}
TEST(MathTests, Tanh_Zero) {
    EXPECT_NEAR(Math::Tanh(0.0), 0.0, kEps);
}

// ---------------------------------------------------------------------------
// Sign
// ---------------------------------------------------------------------------

TEST(MathTests, Sign_Positive) { EXPECT_EQ(Math::Sign(5),    1); }
TEST(MathTests, Sign_Negative) { EXPECT_EQ(Math::Sign(-3),  -1); }
TEST(MathTests, Sign_Zero)     { EXPECT_EQ(Math::Sign(0),    0); }
TEST(MathTests, Sign_DoublePos) { EXPECT_EQ(Math::Sign(2.5),  1); }
TEST(MathTests, Sign_DoubleNeg) { EXPECT_EQ(Math::Sign(-1.5),-1); }
TEST(MathTests, Sign_Double_NaN_Throws) {
    // .NET's Math.Sign(double) throws ArithmeticException for NaN rather than
    // silently returning 0 (both comparisons against NaN are false) - verified
    // against Math.cs.
    EXPECT_THROW(Math::Sign(std::numeric_limits<double>::quiet_NaN()), System::ArithmeticException);
}
TEST(MathTests, Sign_Float_NaN_Throws) {
    EXPECT_THROW(Math::Sign(std::numeric_limits<float>::quiet_NaN()), System::ArithmeticException);
}

// ---------------------------------------------------------------------------
// Truncate
// ---------------------------------------------------------------------------

TEST(MathTests, Truncate_Positive) {
    EXPECT_NEAR(Math::Truncate(3.9), 3.0, kEps);
}
TEST(MathTests, Truncate_Negative) {
    EXPECT_NEAR(Math::Truncate(-3.9), -3.0, kEps);
}

// ---------------------------------------------------------------------------
// IEEERemainder
// ---------------------------------------------------------------------------

TEST(MathTests, IEEERemainder_Basic) {
    // 7 - (4 * round(7/4)) = 7 - 8 = -1
    EXPECT_NEAR(Math::IEEERemainder(7.0, 4.0), -1.0, kEps);
}

// ---------------------------------------------------------------------------
// DivRem
// ---------------------------------------------------------------------------

TEST(MathTests, DivRem_Basic) {
    int rem = 0;
    int q = Math::DivRem(17, 5, rem);
    EXPECT_EQ(q, 3);
    EXPECT_EQ(rem, 2);
}

// Plain C++ integer division by zero is undefined behavior (unlike the CLR's `div`/`rem`
// instructions, which raise a catchable DivideByZeroException); DivRem must check explicitly
// to get equivalent, catchable behavior instead of a crash.
TEST(MathTests, DivRem_ByZero_ThrowsDivideByZeroException) {
    int rem = 0;
    EXPECT_THROW(Math::DivRem(17, 0, rem), System::DivideByZeroException);
}

TEST(MathTests, DivRem_Pair_ByZero_ThrowsDivideByZeroException) {
    EXPECT_THROW(Math::DivRem(17, 0), System::DivideByZeroException);
}

// Regression for ticket 346: int.MinValue / -1 is a real hardware trap (SIGFPE on x86, confirmed
// via a standalone repro) in plain C++ integer division, matching the CLR's idiv instruction
// which surfaces this exact case as a catchable OverflowException. DivRem previously checked
// only for the zero-divisor case, missing this second hardware-trap hazard entirely -- same bug
// class already fixed in Int32::DivRem/Int64::DivRem elsewhere in this codebase but missed here.
TEST(MathTests, DivRem_MinValueByNegativeOne_ThrowsOverflowException) {
    int rem = 0;
    EXPECT_THROW(Math::DivRem(std::numeric_limits<int>::min(), -1, rem), System::OverflowException);
}

TEST(MathTests, DivRem_Pair_MinValueByNegativeOne_ThrowsOverflowException) {
    EXPECT_THROW(Math::DivRem(std::numeric_limits<int>::min(), -1), System::OverflowException);
}

// ---------------------------------------------------------------------------
// BigMul
// ---------------------------------------------------------------------------

TEST(MathTests, BigMul_Basic) {
    EXPECT_EQ(Math::BigMul(100000, 100000), 10000000000LL);
}

// ---------------------------------------------------------------------------
// ScaleB
// ---------------------------------------------------------------------------

TEST(MathTests, ScaleB_Basic) {
    EXPECT_NEAR(Math::ScaleB(1.0, 3), 8.0, kEps);
}
TEST(MathTests, ScaleB_Negative) {
    EXPECT_NEAR(Math::ScaleB(8.0, -3), 1.0, kEps);
}

// ---------------------------------------------------------------------------
// Cbrt
// ---------------------------------------------------------------------------
TEST(MathTests, Cbrt_PositivePerfect) { EXPECT_NEAR(Math::Cbrt(27.0),  3.0, 1e-10); }
TEST(MathTests, Cbrt_NegativeValue)   { EXPECT_NEAR(Math::Cbrt(-8.0), -2.0, 1e-10); }
TEST(MathTests, Cbrt_Zero)            { EXPECT_EQ(Math::Cbrt(0.0), 0.0); }

// ---------------------------------------------------------------------------
// Acosh / Asinh / Atanh
// ---------------------------------------------------------------------------
TEST(MathTests, Acosh_One)     { EXPECT_NEAR(Math::Acosh(1.0),   0.0,         1e-10); }
TEST(MathTests, Acosh_Larger)  { EXPECT_NEAR(Math::Acosh(std::cosh(2.0)), 2.0, 1e-10); }
TEST(MathTests, Asinh_Zero)    { EXPECT_NEAR(Math::Asinh(0.0),   0.0,         1e-10); }
TEST(MathTests, Asinh_RoundTrip) { EXPECT_NEAR(Math::Asinh(std::sinh(1.5)), 1.5, 1e-10); }
TEST(MathTests, Atanh_Zero)    { EXPECT_NEAR(Math::Atanh(0.0),   0.0,         1e-10); }
TEST(MathTests, Atanh_RoundTrip) { EXPECT_NEAR(Math::Atanh(std::tanh(0.7)), 0.7, 1e-10); }

// ---------------------------------------------------------------------------
// Round(value, digits)
// ---------------------------------------------------------------------------
TEST(MathTests, Round_TwoDigits)   { EXPECT_NEAR(Math::Round(3.14159, 2), 3.14, 1e-10); }
TEST(MathTests, Round_ZeroDigits)  { EXPECT_NEAR(Math::Round(2.7, 0),     3.0,  1e-10); }
TEST(MathTests, Round_ThreeDigits) {
    // 1.2345 rounded to 3 digits is a tie in exact decimal, but round-to-even applies
    // to the actual double value (which is a hair below the mathematical 1.2345) -
    // matches Python's round(1.2345, 3) == 1.234, and .NET's default ToEven mode.
    EXPECT_NEAR(Math::Round(1.2345, 3), 1.234, 1e-6);
}

TEST(MathTests, Round_Digits_OutOfRange_Throws) {
    // .NET's Math.Round(double, int) throws ArgumentOutOfRangeException if digits
    // is outside [0, 15] - verified against Math.cs.
    EXPECT_THROW(Math::Round(1.5, -1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(Math::Round(1.5, 16), System::ArgumentOutOfRangeException);
    EXPECT_NO_THROW(Math::Round(1.5, 0));
    EXPECT_NO_THROW(Math::Round(1.5, 15));
}

TEST(MathTests, Round_Digits_LargeMagnitude_ReturnedUnchanged) {
    // Values with magnitude >= 1e16 have no fractional part representable in a
    // double, so .NET's Math.Round(double, int) returns them unchanged rather than
    // multiplying by a power of ten (which could lose precision or overflow).
    double huge = 1.2345e17;
    EXPECT_EQ(Math::Round(huge, 2), huge);
}

// ---------------------------------------------------------------------------
// CopySign
// ---------------------------------------------------------------------------
TEST(MathTests, CopySign_PositiveMag_NegativeSign) { EXPECT_EQ(Math::CopySign(3.0, -1.0), -3.0); }
TEST(MathTests, CopySign_NegativeMag_PositiveSign) { EXPECT_EQ(Math::CopySign(-5.0, 1.0),  5.0); }

// ---------------------------------------------------------------------------
// BitIncrement / BitDecrement
// ---------------------------------------------------------------------------
TEST(MathTests, BitIncrement_GreaterThanOriginal) {
    double x = 1.0;
    EXPECT_GT(Math::BitIncrement(x), x);
}
TEST(MathTests, BitDecrement_LessThanOriginal) {
    double x = 1.0;
    EXPECT_LT(Math::BitDecrement(x), x);
}

// ---------------------------------------------------------------------------
// FusedMultiplyAdd
// ---------------------------------------------------------------------------
TEST(MathTests, FusedMultiplyAdd_Basic) {
    EXPECT_NEAR(Math::FusedMultiplyAdd(2.0, 3.0, 4.0), 10.0, 1e-10);
}
TEST(MathTests, FusedMultiplyAdd_Zero) {
    EXPECT_NEAR(Math::FusedMultiplyAdd(0.0, 99.0, 7.0), 7.0, 1e-10);
}

// ---------------------------------------------------------------------------
// longcs overloads: Abs, Min, Max, Clamp
// ---------------------------------------------------------------------------

TEST(MathTests, Abs_Long_Positive)  { EXPECT_EQ(Math::Abs(static_cast<SharpRuntime::longcs>(5LL)), 5LL); }
TEST(MathTests, Abs_Long_Negative)  { EXPECT_EQ(Math::Abs(static_cast<SharpRuntime::longcs>(-5LL)), 5LL); }
TEST(MathTests, Abs_Long_Zero)      { EXPECT_EQ(Math::Abs(static_cast<SharpRuntime::longcs>(0LL)), 0LL); }

TEST(MathTests, Min_Long_Basic)     { EXPECT_EQ(Math::Min(static_cast<SharpRuntime::longcs>(3LL), static_cast<SharpRuntime::longcs>(7LL)), 3LL); }
TEST(MathTests, Max_Long_Basic)     { EXPECT_EQ(Math::Max(static_cast<SharpRuntime::longcs>(3LL), static_cast<SharpRuntime::longcs>(7LL)), 7LL); }

TEST(MathTests, Clamp_Long_Below)   { EXPECT_EQ(Math::Clamp(static_cast<SharpRuntime::longcs>(0LL), static_cast<SharpRuntime::longcs>(1LL), static_cast<SharpRuntime::longcs>(10LL)), 1LL); }
TEST(MathTests, Clamp_Long_Above)   { EXPECT_EQ(Math::Clamp(static_cast<SharpRuntime::longcs>(20LL), static_cast<SharpRuntime::longcs>(1LL), static_cast<SharpRuntime::longcs>(10LL)), 10LL); }
TEST(MathTests, Clamp_Long_InRange) { EXPECT_EQ(Math::Clamp(static_cast<SharpRuntime::longcs>(5LL), static_cast<SharpRuntime::longcs>(1LL), static_cast<SharpRuntime::longcs>(10LL)), 5LL); }

// ---------------------------------------------------------------------------
// float overloads: Abs, Min, Max, Clamp
// ---------------------------------------------------------------------------

TEST(MathTests, Abs_Float_Negative)  { EXPECT_EQ(Math::Abs(-3.5f), 3.5f); }
TEST(MathTests, Abs_Float_Positive)  { EXPECT_EQ(Math::Abs(2.0f),  2.0f); }
TEST(MathTests, Min_Float_Basic)     { EXPECT_EQ(Math::Min(1.5f, 2.5f), 1.5f); }
TEST(MathTests, Max_Float_Basic)     { EXPECT_EQ(Math::Max(1.5f, 2.5f), 2.5f); }
// Same std::min/std::max NaN/signed-zero mismatch as the double overload above, fixed alongside it.
TEST(MathTests, MinFloat_NaN_AsSecondArg_Propagates) {
    EXPECT_TRUE(std::isnan(Math::Min(1.0f, std::numeric_limits<float>::quiet_NaN())));
}
TEST(MathTests, MaxFloat_NaN_AsSecondArg_Propagates) {
    EXPECT_TRUE(std::isnan(Math::Max(1.0f, std::numeric_limits<float>::quiet_NaN())));
}
TEST(MathTests, MinFloat_PositiveAndNegativeZero_ReturnsNegativeZero) {
    EXPECT_TRUE(std::signbit(Math::Min(0.0f, -0.0f)));
}
TEST(MathTests, MaxFloat_PositiveAndNegativeZero_ReturnsPositiveZero) {
    EXPECT_FALSE(std::signbit(Math::Max(0.0f, -0.0f)));
}
TEST(MathTests, Clamp_Float_Below)   { EXPECT_EQ(Math::Clamp(0.0f, 1.0f, 10.0f), 1.0f); }
TEST(MathTests, Clamp_Float_Above)   { EXPECT_EQ(Math::Clamp(20.0f, 1.0f, 10.0f), 10.0f); }
TEST(MathTests, Clamp_Float_InRange) { EXPECT_EQ(Math::Clamp(5.0f, 1.0f, 10.0f), 5.0f); }

// ---------------------------------------------------------------------------
// Sign — longcs and float overloads
// ---------------------------------------------------------------------------

TEST(MathTests, Sign_Long_Negative) { EXPECT_EQ(Math::Sign(static_cast<SharpRuntime::longcs>(-999999999999LL)), -1); }
TEST(MathTests, Sign_Long_Zero)     { EXPECT_EQ(Math::Sign(static_cast<SharpRuntime::longcs>(0LL)), 0); }
TEST(MathTests, Sign_Long_Positive) { EXPECT_EQ(Math::Sign(static_cast<SharpRuntime::longcs>(123456789012LL)), 1); }

TEST(MathTests, Sign_Float_Negative) { EXPECT_EQ(Math::Sign(-3.5f), -1); }
TEST(MathTests, Sign_Float_Zero)     { EXPECT_EQ(Math::Sign(0.0f),   0); }
TEST(MathTests, Sign_Float_Positive) { EXPECT_EQ(Math::Sign(2.7f),   1); }

// ---------------------------------------------------------------------------
// DivRem — longcs overload
// ---------------------------------------------------------------------------
TEST(MathTests, DivRem_Long_Positive) {
    SharpRuntime::longcs rem = 0;
    SharpRuntime::longcs q = Math::DivRem(static_cast<SharpRuntime::longcs>(10LL),
                                           static_cast<SharpRuntime::longcs>(3LL), rem);
    EXPECT_EQ(q, 3LL);
    EXPECT_EQ(rem, 1LL);
}
TEST(MathTests, DivRem_Long_Negative) {
    SharpRuntime::longcs rem = 0;
    SharpRuntime::longcs q = Math::DivRem(static_cast<SharpRuntime::longcs>(-10LL),
                                           static_cast<SharpRuntime::longcs>(3LL), rem);
    EXPECT_EQ(q, -3LL);
    EXPECT_EQ(rem, -1LL);
}
TEST(MathTests, DivRem_Long_Exact) {
    SharpRuntime::longcs rem = -1;
    Math::DivRem(static_cast<SharpRuntime::longcs>(9LL),
                 static_cast<SharpRuntime::longcs>(3LL), rem);
    EXPECT_EQ(rem, 0LL);
}

TEST(MathTests, DivRem_Long_ByZero_ThrowsDivideByZeroException) {
    SharpRuntime::longcs rem = 0;
    EXPECT_THROW(Math::DivRem(static_cast<SharpRuntime::longcs>(9LL),
                               static_cast<SharpRuntime::longcs>(0LL), rem),
                 System::DivideByZeroException);
}

// Regression for ticket 346: same MinValue/-1 hardware-trap fix as the intcs overload above.
TEST(MathTests, DivRem_Long_MinValueByNegativeOne_ThrowsOverflowException) {
    SharpRuntime::longcs rem = 0;
    EXPECT_THROW(Math::DivRem(std::numeric_limits<SharpRuntime::longcs>::min(),
                               static_cast<SharpRuntime::longcs>(-1LL), rem),
                 System::OverflowException);
}

TEST(MathTests, DivRem_Pair_Long_MinValueByNegativeOne_ThrowsOverflowException) {
    EXPECT_THROW(Math::DivRem(std::numeric_limits<SharpRuntime::longcs>::min(),
                               static_cast<SharpRuntime::longcs>(-1LL)),
                 System::OverflowException);
}

// ---------------------------------------------------------------------------
// MaxMagnitude / MinMagnitude
// ---------------------------------------------------------------------------
TEST(MathTests, MaxMagnitude_LargerFirst)  { EXPECT_EQ(Math::MaxMagnitude(5.0, 3.0),  5.0); }
TEST(MathTests, MaxMagnitude_LargerSecond) { EXPECT_EQ(Math::MaxMagnitude(2.0, -7.0), -7.0); }
TEST(MathTests, MaxMagnitude_Equal_ReturnsPositive) { EXPECT_EQ(Math::MaxMagnitude(3.0, -3.0), 3.0); }
TEST(MathTests, MaxMagnitude_NaN_Propagates) { EXPECT_TRUE(std::isnan(Math::MaxMagnitude(std::numeric_limits<double>::quiet_NaN(), 1.0))); }

TEST(MathTests, MinMagnitude_SmallerFirst)  { EXPECT_EQ(Math::MinMagnitude(2.0, 5.0),   2.0); }
TEST(MathTests, MinMagnitude_SmallerSecond) { EXPECT_EQ(Math::MinMagnitude(-7.0, 2.0),  2.0); }
TEST(MathTests, MinMagnitude_Equal_ReturnsNegative) { EXPECT_EQ(Math::MinMagnitude(-3.0, 3.0), -3.0); }
TEST(MathTests, MinMagnitude_NaN_Propagates) { EXPECT_TRUE(std::isnan(Math::MinMagnitude(std::numeric_limits<double>::quiet_NaN(), 1.0))); }

TEST(MathTests, SinCos_Zero) {
    auto r = Math::SinCos(0.0);
    EXPECT_NEAR(r.Sin, 0.0, 1e-12);
    EXPECT_NEAR(r.Cos, 1.0, 1e-12);
}

TEST(MathTests, SinCos_PiOver2) {
    auto r = Math::SinCos(Math::PI / 2.0);
    EXPECT_NEAR(r.Sin, 1.0, 1e-10);
    EXPECT_NEAR(r.Cos, 0.0, 1e-10);
}

TEST(MathTests, Round_MidpointRounding_ToEven) {
    EXPECT_DOUBLE_EQ(Math::Round(2.5, System::MidpointRounding::ToEven), 2.0);
    EXPECT_DOUBLE_EQ(Math::Round(3.5, System::MidpointRounding::ToEven), 4.0);
}

namespace {
// RAII guard restoring the default (nearest) FP rounding mode even if an assertion inside the
// scope fails and unwinds early -- ticket 1723 regression test needs this since it deliberately
// mutates ambient process-wide FP state via fesetround().
struct RoundingModeGuard {
    int saved = std::fegetround();
    ~RoundingModeGuard() { std::fesetround(saved); }
};
}

// Ticket 1723 (post-stabilization-audit): Math::Round(..., ToEven) previously used
// std::nearbyint, which follows the CURRENT fesetround() mode rather than always implementing
// ties-to-even -- under FE_UPWARD, nearbyint(2.5) silently returned 3.0 instead of 2.0. Verifies
// the fix (a manual floor/fmod-based implementation, immune to the rounding-mode register) still
// produces correct banker's-rounding results under a non-default ambient rounding mode.
TEST(MathTests, Round_ToEven_IndependentOfAmbientRoundingMode) {
    RoundingModeGuard guard;
    std::fesetround(FE_UPWARD);
    EXPECT_DOUBLE_EQ(Math::Round(2.5, System::MidpointRounding::ToEven), 2.0);
    EXPECT_DOUBLE_EQ(Math::Round(3.5, System::MidpointRounding::ToEven), 4.0);
    EXPECT_DOUBLE_EQ(Math::Round(-2.5, System::MidpointRounding::ToEven), -2.0);
    std::fesetround(FE_DOWNWARD);
    EXPECT_DOUBLE_EQ(Math::Round(2.5, System::MidpointRounding::ToEven), 2.0);
    EXPECT_DOUBLE_EQ(Math::Round(3.5, System::MidpointRounding::ToEven), 4.0);
    // No-mode-argument Round(double) delegates to ToEven -- verify it's also immune.
    EXPECT_DOUBLE_EQ(Math::Round(2.5), 2.0);
}

TEST(MathTests, Round_MidpointRounding_AwayFromZero) {
    EXPECT_DOUBLE_EQ(Math::Round(2.5, System::MidpointRounding::AwayFromZero), 3.0);
    EXPECT_DOUBLE_EQ(Math::Round(-2.5, System::MidpointRounding::AwayFromZero), -3.0);
}

TEST(MathTests, Round_Digits_MidpointRounding) {
    EXPECT_NEAR(Math::Round(1.455, 2, System::MidpointRounding::AwayFromZero), 1.46, 1e-9);
}

TEST(MathTests, ReciprocalSqrtEstimate_Four) {
    EXPECT_NEAR(Math::ReciprocalSqrtEstimate(4.0), 0.5, 1e-10);
}

TEST(MathTests, ReciprocalSqrtEstimate_One) {
    EXPECT_NEAR(Math::ReciprocalSqrtEstimate(1.0), 1.0, 1e-10);
}
