// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UInt32.hpp"

using System::UInt32;
using SharpRuntime::uintcs;

// Additional UInt32 tests (Parse/TryParse/ToString already in PrimitiveTypeTests.cpp)

// Regression tests (added 2026-07-14, duplicated-implementation audit finding): UInt32 was the
// only one of the 8 integer types missing the 2-arg ToString(value, format) overload -- ported
// code calling UInt32::ToString(value, "X8"), which compiles for every sibling integer type,
// failed to compile for UInt32 alone. Mirrors UInt16Tests/UInt64Test's identical coverage.
TEST(UInt32NewTests, ToStringHex) {
    EXPECT_EQ(UInt32::ToString(255u, "X"), "FF");
    EXPECT_EQ(UInt32::ToString(255u, "x"), "ff");
}

TEST(UInt32NewTests, ToStringPadded) {
    EXPECT_EQ(UInt32::ToString(10u, "D4"), "0010");
}

TEST(UInt32NewTests, ToString_EmptyFormat_SameAsOneArg) {
    EXPECT_EQ(UInt32::ToString(1000u, ""), UInt32::ToString(1000u));
}

TEST(UInt32NewTests, ToString_MalformedWidth_ThrowsFormatException) {
    EXPECT_THROW(UInt32::ToString(5u, "Xz"), System::FormatException);
}

TEST(UInt32NewTests, MaxValue_Correct) { EXPECT_EQ(UInt32::MaxValue, 4294967295u); }
TEST(UInt32NewTests, MinValue_IsZero)  { EXPECT_EQ(UInt32::MinValue, 0u); }

TEST(UInt32NewTests, CompareTo_Equal_Zero) { EXPECT_EQ(UInt32::CompareTo(5u, 5u), 0); }
TEST(UInt32NewTests, CompareTo_Less_Negative) { EXPECT_LT(UInt32::CompareTo(3u, 5u), 0); }
TEST(UInt32NewTests, CompareTo_Greater_Positive) { EXPECT_GT(UInt32::CompareTo(7u, 5u), 0); }

TEST(UInt32NewTests, Equals_Same_True) { EXPECT_TRUE(UInt32::Equals(42u, 42u)); }
TEST(UInt32NewTests, Equals_Diff_False) { EXPECT_FALSE(UInt32::Equals(1u, 2u)); }

TEST(UInt32NewTests, GetHashCode_SameValue_SameHash) {
    EXPECT_EQ(UInt32::GetHashCode(99u), UInt32::GetHashCode(99u));
}

TEST(UInt32NewTests, Clamp_Within) { EXPECT_EQ(UInt32::Clamp(5u,0u,10u), 5u); }
TEST(UInt32NewTests, Clamp_Below) { EXPECT_EQ(UInt32::Clamp(0u,3u,10u), 3u); }
TEST(UInt32NewTests, Clamp_Above) { EXPECT_EQ(UInt32::Clamp(20u,0u,10u), 10u); }

TEST(UInt32NewTests, Max_ReturnsLarger) { EXPECT_EQ(UInt32::Max(3u,7u), 7u); }
TEST(UInt32NewTests, Min_ReturnsSmaller) { EXPECT_EQ(UInt32::Min(3u,7u), 3u); }

TEST(UInt32NewTests, Sign_Zero_ReturnsZero) { EXPECT_EQ(UInt32::Sign(0u), 0); }
TEST(UInt32NewTests, Sign_Nonzero_ReturnsOne) { EXPECT_EQ(UInt32::Sign(42u), 1); }

TEST(UInt32NewTests, DivRem_Exact) {
    auto [q,r] = UInt32::DivRem(10u, 2u);
    EXPECT_EQ(q, 5u); EXPECT_EQ(r, 0u);
}
TEST(UInt32NewTests, DivRem_WithRemainder) {
    auto [q,r] = UInt32::DivRem(10u, 3u);
    EXPECT_EQ(q, 3u); EXPECT_EQ(r, 1u);
}
TEST(UInt32NewTests, DivRem_ByZero_ThrowsDivideByZeroException) {
    EXPECT_THROW(UInt32::DivRem(10u, 0u), System::DivideByZeroException);
}

TEST(UInt32NewTests, IsEvenInteger_Zero_True) { EXPECT_TRUE(UInt32::IsEvenInteger(0u)); }
TEST(UInt32NewTests, IsEvenInteger_One_False) { EXPECT_FALSE(UInt32::IsEvenInteger(1u)); }
TEST(UInt32NewTests, IsOddInteger_One_True)   { EXPECT_TRUE(UInt32::IsOddInteger(1u)); }

TEST(UInt32NewTests, IsPow2_One_True)   { EXPECT_TRUE(UInt32::IsPow2(1u)); }
TEST(UInt32NewTests, IsPow2_Two_True)   { EXPECT_TRUE(UInt32::IsPow2(2u)); }
TEST(UInt32NewTests, IsPow2_Three_False){ EXPECT_FALSE(UInt32::IsPow2(3u)); }
TEST(UInt32NewTests, IsPow2_Zero_False) { EXPECT_FALSE(UInt32::IsPow2(0u)); }

TEST(UInt32NewTests, LeadingZeroCount_One_Is31) { EXPECT_EQ(UInt32::LeadingZeroCount(1u), 31u); }
TEST(UInt32NewTests, LeadingZeroCount_MaxValue_Is0) { EXPECT_EQ(UInt32::LeadingZeroCount(0x80000000u), 0u); }

TEST(UInt32NewTests, PopCount_Zero_IsZero) { EXPECT_EQ(UInt32::PopCount(0u), 0u); }
TEST(UInt32NewTests, PopCount_MaxValue_Is32) { EXPECT_EQ(UInt32::PopCount(0xFFFFFFFFu), 32u); }

TEST(UInt32NewTests, TrailingZeroCount_One_IsZero) { EXPECT_EQ(UInt32::TrailingZeroCount(1u), 0u); }
TEST(UInt32NewTests, TrailingZeroCount_Four_IsTwo) { EXPECT_EQ(UInt32::TrailingZeroCount(4u), 2u); }
TEST(UInt32NewTests, TrailingZeroCount_Zero_Is32) { EXPECT_EQ(UInt32::TrailingZeroCount(0u), 32u); }

TEST(UInt32NewTests, RotateLeft_1_By1_Is2) { EXPECT_EQ(UInt32::RotateLeft(1u, 1), 2u); }
TEST(UInt32NewTests, RotateRight_2_By1_Is1) { EXPECT_EQ(UInt32::RotateRight(2u, 1), 1u); }

TEST(UInt32NewTests, Log2_One_IsZero) { EXPECT_EQ(UInt32::Log2(1u), 0u); }
TEST(UInt32NewTests, Log2_Two_IsOne)  { EXPECT_EQ(UInt32::Log2(2u), 1u); }
TEST(UInt32NewTests, Log2_Zero_IsZero) { EXPECT_EQ(UInt32::Log2(0u), 0u); } // .NET's Log2(0) contract: returns 0, does not throw
