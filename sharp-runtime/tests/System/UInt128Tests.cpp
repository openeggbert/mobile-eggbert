// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UInt128.hpp"

using System::UInt128;

TEST(UInt128Test, DefaultCtorIsZero) {
    UInt128 v;
    EXPECT_EQ(v.getLowerProperty(), 0ULL);
    EXPECT_EQ(v.getUpperProperty(), 0ULL);
}

TEST(UInt128Test, ConstructFromUpperLower) {
    UInt128 v(1ULL, 2ULL);
    EXPECT_EQ(v.getUpperProperty(), 1ULL);
    EXPECT_EQ(v.getLowerProperty(), 2ULL);
}

TEST(UInt128Test, Addition) {
    UInt128 a(0, 10), b(0, 20);
    UInt128 c = a + b;
    EXPECT_EQ(c.getLowerProperty(), 30ULL);
}

TEST(UInt128Test, Subtraction) {
    UInt128 a(0, 50), b(0, 20);
    UInt128 c = a - b;
    EXPECT_EQ(c.getLowerProperty(), 30ULL);
}

TEST(UInt128Test, Multiplication) {
    UInt128 a(0, 6), b(0, 7);
    UInt128 c = a * b;
    EXPECT_EQ(c.getLowerProperty(), 42ULL);
}

TEST(UInt128Test, Division) {
    UInt128 a(0, 42), b(0, 6);
    UInt128 c = a / b;
    EXPECT_EQ(c.getLowerProperty(), 7ULL);
}

TEST(UInt128Test, Modulo) {
    UInt128 a(0, 17), b(0, 5);
    UInt128 c = a % b;
    EXPECT_EQ(c.getLowerProperty(), 2ULL);
}

TEST(UInt128Test, DivisionByZero_Throws) {
    UInt128 a(0, 42), zero(0, 0);
    EXPECT_THROW(a / zero, System::DivideByZeroException);
}

TEST(UInt128Test, ModuloByZero_Throws) {
    UInt128 a(0, 42), zero(0, 0);
    EXPECT_THROW(a % zero, System::DivideByZeroException);
}

TEST(UInt128Test, EqualityTrue) {
    UInt128 a(1, 2), b(1, 2);
    EXPECT_TRUE(a == b);
}

TEST(UInt128Test, EqualityFalse) {
    UInt128 a(1, 2), b(1, 3);
    EXPECT_TRUE(a != b);
}

TEST(UInt128Test, LessThan) {
    UInt128 a(0, 5), b(0, 10);
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

TEST(UInt128Test, MinValue) {
    UInt128 v = UInt128::MinValue();
    EXPECT_EQ(v.getLowerProperty(), 0ULL);
    EXPECT_EQ(v.getUpperProperty(), 0ULL);
}

TEST(UInt128Test, MaxValueUpperBits) {
    UInt128 v = UInt128::MaxValue();
    EXPECT_EQ(v.getLowerProperty(), 0xFFFFFFFFFFFFFFFFULL);
    EXPECT_EQ(v.getUpperProperty(), 0xFFFFFFFFFFFFFFFFULL);
}

TEST(UInt128Test, ToStringZero) {
    UInt128 v;
    EXPECT_EQ(v.ToString(), "0");
}

TEST(UInt128Test, ToStringSmall) {
    UInt128 v(0, 12345);
    EXPECT_EQ(v.ToString(), "12345");
}

TEST(UInt128Test, LeftShift) {
    UInt128 v(0, 1);
    UInt128 shifted = v << 4;
    EXPECT_EQ(shifted.getLowerProperty(), 16ULL);
}

TEST(UInt128Test, ZeroSingleton) {
    UInt128 z = UInt128::Zero();
    EXPECT_EQ(z.getLowerProperty(), 0ULL);
}

TEST(UInt128Test, CompareTo) {
    UInt128 a(0, 5), b(0, 10);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_GT(b.CompareTo(a), 0);
    EXPECT_EQ(a.CompareTo(a), 0);
}

TEST(UInt128Test, Equals) {
    UInt128 a(1, 2), b(1, 2), c(1, 3);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_FALSE(a.Equals(c));
}

TEST(UInt128Test, GetHashCode_SameValue_SameHash) {
    UInt128 a(1, 2), b(1, 2);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(UInt128Test, MaxMin) {
    UInt128 a(0, 5), b(0, 10);
    EXPECT_TRUE(UInt128::Max(a, b) == b);
    EXPECT_TRUE(UInt128::Min(a, b) == a);
}

TEST(UInt128Test, Clamp) {
    UInt128 lo(0, 0), hi(0, 100);
    EXPECT_TRUE(UInt128::Clamp(UInt128(0, 50), lo, hi) == UInt128(0, 50));
    EXPECT_TRUE(UInt128::Clamp(UInt128(0, 200), lo, hi) == hi);
}

TEST(UInt128Test, Sign) {
    EXPECT_EQ(UInt128::Sign(UInt128(0, 0)), 0);
    EXPECT_EQ(UInt128::Sign(UInt128(0, 1)), 1);
}

TEST(UInt128Test, IsEvenOddPow2) {
    EXPECT_TRUE(UInt128::IsEvenInteger(UInt128(0, 4)));
    EXPECT_TRUE(UInt128::IsOddInteger(UInt128(0, 5)));
    EXPECT_TRUE(UInt128::IsPow2(UInt128(0, 8)));
    EXPECT_FALSE(UInt128::IsPow2(UInt128(0, 6)));
}

// ---------------------------------------------------------------------------
// Parse / TryParse / ToString(format) -- regression: these were entirely
// missing despite Int128 (the signed sibling) already having them.
// ---------------------------------------------------------------------------

TEST(UInt128Test, Parse_SimpleDecimal_RoundTrips) {
    UInt128 v = UInt128::Parse("12345678901234567890");
    EXPECT_EQ(v.ToString(), "12345678901234567890");
}

TEST(UInt128Test, Parse_Zero) {
    EXPECT_TRUE(UInt128::Parse("0") == UInt128::Zero());
}

TEST(UInt128Test, Parse_MaxValue_RoundTrips) {
    UInt128 v = UInt128::Parse(UInt128::MaxValue().ToString());
    EXPECT_TRUE(v == UInt128::MaxValue());
}

TEST(UInt128Test, Parse_LeadingPlus_Accepted) {
    UInt128 v = UInt128::Parse("+42");
    EXPECT_TRUE(v == UInt128(0, 42));
}

TEST(UInt128Test, Parse_LeadingMinus_ThrowsOverflow) {
    EXPECT_THROW(UInt128::Parse("-1"), System::OverflowException);
}

TEST(UInt128Test, Parse_LeadingMinusZero_ThrowsOverflow) {
    EXPECT_THROW(UInt128::Parse("-0"), System::OverflowException);
}

TEST(UInt128Test, Parse_Empty_ThrowsFormat) {
    EXPECT_THROW(UInt128::Parse(""), System::FormatException);
}

TEST(UInt128Test, Parse_NonDigit_ThrowsFormat) {
    EXPECT_THROW(UInt128::Parse("12a34"), System::FormatException);
}

TEST(UInt128Test, Parse_ExceedsMaxValue_ThrowsOverflow) {
    // MaxValue is 340282366920938463463374607431768211455; append a digit to overflow.
    EXPECT_THROW(UInt128::Parse("3402823669209384634633746074317682114550"), System::OverflowException);
}

TEST(UInt128Test, TryParse_Valid_ReturnsTrue) {
    UInt128 result;
    EXPECT_TRUE(UInt128::TryParse("999", result));
    EXPECT_TRUE(result == UInt128(0, 999));
}

TEST(UInt128Test, TryParse_Invalid_ReturnsFalseAndZero) {
    UInt128 result(0, 123);
    EXPECT_FALSE(UInt128::TryParse("-5", result));
    EXPECT_TRUE(result == UInt128::Zero());
}

TEST(UInt128Test, ToString_HexFormat) {
    UInt128 v(0x1, 0xABCD);
    EXPECT_EQ(v.ToString("X"), "1000000000000ABCD");
}

TEST(UInt128Test, ToString_HexFormatLowercase) {
    UInt128 v(0, 0xABCD);
    EXPECT_EQ(v.ToString("x"), "abcd");
}

TEST(UInt128Test, ToString_DecimalWithWidth) {
    UInt128 v(0, 5);
    EXPECT_EQ(v.ToString("D10"), "0000000005");
}

TEST(UInt128Test, ToString_EmptyFormat_MatchesPlainToString) {
    UInt128 v(0, 42);
    EXPECT_EQ(v.ToString(""), v.ToString());
}
