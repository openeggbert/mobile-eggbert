// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <cmath>
#include <limits>

#include "System/Double.hpp"

using System::Double;

// Suite name uses a Tests2 suffix per NEXT.md's documented duplicate-suite-name
// policy: DoubleTests already exists in DoubleTests.cpp / PrimitiveTypeTests2.cpp.

// ---------------------------------------------------------------------------
// MaxMagnitude / MinMagnitude
// ---------------------------------------------------------------------------

TEST(DoubleTests2, MaxMagnitude_PicksLargerAbsoluteValue) {
    EXPECT_EQ(Double::MaxMagnitude(-5.0, 3.0), -5.0);
    EXPECT_EQ(Double::MaxMagnitude(-5.0, 5.0), 5.0); // tie -> prefers the positive value
}

TEST(DoubleTests2, MinMagnitude_PicksSmallerAbsoluteValue) {
    EXPECT_EQ(Double::MinMagnitude(-5.0, 3.0), 3.0);
    EXPECT_EQ(Double::MinMagnitude(-5.0, 5.0), -5.0); // tie -> prefers the negative value
}

// ---------------------------------------------------------------------------
// Rounding
// ---------------------------------------------------------------------------

TEST(DoubleTests2, Ceiling) { EXPECT_EQ(Double::Ceiling(1.2), 2.0); }
TEST(DoubleTests2, Floor)   { EXPECT_EQ(Double::Floor(1.8), 1.0); }
TEST(DoubleTests2, Truncate_NegativeTowardZero) { EXPECT_EQ(Double::Truncate(-1.8), -1.0); }
TEST(DoubleTests2, Round_TiesToEven) {
    EXPECT_EQ(Double::Round(2.5), 2.0);
    EXPECT_EQ(Double::Round(3.5), 4.0);
}
TEST(DoubleTests2, Round_WithDigits) {
    EXPECT_NEAR(Double::Round(3.14159, 2), 3.14, 1e-9);
}

// ---------------------------------------------------------------------------
// Exponential / logarithmic
// ---------------------------------------------------------------------------

TEST(DoubleTests2, Exp_Zero_IsOne)  { EXPECT_DOUBLE_EQ(Double::Exp(0.0), 1.0); }
TEST(DoubleTests2, Exp2_Three)      { EXPECT_DOUBLE_EQ(Double::Exp2(3.0), 8.0); }
TEST(DoubleTests2, Exp10_Two)       { EXPECT_NEAR(Double::Exp10(2.0), 100.0, 1e-9); }
TEST(DoubleTests2, Log_E_IsOne)     { EXPECT_NEAR(Double::Log(Double::E), 1.0, 1e-12); }
TEST(DoubleTests2, Log_Base8_2Is3)  { EXPECT_NEAR(Double::Log(8.0, 2.0), 3.0, 1e-9); }
TEST(DoubleTests2, Log2_Eight)      { EXPECT_DOUBLE_EQ(Double::Log2(8.0), 3.0); }
TEST(DoubleTests2, Log10_Hundred)   { EXPECT_DOUBLE_EQ(Double::Log10(100.0), 2.0); }
TEST(DoubleTests2, Pow_TwoCubed)    { EXPECT_DOUBLE_EQ(Double::Pow(2.0, 3.0), 8.0); }

// ---------------------------------------------------------------------------
// Root / reciprocal
// ---------------------------------------------------------------------------

TEST(DoubleTests2, Sqrt_Nine)          { EXPECT_DOUBLE_EQ(Double::Sqrt(9.0), 3.0); }
TEST(DoubleTests2, Cbrt_TwentySeven)   { EXPECT_NEAR(Double::Cbrt(27.0), 3.0, 1e-12); }
TEST(DoubleTests2, RootN_FourthRoot)   { EXPECT_NEAR(Double::RootN(16.0, 4), 2.0, 1e-9); }
TEST(DoubleTests2, ReciprocalEstimate) { EXPECT_NEAR(Double::ReciprocalEstimate(4.0), 0.25, 1e-9); }
TEST(DoubleTests2, ReciprocalSqrtEstimate) { EXPECT_NEAR(Double::ReciprocalSqrtEstimate(4.0), 0.5, 1e-9); }

// ---------------------------------------------------------------------------
// Trigonometric
// ---------------------------------------------------------------------------

TEST(DoubleTests2, Sin_Zero)   { EXPECT_NEAR(Double::Sin(0.0), 0.0, 1e-12); }
TEST(DoubleTests2, Cos_Zero)   { EXPECT_NEAR(Double::Cos(0.0), 1.0, 1e-12); }
TEST(DoubleTests2, Tan_Zero)   { EXPECT_NEAR(Double::Tan(0.0), 0.0, 1e-12); }
TEST(DoubleTests2, Asin_One)   { EXPECT_NEAR(Double::Asin(1.0), Double::Pi / 2.0, 1e-9); }
TEST(DoubleTests2, Acos_One)   { EXPECT_NEAR(Double::Acos(1.0), 0.0, 1e-12); }
TEST(DoubleTests2, Atan_One)   { EXPECT_NEAR(Double::Atan(1.0), Double::Pi / 4.0, 1e-9); }
TEST(DoubleTests2, Atan2_Basic) { EXPECT_NEAR(Double::Atan2(1.0, 1.0), Double::Pi / 4.0, 1e-9); }
TEST(DoubleTests2, SinPi_Half)  { EXPECT_NEAR(Double::SinPi(0.5), 1.0, 1e-9); }
TEST(DoubleTests2, CosPi_Zero)  { EXPECT_NEAR(Double::CosPi(0.0), 1.0, 1e-12); }
TEST(DoubleTests2, TanPi_Quarter) { EXPECT_NEAR(Double::TanPi(0.25), 1.0, 1e-9); }
TEST(DoubleTests2, AcosPi_One)  { EXPECT_NEAR(Double::AcosPi(1.0), 0.0, 1e-12); }
TEST(DoubleTests2, AsinPi_One)  { EXPECT_NEAR(Double::AsinPi(1.0), 0.5, 1e-9); }
TEST(DoubleTests2, AtanPi_One)  { EXPECT_NEAR(Double::AtanPi(1.0), 0.25, 1e-9); }
TEST(DoubleTests2, Atan2Pi_Basic) { EXPECT_NEAR(Double::Atan2Pi(1.0, 1.0), 0.25, 1e-9); }

TEST(DoubleTests2, SinCos_MatchesIndividual) {
    auto r = Double::SinCos(0.7);
    EXPECT_NEAR(r.Sin, Double::Sin(0.7), 1e-12);
    EXPECT_NEAR(r.Cos, Double::Cos(0.7), 1e-12);
}

TEST(DoubleTests2, SinCosPi_MatchesIndividual) {
    auto r = Double::SinCosPi(0.3);
    EXPECT_NEAR(r.SinPi, Double::SinPi(0.3), 1e-12);
    EXPECT_NEAR(r.CosPi, Double::CosPi(0.3), 1e-12);
}

// ---------------------------------------------------------------------------
// Hyperbolic
// ---------------------------------------------------------------------------

TEST(DoubleTests2, Sinh_Zero)  { EXPECT_NEAR(Double::Sinh(0.0), 0.0, 1e-12); }
TEST(DoubleTests2, Cosh_Zero)  { EXPECT_NEAR(Double::Cosh(0.0), 1.0, 1e-12); }
TEST(DoubleTests2, Tanh_Zero)  { EXPECT_NEAR(Double::Tanh(0.0), 0.0, 1e-12); }
TEST(DoubleTests2, Asinh_Zero) { EXPECT_NEAR(Double::Asinh(0.0), 0.0, 1e-12); }
TEST(DoubleTests2, Acosh_One)  { EXPECT_NEAR(Double::Acosh(1.0), 0.0, 1e-12); }
TEST(DoubleTests2, Atanh_Zero) { EXPECT_NEAR(Double::Atanh(0.0), 0.0, 1e-12); }

// ---------------------------------------------------------------------------
// IEEE 754 utilities
// ---------------------------------------------------------------------------

TEST(DoubleTests2, Hypot_3_4_Is5) { EXPECT_NEAR(Double::Hypot(3.0, 4.0), 5.0, 1e-12); }

TEST(DoubleTests2, FusedMultiplyAdd) {
    EXPECT_DOUBLE_EQ(Double::FusedMultiplyAdd(2.0, 3.0, 1.0), 7.0);
}

TEST(DoubleTests2, Ieee754Remainder) {
    EXPECT_NEAR(Double::Ieee754Remainder(5.0, 3.0), -1.0, 1e-12);
}

TEST(DoubleTests2, BitIncrement_IncreasesValue) {
    EXPECT_GT(Double::BitIncrement(1.0), 1.0);
}

TEST(DoubleTests2, BitDecrement_DecreasesValue) {
    EXPECT_LT(Double::BitDecrement(1.0), 1.0);
}

TEST(DoubleTests2, ILogB_PowerOfTwo) {
    EXPECT_EQ(Double::ILogB(8.0), 3);
}

TEST(DoubleTests2, ScaleB_MultipliesByPowerOfTwo) {
    EXPECT_DOUBLE_EQ(Double::ScaleB(1.0, 3), 8.0);
}

// ---------------------------------------------------------------------------
// Angle conversion
// ---------------------------------------------------------------------------

TEST(DoubleTests2, DegreesToRadians_180IsPi) {
    EXPECT_NEAR(Double::DegreesToRadians(180.0), Double::Pi, 1e-12);
}

TEST(DoubleTests2, RadiansToDegrees_PiIs180) {
    EXPECT_NEAR(Double::RadiansToDegrees(Double::Pi), 180.0, 1e-9);
}

// ---------------------------------------------------------------------------
// Comparison: CompareTo / Equals / GetHashCode
// (.NET parity: NaN sorts below all values and equals itself in CompareTo/Equals,
// but == still returns false for NaN == NaN.)
// ---------------------------------------------------------------------------

TEST(DoubleTests2, CompareTo_Ordering) {
    EXPECT_LT(Double::CompareTo(1.0, 2.0), 0);
    EXPECT_GT(Double::CompareTo(2.0, 1.0), 0);
    EXPECT_EQ(Double::CompareTo(1.0, 1.0), 0);
}

TEST(DoubleTests2, CompareTo_NaNSortsBelowEverything) {
    EXPECT_LT(Double::CompareTo(Double::NaN, Double::NegativeInfinity), 0);
    EXPECT_GT(Double::CompareTo(Double::NegativeInfinity, Double::NaN), 0);
}

TEST(DoubleTests2, CompareTo_NaNEqualsNaN) {
    EXPECT_EQ(Double::CompareTo(Double::NaN, Double::NaN), 0);
}

TEST(DoubleTests2, Equals_NaNEqualsNaN) {
    EXPECT_TRUE(Double::Equals(Double::NaN, Double::NaN));
    // Contrast with operator==, which follows IEEE 754 (NaN != NaN).
    EXPECT_FALSE(Double::NaN == Double::NaN);
}

TEST(DoubleTests2, Equals_RegularValues) {
    EXPECT_TRUE(Double::Equals(1.5, 1.5));
    EXPECT_FALSE(Double::Equals(1.5, 2.5));
}

TEST(DoubleTests2, GetHashCode_EqualValuesHaveSameHash) {
    EXPECT_EQ(Double::GetHashCode(1.5), Double::GetHashCode(1.5));
}

TEST(DoubleTests2, GetHashCode_PositiveAndNegativeZeroMatch) {
    EXPECT_EQ(Double::GetHashCode(0.0), Double::GetHashCode(Double::NegativeZero));
}

TEST(DoubleTests2, GetHashCode_AllNaNsMatch) {
    double nan1 = Double::NaN;
    double nan2 = -Double::NaN;
    EXPECT_EQ(Double::GetHashCode(nan1), Double::GetHashCode(nan2));
}
