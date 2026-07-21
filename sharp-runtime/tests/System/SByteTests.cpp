// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/OverflowException.hpp"
#include "System/SByte.hpp"

using System::SByte;
using SharpRuntime::sbytecs;

TEST(SByteTest, MaxValue) { EXPECT_EQ(SByte::MaxValue, sbytecs(127)); }
TEST(SByteTest, MinValue) { EXPECT_EQ(SByte::MinValue, sbytecs(-128)); }

TEST(SByteTest, Parse_Valid) { EXPECT_EQ(SByte::Parse("42"), sbytecs(42)); }
TEST(SByteTest, Parse_Negative) { EXPECT_EQ(SByte::Parse("-10"), sbytecs(-10)); }
TEST(SByteTest, Parse_OutOfRange_Throws) { EXPECT_THROW(SByte::Parse("200"), System::OverflowException); }
TEST(SByteTest, Parse_TrailingGarbage_Throws) {
    // std::stoi(s) without capturing the parse-end position previously accepted trailing
    // garbage silently (e.g. "5abc" parsed as 5).
    EXPECT_THROW(SByte::Parse("5abc"), System::FormatException);
}
TEST(SByteTest, Parse_NegativeZero_IsZero) {
    // SByte is signed, so "-0" legitimately parses to 0 (unlike the unsigned integer types).
    EXPECT_EQ(SByte::Parse("-0"), sbytecs(0));
}

TEST(SByteTest, TryParse_Valid) {
    sbytecs r = 0;
    EXPECT_TRUE(SByte::TryParse("99", r));
    EXPECT_EQ(r, sbytecs(99));
}
TEST(SByteTest, TryParse_Invalid) {
    sbytecs r = 0;
    EXPECT_FALSE(SByte::TryParse("abc", r));
}

TEST(SByteTest, ToString_Positive) { EXPECT_EQ(SByte::ToString(sbytecs(7)), "7"); }
TEST(SByteTest, ToString_Negative) { EXPECT_EQ(SByte::ToString(sbytecs(-5)), "-5"); }
TEST(SByteTest, ToString_Hex) { EXPECT_EQ(SByte::ToString(sbytecs(255), "X2"), "FF"); }
TEST(SByteTest, ToString_MalformedWidth_ThrowsFormatException) {
    EXPECT_THROW(SByte::ToString(sbytecs(5), "Xz"), System::FormatException);
    EXPECT_THROW(SByte::ToString(sbytecs(5), "X99999999999999999999"), System::FormatException);
}

TEST(SByteTest, Abs_Positive) { EXPECT_EQ(SByte::Abs(sbytecs(5)), sbytecs(5)); }
TEST(SByteTest, Abs_Negative) { EXPECT_EQ(SByte::Abs(sbytecs(-5)), sbytecs(5)); }
TEST(SByteTest, Abs_Zero) { EXPECT_EQ(SByte::Abs(sbytecs(0)), sbytecs(0)); }
TEST(SByteTest, Abs_MinValue_Throws) { EXPECT_THROW(SByte::Abs(SByte::MinValue), System::OverflowException); }

TEST(SByteTest, CopySign_PositiveSign) { EXPECT_EQ(SByte::CopySign(sbytecs(-3), sbytecs(1)), sbytecs(3)); }
TEST(SByteTest, CopySign_NegativeSign) { EXPECT_EQ(SByte::CopySign(sbytecs(3), sbytecs(-1)), sbytecs(-3)); }
TEST(SByteTest, CopySign_MinValue_NonNegativeSign_Throws) {
    EXPECT_THROW(SByte::CopySign(SByte::MinValue, sbytecs(1)), System::OverflowException);
}
TEST(SByteTest, CopySign_MinValue_NegativeSign_ReturnsMinValue) {
    // Matches .NET: the negation double-wraps back to MinValue without throwing
    // when sign < 0 (verified against SByte.cs - the overflow check only runs in
    // the sign >= 0 branch).
    EXPECT_EQ(SByte::CopySign(SByte::MinValue, sbytecs(-1)), SByte::MinValue);
}

TEST(SByteTest, IsNegative_True)  { EXPECT_TRUE(SByte::IsNegative(sbytecs(-1))); }
TEST(SByteTest, IsNegative_False) { EXPECT_FALSE(SByte::IsNegative(sbytecs(0))); }
TEST(SByteTest, IsPositive_True)  { EXPECT_TRUE(SByte::IsPositive(sbytecs(1))); }
TEST(SByteTest, IsPositive_False) { EXPECT_FALSE(SByte::IsPositive(sbytecs(0))); }

TEST(SByteTest, Log10_One)     { EXPECT_EQ(SByte::Log10(sbytecs(1)), sbytecs(0)); }
TEST(SByteTest, Log10_Ten)     { EXPECT_EQ(SByte::Log10(sbytecs(10)), sbytecs(1)); }
// .NET SByte.Log10(0) returns 0 (uint.Log10 convention), it does not throw; only a
// negative value throws (ArgumentOutOfRangeException, per SByte.cs).
TEST(SByteTest, Log10_Zero_IsZero) { EXPECT_EQ(SByte::Log10(sbytecs(0)), sbytecs(0)); }
TEST(SByteTest, Log10_Negative_Throws) { EXPECT_THROW(SByte::Log10(sbytecs(-1)), System::ArgumentOutOfRangeException); }

TEST(SByteTest, MaxMagnitude_Larger)   { EXPECT_EQ(SByte::MaxMagnitude(sbytecs(-10), sbytecs(5)), sbytecs(-10)); }
TEST(SByteTest, MinMagnitude_Smaller)  { EXPECT_EQ(SByte::MinMagnitude(sbytecs(-10), sbytecs(5)), sbytecs(5)); }

// MaxMagnitude/MinMagnitude with MinValue: MinValue has no representable positive
// magnitude, so .NET special-cases it to always win MaxMagnitude and always lose
// MinMagnitude, regardless of the other operand (same bug class as Int32/Int64
// found earlier this session).
TEST(SByteTest, MaxMagnitude_MinValueAlwaysWins) {
    EXPECT_EQ(SByte::MaxMagnitude(SByte::MinValue, sbytecs(5)), SByte::MinValue);
    EXPECT_EQ(SByte::MaxMagnitude(sbytecs(5), SByte::MinValue), SByte::MinValue);
}
TEST(SByteTest, MinMagnitude_MinValueAlwaysLoses) {
    EXPECT_EQ(SByte::MinMagnitude(SByte::MinValue, sbytecs(5)), sbytecs(5));
    EXPECT_EQ(SByte::MinMagnitude(sbytecs(5), SByte::MinValue), sbytecs(5));
}

TEST(SByteTest, RotateLeft_OneStep) {
    EXPECT_EQ(SByte::RotateLeft(sbytecs(1), 1), sbytecs(2));
}
TEST(SByteTest, RotateLeft_ByEight_Identity) {
    EXPECT_EQ(SByte::RotateLeft(sbytecs(0x55), 8), sbytecs(0x55));
}
TEST(SByteTest, RotateRight_OneStep) {
    EXPECT_EQ(SByte::RotateRight(sbytecs(2), 1), sbytecs(1));
}
TEST(SByteTest, RotateRight_ByEight_Identity) {
    EXPECT_EQ(SByte::RotateRight(sbytecs(0x55), 8), sbytecs(0x55));
}

TEST(SByteTest, LeadingZeroCount_One)  { EXPECT_EQ(SByte::LeadingZeroCount(sbytecs(1)), sbytecs(7)); }
TEST(SByteTest, TrailingZeroCount_Two) { EXPECT_EQ(SByte::TrailingZeroCount(sbytecs(4)), sbytecs(2)); }
TEST(SByteTest, PopCount_Seven)        { EXPECT_EQ(SByte::PopCount(sbytecs(127)), sbytecs(7)); }

// Negative values are reinterpreted as their raw 8-bit pattern (SByte.cs:
// `(sbyte)(BitOperations.LeadingZeroCount((byte)value) - 24)`), not special-cased to 8 --
// e.g. -1's bit pattern is 0xFF, which has zero leading zero bits.
TEST(SByteTest, LeadingZeroCount_NegOne_IsZero) { EXPECT_EQ(SByte::LeadingZeroCount(sbytecs(-1)), sbytecs(0)); }
TEST(SByteTest, LeadingZeroCount_MinValue_IsZero) { EXPECT_EQ(SByte::LeadingZeroCount(SByte::MinValue), sbytecs(0)); }
TEST(SByteTest, LeadingZeroCount_Zero_IsEight) { EXPECT_EQ(SByte::LeadingZeroCount(sbytecs(0)), sbytecs(8)); }
