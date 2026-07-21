// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Byte.hpp"

using System::Byte;
using SharpRuntime::bytecs;

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

TEST(ByteTests, MaxValue_Is255) { EXPECT_EQ(Byte::MaxValue, bytecs(255)); }
TEST(ByteTests, MinValue_IsZero) { EXPECT_EQ(Byte::MinValue, bytecs(0)); }

// ---------------------------------------------------------------------------
// Parse / TryParse
// ---------------------------------------------------------------------------

TEST(ByteTests, Parse_Zero) { EXPECT_EQ(Byte::Parse("0"), bytecs(0)); }
TEST(ByteTests, Parse_MaxValue) { EXPECT_EQ(Byte::Parse("255"), bytecs(255)); }
TEST(ByteTests, Parse_Midrange) { EXPECT_EQ(Byte::Parse("128"), bytecs(128)); }
TEST(ByteTests, Parse_Negative_Throws) {
    EXPECT_THROW(Byte::Parse("-1"), System::OverflowException);
}
TEST(ByteTests, Parse_TooLarge_Throws) {
    EXPECT_THROW(Byte::Parse("256"), System::OverflowException);
}
TEST(ByteTests, Parse_Invalid_Throws) {
    EXPECT_THROW(Byte::Parse("abc"), System::FormatException);
}
TEST(ByteTests, Parse_NegativeZero_Throws) {
    // Unsigned types reject any leading '-', even "-0" -- std::stoi("-0") returns the
    // literal int 0 (not negative), which previously passed both the v<0 and v>MaxValue
    // checks and silently returned 0 instead of throwing OverflowException.
    EXPECT_THROW(Byte::Parse("-0"), System::OverflowException);
}

TEST(ByteTests, TryParse_Valid_ReturnsTrue) {
    bytecs r = 0;
    EXPECT_TRUE(Byte::TryParse("200", r));
    EXPECT_EQ(r, bytecs(200));
}
TEST(ByteTests, TryParse_Invalid_ReturnsFalse) {
    bytecs r = 0;
    EXPECT_FALSE(Byte::TryParse("xyz", r));
    EXPECT_EQ(r, bytecs(0));
}
TEST(ByteTests, TryParse_Overflow_ReturnsFalse) {
    bytecs r = 0;
    EXPECT_FALSE(Byte::TryParse("256", r));
}
TEST(ByteTests, Parse_TrailingGarbage_Throws) {
    EXPECT_THROW(Byte::Parse("128abc"), System::FormatException);
}
TEST(ByteTests, Parse_TrailingWhitespace_Ok) {
    EXPECT_EQ(Byte::Parse("128 "), bytecs(128));
}
TEST(ByteTests, Parse_LeadingWhitespaceAndSign_Ok) {
    EXPECT_EQ(Byte::Parse("  +42"), bytecs(42));
}
TEST(ByteTests, TryParse_TrailingGarbage_ReturnsFalse) {
    bytecs r = 0;
    EXPECT_FALSE(Byte::TryParse("12 3", r));
    EXPECT_EQ(r, bytecs(0));
}

// ---------------------------------------------------------------------------
// ToString
// ---------------------------------------------------------------------------

TEST(ByteTests, ToString_Zero) { EXPECT_EQ(Byte::ToString(bytecs(0)), "0"); }
TEST(ByteTests, ToString_MaxValue) { EXPECT_EQ(Byte::ToString(bytecs(255)), "255"); }
TEST(ByteTests, ToString_Midrange) { EXPECT_EQ(Byte::ToString(bytecs(42)), "42"); }
TEST(ByteTests, ToString_MalformedWidth_ThrowsFormatException) {
    // std::stoi's raw std::invalid_argument/out_of_range must not escape as-is.
    EXPECT_THROW(Byte::ToString(bytecs(5), "Xz"), System::FormatException);
    EXPECT_THROW(Byte::ToString(bytecs(5), "X99999999999999999999"), System::FormatException);
}

// ---------------------------------------------------------------------------
// CompareTo
// ---------------------------------------------------------------------------

TEST(ByteTests, CompareTo_Equal_ReturnsZero) {
    EXPECT_EQ(Byte::CompareTo(bytecs(5), bytecs(5)), 0);
}
TEST(ByteTests, CompareTo_Less_ReturnsNegative) {
    EXPECT_LT(Byte::CompareTo(bytecs(3), bytecs(5)), 0);
}
TEST(ByteTests, CompareTo_Greater_ReturnsPositive) {
    EXPECT_GT(Byte::CompareTo(bytecs(7), bytecs(5)), 0);
}

// ---------------------------------------------------------------------------
// Equals / GetHashCode
// ---------------------------------------------------------------------------

TEST(ByteTests, Equals_SameValue_True) {
    EXPECT_TRUE(Byte::Equals(bytecs(10), bytecs(10)));
}
TEST(ByteTests, Equals_DiffValue_False) {
    EXPECT_FALSE(Byte::Equals(bytecs(10), bytecs(11)));
}
TEST(ByteTests, GetHashCode_SameValue_SameHash) {
    EXPECT_EQ(Byte::GetHashCode(bytecs(42)), Byte::GetHashCode(bytecs(42)));
}
TEST(ByteTests, GetHashCode_ValueIsHash) {
    EXPECT_EQ(Byte::GetHashCode(bytecs(7)), 7);
}

// ---------------------------------------------------------------------------
// Clamp / Max / Min / Sign
// ---------------------------------------------------------------------------

TEST(ByteTests, Clamp_WithinRange) {
    EXPECT_EQ(Byte::Clamp(bytecs(5), bytecs(0), bytecs(10)), bytecs(5));
}
TEST(ByteTests, Clamp_BelowMin) {
    EXPECT_EQ(Byte::Clamp(bytecs(0), bytecs(3), bytecs(10)), bytecs(3));
}
TEST(ByteTests, Clamp_AboveMax) {
    EXPECT_EQ(Byte::Clamp(bytecs(20), bytecs(0), bytecs(10)), bytecs(10));
}

TEST(ByteTests, Max_ReturnsLarger) {
    EXPECT_EQ(Byte::Max(bytecs(3), bytecs(7)), bytecs(7));
}
TEST(ByteTests, Max_Equal) {
    EXPECT_EQ(Byte::Max(bytecs(5), bytecs(5)), bytecs(5));
}

TEST(ByteTests, Min_ReturnsSmaller) {
    EXPECT_EQ(Byte::Min(bytecs(3), bytecs(7)), bytecs(3));
}
TEST(ByteTests, Min_Equal) {
    EXPECT_EQ(Byte::Min(bytecs(5), bytecs(5)), bytecs(5));
}

TEST(ByteTests, Sign_Zero_ReturnsZero) {
    EXPECT_EQ(Byte::Sign(bytecs(0)), 0);
}
TEST(ByteTests, Sign_Nonzero_ReturnsOne) {
    EXPECT_EQ(Byte::Sign(bytecs(42)), 1);
}
TEST(ByteTests, Sign_MaxValue_ReturnsOne) {
    EXPECT_EQ(Byte::Sign(bytecs(255)), 1);
}

// ---------------------------------------------------------------------------
// DivRem
// ---------------------------------------------------------------------------

TEST(ByteTests, DivRem_ExactDivision) {
    auto [q, r] = Byte::DivRem(bytecs(10), bytecs(2));
    EXPECT_EQ(q, bytecs(5));
    EXPECT_EQ(r, bytecs(0));
}
TEST(ByteTests, DivRem_WithRemainder) {
    auto [q, r] = Byte::DivRem(bytecs(10), bytecs(3));
    EXPECT_EQ(q, bytecs(3));
    EXPECT_EQ(r, bytecs(1));
}
TEST(ByteTests, DivRem_ByZero_ThrowsDivideByZeroException) {
    EXPECT_THROW(Byte::DivRem(bytecs(10), bytecs(0)), System::DivideByZeroException);
}

// ---------------------------------------------------------------------------
// IsEvenInteger / IsOddInteger
// ---------------------------------------------------------------------------

TEST(ByteTests, IsEvenInteger_Zero_True) {
    EXPECT_TRUE(Byte::IsEvenInteger(bytecs(0)));
}
TEST(ByteTests, IsEvenInteger_Two_True) {
    EXPECT_TRUE(Byte::IsEvenInteger(bytecs(2)));
}
TEST(ByteTests, IsEvenInteger_One_False) {
    EXPECT_FALSE(Byte::IsEvenInteger(bytecs(1)));
}
TEST(ByteTests, IsOddInteger_One_True) {
    EXPECT_TRUE(Byte::IsOddInteger(bytecs(1)));
}
TEST(ByteTests, IsOddInteger_Two_False) {
    EXPECT_FALSE(Byte::IsOddInteger(bytecs(2)));
}

// ---------------------------------------------------------------------------
// IsPow2
// ---------------------------------------------------------------------------

TEST(ByteTests, IsPow2_One_True) { EXPECT_TRUE(Byte::IsPow2(bytecs(1))); }
TEST(ByteTests, IsPow2_Two_True) { EXPECT_TRUE(Byte::IsPow2(bytecs(2))); }
TEST(ByteTests, IsPow2_128_True) { EXPECT_TRUE(Byte::IsPow2(bytecs(128))); }
TEST(ByteTests, IsPow2_Zero_False) { EXPECT_FALSE(Byte::IsPow2(bytecs(0))); }
TEST(ByteTests, IsPow2_Three_False) { EXPECT_FALSE(Byte::IsPow2(bytecs(3))); }

// ---------------------------------------------------------------------------
// LeadingZeroCount / TrailingZeroCount / PopCount
// ---------------------------------------------------------------------------

TEST(ByteTests, LeadingZeroCount_Zero_IsEight) {
    EXPECT_EQ(Byte::LeadingZeroCount(bytecs(0)), bytecs(8));
}
TEST(ByteTests, LeadingZeroCount_255_IsZero) {
    EXPECT_EQ(Byte::LeadingZeroCount(bytecs(255)), bytecs(0));
}
TEST(ByteTests, LeadingZeroCount_One_IsSeven) {
    EXPECT_EQ(Byte::LeadingZeroCount(bytecs(1)), bytecs(7));
}
TEST(ByteTests, LeadingZeroCount_128_IsZero) {
    EXPECT_EQ(Byte::LeadingZeroCount(bytecs(128)), bytecs(0));
}

TEST(ByteTests, TrailingZeroCount_Zero_IsEight) {
    EXPECT_EQ(Byte::TrailingZeroCount(bytecs(0)), bytecs(8));
}
TEST(ByteTests, TrailingZeroCount_One_IsZero) {
    EXPECT_EQ(Byte::TrailingZeroCount(bytecs(1)), bytecs(0));
}
TEST(ByteTests, TrailingZeroCount_Four_IsTwo) {
    EXPECT_EQ(Byte::TrailingZeroCount(bytecs(4)), bytecs(2));
}

TEST(ByteTests, PopCount_Zero_IsZero) {
    EXPECT_EQ(Byte::PopCount(bytecs(0)), bytecs(0));
}
TEST(ByteTests, PopCount_255_IsEight) {
    EXPECT_EQ(Byte::PopCount(bytecs(255)), bytecs(8));
}
TEST(ByteTests, PopCount_170_IsFour) {
    EXPECT_EQ(Byte::PopCount(bytecs(0b10101010)), bytecs(4));
}

// ---------------------------------------------------------------------------
// RotateLeft / RotateRight
// ---------------------------------------------------------------------------

TEST(ByteTests, RotateLeft_1_By1) {
    EXPECT_EQ(Byte::RotateLeft(bytecs(1), 1), bytecs(2));
}
TEST(ByteTests, RotateLeft_128_By1) {
    EXPECT_EQ(Byte::RotateLeft(bytecs(128), 1), bytecs(1));
}
TEST(ByteTests, RotateLeft_By0_Unchanged) {
    EXPECT_EQ(Byte::RotateLeft(bytecs(42), 0), bytecs(42));
}

TEST(ByteTests, RotateRight_2_By1) {
    EXPECT_EQ(Byte::RotateRight(bytecs(2), 1), bytecs(1));
}
TEST(ByteTests, RotateRight_1_By1) {
    EXPECT_EQ(Byte::RotateRight(bytecs(1), 1), bytecs(128));
}
TEST(ByteTests, RotateRight_By0_Unchanged) {
    EXPECT_EQ(Byte::RotateRight(bytecs(42), 0), bytecs(42));
}

// ---------------------------------------------------------------------------
// Log2
// ---------------------------------------------------------------------------

TEST(ByteTests, Log2_One_IsZero) { EXPECT_EQ(Byte::Log2(bytecs(1)), bytecs(0)); }
TEST(ByteTests, Log2_Two_IsOne) { EXPECT_EQ(Byte::Log2(bytecs(2)), bytecs(1)); }
TEST(ByteTests, Log2_128_IsSeven) { EXPECT_EQ(Byte::Log2(bytecs(128)), bytecs(7)); }
TEST(ByteTests, Log2_255_IsSeven) { EXPECT_EQ(Byte::Log2(bytecs(255)), bytecs(7)); }
// .NET Byte.Log2(0) returns 0 (BitOperations.Log2 convention); byte can never be
// negative, so there is nothing for it to throw for.
TEST(ByteTests, Log2_Zero_IsZero) {
    EXPECT_EQ(Byte::Log2(bytecs(0)), bytecs(0));
}

// ---------------------------------------------------------------------------
// Log10
// ---------------------------------------------------------------------------

TEST(ByteTests, Log10_One_IsZero) { EXPECT_EQ(Byte::Log10(bytecs(1)), bytecs(0)); }
TEST(ByteTests, Log10_Nine_IsZero) { EXPECT_EQ(Byte::Log10(bytecs(9)), bytecs(0)); }
TEST(ByteTests, Log10_Ten_IsOne) { EXPECT_EQ(Byte::Log10(bytecs(10)), bytecs(1)); }
TEST(ByteTests, Log10_99_IsOne) { EXPECT_EQ(Byte::Log10(bytecs(99)), bytecs(1)); }
TEST(ByteTests, Log10_100_IsTwo) { EXPECT_EQ(Byte::Log10(bytecs(100)), bytecs(2)); }
TEST(ByteTests, Log10_255_IsTwo) { EXPECT_EQ(Byte::Log10(bytecs(255)), bytecs(2)); }
// .NET Byte.Log10(0) returns 0 (uint.Log10 convention); byte can never be negative,
// so there is nothing for it to throw for.
TEST(ByteTests, Log10_Zero_IsZero) {
    EXPECT_EQ(Byte::Log10(bytecs(0)), bytecs(0));
}
