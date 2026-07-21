// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include "System/Int16.hpp"
#include "System/OverflowException.hpp"

using System::Int16;
using SharpRuntime::shortcs;

// Additional Int16 tests (Parse/TryParse/ToString already in PrimitiveTypeTests2.cpp)

TEST(Int16NewTests, CompareTo_Equal_Zero)      { EXPECT_EQ(Int16::CompareTo(5, 5), 0); }
TEST(Int16NewTests, CompareTo_Less_Negative)   { EXPECT_LT(Int16::CompareTo(3, 5), 0); }
TEST(Int16NewTests, CompareTo_Greater_Positive){ EXPECT_GT(Int16::CompareTo(7, 5), 0); }

TEST(Int16NewTests, Equals_Same_True)  { EXPECT_TRUE(Int16::Equals(42, 42)); }
TEST(Int16NewTests, Equals_Diff_False) { EXPECT_FALSE(Int16::Equals(1, 2)); }

TEST(Int16NewTests, GetHashCode_SameValue_SameHash) {
    EXPECT_EQ(Int16::GetHashCode(100), Int16::GetHashCode(100));
}

TEST(Int16NewTests, Abs_Positive) { EXPECT_EQ(Int16::Abs(5), 5); }
TEST(Int16NewTests, Abs_Negative) { EXPECT_EQ(Int16::Abs(-5), 5); }
TEST(Int16NewTests, Abs_Zero)     { EXPECT_EQ(Int16::Abs(0), 0); }
TEST(Int16NewTests, Abs_MinValue_Throws) {
    EXPECT_THROW(Int16::Abs(Int16::MinValue), System::OverflowException);
}

// Regression tests (added 2026-07-14, duplicated-implementation audit finding): Int16 was the
// sole signed integer type missing CopySign/IsNegative/IsPositive/MaxMagnitude/MinMagnitude --
// mirrors SByteTests.cpp's identical coverage for its sibling SByte.

TEST(Int16NewTests, CopySign_PositiveSign) { EXPECT_EQ(Int16::CopySign(-3, 1), 3); }
TEST(Int16NewTests, CopySign_NegativeSign) { EXPECT_EQ(Int16::CopySign(3, -1), -3); }
TEST(Int16NewTests, CopySign_MinValue_NonNegativeSign_Throws) {
    EXPECT_THROW(Int16::CopySign(Int16::MinValue, 1), System::OverflowException);
}
TEST(Int16NewTests, CopySign_MinValue_NegativeSign_ReturnsMinValue) {
    EXPECT_EQ(Int16::CopySign(Int16::MinValue, -1), Int16::MinValue);
}

TEST(Int16NewTests, IsNegative_True)  { EXPECT_TRUE(Int16::IsNegative(-1)); }
TEST(Int16NewTests, IsNegative_False) { EXPECT_FALSE(Int16::IsNegative(0)); }
TEST(Int16NewTests, IsPositive_True)  { EXPECT_TRUE(Int16::IsPositive(1)); }
TEST(Int16NewTests, IsPositive_False) { EXPECT_FALSE(Int16::IsPositive(0)); }

TEST(Int16NewTests, MaxMagnitude_Larger)  { EXPECT_EQ(Int16::MaxMagnitude(-10, 5), -10); }
TEST(Int16NewTests, MinMagnitude_Smaller) { EXPECT_EQ(Int16::MinMagnitude(-10, 5), 5); }
TEST(Int16NewTests, MaxMagnitude_MinValueAlwaysWins) {
    EXPECT_EQ(Int16::MaxMagnitude(Int16::MinValue, 5), Int16::MinValue);
    EXPECT_EQ(Int16::MaxMagnitude(5, Int16::MinValue), Int16::MinValue);
}
TEST(Int16NewTests, MinMagnitude_MinValueAlwaysLoses) {
    EXPECT_EQ(Int16::MinMagnitude(Int16::MinValue, 5), 5);
    EXPECT_EQ(Int16::MinMagnitude(5, Int16::MinValue), 5);
}

TEST(Int16NewTests, Parse_TrailingGarbage_ThrowsFormatException) {
    EXPECT_THROW(Int16::Parse("123abc"), System::FormatException);
}

TEST(Int16NewTests, Clamp_Within) { EXPECT_EQ(Int16::Clamp(5, 0, 10), 5); }
TEST(Int16NewTests, Clamp_Below)  { EXPECT_EQ(Int16::Clamp(-5, 0, 10), 0); }
TEST(Int16NewTests, Clamp_Above)  { EXPECT_EQ(Int16::Clamp(20, 0, 10), 10); }

TEST(Int16NewTests, Max_ReturnsLarger)  { EXPECT_EQ(Int16::Max(3, 7), 7); }
TEST(Int16NewTests, Min_ReturnsSmaller) { EXPECT_EQ(Int16::Min(3, 7), 3); }

TEST(Int16NewTests, Sign_Positive_ReturnsOne)   { EXPECT_EQ(Int16::Sign(42), 1); }
TEST(Int16NewTests, Sign_Zero_ReturnsZero)      { EXPECT_EQ(Int16::Sign(0), 0); }
TEST(Int16NewTests, Sign_Negative_ReturnsMinus1){ EXPECT_EQ(Int16::Sign(-5), -1); }

TEST(Int16NewTests, DivRem_Exact) {
    auto [q,r] = Int16::DivRem(10, 2);
    EXPECT_EQ(q, 5); EXPECT_EQ(r, 0);
}
TEST(Int16NewTests, DivRem_WithRemainder) {
    auto [q,r] = Int16::DivRem(10, 3);
    EXPECT_EQ(q, 3); EXPECT_EQ(r, 1);
}
TEST(Int16NewTests, DivRem_ByZero_ThrowsDivideByZeroException) {
    EXPECT_THROW(Int16::DivRem(10, 0), System::DivideByZeroException);
}

TEST(Int16NewTests, IsEvenInteger_Zero_True) { EXPECT_TRUE(Int16::IsEvenInteger(0)); }
TEST(Int16NewTests, IsEvenInteger_One_False) { EXPECT_FALSE(Int16::IsEvenInteger(1)); }
TEST(Int16NewTests, IsOddInteger_One_True)   { EXPECT_TRUE(Int16::IsOddInteger(1)); }

TEST(Int16NewTests, IsPow2_One_True)     { EXPECT_TRUE(Int16::IsPow2(1)); }
TEST(Int16NewTests, IsPow2_Two_True)     { EXPECT_TRUE(Int16::IsPow2(2)); }
TEST(Int16NewTests, IsPow2_Three_False)  { EXPECT_FALSE(Int16::IsPow2(3)); }
TEST(Int16NewTests, IsPow2_NegOne_False) { EXPECT_FALSE(Int16::IsPow2(-1)); }

TEST(Int16NewTests, LeadingZeroCount_One_Is15) { EXPECT_EQ(Int16::LeadingZeroCount(1), 15); }
TEST(Int16NewTests, PopCount_Zero_IsZero) { EXPECT_EQ(Int16::PopCount(0), 0); }
TEST(Int16NewTests, PopCount_NegOne_Is16) {
    // All 16 bits set as unsigned = -1 as signed
    EXPECT_EQ(Int16::PopCount(-1), 16);
}

TEST(Int16NewTests, TrailingZeroCount_One_IsZero) { EXPECT_EQ(Int16::TrailingZeroCount(1), 0); }
TEST(Int16NewTests, TrailingZeroCount_Four_IsTwo)  { EXPECT_EQ(Int16::TrailingZeroCount(4), 2); }
TEST(Int16NewTests, TrailingZeroCount_Zero_Is16)   { EXPECT_EQ(Int16::TrailingZeroCount(0), 16); }

TEST(Int16NewTests, RotateLeft_1_By1_Is2) { EXPECT_EQ(Int16::RotateLeft(1, 1), 2); }
TEST(Int16NewTests, RotateRight_2_By1_Is1){ EXPECT_EQ(Int16::RotateRight(2, 1), 1); }

TEST(Int16NewTests, Log2_One_IsZero)     { EXPECT_EQ(Int16::Log2(1), 0); }
TEST(Int16NewTests, Log2_Two_IsOne)      { EXPECT_EQ(Int16::Log2(2), 1); }
TEST(Int16NewTests, Log2_Zero_IsZero) {
    // Matches .NET's actual BitOperations.Log2(0) semantics: 0, not an error.
    EXPECT_EQ(Int16::Log2(0), 0);
}
TEST(Int16NewTests, Log2_Negative_Throws){ EXPECT_THROW(Int16::Log2(-1), System::ArgumentOutOfRangeException); }
