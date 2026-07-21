// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Single.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArithmeticException.hpp"
#include "System/FormatException.hpp"
#include <cmath>

using System::Single;
using System::ArithmeticException;

// Constants
TEST(SingleTest, MaxValue_Positive)      { EXPECT_GT(Single::MaxValue, 0.0f); }
TEST(SingleTest, MinValue_Negative)      { EXPECT_LT(Single::MinValue, 0.0f); }
TEST(SingleTest, Epsilon_Positive)       { EXPECT_GT(Single::Epsilon, 0.0f); }
TEST(SingleTest, NaN_IsNaN)             { EXPECT_TRUE(std::isnan(Single::NaN)); }
TEST(SingleTest, PositiveInfinity)       { EXPECT_TRUE(std::isinf(Single::PositiveInfinity)); EXPECT_GT(Single::PositiveInfinity, 0.0f); }
TEST(SingleTest, NegativeInfinity)       { EXPECT_TRUE(std::isinf(Single::NegativeInfinity)); EXPECT_LT(Single::NegativeInfinity, 0.0f); }
TEST(SingleTest, Pi_Approx)             { EXPECT_NEAR(Single::Pi, 3.14159f, 0.0001f); }
TEST(SingleTest, E_Approx)              { EXPECT_NEAR(Single::E, 2.71828f, 0.0001f); }
TEST(SingleTest, Tau_Is2Pi)             { EXPECT_NEAR(Single::Tau, 2.0f * Single::Pi, 0.0001f); }

// Classification
TEST(SingleTest, IsNaN_True)            { EXPECT_TRUE(Single::IsNaN(Single::NaN)); }
TEST(SingleTest, IsNaN_False)           { EXPECT_FALSE(Single::IsNaN(1.0f)); }
TEST(SingleTest, IsInfinity_PosInf)     { EXPECT_TRUE(Single::IsInfinity(Single::PositiveInfinity)); }
TEST(SingleTest, IsInfinity_NegInf)     { EXPECT_TRUE(Single::IsInfinity(Single::NegativeInfinity)); }
TEST(SingleTest, IsInfinity_False)      { EXPECT_FALSE(Single::IsInfinity(1.0f)); }
TEST(SingleTest, IsPositiveInfinity)    { EXPECT_TRUE(Single::IsPositiveInfinity(Single::PositiveInfinity)); }
TEST(SingleTest, IsNegativeInfinity)    { EXPECT_TRUE(Single::IsNegativeInfinity(Single::NegativeInfinity)); }
TEST(SingleTest, IsFinite_True)         { EXPECT_TRUE(Single::IsFinite(1.0f)); }
TEST(SingleTest, IsFinite_Inf_False)    { EXPECT_FALSE(Single::IsFinite(Single::PositiveInfinity)); }
TEST(SingleTest, IsNormal_True)         { EXPECT_TRUE(Single::IsNormal(1.0f)); }
TEST(SingleTest, IsNegative_Neg)        { EXPECT_TRUE(Single::IsNegative(-1.0f)); }
TEST(SingleTest, IsNegative_Pos_False)  { EXPECT_FALSE(Single::IsNegative(1.0f)); }
TEST(SingleTest, IsPositive_Pos)        { EXPECT_TRUE(Single::IsPositive(1.0f)); }
TEST(SingleTest, IsInteger_True)        { EXPECT_TRUE(Single::IsInteger(3.0f)); }
TEST(SingleTest, IsInteger_False)       { EXPECT_FALSE(Single::IsInteger(3.5f)); }
TEST(SingleTest, IsEvenInteger)         { EXPECT_TRUE(Single::IsEvenInteger(4.0f)); EXPECT_FALSE(Single::IsEvenInteger(3.0f)); }
TEST(SingleTest, IsOddInteger)          { EXPECT_TRUE(Single::IsOddInteger(3.0f)); EXPECT_FALSE(Single::IsOddInteger(4.0f)); }
TEST(SingleTest, IsRealNumber_True)     { EXPECT_TRUE(Single::IsRealNumber(1.0f)); EXPECT_TRUE(Single::IsRealNumber(Single::PositiveInfinity)); }
TEST(SingleTest, IsRealNumber_NaN_False){ EXPECT_FALSE(Single::IsRealNumber(Single::NaN)); }
TEST(SingleTest, IsPow2_True)           { EXPECT_TRUE(Single::IsPow2(8.0f)); }
TEST(SingleTest, IsPow2_False)          { EXPECT_FALSE(Single::IsPow2(6.0f)); EXPECT_FALSE(Single::IsPow2(0.0f)); EXPECT_FALSE(Single::IsPow2(-8.0f)); }

// Arithmetic
TEST(SingleTest, Abs_Negative)          { EXPECT_EQ(Single::Abs(-5.0f), 5.0f); }
TEST(SingleTest, Abs_Positive)          { EXPECT_EQ(Single::Abs(5.0f), 5.0f); }
TEST(SingleTest, Sign_Positive)         { EXPECT_EQ(Single::Sign(3.0f), 1); }
TEST(SingleTest, Sign_Negative)         { EXPECT_EQ(Single::Sign(-3.0f), -1); }
TEST(SingleTest, Sign_Zero)             { EXPECT_EQ(Single::Sign(0.0f), 0); }
TEST(SingleTest, Sign_NaN_Throws)       { EXPECT_THROW(Single::Sign(Single::NaN), ArithmeticException); }
TEST(SingleTest, Clamp_Within)          { EXPECT_EQ(Single::Clamp(5.0f, 0.0f, 10.0f), 5.0f); }
TEST(SingleTest, Clamp_Below)           { EXPECT_EQ(Single::Clamp(-1.0f, 0.0f, 10.0f), 0.0f); }
TEST(SingleTest, Clamp_Above)           { EXPECT_EQ(Single::Clamp(11.0f, 0.0f, 10.0f), 10.0f); }
TEST(SingleTest, Clamp_MinGreaterThanMax_Throws) { EXPECT_THROW(Single::Clamp(1.0f, 10.0f, 0.0f), System::ArgumentException); }
TEST(SingleTest, Max)                   { EXPECT_EQ(Single::Max(3.0f, 7.0f), 7.0f); }
TEST(SingleTest, Min)                   { EXPECT_EQ(Single::Min(3.0f, 7.0f), 3.0f); }
TEST(SingleTest, Max_PropagatesNaN)     { EXPECT_TRUE(Single::IsNaN(Single::Max(Single::NaN, 1.0f))); EXPECT_TRUE(Single::IsNaN(Single::Max(1.0f, Single::NaN))); }
TEST(SingleTest, Min_PropagatesNaN)     { EXPECT_TRUE(Single::IsNaN(Single::Min(Single::NaN, 1.0f))); EXPECT_TRUE(Single::IsNaN(Single::Min(1.0f, Single::NaN))); }
TEST(SingleTest, Max_PositiveZeroBeatsNegativeZero) { EXPECT_TRUE(Single::IsNegative(Single::Max(-0.0f, -0.0f))); EXPECT_FALSE(Single::IsNegative(Single::Max(0.0f, -0.0f))); }
TEST(SingleTest, MaxMagnitude_TieBreaksToPositive)  { EXPECT_EQ(Single::MaxMagnitude(-0.0f, 0.0f), 0.0f); EXPECT_FALSE(Single::IsNegative(Single::MaxMagnitude(-0.0f, 0.0f))); }
TEST(SingleTest, MinMagnitude_TieBreaksToNegative)  { EXPECT_TRUE(Single::IsNegative(Single::MinMagnitude(0.0f, -0.0f))); }
TEST(SingleTest, MaxMagnitude_Basic)    { EXPECT_EQ(Single::MaxMagnitude(-7.0f, 3.0f), -7.0f); }
TEST(SingleTest, MinMagnitude_Basic)    { EXPECT_EQ(Single::MinMagnitude(-7.0f, 3.0f), 3.0f); }
TEST(SingleTest, CopySign_Neg)          { EXPECT_EQ(Single::CopySign(3.0f, -1.0f), -3.0f); }
TEST(SingleTest, CopySign_Pos)          { EXPECT_EQ(Single::CopySign(-3.0f, 1.0f), 3.0f); }

// Rounding
TEST(SingleTest, Ceiling)               { EXPECT_EQ(Single::Ceiling(1.2f), 2.0f); }
TEST(SingleTest, Floor)                 { EXPECT_EQ(Single::Floor(1.9f), 1.0f); }
TEST(SingleTest, Truncate)              { EXPECT_EQ(Single::Truncate(1.9f), 1.0f); EXPECT_EQ(Single::Truncate(-1.9f), -1.0f); }
TEST(SingleTest, Round_Half)            { EXPECT_EQ(Single::Round(0.5f), 0.0f); } // ties to even
TEST(SingleTest, Round_Digits)          { EXPECT_NEAR(Single::Round(1.456f, 2), 1.46f, 0.001f); }

// Exponential / log
TEST(SingleTest, Exp)                   { EXPECT_NEAR(Single::Exp(1.0f), Single::E, 0.001f); }
TEST(SingleTest, Log_E)                 { EXPECT_NEAR(Single::Log(Single::E), 1.0f, 0.001f); }
TEST(SingleTest, Log2_8)                { EXPECT_NEAR(Single::Log2(8.0f), 3.0f, 0.001f); }
TEST(SingleTest, Log10_100)             { EXPECT_NEAR(Single::Log10(100.0f), 2.0f, 0.001f); }
TEST(SingleTest, Pow)                   { EXPECT_NEAR(Single::Pow(2.0f, 10.0f), 1024.0f, 0.01f); }

// Root
TEST(SingleTest, Sqrt)                  { EXPECT_NEAR(Single::Sqrt(9.0f), 3.0f, 0.001f); }
TEST(SingleTest, Cbrt)                  { EXPECT_NEAR(Single::Cbrt(27.0f), 3.0f, 0.001f); }
TEST(SingleTest, Hypot)                 { EXPECT_NEAR(Single::Hypot(3.0f, 4.0f), 5.0f, 0.001f); }
TEST(SingleTest, RootN_PositiveBase)    { EXPECT_NEAR(Single::RootN(8.0f, 3), 2.0f, 0.001f); }
TEST(SingleTest, RootN_NegativeBase_OddRoot) { EXPECT_NEAR(Single::RootN(-8.0f, 3), -2.0f, 0.001f); }
TEST(SingleTest, RootN_NegativeBase_EvenRoot_NaN) { EXPECT_TRUE(Single::IsNaN(Single::RootN(-8.0f, 4))); }
TEST(SingleTest, RootN_NegativeN)       { EXPECT_NEAR(Single::RootN(8.0f, -3), 0.5f, 0.001f); }

// Trig
TEST(SingleTest, Sin)                   { EXPECT_NEAR(Single::Sin(0.0f), 0.0f, 0.001f); }
TEST(SingleTest, Cos)                   { EXPECT_NEAR(Single::Cos(0.0f), 1.0f, 0.001f); }
TEST(SingleTest, Atan2)                 { EXPECT_NEAR(Single::Atan2(1.0f, 1.0f), Single::Pi / 4.0f, 0.001f); }
TEST(SingleTest, SinCos)               {
    auto r = Single::SinCos(0.0f);
    EXPECT_NEAR(r.Sin, 0.0f, 0.001f);
    EXPECT_NEAR(r.Cos, 1.0f, 0.001f);
}

// Angle conversion
TEST(SingleTest, DegreesToRadians)      { EXPECT_NEAR(Single::DegreesToRadians(180.0f), Single::Pi, 0.001f); }
TEST(SingleTest, RadiansToDegrees)      { EXPECT_NEAR(Single::RadiansToDegrees(Single::Pi), 180.0f, 0.01f); }

// IEEE utilities
TEST(SingleTest, FusedMultiplyAdd)      { EXPECT_NEAR(Single::FusedMultiplyAdd(2.0f, 3.0f, 1.0f), 7.0f, 0.001f); }
TEST(SingleTest, ScaleB)               { EXPECT_EQ(Single::ScaleB(1.0f, 3), 8.0f); }
TEST(SingleTest, ILogB)                { EXPECT_EQ(Single::ILogB(8.0f), 3); }
TEST(SingleTest, BitDecrement)          { EXPECT_LT(Single::BitDecrement(1.0f), 1.0f); }
TEST(SingleTest, BitIncrement)          { EXPECT_GT(Single::BitIncrement(1.0f), 1.0f); }
TEST(SingleTest, ReciprocalEstimate)    { EXPECT_NEAR(Single::ReciprocalEstimate(4.0f), 0.25f, 0.001f); }
TEST(SingleTest, ReciprocalSqrtEstimate){ EXPECT_NEAR(Single::ReciprocalSqrtEstimate(4.0f), 0.5f, 0.001f); }

// Comparison / hash
TEST(SingleTest, CompareTo_Less)        { EXPECT_LT(Single::CompareTo(1.0f, 2.0f), 0); }
TEST(SingleTest, CompareTo_Equal)       { EXPECT_EQ(Single::CompareTo(1.0f, 1.0f), 0); }
TEST(SingleTest, CompareTo_Greater)     { EXPECT_GT(Single::CompareTo(2.0f, 1.0f), 0); }
TEST(SingleTest, Equals_True)           { EXPECT_TRUE(Single::Equals(1.0f, 1.0f)); }
TEST(SingleTest, Equals_False)          { EXPECT_FALSE(Single::Equals(1.0f, 2.0f)); }
TEST(SingleTest, Equals_NaN_EqualsNaN)  { EXPECT_TRUE(Single::Equals(Single::NaN, Single::NaN)); }
TEST(SingleTest, GetHashCode_AllNaNsMatch) {
    float nan2 = -Single::NaN;
    EXPECT_EQ(Single::GetHashCode(Single::NaN), Single::GetHashCode(nan2));
}
TEST(SingleTest, GetHashCode_BothZerosMatch) { EXPECT_EQ(Single::GetHashCode(0.0f), Single::GetHashCode(-0.0f)); }

// Parse / ToString
TEST(SingleTest, Parse_Normal)          { EXPECT_NEAR(Single::Parse("3.14"), 3.14f, 0.001f); }
TEST(SingleTest, Parse_NaN)             { EXPECT_TRUE(std::isnan(Single::Parse("NaN"))); }
TEST(SingleTest, Parse_PosInf)          { EXPECT_TRUE(std::isinf(Single::Parse("Infinity"))); }
TEST(SingleTest, Parse_NegInf)          { EXPECT_TRUE(std::isinf(Single::Parse("-Infinity"))); }
TEST(SingleTest, Parse_Invalid_Throws)  { EXPECT_THROW(Single::Parse("abc"), System::FormatException); }
TEST(SingleTest, TryParse_Valid)        { float r = 0; EXPECT_TRUE(Single::TryParse("1.5", r)); EXPECT_NEAR(r, 1.5f, 0.001f); }
TEST(SingleTest, TryParse_Invalid)      { float r = 0; EXPECT_FALSE(Single::TryParse("xyz", r)); }

// Regression tests for a code-audit finding (ticket 249, same bug class as ticket 245's
// System::Double fix): Parse/TryParse only special-cased the exact-cased strings
// "NaN"/"Infinity"/"-Infinity" and fell through to std::from_chars for everything else, which
// per the C++ standard still recognizes "inf"/"infinity"/"nan"/"nan(n-char-seq)"
// case-insensitively regardless of chars_format -- so abbreviated/miscased spellings like "inf"
// or "nan(123)" were silently ACCEPTED even though real .NET's float.Parse throws
// FormatException for all of them (verified against Number.Parsing.cs's TryParseFloat, which
// only recognizes Infinity/+Infinity/-Infinity/NaN/+NaN/-NaN case-insensitively).
TEST(SingleTest, TryParse_AbbreviatedInfinitySpelling_ReturnsFalse) {
    float r = 0;
    EXPECT_FALSE(Single::TryParse("inf", r));
    EXPECT_FALSE(Single::TryParse("-inf", r));
    EXPECT_FALSE(Single::TryParse("INF", r));
}

TEST(SingleTest, TryParse_NanWithCharSequence_ReturnsFalse) {
    float r = 0;
    EXPECT_FALSE(Single::TryParse("nan(123)", r));
}

TEST(SingleTest, TryParse_CaseInsensitiveCanonicalSpellings_ReturnsTrue) {
    float r = 0;
    EXPECT_TRUE(Single::TryParse("infinity", r));
    EXPECT_TRUE(std::isinf(r) && r > 0);
    EXPECT_TRUE(Single::TryParse("nan", r));
    EXPECT_TRUE(std::isnan(r));
    EXPECT_TRUE(Single::TryParse("+Infinity", r));
    EXPECT_TRUE(std::isinf(r) && r > 0);
}

TEST(SingleTest, Parse_AbbreviatedInfinitySpelling_Throws) {
    EXPECT_THROW(Single::Parse("inf"), System::FormatException);
    EXPECT_THROW(Single::Parse("nan(123)"), System::FormatException);
}
TEST(SingleTest, ToString_Normal)       { EXPECT_EQ(Single::ToString(Single::NaN), "NaN"); }
TEST(SingleTest, ToString_Format_F2)    { EXPECT_EQ(Single::ToString(3.14159f, "F2"), "3.14"); }
