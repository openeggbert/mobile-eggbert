// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/SByte.hpp"

using System::SByte;
using SharpRuntime::sbytecs;

// Additional SByte tests (Parse/TryParse/ToString already in PrimitiveTypeTests2.cpp)

TEST(SByteNewTests, CompareTo_Equal_Zero)    { EXPECT_EQ(SByte::CompareTo(sbytecs(5), sbytecs(5)), 0); }
TEST(SByteNewTests, CompareTo_Less_Negative) { EXPECT_LT(SByte::CompareTo(sbytecs(3), sbytecs(5)), 0); }
TEST(SByteNewTests, CompareTo_Greater_Positive){ EXPECT_GT(SByte::CompareTo(sbytecs(7), sbytecs(5)), 0); }

TEST(SByteNewTests, Equals_Same_True)  { EXPECT_TRUE(SByte::Equals(sbytecs(42), sbytecs(42))); }
TEST(SByteNewTests, Equals_Diff_False) { EXPECT_FALSE(SByte::Equals(sbytecs(1), sbytecs(2))); }

TEST(SByteNewTests, GetHashCode_ValueIsHash) { EXPECT_EQ(SByte::GetHashCode(sbytecs(7)), 7); }
TEST(SByteNewTests, GetHashCode_NegativeValue) { EXPECT_EQ(SByte::GetHashCode(sbytecs(-1)), -1); }

TEST(SByteNewTests, Clamp_Within)  { EXPECT_EQ(SByte::Clamp(sbytecs(5), sbytecs(0), sbytecs(10)), sbytecs(5)); }
TEST(SByteNewTests, Clamp_Below)   { EXPECT_EQ(SByte::Clamp(sbytecs(-5),sbytecs(0), sbytecs(10)), sbytecs(0)); }
TEST(SByteNewTests, Clamp_Above)   { EXPECT_EQ(SByte::Clamp(sbytecs(20),sbytecs(0), sbytecs(10)), sbytecs(10)); }

TEST(SByteNewTests, Max_ReturnsLarger)  { EXPECT_EQ(SByte::Max(sbytecs(3), sbytecs(7)), sbytecs(7)); }
TEST(SByteNewTests, Min_ReturnsSmaller) { EXPECT_EQ(SByte::Min(sbytecs(3), sbytecs(7)), sbytecs(3)); }

TEST(SByteNewTests, Sign_Positive_ReturnsOne)  { EXPECT_EQ(SByte::Sign(sbytecs(42)), 1); }
TEST(SByteNewTests, Sign_Zero_ReturnsZero)     { EXPECT_EQ(SByte::Sign(sbytecs(0)), 0); }
TEST(SByteNewTests, Sign_Negative_ReturnsMinus1){ EXPECT_EQ(SByte::Sign(sbytecs(-5)), -1); }

TEST(SByteNewTests, DivRem_Exact) {
    auto [q, r] = SByte::DivRem(sbytecs(10), sbytecs(2));
    EXPECT_EQ(q, sbytecs(5)); EXPECT_EQ(r, sbytecs(0));
}
TEST(SByteNewTests, DivRem_WithRemainder) {
    auto [q, r] = SByte::DivRem(sbytecs(10), sbytecs(3));
    EXPECT_EQ(q, sbytecs(3)); EXPECT_EQ(r, sbytecs(1));
}
TEST(SByteNewTests, DivRem_ByZero_ThrowsDivideByZeroException) {
    EXPECT_THROW(SByte::DivRem(sbytecs(10), sbytecs(0)), System::DivideByZeroException);
}

TEST(SByteNewTests, IsEvenInteger_Zero_True) { EXPECT_TRUE(SByte::IsEvenInteger(sbytecs(0))); }
TEST(SByteNewTests, IsEvenInteger_One_False) { EXPECT_FALSE(SByte::IsEvenInteger(sbytecs(1))); }
TEST(SByteNewTests, IsOddInteger_One_True)   { EXPECT_TRUE(SByte::IsOddInteger(sbytecs(1))); }

TEST(SByteNewTests, IsPow2_One_True)    { EXPECT_TRUE(SByte::IsPow2(sbytecs(1))); }
TEST(SByteNewTests, IsPow2_Two_True)    { EXPECT_TRUE(SByte::IsPow2(sbytecs(2))); }
TEST(SByteNewTests, IsPow2_Three_False) { EXPECT_FALSE(SByte::IsPow2(sbytecs(3))); }
TEST(SByteNewTests, IsPow2_NegOne_False){ EXPECT_FALSE(SByte::IsPow2(sbytecs(-1))); }

TEST(SByteNewTests, PopCount_Zero_IsZero)    { EXPECT_EQ(SByte::PopCount(sbytecs(0)), sbytecs(0)); }
TEST(SByteNewTests, PopCount_MaxValue_Is7)   { EXPECT_EQ(SByte::PopCount(sbytecs(127)), sbytecs(7)); }

TEST(SByteNewTests, Log2_One_IsZero)  { EXPECT_EQ(SByte::Log2(sbytecs(1)), sbytecs(0)); }
TEST(SByteNewTests, Log2_Two_IsOne)   { EXPECT_EQ(SByte::Log2(sbytecs(2)), sbytecs(1)); }
// .NET SByte.Log2(0) returns 0 (BitOperations.Log2 convention), it does not throw.
TEST(SByteNewTests, Log2_Zero_IsZero) { EXPECT_EQ(SByte::Log2(sbytecs(0)), sbytecs(0)); }
TEST(SByteNewTests, Log2_Neg_Throws)  { EXPECT_THROW(SByte::Log2(sbytecs(-1)), System::ArgumentOutOfRangeException); }
