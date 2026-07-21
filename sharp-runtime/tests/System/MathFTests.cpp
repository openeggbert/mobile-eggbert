// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <cmath>
#include "System/MathF.hpp"

using System::MathF;

TEST(MathFTest, Constants) {
    EXPECT_NEAR(MathF::E,   2.71828f, 1e-4f);
    EXPECT_NEAR(MathF::PI,  3.14159f, 1e-4f);
    EXPECT_NEAR(MathF::Tau, 6.28318f, 1e-4f);
}

TEST(MathFTest, Abs) {
    EXPECT_FLOAT_EQ(MathF::Abs(-3.5f), 3.5f);
    EXPECT_FLOAT_EQ(MathF::Abs(3.5f),  3.5f);
    EXPECT_FLOAT_EQ(MathF::Abs(0.0f),  0.0f);
}

TEST(MathFTest, CeilingFloor) {
    EXPECT_FLOAT_EQ(MathF::Ceiling(1.2f), 2.0f);
    EXPECT_FLOAT_EQ(MathF::Floor(1.9f),   1.0f);
}

TEST(MathFTest, Round) {
    // .NET's MathF.Round() default is round-to-even (banker's rounding), not
    // away-from-zero - verified against MathF.cs. 1.5 -> 2 (even) and 2.5 -> 2
    // (even), not 3.
    EXPECT_FLOAT_EQ(MathF::Round(1.5f), 2.0f);
    EXPECT_FLOAT_EQ(MathF::Round(2.5f), 2.0f);
    EXPECT_FLOAT_EQ(MathF::Round(1.456f, 2), 1.46f);
}

TEST(MathFTest, Sqrt) {
    EXPECT_NEAR(MathF::Sqrt(4.0f), 2.0f, 1e-6f);
    EXPECT_NEAR(MathF::Sqrt(9.0f), 3.0f, 1e-6f);
}

TEST(MathFTest, Pow) {
    EXPECT_NEAR(MathF::Pow(2.0f, 10.0f), 1024.0f, 1e-3f);
}

TEST(MathFTest, Log) {
    EXPECT_NEAR(MathF::Log(MathF::E), 1.0f, 1e-5f);
    EXPECT_NEAR(MathF::Log(100.0f, 10.0f), 2.0f, 1e-5f);
    EXPECT_NEAR(MathF::Log2(8.0f),  3.0f, 1e-5f);
    EXPECT_NEAR(MathF::Log10(1000.0f), 3.0f, 1e-5f);
}

TEST(MathFTest, Log_BaseOne_IsAlwaysNaN) {
    // .NET's MathF.Log(x, y) special-cases y == 1 to always return NaN, since a
    // naive log(x)/log(y) would divide by log(1) == 0 and give +-infinity or NaN
    // depending on x - verified against MathF.cs.
    EXPECT_TRUE(std::isnan(MathF::Log(5.0f, 1.0f)));
    EXPECT_TRUE(std::isnan(MathF::Log(1.0f, 1.0f)));
}

TEST(MathFTest, Log_BaseZeroOrInfinity_NaNUnlessXIsOne) {
    EXPECT_TRUE(std::isnan(MathF::Log(5.0f, 0.0f)));
    EXPECT_TRUE(std::isnan(MathF::Log(5.0f, std::numeric_limits<float>::infinity())));
    EXPECT_NEAR(MathF::Log(1.0f, 0.0f), 0.0f, 1e-6f);
}

TEST(MathFTest, Log_NaNInputs_Propagate) {
    float nan = std::numeric_limits<float>::quiet_NaN();
    EXPECT_TRUE(std::isnan(MathF::Log(nan, 2.0f)));
    EXPECT_TRUE(std::isnan(MathF::Log(2.0f, nan)));
}

TEST(MathFTest, Trig) {
    EXPECT_NEAR(MathF::Sin(0.0f), 0.0f, 1e-6f);
    EXPECT_NEAR(MathF::Cos(0.0f), 1.0f, 1e-6f);
    EXPECT_NEAR(MathF::Sin(MathF::PI / 2.0f), 1.0f, 1e-5f);
    EXPECT_NEAR(MathF::Tan(MathF::PI / 4.0f), 1.0f, 1e-5f);
}

TEST(MathFTest, InverseTrig) {
    EXPECT_NEAR(MathF::Asin(1.0f), MathF::PI / 2.0f, 1e-5f);
    EXPECT_NEAR(MathF::Acos(1.0f), 0.0f, 1e-5f);
    EXPECT_NEAR(MathF::Atan(1.0f), MathF::PI / 4.0f, 1e-5f);
    EXPECT_NEAR(MathF::Atan2(1.0f, 1.0f), MathF::PI / 4.0f, 1e-5f);
}

TEST(MathFTest, MinMax) {
    EXPECT_FLOAT_EQ(MathF::Min(2.0f, 5.0f), 2.0f);
    EXPECT_FLOAT_EQ(MathF::Max(2.0f, 5.0f), 5.0f);
}

TEST(MathFTest, MinMax_PropagatesNaN) {
    // .NET's MathF.Max/Min propagate NaN if either argument is NaN (IEEE 754-2019
    // `maximum`/`minimum`), unlike std::fmax/fmin which return the non-NaN side -
    // verified against Math.cs.
    float nan = std::numeric_limits<float>::quiet_NaN();
    EXPECT_TRUE(std::isnan(MathF::Max(nan, 1.0f)));
    EXPECT_TRUE(std::isnan(MathF::Max(1.0f, nan)));
    EXPECT_TRUE(std::isnan(MathF::Min(nan, 1.0f)));
    EXPECT_TRUE(std::isnan(MathF::Min(1.0f, nan)));
}

TEST(MathFTest, MinMax_PositiveZeroBeatsNegativeZero) {
    // Matches .NET: ties between +0 and -0 are broken with +0 > -0.
    float posZero = 0.0f, negZero = -0.0f;
    EXPECT_TRUE(std::signbit(negZero));
    EXPECT_EQ(MathF::Max(posZero, negZero), posZero);
    EXPECT_FALSE(std::signbit(MathF::Max(posZero, negZero)));
    EXPECT_EQ(MathF::Min(posZero, negZero), negZero);
    EXPECT_TRUE(std::signbit(MathF::Min(posZero, negZero)));
}

TEST(MathFTest, Clamp) {
    EXPECT_FLOAT_EQ(MathF::Clamp(5.0f, 0.0f, 3.0f), 3.0f);
    EXPECT_FLOAT_EQ(MathF::Clamp(-1.0f, 0.0f, 3.0f), 0.0f);
    EXPECT_FLOAT_EQ(MathF::Clamp(2.0f, 0.0f, 3.0f), 2.0f);
}

TEST(MathFTest, Sign) {
    EXPECT_EQ(MathF::Sign(-5.0f), -1);
    EXPECT_EQ(MathF::Sign(0.0f),   0);
    EXPECT_EQ(MathF::Sign(5.0f),   1);
}

TEST(MathFTest, Sign_NaN_Throws) {
    // .NET's Math.Sign(float) throws ArithmeticException for NaN - verified
    // against Math.cs. A naive comparison-based Sign() would silently return 0
    // instead, since all relational comparisons against NaN are false.
    EXPECT_THROW(MathF::Sign(std::numeric_limits<float>::quiet_NaN()), System::ArithmeticException);
}

TEST(MathFTest, IsNaNInfinity) {
    EXPECT_TRUE(MathF::IsNaN(std::numeric_limits<float>::quiet_NaN()));
    EXPECT_FALSE(MathF::IsNaN(1.0f));
    EXPECT_TRUE(MathF::IsInfinity(std::numeric_limits<float>::infinity()));
    EXPECT_TRUE(MathF::IsPositiveInfinity(std::numeric_limits<float>::infinity()));
    EXPECT_TRUE(MathF::IsNegativeInfinity(-std::numeric_limits<float>::infinity()));
    EXPECT_TRUE(MathF::IsFinite(1.0f));
    EXPECT_FALSE(MathF::IsFinite(std::numeric_limits<float>::infinity()));
}

TEST(MathFTest, Truncate) {
    EXPECT_FLOAT_EQ(MathF::Truncate(3.9f),  3.0f);
    EXPECT_FLOAT_EQ(MathF::Truncate(-3.9f), -3.0f);
}

TEST(MathFTest, IEEERemainder) {
    EXPECT_NEAR(MathF::IEEERemainder(3.0f, 2.0f), -1.0f, 1e-5f);
}

TEST(MathFTest, SinCos) {
    auto result = MathF::SinCos(0.0f);
    EXPECT_NEAR(result.Sin, 0.0f, 1e-6f);
    EXPECT_NEAR(result.Cos, 1.0f, 1e-6f);

    auto r2 = MathF::SinCos(MathF::PI / 2.0f);
    EXPECT_NEAR(r2.Sin, 1.0f, 1e-5f);
    EXPECT_NEAR(r2.Cos, 0.0f, 1e-5f);
}

TEST(MathFTest, CopySign) {
    EXPECT_FLOAT_EQ(MathF::CopySign(3.0f, -1.0f), -3.0f);
    EXPECT_FLOAT_EQ(MathF::CopySign(-3.0f, 1.0f),  3.0f);
}

TEST(MathFTest, FusedMultiplyAdd) {
    EXPECT_NEAR(MathF::FusedMultiplyAdd(2.0f, 3.0f, 4.0f), 10.0f, 1e-5f);
}

TEST(MathFTest, ScaleB) {
    EXPECT_FLOAT_EQ(MathF::ScaleB(1.0f, 3), 8.0f);
}

TEST(MathFTest, MaxMinMagnitude) {
    EXPECT_FLOAT_EQ(MathF::MaxMagnitude(-5.0f, 3.0f), -5.0f);
    EXPECT_FLOAT_EQ(MathF::MinMagnitude(-5.0f, 3.0f),  3.0f);
}
