// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include "System/Int64.hpp"
#include "System/OverflowException.hpp"

using System::Int64;
using SharpRuntime::longcs;

// Additional Int64 tests (Parse/TryParse/ToString already in PrimitiveTypeTests.cpp)

TEST(Int64NewTests, CompareTo_Equal_Zero)    { EXPECT_EQ(Int64::CompareTo(5LL, 5LL), 0); }
TEST(Int64NewTests, CompareTo_Less_Negative) { EXPECT_LT(Int64::CompareTo(3LL, 5LL), 0); }
TEST(Int64NewTests, CompareTo_Greater_Positive){ EXPECT_GT(Int64::CompareTo(7LL, 5LL), 0); }

TEST(Int64NewTests, Equals_Same_True)  { EXPECT_TRUE(Int64::Equals(42LL, 42LL)); }
TEST(Int64NewTests, Equals_Diff_False) { EXPECT_FALSE(Int64::Equals(1LL, 2LL)); }

TEST(Int64NewTests, GetHashCode_SameValue_SameHash) {
    EXPECT_EQ(Int64::GetHashCode(100LL), Int64::GetHashCode(100LL));
}

TEST(Int64NewTests, Abs_Positive) { EXPECT_EQ(Int64::Abs(5LL), 5LL); }
TEST(Int64NewTests, Abs_Negative) { EXPECT_EQ(Int64::Abs(-5LL), 5LL); }
TEST(Int64NewTests, Abs_Zero)     { EXPECT_EQ(Int64::Abs(0LL), 0LL); }
TEST(Int64NewTests, Abs_MinValue_Throws) {
    EXPECT_THROW(Int64::Abs(Int64::MinValue), System::OverflowException);
}

TEST(Int64NewTests, Parse_TrailingGarbage_ThrowsFormatException) {
    EXPECT_THROW(Int64::Parse("123abc"), System::FormatException);
}

TEST(Int64NewTests, Clamp_Within) { EXPECT_EQ(Int64::Clamp(5LL,0LL,10LL), 5LL); }
TEST(Int64NewTests, Clamp_Below)  { EXPECT_EQ(Int64::Clamp(-5LL,0LL,10LL), 0LL); }
TEST(Int64NewTests, Clamp_Above)  { EXPECT_EQ(Int64::Clamp(20LL,0LL,10LL), 10LL); }

TEST(Int64NewTests, Max_ReturnsLarger)  { EXPECT_EQ(Int64::Max(3LL, 7LL), 7LL); }
TEST(Int64NewTests, Min_ReturnsSmaller) { EXPECT_EQ(Int64::Min(3LL, 7LL), 3LL); }

TEST(Int64NewTests, Sign_Positive_ReturnsOne)   { EXPECT_EQ(Int64::Sign(42LL), 1); }
TEST(Int64NewTests, Sign_Zero_ReturnsZero)      { EXPECT_EQ(Int64::Sign(0LL), 0); }
TEST(Int64NewTests, Sign_Negative_ReturnsMinus1){ EXPECT_EQ(Int64::Sign(-5LL), -1); }

TEST(Int64NewTests, DivRem_Exact) {
    auto [q,r] = Int64::DivRem(10LL, 2LL);
    EXPECT_EQ(q, 5LL); EXPECT_EQ(r, 0LL);
}
TEST(Int64NewTests, DivRem_ByZero_ThrowsDivideByZeroException) {
    EXPECT_THROW(Int64::DivRem(10LL, 0LL), System::DivideByZeroException);
}
TEST(Int64NewTests, DivRem_MinValueByNegativeOne_ThrowsOverflowException) {
    EXPECT_THROW(Int64::DivRem(Int64::MinValue, -1LL), System::OverflowException);
}
TEST(Int64NewTests, DivRem_WithRemainder) {
    auto [q,r] = Int64::DivRem(10LL, 3LL);
    EXPECT_EQ(q, 3LL); EXPECT_EQ(r, 1LL);
}

TEST(Int64NewTests, IsEvenInteger_Zero_True) { EXPECT_TRUE(Int64::IsEvenInteger(0LL)); }
TEST(Int64NewTests, IsEvenInteger_One_False) { EXPECT_FALSE(Int64::IsEvenInteger(1LL)); }
TEST(Int64NewTests, IsOddInteger_One_True)   { EXPECT_TRUE(Int64::IsOddInteger(1LL)); }

TEST(Int64NewTests, IsPow2_One_True)     { EXPECT_TRUE(Int64::IsPow2(1LL)); }
TEST(Int64NewTests, IsPow2_Two_True)     { EXPECT_TRUE(Int64::IsPow2(2LL)); }
TEST(Int64NewTests, IsPow2_Three_False)  { EXPECT_FALSE(Int64::IsPow2(3LL)); }
TEST(Int64NewTests, IsPow2_NegOne_False) { EXPECT_FALSE(Int64::IsPow2(-1LL)); }

TEST(Int64NewTests, LeadingZeroCount_One_Is63) { EXPECT_EQ(Int64::LeadingZeroCount(1LL), 63); }
TEST(Int64NewTests, PopCount_Zero_IsZero) { EXPECT_EQ(Int64::PopCount(0LL), 0); }
TEST(Int64NewTests, PopCount_MaxUint64_Is64) {
    // All 64 bits set as unsigned = -1 as signed
    EXPECT_EQ(Int64::PopCount(-1LL), 64);
}

TEST(Int64NewTests, TrailingZeroCount_One_IsZero) { EXPECT_EQ(Int64::TrailingZeroCount(1LL), 0); }
TEST(Int64NewTests, TrailingZeroCount_Four_IsTwo)  { EXPECT_EQ(Int64::TrailingZeroCount(4LL), 2); }
TEST(Int64NewTests, TrailingZeroCount_Zero_Is64)   { EXPECT_EQ(Int64::TrailingZeroCount(0LL), 64); }

TEST(Int64NewTests, RotateLeft_1_By1_Is2) { EXPECT_EQ(Int64::RotateLeft(1LL, 1), 2LL); }
TEST(Int64NewTests, RotateRight_2_By1_Is1){ EXPECT_EQ(Int64::RotateRight(2LL, 1), 1LL); }

TEST(Int64NewTests, Log2_One_IsZero)     { EXPECT_EQ(Int64::Log2(1LL), 0); }
TEST(Int64NewTests, Log2_Two_IsOne)      { EXPECT_EQ(Int64::Log2(2LL), 1); }
TEST(Int64NewTests, Log2_Zero_IsZero) {
    // Matches .NET's actual BitOperations.Log2(0) semantics: 0, not an error.
    EXPECT_EQ(Int64::Log2(0LL), 0);
}
TEST(Int64NewTests, Log2_Negative_Throws){ EXPECT_THROW(Int64::Log2(-1LL), System::ArgumentOutOfRangeException); }

TEST(Int64NewTests, BigMul_SmallValues) {
    System::Int128 result = Int64::BigMul(6LL, 7LL);
    EXPECT_EQ(static_cast<long long>(result), 42LL);
}
TEST(Int64NewTests, BigMul_OverflowsInto128Bits) {
    System::Int128 result = Int64::BigMul(Int64::MaxValue, Int64::MaxValue);
    System::Int128 expected = System::Int128(static_cast<__int128>(Int64::MaxValue)) * System::Int128(static_cast<__int128>(Int64::MaxValue));
    EXPECT_EQ(result, expected);
}

TEST(Int64NewTests, CopySign_PositiveSign_ReturnsPositive) { EXPECT_EQ(Int64::CopySign(-5LL, 3LL), 5LL); }
TEST(Int64NewTests, CopySign_NegativeSign_ReturnsNegative) { EXPECT_EQ(Int64::CopySign(5LL, -3LL), -5LL); }
TEST(Int64NewTests, CopySign_MinValue_NonNegativeSign_Throws) {
    EXPECT_THROW(Int64::CopySign(Int64::MinValue, 1LL), System::OverflowException);
}
// Regression for ticket 337: MinValue is already negative, so copying a negative sign onto it is
// a no-op -- must not attempt to negate MinValue, which is undefined behavior in C++ (real .NET
// relies on C#'s well-defined unchecked-arithmetic wraparound for this exact case). Confirmed via
// UBSan that the pre-fix `-absValue` expression was reachable, real UB for this input.
TEST(Int64NewTests, CopySign_MinValue_NegativeSign_ReturnsMinValue) {
    EXPECT_EQ(Int64::CopySign(Int64::MinValue, -1LL), Int64::MinValue);
}

TEST(Int64NewTests, IsNegative_True)  { EXPECT_TRUE(Int64::IsNegative(-1LL)); }
TEST(Int64NewTests, IsNegative_False) { EXPECT_FALSE(Int64::IsNegative(0LL)); }
TEST(Int64NewTests, IsPositive_True)  { EXPECT_TRUE(Int64::IsPositive(0LL)); }
TEST(Int64NewTests, IsPositive_False) { EXPECT_FALSE(Int64::IsPositive(-1LL)); }

// MaxMagnitude/MinMagnitude with MinValue: MinValue has no representable positive
// magnitude, so .NET special-cases it to always win MaxMagnitude and always lose
// MinMagnitude, regardless of the other operand.
TEST(Int64NewTests, MaxMagnitude_MinValueAlwaysWins) {
    EXPECT_EQ(Int64::MaxMagnitude(Int64::MinValue, 5LL), Int64::MinValue);
    EXPECT_EQ(Int64::MaxMagnitude(5LL, Int64::MinValue), Int64::MinValue);
}
TEST(Int64NewTests, MinMagnitude_MinValueAlwaysLoses) {
    EXPECT_EQ(Int64::MinMagnitude(Int64::MinValue, 5LL), 5LL);
    EXPECT_EQ(Int64::MinMagnitude(5LL, Int64::MinValue), 5LL);
}
TEST(Int64NewTests, MaxMagnitude_LargerFirst)  { EXPECT_EQ(Int64::MaxMagnitude(-10LL, 5LL), -10LL); }
TEST(Int64NewTests, MinMagnitude_SmallerSecond){ EXPECT_EQ(Int64::MinMagnitude(-10LL, 5LL), 5LL); }
