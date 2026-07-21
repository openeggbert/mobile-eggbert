// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for: MathF (single-precision math), BitOperations (bit manipulation).
#include <gtest/gtest.h>
#include <cmath>
#include "System/MathF.hpp"
#include "System/MidpointRounding.hpp"
#include "System/Numerics/BitOperations.hpp"

using System::MathF;
using System::MidpointRounding;
using System::Numerics::BitOperations;

// ===========================================================================
// MathF
// ===========================================================================

TEST(MathFTests, Constants) {
    EXPECT_FLOAT_EQ(MathF::PI, 3.14159265f);
    EXPECT_FLOAT_EQ(MathF::E,  2.71828183f);
    EXPECT_FLOAT_EQ(MathF::Tau, MathF::PI * 2.0f);
}
TEST(MathFTests, Abs_NegativeToPositive) {
    EXPECT_FLOAT_EQ(MathF::Abs(-3.5f), 3.5f);
}
TEST(MathFTests, Ceiling_RoundsUp) {
    EXPECT_FLOAT_EQ(MathF::Ceiling(1.2f), 2.0f);
}
TEST(MathFTests, Floor_RoundsDown) {
    EXPECT_FLOAT_EQ(MathF::Floor(1.9f), 1.0f);
}
TEST(MathFTests, Round_MidpointToNearest) {
    // .NET's MathF.Round() rounds midpoints to even (banker's rounding) by default,
    // not away from zero - verified against MathF.cs. 2.5 -> 2 (nearest even).
    EXPECT_FLOAT_EQ(MathF::Round(2.5f), 2.0f);
}
TEST(MathFTests, Truncate_DropsDecimal) {
    EXPECT_FLOAT_EQ(MathF::Truncate(3.9f), 3.0f);
    EXPECT_FLOAT_EQ(MathF::Truncate(-3.9f), -3.0f);
}
TEST(MathFTests, Sqrt_SquareRoot) {
    EXPECT_FLOAT_EQ(MathF::Sqrt(9.0f), 3.0f);
}
TEST(MathFTests, Pow_Power) {
    EXPECT_FLOAT_EQ(MathF::Pow(2.0f, 10.0f), 1024.0f);
}
TEST(MathFTests, Log2_PowerOfTwo) {
    EXPECT_FLOAT_EQ(MathF::Log2(8.0f), 3.0f);
}
TEST(MathFTests, Log10_PowerOfTen) {
    EXPECT_NEAR(MathF::Log10(100.0f), 2.0f, 1e-6f);
}
TEST(MathFTests, Sin_Cos) {
    EXPECT_NEAR(MathF::Sin(MathF::PI / 2.0f), 1.0f, 1e-6f);
    EXPECT_NEAR(MathF::Cos(0.0f), 1.0f, 1e-6f);
}
TEST(MathFTests, Atan2_FirstQuadrant) {
    EXPECT_NEAR(MathF::Atan2(1.0f, 1.0f), MathF::PI / 4.0f, 1e-6f);
}
TEST(MathFTests, Max_Min) {
    EXPECT_FLOAT_EQ(MathF::Max(3.0f, 7.0f), 7.0f);
    EXPECT_FLOAT_EQ(MathF::Min(3.0f, 7.0f), 3.0f);
}
TEST(MathFTests, Clamp_InRange) {
    EXPECT_FLOAT_EQ(MathF::Clamp(5.0f, 0.0f, 10.0f), 5.0f);
}
TEST(MathFTests, Clamp_BelowMin) {
    EXPECT_FLOAT_EQ(MathF::Clamp(-5.0f, 0.0f, 10.0f), 0.0f);
}
TEST(MathFTests, Clamp_AboveMax) {
    EXPECT_FLOAT_EQ(MathF::Clamp(15.0f, 0.0f, 10.0f), 10.0f);
}
TEST(MathFTests, Sign_Positive_Negative_Zero) {
    EXPECT_EQ(MathF::Sign(3.0f),  1);
    EXPECT_EQ(MathF::Sign(-3.0f), -1);
    EXPECT_EQ(MathF::Sign(0.0f),  0);
}
TEST(MathFTests, IsNaN_IsInfinity) {
    EXPECT_TRUE(MathF::IsNaN(std::numeric_limits<float>::quiet_NaN()));
    EXPECT_FALSE(MathF::IsNaN(1.0f));
    EXPECT_TRUE(MathF::IsInfinity(std::numeric_limits<float>::infinity()));
    EXPECT_FALSE(MathF::IsInfinity(1.0f));
}
TEST(MathFTests, IsPositiveInfinity_IsNegativeInfinity) {
    float pos_inf = std::numeric_limits<float>::infinity();
    EXPECT_TRUE(MathF::IsPositiveInfinity(pos_inf));
    EXPECT_TRUE(MathF::IsNegativeInfinity(-pos_inf));
    EXPECT_FALSE(MathF::IsPositiveInfinity(-pos_inf));
}

// ===========================================================================
// BitOperations
// ===========================================================================

TEST(BitOperationsTests, IsPow2_True_ForPowers) {
    EXPECT_TRUE(BitOperations::IsPow2(uint32_t(1)));
    EXPECT_TRUE(BitOperations::IsPow2(uint32_t(16)));
    EXPECT_TRUE(BitOperations::IsPow2(uint32_t(1024)));
}
TEST(BitOperationsTests, IsPow2_False_ForNonPowers) {
    EXPECT_FALSE(BitOperations::IsPow2(uint32_t(0)));
    EXPECT_FALSE(BitOperations::IsPow2(uint32_t(3)));
    EXPECT_FALSE(BitOperations::IsPow2(uint32_t(6)));
}
TEST(BitOperationsTests, IsPow2_Int32) {
    EXPECT_TRUE(BitOperations::IsPow2(int32_t(8)));
    EXPECT_FALSE(BitOperations::IsPow2(int32_t(0)));
    EXPECT_FALSE(BitOperations::IsPow2(int32_t(-4)));
}
TEST(BitOperationsTests, RoundUpToPowerOf2_Zero_ReturnsZero) {
    EXPECT_EQ(BitOperations::RoundUpToPowerOf2(uint32_t(0)), 0u);
    EXPECT_EQ(BitOperations::RoundUpToPowerOf2(uint64_t(0)), 0u);
}
TEST(BitOperationsTests, RoundUpToPowerOf2_AlreadyPow2_Unchanged) {
    EXPECT_EQ(BitOperations::RoundUpToPowerOf2(uint32_t(8)), 8u);
}
TEST(BitOperationsTests, RoundUpToPowerOf2_NotPow2_RoundsUp) {
    EXPECT_EQ(BitOperations::RoundUpToPowerOf2(uint32_t(5)), 8u);
    EXPECT_EQ(BitOperations::RoundUpToPowerOf2(uint32_t(9)), 16u);
}
TEST(BitOperationsTests, LeadingZeroCount_32) {
    EXPECT_EQ(BitOperations::LeadingZeroCount(uint32_t(0)), 32);
    EXPECT_EQ(BitOperations::LeadingZeroCount(uint32_t(1)), 31);
    EXPECT_EQ(BitOperations::LeadingZeroCount(uint32_t(0x80000000u)), 0);
}
TEST(BitOperationsTests, Log2_Values) {
    EXPECT_EQ(BitOperations::Log2(uint32_t(1)),  0);
    EXPECT_EQ(BitOperations::Log2(uint32_t(2)),  1);
    EXPECT_EQ(BitOperations::Log2(uint32_t(8)),  3);
    EXPECT_EQ(BitOperations::Log2(uint32_t(255)), 7);
}
TEST(BitOperationsTests, PopCount_32) {
    EXPECT_EQ(BitOperations::PopCount(uint32_t(0)), 0);
    EXPECT_EQ(BitOperations::PopCount(uint32_t(0xFFFFFFFFu)), 32);
    EXPECT_EQ(BitOperations::PopCount(uint32_t(0b10110101)), 5);
}
TEST(BitOperationsTests, TrailingZeroCount_32) {
    EXPECT_EQ(BitOperations::TrailingZeroCount(uint32_t(0)), 32);
    EXPECT_EQ(BitOperations::TrailingZeroCount(uint32_t(1)), 0);
    EXPECT_EQ(BitOperations::TrailingZeroCount(uint32_t(8)), 3);
}
TEST(BitOperationsTests, RotateLeft_32) {
    uint32_t val = 0x80000000u;
    EXPECT_EQ(BitOperations::RotateLeft(val, 1), 1u);
}
TEST(BitOperationsTests, RotateRight_32) {
    uint32_t val = 1u;
    EXPECT_EQ(BitOperations::RotateRight(val, 1), 0x80000000u);
}
TEST(BitOperationsTests, ReverseBits_Zero_One) {
    EXPECT_EQ(BitOperations::ReverseBits(uint32_t(0)), 0u);
    EXPECT_EQ(BitOperations::ReverseBits(uint32_t(1)), 0x80000000u);
    EXPECT_EQ(BitOperations::ReverseBits(uint32_t(0x80000000u)), 1u);
}

// ---------------------------------------------------------------------------
// MathF parity: Acosh/Asinh/Atanh, CopySign, BitIncrement/BitDecrement,
//               FusedMultiplyAdd, Round(float, digits)
// ---------------------------------------------------------------------------

TEST(MathFTests, Acosh_One)       { EXPECT_NEAR(MathF::Acosh(1.0f), 0.0f, 1e-6f); }
TEST(MathFTests, Acosh_RoundTrip) { EXPECT_NEAR(MathF::Acosh(std::cosh(2.0f)), 2.0f, 1e-5f); }
TEST(MathFTests, Asinh_Zero)      { EXPECT_NEAR(MathF::Asinh(0.0f), 0.0f, 1e-6f); }
TEST(MathFTests, Asinh_RoundTrip) { EXPECT_NEAR(MathF::Asinh(std::sinh(1.0f)), 1.0f, 1e-5f); }
TEST(MathFTests, Atanh_Zero)      { EXPECT_NEAR(MathF::Atanh(0.0f), 0.0f, 1e-6f); }
TEST(MathFTests, Atanh_RoundTrip) { EXPECT_NEAR(MathF::Atanh(std::tanh(0.5f)), 0.5f, 1e-5f); }

TEST(MathFTests, CopySign_PosToNeg) { EXPECT_EQ(MathF::CopySign(3.0f, -1.0f), -3.0f); }
TEST(MathFTests, CopySign_NegToPos) { EXPECT_EQ(MathF::CopySign(-5.0f, 1.0f),  5.0f); }

TEST(MathFTests, BitIncrement_Greater) {
    float x = 1.0f;
    EXPECT_GT(MathF::BitIncrement(x), x);
}
TEST(MathFTests, BitDecrement_Less) {
    float x = 1.0f;
    EXPECT_LT(MathF::BitDecrement(x), x);
}
TEST(MathFTests, FusedMultiplyAdd_Basic) {
    EXPECT_NEAR(MathF::FusedMultiplyAdd(2.0f, 3.0f, 4.0f), 10.0f, 1e-5f);
}
TEST(MathFTests, Round_TwoDigits)  { EXPECT_NEAR(MathF::Round(3.14159f, 2), 3.14f, 1e-5f); }
TEST(MathFTests, Round_ZeroDigits) { EXPECT_NEAR(MathF::Round(2.7f, 0),     3.0f,  1e-5f); }
TEST(MathFTests, Round_MidpointRounding_AwayFromZero) {
    EXPECT_FLOAT_EQ(MathF::Round(2.5f, MidpointRounding::AwayFromZero), 3.0f);
    EXPECT_FLOAT_EQ(MathF::Round(-2.5f, MidpointRounding::AwayFromZero), -3.0f);
}
TEST(MathFTests, Round_MidpointRounding_ToEven) {
    EXPECT_FLOAT_EQ(MathF::Round(2.5f, MidpointRounding::ToEven), 2.0f);
    EXPECT_FLOAT_EQ(MathF::Round(3.5f, MidpointRounding::ToEven), 4.0f);
}
TEST(MathFTests, Round_MidpointRounding_ToZero) {
    EXPECT_FLOAT_EQ(MathF::Round(2.9f, MidpointRounding::ToZero), 2.0f);
    EXPECT_FLOAT_EQ(MathF::Round(-2.9f, MidpointRounding::ToZero), -2.0f);
}
TEST(MathFTests, Round_Digits_MidpointRounding) {
    EXPECT_NEAR(MathF::Round(2.345f, 2, MidpointRounding::AwayFromZero), 2.35f, 1e-5f);
}

// Regression tests for ticket 307: Round(float, int[, MidpointRounding]) never validated
// `digits`, silently accepting any value instead of throwing ArgumentOutOfRangeException for
// digits outside [0, 6] (real .NET's MathF.cs maxRoundingDigits), and never guarded large
// magnitudes -- for |x| >= 1e8 (singleRoundLimit), real .NET returns x completely unchanged
// instead of attempting the multiply/round/divide dance, which for x near FLT_MAX overflows
// to infinity.
TEST(MathFTests, Round_DigitsTooLarge_Throws) {
    EXPECT_THROW(MathF::Round(1.2345f, 7), System::ArgumentOutOfRangeException);
}
TEST(MathFTests, Round_NegativeDigits_Throws) {
    EXPECT_THROW(MathF::Round(1.2345f, -1), System::ArgumentOutOfRangeException);
}
TEST(MathFTests, Round_LargeMagnitude_ReturnsUnchanged) {
    float huge = 3.0e38f;
    EXPECT_EQ(MathF::Round(huge, 6), huge);
}
TEST(MathFTests, Round_JustBelowLimit_StillRounds) {
    // Sanity check the fix didn't over-widen the guard: values still below 1e8 round normally.
    EXPECT_NEAR(MathF::Round(1.23456f, 2), 1.23f, 1e-5f);
}

// ---------------------------------------------------------------------------
// IsFinite, IsNormal, IsSubnormal, IsNegative
// ---------------------------------------------------------------------------
TEST(MathFTests, IsFinite_Normal)    { EXPECT_TRUE(MathF::IsFinite(1.0f)); }
TEST(MathFTests, IsFinite_Inf)       { EXPECT_FALSE(MathF::IsFinite(std::numeric_limits<float>::infinity())); }
TEST(MathFTests, IsFinite_NaN)       { EXPECT_FALSE(MathF::IsFinite(std::numeric_limits<float>::quiet_NaN())); }
TEST(MathFTests, IsNormal_Normal)    { EXPECT_TRUE(MathF::IsNormal(1.0f)); }
TEST(MathFTests, IsNormal_Zero)      { EXPECT_FALSE(MathF::IsNormal(0.0f)); }
TEST(MathFTests, IsNormal_Subnormal) { EXPECT_FALSE(MathF::IsNormal(std::numeric_limits<float>::denorm_min())); }
TEST(MathFTests, IsSubnormal_Yes)    { EXPECT_TRUE(MathF::IsSubnormal(std::numeric_limits<float>::denorm_min())); }
TEST(MathFTests, IsSubnormal_Normal) { EXPECT_FALSE(MathF::IsSubnormal(1.0f)); }
TEST(MathFTests, IsNegative_Neg)     { EXPECT_TRUE(MathF::IsNegative(-1.0f)); }
TEST(MathFTests, IsNegative_Pos)     { EXPECT_FALSE(MathF::IsNegative(1.0f)); }
TEST(MathFTests, IsNegative_NegZero) { EXPECT_TRUE(MathF::IsNegative(-0.0f)); }

// ---------------------------------------------------------------------------
// ScaleB
// ---------------------------------------------------------------------------
TEST(MathFTests, ScaleB_Basic)    { EXPECT_NEAR(MathF::ScaleB(1.0f, 3),  8.0f, 1e-5f); }
TEST(MathFTests, ScaleB_Negative) { EXPECT_NEAR(MathF::ScaleB(8.0f, -1), 4.0f, 1e-5f); }

// ---------------------------------------------------------------------------
// MaxMagnitude / MinMagnitude
// ---------------------------------------------------------------------------
TEST(MathFTests, MaxMagnitude_LargerFirst)   { EXPECT_EQ(MathF::MaxMagnitude(5.0f, 3.0f),  5.0f); }
TEST(MathFTests, MaxMagnitude_LargerSecond)  { EXPECT_EQ(MathF::MaxMagnitude(2.0f, -7.0f), -7.0f); }
TEST(MathFTests, MaxMagnitude_Equal_RetPos)  { EXPECT_EQ(MathF::MaxMagnitude(3.0f, -3.0f), 3.0f); }
TEST(MathFTests, MinMagnitude_SmallerFirst)  { EXPECT_EQ(MathF::MinMagnitude(2.0f, 5.0f),   2.0f); }
TEST(MathFTests, MinMagnitude_SmallerSecond) { EXPECT_EQ(MathF::MinMagnitude(-7.0f, 2.0f),  2.0f); }
TEST(MathFTests, MinMagnitude_Equal_RetNeg)  { EXPECT_EQ(MathF::MinMagnitude(-3.0f, 3.0f), -3.0f); }

TEST(MathFTests, ILogB_One)    { EXPECT_EQ(MathF::ILogB(1.0f), 0); }
TEST(MathFTests, ILogB_Two)    { EXPECT_EQ(MathF::ILogB(2.0f), 1); }
TEST(MathFTests, ILogB_Eight)  { EXPECT_EQ(MathF::ILogB(8.0f), 3); }

// Regression test for ticket 307: std::ilogb's NaN sentinel (FP_ILOGBNAN) is
// implementation-defined and on glibc is actually INT_MIN, not INT_MAX -- diverging from real
// .NET's MathF.ILogB(NaN), which always returns int.MaxValue (same as infinity).
TEST(MathFTests, ILogB_NaN_ReturnsIntMax) {
    EXPECT_EQ(MathF::ILogB(std::numeric_limits<float>::quiet_NaN()), std::numeric_limits<int>::max());
}
TEST(MathFTests, ILogB_Infinity_ReturnsIntMax) {
    EXPECT_EQ(MathF::ILogB(std::numeric_limits<float>::infinity()), std::numeric_limits<int>::max());
}
TEST(MathFTests, ILogB_Zero_ReturnsIntMin) {
    EXPECT_EQ(MathF::ILogB(0.0f), std::numeric_limits<int>::min());
}

TEST(MathFTests, ReciprocalEstimate_One)  { EXPECT_FLOAT_EQ(MathF::ReciprocalEstimate(1.0f),  1.0f); }
TEST(MathFTests, ReciprocalEstimate_Two)  { EXPECT_FLOAT_EQ(MathF::ReciprocalEstimate(2.0f),  0.5f); }
TEST(MathFTests, ReciprocalEstimate_Four) { EXPECT_FLOAT_EQ(MathF::ReciprocalEstimate(4.0f), 0.25f); }

TEST(MathFTests, ReciprocalSqrtEstimate_One)  { EXPECT_FLOAT_EQ(MathF::ReciprocalSqrtEstimate(1.0f), 1.0f); }
TEST(MathFTests, ReciprocalSqrtEstimate_Four) { EXPECT_FLOAT_EQ(MathF::ReciprocalSqrtEstimate(4.0f), 0.5f); }
