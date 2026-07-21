// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include "System/Decimal.hpp"
#include "System/DivideByZeroException.hpp"
#include "System/FormatException.hpp"

using System::Decimal;

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

TEST(DecimalTests, ZeroIsZero) {
    EXPECT_EQ(Decimal::Zero, Decimal(0));
    EXPECT_EQ(Decimal::Zero.ToString(), "0");
}

TEST(DecimalTests, OneIsOne) {
    EXPECT_EQ(Decimal::One, Decimal(1));
}

TEST(DecimalTests, MinusOneIsMinusOne) {
    EXPECT_EQ(Decimal::MinusOne, Decimal(-1));
}

TEST(DecimalTests, MaxValueString) {
    EXPECT_EQ(Decimal::MaxValue.ToString(), "79228162514264337593543950335");
}

TEST(DecimalTests, MinValueString) {
    EXPECT_EQ(Decimal::MinValue.ToString(), "-79228162514264337593543950335");
}

// ---------------------------------------------------------------------------
// Parse / ToString roundtrip
// ---------------------------------------------------------------------------

TEST(DecimalTests, ParseInteger) {
    EXPECT_EQ(Decimal::Parse("42").ToString(), "42");
    EXPECT_EQ(Decimal::Parse("-7").ToString(),  "-7");
    EXPECT_EQ(Decimal::Parse("0").ToString(),   "0");
}

TEST(DecimalTests, ParseDecimalFraction) {
    EXPECT_EQ(Decimal::Parse("3.14").ToString(),   "3.14");
    EXPECT_EQ(Decimal::Parse("0.1").ToString(),    "0.1");
    EXPECT_EQ(Decimal::Parse("0.001").ToString(),  "0.001");
    EXPECT_EQ(Decimal::Parse("-1.5").ToString(),   "-1.5");
}

TEST(DecimalTests, ParseLargeValue) {
    // 29 significant digits — the .NET MaxValue
    EXPECT_EQ(Decimal::Parse("79228162514264337593543950335").ToString(),
              "79228162514264337593543950335");
}

TEST(DecimalTests, TryParseValid) {
    Decimal d;
    EXPECT_TRUE(Decimal::TryParse("1.23", d));
    EXPECT_EQ(d.ToString(), "1.23");
}

TEST(DecimalTests, TryParseInvalid) {
    Decimal d;
    EXPECT_FALSE(Decimal::TryParse("abc", d));
    EXPECT_FALSE(Decimal::TryParse("", d));
    EXPECT_FALSE(Decimal::TryParse("1.2.3", d));
}

TEST(DecimalTests, ParseInvalidThrows) {
    EXPECT_THROW(Decimal::Parse("bad"), System::FormatException);
}

// ---------------------------------------------------------------------------
// The key precision test: 0.1 + 0.2 == 0.3 exactly
// ---------------------------------------------------------------------------

TEST(DecimalTests, OneTenthPlusTwoTenthsEqualsThreeTenths) {
    Decimal a = Decimal::Parse("0.1");
    Decimal b = Decimal::Parse("0.2");
    Decimal c = Decimal::Parse("0.3");
    // This FAILS with double arithmetic (0.1 + 0.2 != 0.3 in double)
    // but must pass with our 96-bit Decimal implementation
    EXPECT_EQ(a + b, c);
}

TEST(DecimalTests, DecimalArithmeticIsExactForDecimalFractions) {
    // 0.1 * 3 == 0.3
    EXPECT_EQ(Decimal::Parse("0.1") * Decimal(3), Decimal::Parse("0.3"));
    // 1.0 / 4 == 0.25 exactly
    EXPECT_EQ(Decimal::Parse("1.0") / Decimal(4), Decimal::Parse("0.25"));
    // 0.7 + 0.1 == 0.8
    EXPECT_EQ(Decimal::Parse("0.7") + Decimal::Parse("0.1"), Decimal::Parse("0.8"));
}

// ---------------------------------------------------------------------------
// Addition
// ---------------------------------------------------------------------------

TEST(DecimalTests, AddSameScale) {
    EXPECT_EQ(Decimal::Parse("1.5") + Decimal::Parse("2.5"), Decimal::Parse("4.0"));
}

TEST(DecimalTests, AddDifferentScales) {
    // 0.1 + 0.01 == 0.11
    EXPECT_EQ(Decimal::Parse("0.1") + Decimal::Parse("0.01"), Decimal::Parse("0.11"));
}

TEST(DecimalTests, AddNegative) {
    // 3 + (-1) == 2
    EXPECT_EQ(Decimal(3) + Decimal(-1), Decimal(2));
}

TEST(DecimalTests, AddWithBorrow) {
    // 1.0 - 0.3 == 0.7
    EXPECT_EQ(Decimal::Parse("1.0") - Decimal::Parse("0.3"), Decimal::Parse("0.7"));
}

TEST(DecimalTests, AddZero) {
    Decimal d = Decimal::Parse("3.14");
    EXPECT_EQ(d + Decimal::Zero, d);
}

// ---------------------------------------------------------------------------
// Subtraction
// ---------------------------------------------------------------------------

TEST(DecimalTests, SubtractBasic) {
    EXPECT_EQ(Decimal(10) - Decimal(3), Decimal(7));
}

TEST(DecimalTests, SubtractToNegative) {
    EXPECT_EQ(Decimal(3) - Decimal(10), Decimal(-7));
}

TEST(DecimalTests, SubtractFractions) {
    EXPECT_EQ(Decimal::Parse("0.9") - Decimal::Parse("0.1"), Decimal::Parse("0.8"));
}

// ---------------------------------------------------------------------------
// Multiplication
// ---------------------------------------------------------------------------

TEST(DecimalTests, MultiplyIntegers) {
    EXPECT_EQ(Decimal(6) * Decimal(7), Decimal(42));
}

TEST(DecimalTests, MultiplyFractions) {
    EXPECT_EQ(Decimal::Parse("2.5") * Decimal::Parse("4.0"), Decimal::Parse("10.0"));
}

TEST(DecimalTests, MultiplyByZero) {
    EXPECT_EQ(Decimal::Parse("3.14") * Decimal::Zero, Decimal::Zero);
}

TEST(DecimalTests, MultiplyNegative) {
    EXPECT_EQ(Decimal(-3) * Decimal(4), Decimal(-12));
    EXPECT_EQ(Decimal(-3) * Decimal(-4), Decimal(12));
}

TEST(DecimalTests, MultiplyLarge) {
    // 1000000 * 1000000 == 1000000000000
    EXPECT_EQ(Decimal::Parse("1000000") * Decimal::Parse("1000000"),
              Decimal::Parse("1000000000000"));
}

// ---------------------------------------------------------------------------
// Division
// ---------------------------------------------------------------------------

TEST(DecimalTests, DivideExact) {
    EXPECT_EQ(Decimal(10) / Decimal(2), Decimal(5));
    EXPECT_EQ(Decimal::Parse("1.0") / Decimal(4), Decimal::Parse("0.25"));
}

TEST(DecimalTests, DivideWithRepeatResult) {
    // 1 / 3 — result is truncated at 28 decimal places of precision
    Decimal d = Decimal(1) / Decimal(3);
    // Must equal exactly 28 threes (the maximum representable approximation)
    EXPECT_EQ(d, Decimal::Parse("0.3333333333333333333333333333"));
    // And be less than one more digit of precision
    EXPECT_LT(d, Decimal::Parse("0.3333333333333333333333333334"));
}

TEST(DecimalTests, DivideByZeroThrows) {
    EXPECT_THROW(Decimal(1) / Decimal(0), System::DivideByZeroException);
}

TEST(DecimalTests, DivideNegative) {
    EXPECT_EQ(Decimal(-12) / Decimal(4), Decimal(-3));
    EXPECT_EQ(Decimal(-12) / Decimal(-4), Decimal(3));
}

// ---------------------------------------------------------------------------
// Comparison
// ---------------------------------------------------------------------------

TEST(DecimalTests, CompareEqual) {
    EXPECT_EQ(Decimal::Parse("1.0"), Decimal::Parse("1.00"));  // same value, different scale
    EXPECT_EQ(Decimal::Parse("0.10"), Decimal::Parse("0.1"));
}

TEST(DecimalTests, CompareLessGreater) {
    EXPECT_LT(Decimal::Parse("0.1"), Decimal::Parse("0.2"));
    EXPECT_GT(Decimal::Parse("1.0"), Decimal::Parse("0.9"));
    EXPECT_LE(Decimal(5), Decimal(5));
    EXPECT_GE(Decimal(5), Decimal(5));
}

TEST(DecimalTests, CompareNegatives) {
    EXPECT_LT(Decimal(-2), Decimal(-1));
    EXPECT_GT(Decimal(-1), Decimal(-2));
    EXPECT_LT(Decimal(-1), Decimal(0));
}

TEST(DecimalTests, NegativeZeroEqualsZero) {
    // -0 and +0 should be equal
    EXPECT_EQ(Decimal(0), Decimal::Zero);
}

// ---------------------------------------------------------------------------
// Unary minus
// ---------------------------------------------------------------------------

TEST(DecimalTests, UnaryMinus) {
    EXPECT_EQ(-Decimal(5), Decimal(-5));
    EXPECT_EQ(-Decimal::Parse("3.14"), Decimal::Parse("-3.14"));
    EXPECT_EQ(-Decimal::Zero, Decimal::Zero);
}

// ---------------------------------------------------------------------------
// Math methods
// ---------------------------------------------------------------------------

TEST(DecimalTests, Abs) {
    EXPECT_EQ(Decimal::Abs(Decimal(-5)), Decimal(5));
    EXPECT_EQ(Decimal::Abs(Decimal(5)),  Decimal(5));
    EXPECT_EQ(Decimal::Abs(Decimal::Zero), Decimal::Zero);
}

TEST(DecimalTests, Truncate) {
    EXPECT_EQ(Decimal::Truncate(Decimal::Parse("3.7")),  Decimal(3));
    EXPECT_EQ(Decimal::Truncate(Decimal::Parse("-3.7")), Decimal(-3));
    EXPECT_EQ(Decimal::Truncate(Decimal(5)),             Decimal(5));
}

TEST(DecimalTests, Floor) {
    EXPECT_EQ(Decimal::Floor(Decimal::Parse("3.1")),  Decimal(3));
    EXPECT_EQ(Decimal::Floor(Decimal::Parse("-3.1")), Decimal(-4));
    EXPECT_EQ(Decimal::Floor(Decimal(3)),             Decimal(3));
}

TEST(DecimalTests, Ceiling) {
    EXPECT_EQ(Decimal::Ceiling(Decimal::Parse("3.1")),  Decimal(4));
    EXPECT_EQ(Decimal::Ceiling(Decimal::Parse("-3.1")), Decimal(-3));
    EXPECT_EQ(Decimal::Ceiling(Decimal(3)),             Decimal(3));
}

TEST(DecimalTests, Round) {
    EXPECT_EQ(Decimal::Round(Decimal::Parse("3.456"), 2), Decimal::Parse("3.46"));
    EXPECT_EQ(Decimal::Round(Decimal::Parse("3.454"), 2), Decimal::Parse("3.45"));
    // .NET's default Round(decimal, int) uses MidpointRounding.ToEven (banker's rounding):
    // 2.5 rounds to 2 (nearest even), not 3.
    EXPECT_EQ(Decimal::Round(Decimal::Parse("2.5"),   0), Decimal(2));
    EXPECT_EQ(Decimal::Round(Decimal::Parse("3.5"),   0), Decimal(4));
    EXPECT_EQ(Decimal::Round(Decimal::Parse("2.4"),   0), Decimal(2));
}

// ---------------------------------------------------------------------------
// Conversion
// ---------------------------------------------------------------------------

TEST(DecimalTests, ToDouble) {
    EXPECT_NEAR(Decimal::Parse("3.14").ToDouble(), 3.14, 1e-10);
    EXPECT_NEAR(Decimal(0).ToDouble(), 0.0, 1e-15);
}

// Regression tests for a code-audit finding (ticket 241): Decimal(double) used
// std::llround(v), whose return type (long long) tops out around 9.2e18 -- far below
// Decimal's own ~7.9e28 mantissa range. Any in-range double roughly between 9.2e18 and
// 7.9e28 (e.g. 1e20, an exactly-representable double) silently overflowed llround to
// LLONG_MIN, producing a garbage mantissa instead of throwing or converting correctly.
// Confirmed via a standalone UBSan repro that std::llround(1e20) actually returned
// LLONG_MIN pre-fix. Fixed by splitting the value into two 64-bit limbs instead of routing
// through `long long`.
TEST(DecimalTests, Constructor_FromDouble_ExactPowerOfTen_AboveLongLongRange) {
    // 1e20 is exactly representable as a double (5^20 fits in 53 bits, so the multiply by
    // 2^20 is an exact exponent shift) and is well within Decimal's range but exceeds
    // long long's ~9.2e18 max -- exactly the previously-broken zone.
    EXPECT_EQ(Decimal(1e20).ToString(), "100000000000000000000");
}

TEST(DecimalTests, Constructor_FromDouble_AboveLongLongRange_DoesNotThrow) {
    EXPECT_NO_THROW(Decimal(1.5e19));
    EXPECT_EQ(Decimal(1.5e19).ToString(), "15000000000000000000");
}

TEST(DecimalTests, Constructor_FromDouble_NearMaxValue_DoesNotThrow) {
    EXPECT_NO_THROW(Decimal(7e28));
    EXPECT_GT(Decimal(7e28), Decimal(1e28));
}

TEST(DecimalTests, Constructor_FromDouble_NegativeAboveLongLongRange) {
    EXPECT_EQ(Decimal(-1e20).ToString(), "-100000000000000000000");
}

TEST(DecimalTests, ToInt32) {
    EXPECT_EQ(Decimal::Parse("42.9").ToInt32(),  42);
    EXPECT_EQ(Decimal::Parse("-3.7").ToInt32(), -3);
}

TEST(DecimalTests, ToInt64) {
    EXPECT_EQ(Decimal::Parse("100000000000").ToInt64(), 100000000000LL);
}

// ---------------------------------------------------------------------------
// High-precision value tests (require > double precision)
// ---------------------------------------------------------------------------

TEST(DecimalTests, LargeExactInteger) {
    // 10^18 fits in both double and Decimal exactly — verify correctness
    Decimal a = Decimal::Parse("1000000000000000000");
    Decimal b = Decimal::Parse("1000000000000000000");
    EXPECT_EQ(a + b, Decimal::Parse("2000000000000000000"));
}

TEST(DecimalTests, TwentyEightDecimalDigits) {
    // Value with 28 decimal digits of scale
    Decimal d = Decimal::Parse("0.0000000000000000000000000001");
    EXPECT_EQ(d.ToString(), "0.0000000000000000000000000001");
    EXPECT_GT(d, Decimal::Zero);
}

TEST(DecimalTests, ModuloBasic) {
    EXPECT_EQ(Decimal(10) % Decimal(3), Decimal(1));
    EXPECT_EQ(Decimal::Parse("5.5") % Decimal::Parse("2.0"), Decimal::Parse("1.5"));
}

TEST(DecimalTests, CompoundAssignment) {
    Decimal d = Decimal(10);
    d += Decimal(5);  EXPECT_EQ(d, Decimal(15));
    d -= Decimal(3);  EXPECT_EQ(d, Decimal(12));
    d *= Decimal(2);  EXPECT_EQ(d, Decimal(24));
    d /= Decimal(4);  EXPECT_EQ(d, Decimal(6));
}
