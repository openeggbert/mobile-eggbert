// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Decimal.hpp"
#include "System/MidpointRounding.hpp"
#include "System/OverflowException.hpp"

using System::Decimal;
using System::MidpointRounding;
using SharpRuntime::intcs;

// Suite name uses a Tests2 suffix per NEXT.md's documented duplicate-suite-name
// policy: DecimalTests already exists in DecimalTests.cpp / DecimalNewTests.cpp.

// ---------------------------------------------------------------------------
// New constructors
// ---------------------------------------------------------------------------

TEST(DecimalTests2, CtorFromUnsignedInt) {
    Decimal d(SharpRuntime::uintcs(42));
    EXPECT_EQ(d, Decimal(42));
    EXPECT_FALSE(d.getIsNegativeProperty());
}

TEST(DecimalTests2, CtorFromUnsignedLong) {
    Decimal d(SharpRuntime::ulongcs(123456789012345ULL));
    EXPECT_EQ(d.ToUInt64(), 123456789012345ULL);
}

TEST(DecimalTests2, CtorFromFloat) {
    Decimal d(1.5f);
    EXPECT_NEAR(d.ToDouble(), 1.5, 1e-6);
}

TEST(DecimalTests2, CtorFromBits_RoundTrips) {
    Decimal original = Decimal::Parse("12345.6789");
    intcs lo, mid, hi, flags;
    Decimal::GetBits(original, lo, mid, hi, flags);
    Decimal rebuilt(lo, mid, hi, flags < 0, SharpRuntime::bytecs((uint32_t(flags) >> 16) & 0xFF));
    EXPECT_EQ(rebuilt, original);
    EXPECT_EQ(rebuilt.getScaleProperty(), original.getScaleProperty());
}

TEST(DecimalTests2, CtorFromBits_Negative) {
    Decimal d(5, 0, 0, true, 1); // -0.5
    EXPECT_TRUE(d.getIsNegativeProperty());
    EXPECT_EQ(d, Decimal::Parse("-0.5"));
}

TEST(DecimalTests2, CtorFromBits_ScaleTooLarge_Throws) {
    EXPECT_THROW(Decimal(1, 0, 0, false, 29), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// GetBits
// ---------------------------------------------------------------------------

TEST(DecimalTests2, GetBits_Zero) {
    intcs lo, mid, hi, flags;
    Decimal::GetBits(Decimal::Zero, lo, mid, hi, flags);
    EXPECT_EQ(lo, 0);
    EXPECT_EQ(mid, 0);
    EXPECT_EQ(hi, 0);
    EXPECT_EQ(flags, 0);
}

TEST(DecimalTests2, GetBits_ScaleAndSignEncoded) {
    Decimal d = Decimal::Parse("-1.23");
    intcs lo, mid, hi, flags;
    Decimal::GetBits(d, lo, mid, hi, flags);
    EXPECT_EQ(lo, 123);
    EXPECT_EQ(mid, 0);
    EXPECT_EQ(hi, 0);
    EXPECT_EQ((uint32_t(flags) >> 16) & 0xFF, 2u); // scale = 2
    EXPECT_NE(uint32_t(flags) & 0x80000000u, 0u);  // sign bit set
}

// ---------------------------------------------------------------------------
// Static To* overloads (mirroring .NET's static method surface)
// ---------------------------------------------------------------------------

TEST(DecimalTests2, StaticToInt32_MatchesInstance) {
    Decimal d(42);
    EXPECT_EQ(Decimal::ToInt32(d), d.ToInt32());
}

TEST(DecimalTests2, ToByte_Valid) {
    EXPECT_EQ(Decimal::ToByte(Decimal(200)), 200);
}

TEST(DecimalTests2, ToByte_OutOfRange_Throws) {
    EXPECT_THROW(Decimal::ToByte(Decimal(-1)), System::OverflowException);
    EXPECT_THROW(Decimal::ToByte(Decimal(256)), System::OverflowException);
}

TEST(DecimalTests2, ToSByte_Valid) {
    EXPECT_EQ(Decimal::ToSByte(Decimal(-100)), -100);
}

TEST(DecimalTests2, ToSByte_OutOfRange_Throws) {
    EXPECT_THROW(Decimal::ToSByte(Decimal(200)), System::OverflowException);
}

TEST(DecimalTests2, ToInt16_Valid) {
    EXPECT_EQ(Decimal::ToInt16(Decimal(-30000)), -30000);
}

TEST(DecimalTests2, ToInt16_OutOfRange_Throws) {
    EXPECT_THROW(Decimal::ToInt16(Decimal(100000)), System::OverflowException);
}

TEST(DecimalTests2, ToUInt16_Valid) {
    EXPECT_EQ(Decimal::ToUInt16(Decimal(60000)), 60000);
}

TEST(DecimalTests2, ToUInt16_OutOfRange_Throws) {
    EXPECT_THROW(Decimal::ToUInt16(Decimal(-1)), System::OverflowException);
    EXPECT_THROW(Decimal::ToUInt16(Decimal(70000)), System::OverflowException);
}

// ---------------------------------------------------------------------------
// ToInt32/ToInt64/ToUInt32/ToUInt64 overflow parity (regression: previously
// silently wrapped instead of throwing, and ToUInt64 incorrectly threw for
// negative values that truncate to zero, e.g. -0.5).
// ---------------------------------------------------------------------------

TEST(DecimalTests2, ToInt32_Overflow_Throws) {
    EXPECT_THROW(Decimal::MaxValue.ToInt32(), System::OverflowException);
}

TEST(DecimalTests2, ToInt64_Overflow_Throws) {
    EXPECT_THROW(Decimal::MaxValue.ToInt64(), System::OverflowException);
}

TEST(DecimalTests2, ToUInt64_NegativeTruncatingToZero_ReturnsZero) {
    EXPECT_EQ(Decimal::Parse("-0.5").ToUInt64(), 0ULL);
}

TEST(DecimalTests2, ToUInt64_NegativeNonZero_Throws) {
    EXPECT_THROW(Decimal::Parse("-3.5").ToUInt64(), System::OverflowException);
}

TEST(DecimalTests2, ToUInt32_NegativeTruncatingToZero_ReturnsZero) {
    EXPECT_EQ(Decimal::Parse("-0.5").ToUInt32(), 0u);
}

// ---------------------------------------------------------------------------
// Round: MidpointRounding overloads and multi-digit correctness
// (regression: previous digit-by-digit rounding produced wrong results when
// dropping more than one decimal place, e.g. Round(12.49, 0) used to yield 13)
// ---------------------------------------------------------------------------

TEST(DecimalTests2, Round_MultiDigit_RoundsDownCorrectly) {
    EXPECT_EQ(Decimal::Round(Decimal::Parse("12.49"), 0), Decimal(12));
}

TEST(DecimalTests2, Round_MultiDigit_RoundsUpCorrectly) {
    EXPECT_EQ(Decimal::Round(Decimal::Parse("12.50"), 0), Decimal(12)); // ToEven: 12 is even
    EXPECT_EQ(Decimal::Round(Decimal::Parse("12.51"), 0), Decimal(13));
}

TEST(DecimalTests2, Round_DefaultIsToEven) {
    EXPECT_EQ(Decimal::Round(Decimal::Parse("0.5")), Decimal(0));
    EXPECT_EQ(Decimal::Round(Decimal::Parse("1.5")), Decimal(2));
    EXPECT_EQ(Decimal::Round(Decimal::Parse("2.5")), Decimal(2));
}

TEST(DecimalTests2, Round_AwayFromZero) {
    EXPECT_EQ(Decimal::Round(Decimal::Parse("2.5"), MidpointRounding::AwayFromZero), Decimal(3));
    EXPECT_EQ(Decimal::Round(Decimal::Parse("0.5"), MidpointRounding::AwayFromZero), Decimal(1));
}

TEST(DecimalTests2, Round_ToZero_Truncates) {
    EXPECT_EQ(Decimal::Round(Decimal::Parse("2.9"), 0, MidpointRounding::ToZero), Decimal(2));
    EXPECT_EQ(Decimal::Round(Decimal::Parse("-2.9"), 0, MidpointRounding::ToZero), Decimal(-2));
}

TEST(DecimalTests2, Round_ToNegativeInfinity) {
    EXPECT_EQ(Decimal::Round(Decimal::Parse("2.1"), 0, MidpointRounding::ToNegativeInfinity), Decimal(2));
    EXPECT_EQ(Decimal::Round(Decimal::Parse("-2.1"), 0, MidpointRounding::ToNegativeInfinity), Decimal(-3));
}

TEST(DecimalTests2, Round_ToPositiveInfinity) {
    EXPECT_EQ(Decimal::Round(Decimal::Parse("2.1"), 0, MidpointRounding::ToPositiveInfinity), Decimal(3));
    EXPECT_EQ(Decimal::Round(Decimal::Parse("-2.1"), 0, MidpointRounding::ToPositiveInfinity), Decimal(-2));
}

TEST(DecimalTests2, Round_MultiDecimalPlace) {
    EXPECT_EQ(Decimal::Round(Decimal::Parse("3.14159"), 2), Decimal::Parse("3.14"));
    EXPECT_EQ(Decimal::Round(Decimal::Parse("3.145"), 2), Decimal::Parse("3.14")); // tie -> even
    EXPECT_EQ(Decimal::Round(Decimal::Parse("3.155"), 2), Decimal::Parse("3.16")); // tie -> even
}

// ---------------------------------------------------------------------------
// CopySign / MaxMagnitude / MinMagnitude
// ---------------------------------------------------------------------------

TEST(DecimalTests2, CopySign) {
    EXPECT_EQ(Decimal::CopySign(Decimal(5), Decimal(-1)), Decimal(-5));
    EXPECT_EQ(Decimal::CopySign(Decimal(-5), Decimal(1)), Decimal(5));
}

TEST(DecimalTests2, MaxMagnitude) {
    EXPECT_EQ(Decimal::MaxMagnitude(Decimal(-5), Decimal(3)), Decimal(-5));
    EXPECT_EQ(Decimal::MaxMagnitude(Decimal(-5), Decimal(5)), Decimal(5)); // tie -> non-negative wins
}

TEST(DecimalTests2, MinMagnitude) {
    EXPECT_EQ(Decimal::MinMagnitude(Decimal(-5), Decimal(3)), Decimal(3));
    EXPECT_EQ(Decimal::MinMagnitude(Decimal(-5), Decimal(5)), Decimal(-5)); // tie -> negative wins
}

// ---------------------------------------------------------------------------
// IsInteger / IsNegative / IsPositive / IsEvenInteger / IsOddInteger / IsCanonical
// ---------------------------------------------------------------------------

TEST(DecimalTests2, IsInteger) {
    EXPECT_TRUE(Decimal::IsInteger(Decimal(5)));
    EXPECT_FALSE(Decimal::IsInteger(Decimal::Parse("5.5")));
}

TEST(DecimalTests2, IsNegativeIsPositive) {
    EXPECT_TRUE(Decimal::IsNegative(Decimal(-1)));
    EXPECT_FALSE(Decimal::IsPositive(Decimal(-1)));
    EXPECT_TRUE(Decimal::IsPositive(Decimal(1)));
    EXPECT_TRUE(Decimal::IsPositive(Decimal::Zero));
}

TEST(DecimalTests2, IsEvenOddInteger) {
    EXPECT_TRUE(Decimal::IsEvenInteger(Decimal(4)));
    EXPECT_FALSE(Decimal::IsEvenInteger(Decimal(5)));
    EXPECT_TRUE(Decimal::IsOddInteger(Decimal(5)));
    EXPECT_FALSE(Decimal::IsOddInteger(Decimal(4)));
    EXPECT_FALSE(Decimal::IsEvenInteger(Decimal::Parse("4.5")));
    EXPECT_FALSE(Decimal::IsOddInteger(Decimal::Parse("4.5")));
}

TEST(DecimalTests2, IsCanonical) {
    EXPECT_TRUE(Decimal::IsCanonical(Decimal(5)));
    EXPECT_TRUE(Decimal::IsCanonical(Decimal::Parse("5.5")));
    // 5.50 has a trailing zero in scale 2, so it's not canonical.
    Decimal nonCanonical(550, 0, 0, false, 2);
    EXPECT_FALSE(Decimal::IsCanonical(nonCanonical));
}

// ---------------------------------------------------------------------------
// Unary +, ++, --
// ---------------------------------------------------------------------------

TEST(DecimalTests2, UnaryPlus) {
    Decimal d(5);
    EXPECT_EQ(+d, Decimal(5));
}

TEST(DecimalTests2, PreIncrement) {
    Decimal d(5);
    Decimal& r = ++d;
    EXPECT_EQ(d, Decimal(6));
    EXPECT_EQ(&r, &d);
}

TEST(DecimalTests2, PostIncrement) {
    Decimal d(5);
    Decimal old = d++;
    EXPECT_EQ(old, Decimal(5));
    EXPECT_EQ(d, Decimal(6));
}

TEST(DecimalTests2, PreDecrement) {
    Decimal d(5);
    --d;
    EXPECT_EQ(d, Decimal(4));
}

TEST(DecimalTests2, PostDecrement) {
    Decimal d(5);
    Decimal old = d--;
    EXPECT_EQ(old, Decimal(5));
    EXPECT_EQ(d, Decimal(4));
}

// ---------------------------------------------------------------------------
// Explicit conversion operators
// ---------------------------------------------------------------------------

TEST(DecimalTests2, ExplicitConversionOperators) {
    Decimal d(42);
    EXPECT_EQ(static_cast<SharpRuntime::intcs>(d), 42);
    EXPECT_EQ(static_cast<SharpRuntime::longcs>(d), 42);
    EXPECT_EQ(static_cast<SharpRuntime::uintcs>(d), 42u);
    EXPECT_EQ(static_cast<SharpRuntime::ulongcs>(d), 42u);
    EXPECT_NEAR(static_cast<double>(d), 42.0, 1e-9);
    EXPECT_NEAR(static_cast<float>(d), 42.0f, 1e-6f);
    EXPECT_EQ(static_cast<SharpRuntime::bytecs>(d), 42);
    EXPECT_EQ(static_cast<SharpRuntime::sbytecs>(d), 42);
    EXPECT_EQ(static_cast<SharpRuntime::shortcs>(d), 42);
    EXPECT_EQ(static_cast<SharpRuntime::ushortcs>(d), 42);
}
