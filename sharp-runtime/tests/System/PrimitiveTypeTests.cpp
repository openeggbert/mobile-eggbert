// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include "System/FormatException.hpp"
#include "System/Int32.hpp"
#include "System/Int64.hpp"
#include "System/OverflowException.hpp"
#include "System/UInt32.hpp"

using System::Int32;
using System::Int64;
using System::UInt32;

// ── Int32 ────────────────────────────────────────────────────────────────────

TEST(Int32Tests, MaxMinValues) {
    EXPECT_EQ(Int32::MaxValue,  2147483647);
    EXPECT_EQ(Int32::MinValue, -2147483648);
}

TEST(Int32Tests, ParseValid) {
    EXPECT_EQ(Int32::Parse("0"),           0);
    EXPECT_EQ(Int32::Parse("42"),         42);
    EXPECT_EQ(Int32::Parse("-1"),         -1);
    EXPECT_EQ(Int32::Parse("2147483647"), Int32::MaxValue);
    EXPECT_EQ(Int32::Parse("-2147483648"), Int32::MinValue);
}

TEST(Int32Tests, ParseInvalidThrows) {
    EXPECT_THROW(Int32::Parse("abc"),          System::FormatException);
    EXPECT_THROW(Int32::Parse(""),             System::FormatException);
    EXPECT_THROW(Int32::Parse("99999999999"),  System::OverflowException);
}

TEST(Int32Tests, TryParseSuccess) {
    SharpRuntime::intcs v = 0;
    EXPECT_TRUE(Int32::TryParse("123", v));
    EXPECT_EQ(v, 123);

    EXPECT_TRUE(Int32::TryParse("-2147483648", v));
    EXPECT_EQ(v, Int32::MinValue);
}

TEST(Int32Tests, TryParseFailure) {
    SharpRuntime::intcs v = 99;
    EXPECT_FALSE(Int32::TryParse("nope", v));
    EXPECT_EQ(v, 0);
}

TEST(Int32Tests, ToString) {
    EXPECT_EQ(Int32::ToString(0),   "0");
    EXPECT_EQ(Int32::ToString(-1),  "-1");
    EXPECT_EQ(Int32::ToString(2147483647), "2147483647");
}

// ── Int64 ────────────────────────────────────────────────────────────────────

TEST(Int64Tests, MaxMinValues) {
    EXPECT_EQ(Int64::MaxValue,  9223372036854775807LL);
    EXPECT_EQ(Int64::MinValue, -9223372036854775807LL - 1LL);
}

TEST(Int64Tests, ParseValid) {
    EXPECT_EQ(Int64::Parse("0"),                    0LL);
    EXPECT_EQ(Int64::Parse("9223372036854775807"),  Int64::MaxValue);
    EXPECT_EQ(Int64::Parse("-9223372036854775808"), Int64::MinValue);
    EXPECT_EQ(Int64::Parse("-100"),                -100LL);
}

TEST(Int64Tests, ParseInvalidThrows) {
    EXPECT_THROW(Int64::Parse("xyz"),                   System::FormatException);
    EXPECT_THROW(Int64::Parse(""),                      System::FormatException);
    EXPECT_THROW(Int64::Parse("99999999999999999999"),  System::OverflowException);
}

TEST(Int64Tests, TryParseSuccess) {
    SharpRuntime::longcs v = 0;
    EXPECT_TRUE(Int64::TryParse("9223372036854775807", v));
    EXPECT_EQ(v, Int64::MaxValue);
}

TEST(Int64Tests, TryParseFailure) {
    SharpRuntime::longcs v = 7;
    EXPECT_FALSE(Int64::TryParse("bad", v));
    EXPECT_EQ(v, 0);
}

TEST(Int64Tests, ToString) {
    EXPECT_EQ(Int64::ToString(0LL),                   "0");
    EXPECT_EQ(Int64::ToString(9223372036854775807LL), "9223372036854775807");
    EXPECT_EQ(Int64::ToString(-1LL),                  "-1");
}

// ── UInt32 ───────────────────────────────────────────────────────────────────

TEST(UInt32Tests, MaxMinValues) {
    EXPECT_EQ(UInt32::MaxValue, 4294967295U);
    EXPECT_EQ(UInt32::MinValue, 0U);
}

TEST(UInt32Tests, ParseValid) {
    EXPECT_EQ(UInt32::Parse("0"),          0U);
    EXPECT_EQ(UInt32::Parse("4294967295"), UInt32::MaxValue);
    EXPECT_EQ(UInt32::Parse("100"),        100U);
}

TEST(UInt32Tests, ParseInvalidThrows) {
    EXPECT_THROW(UInt32::Parse("abc"),          System::FormatException);
    EXPECT_THROW(UInt32::Parse(""),             System::FormatException);
    EXPECT_THROW(UInt32::Parse("99999999999"),  System::OverflowException);
}

TEST(UInt32Tests, Parse_TrailingGarbage_Throws) {
    EXPECT_THROW(UInt32::Parse("5abc"), System::FormatException);
}

TEST(UInt32Tests, Parse_NegativeValue_Throws) {
    EXPECT_THROW(UInt32::Parse("-1"), System::OverflowException);
}

TEST(UInt32Tests, Parse_NegativeZero_Throws) {
    // Unsigned types reject any leading '-', even "-0". On an LLP64 platform, where
    // `unsigned long` is the same 32-bit width as UInt32, std::stoul("-1") wraps to
    // ULONG_MAX == UInt32::MaxValue exactly, which the old `v > MaxValue` (not `>=`) check
    // did not catch -- and std::stoul("-0") returns the literal 0 on every platform, which
    // is never caught by any magnitude-based check regardless of width.
    EXPECT_THROW(UInt32::Parse("-0"), System::OverflowException);
}

TEST(UInt32Tests, TryParseSuccess) {
    SharpRuntime::uintcs v = 0;
    EXPECT_TRUE(UInt32::TryParse("4294967295", v));
    EXPECT_EQ(v, UInt32::MaxValue);
}

TEST(UInt32Tests, TryParseFailure) {
    SharpRuntime::uintcs v = 5;
    EXPECT_FALSE(UInt32::TryParse("nope", v));
    EXPECT_EQ(v, 0U);
}

TEST(UInt32Tests, ToString) {
    EXPECT_EQ(UInt32::ToString(0U),          "0");
    EXPECT_EQ(UInt32::ToString(4294967295U), "4294967295");
}

// ---------------------------------------------------------------------------
// Int32::ToString(format)
// ---------------------------------------------------------------------------
TEST(Int32Tests, ToString_Hex_Uppercase) { EXPECT_EQ(Int32::ToString(255, std::string("X")), "FF"); }
TEST(Int32Tests, ToString_Hex_Padded)    { EXPECT_EQ(Int32::ToString(255, std::string("X4")), "00FF"); }
TEST(Int32Tests, ToString_Hex_Lower)     { EXPECT_EQ(Int32::ToString(255, std::string("x")), "ff"); }
TEST(Int32Tests, ToString_D_NoWidth)     { EXPECT_EQ(Int32::ToString(42, std::string("D")), "42"); }
TEST(Int32Tests, ToString_D_Padded)      { EXPECT_EQ(Int32::ToString(7, std::string("D3")), "007"); }
TEST(Int32Tests, ToString_D_Negative)    { EXPECT_EQ(Int32::ToString(-5, std::string("D3")), "-005"); }
TEST(Int32Tests, ToString_G)             { EXPECT_EQ(Int32::ToString(99, std::string("G")), "99"); }
TEST(Int32Tests, ToString_B_Basic)       { EXPECT_EQ(Int32::ToString(5, std::string("B")), "101"); }
TEST(Int32Tests, ToString_B_Padded)      { EXPECT_EQ(Int32::ToString(5, std::string("B8")), "00000101"); }
TEST(Int32Tests, ToString_MalformedWidth_ThrowsFormatException) {
    // std::stoi's raw std::invalid_argument/out_of_range must not escape as-is.
    EXPECT_THROW(Int32::ToString(5, std::string("Xz")), System::FormatException);
    EXPECT_THROW(Int32::ToString(5, std::string("X99999999999999999999")), System::FormatException);
}

// ---------------------------------------------------------------------------
// Int32 — arithmetic helpers
// ---------------------------------------------------------------------------

TEST(Int32Tests, BigMul_PositiveValues) {
    EXPECT_EQ(Int32::BigMul(100000, 100000), 10000000000LL);
}
TEST(Int32Tests, BigMul_NegativeTimesPositive) {
    EXPECT_EQ(Int32::BigMul(-3, 7), -21LL);
}
TEST(Int32Tests, DivRem_Basic) {
    auto [q, r] = Int32::DivRem(10, 3);
    EXPECT_EQ(q, 3);
    EXPECT_EQ(r, 1);
}
TEST(Int32Tests, DivRem_NegativeDividend) {
    auto [q, r] = Int32::DivRem(-10, 3);
    EXPECT_EQ(q, -3);
    EXPECT_EQ(r, -1);
}
TEST(Int32Tests, DivRem_ByZero_ThrowsDivideByZeroException) {
    // Integer division by zero is undefined behavior (hardware trap) in C++, unlike the
    // CLR's div instruction which .NET surfaces as a catchable DivideByZeroException.
    EXPECT_THROW(Int32::DivRem(10, 0), System::DivideByZeroException);
}
TEST(Int32Tests, DivRem_MinValueByNegativeOne_ThrowsOverflowException) {
    EXPECT_THROW(Int32::DivRem(Int32::MinValue, -1), System::OverflowException);
}
TEST(Int32Tests, Abs_Positive) { EXPECT_EQ(Int32::Abs(42), 42); }
TEST(Int32Tests, Abs_Negative) { EXPECT_EQ(Int32::Abs(-42), 42); }
TEST(Int32Tests, Abs_Zero)     { EXPECT_EQ(Int32::Abs(0), 0); }
TEST(Int32Tests, Abs_MinValue_Throws) {
    // Int16/Int64/SByte/Int128 all have this test already; Int32 (the most-used integer type in
    // game code) was missing it despite Abs() being correctly implemented.
    EXPECT_THROW(Int32::Abs(Int32::MinValue), System::OverflowException);
}
TEST(Int32Tests, CopySign_PositiveValuePositiveSign)  { EXPECT_EQ(Int32::CopySign(5, 3), 5); }
TEST(Int32Tests, CopySign_PositiveValueNegativeSign)  { EXPECT_EQ(Int32::CopySign(5, -3), -5); }
TEST(Int32Tests, CopySign_NegativeValuePositiveSign)  { EXPECT_EQ(Int32::CopySign(-5, 3), 5); }
TEST(Int32Tests, CopySign_NegativeValueNegativeSign)  { EXPECT_EQ(Int32::CopySign(-5, -3), -5); }
TEST(Int32Tests, CopySign_MinValueNegativeSign_ReturnsMinValue) {
    // MinValue is already negative, so copying a negative sign onto it is a no-op --
    // must not attempt to negate MinValue, which is undefined behavior in C++.
    EXPECT_EQ(Int32::CopySign(Int32::MinValue, -1), Int32::MinValue);
}
TEST(Int32Tests, CopySign_MinValuePositiveSign_ThrowsOverflowException) {
    EXPECT_THROW(Int32::CopySign(Int32::MinValue, 1), System::OverflowException);
}
TEST(Int32Tests, Clamp_InRange)    { EXPECT_EQ(Int32::Clamp(5, 1, 10), 5); }
TEST(Int32Tests, Clamp_BelowMin)   { EXPECT_EQ(Int32::Clamp(-5, 1, 10), 1); }
TEST(Int32Tests, Clamp_AboveMax)   { EXPECT_EQ(Int32::Clamp(15, 1, 10), 10); }
TEST(Int32Tests, Max_ReturnsLarger) { EXPECT_EQ(Int32::Max(3, 7), 7); }
TEST(Int32Tests, Min_ReturnsSmaller) { EXPECT_EQ(Int32::Min(3, 7), 3); }
TEST(Int32Tests, Sign_Positive) { EXPECT_EQ(Int32::Sign(42), 1); }
TEST(Int32Tests, Sign_Negative) { EXPECT_EQ(Int32::Sign(-42), -1); }
TEST(Int32Tests, Sign_Zero)     { EXPECT_EQ(Int32::Sign(0), 0); }
TEST(Int32Tests, MaxMagnitude_ReturnsCorrect) {
    EXPECT_EQ(Int32::MaxMagnitude(-10, 5), -10);
}
TEST(Int32Tests, MinMagnitude_ReturnsCorrect) {
    EXPECT_EQ(Int32::MinMagnitude(-10, 5), 5);
}

// ---------------------------------------------------------------------------
// Int32 — bit operations
// ---------------------------------------------------------------------------

TEST(Int32Tests, LeadingZeroCount_One)  { EXPECT_EQ(Int32::LeadingZeroCount(1), 31); }
TEST(Int32Tests, LeadingZeroCount_Zero) { EXPECT_EQ(Int32::LeadingZeroCount(0), 32); }
TEST(Int32Tests, TrailingZeroCount_Eight) { EXPECT_EQ(Int32::TrailingZeroCount(8), 3); }
TEST(Int32Tests, TrailingZeroCount_One)   { EXPECT_EQ(Int32::TrailingZeroCount(1), 0); }
TEST(Int32Tests, PopCount_Zero)    { EXPECT_EQ(Int32::PopCount(0), 0); }
TEST(Int32Tests, PopCount_Seven)   { EXPECT_EQ(Int32::PopCount(7), 3); }
TEST(Int32Tests, RotateLeft_Basic) { EXPECT_EQ(Int32::RotateLeft(1, 1), 2); }
TEST(Int32Tests, RotateRight_Basic) { EXPECT_EQ(Int32::RotateRight(2, 1), 1); }
TEST(Int32Tests, IsPow2_True)  { EXPECT_TRUE(Int32::IsPow2(64)); }
TEST(Int32Tests, IsPow2_False) { EXPECT_FALSE(Int32::IsPow2(63)); }
TEST(Int32Tests, IsPow2_NegativeFalse) { EXPECT_FALSE(Int32::IsPow2(-4)); }
TEST(Int32Tests, Log2_Eight)  { EXPECT_EQ(Int32::Log2(8), 3); }
TEST(Int32Tests, Log2_One)    { EXPECT_EQ(Int32::Log2(1), 0); }

// ---------------------------------------------------------------------------
// Int32 — classification predicates
// ---------------------------------------------------------------------------

TEST(Int32Tests, IsEvenInteger_Even) { EXPECT_TRUE(Int32::IsEvenInteger(4)); }
TEST(Int32Tests, IsEvenInteger_Odd)  { EXPECT_FALSE(Int32::IsEvenInteger(3)); }
TEST(Int32Tests, IsOddInteger_Odd)   { EXPECT_TRUE(Int32::IsOddInteger(3)); }
TEST(Int32Tests, IsOddInteger_Even)  { EXPECT_FALSE(Int32::IsOddInteger(4)); }
TEST(Int32Tests, IsNegative_True)    { EXPECT_TRUE(Int32::IsNegative(-1)); }
TEST(Int32Tests, IsNegative_False)   { EXPECT_FALSE(Int32::IsNegative(0)); }
TEST(Int32Tests, IsPositive_True)    { EXPECT_TRUE(Int32::IsPositive(0)); }
TEST(Int32Tests, IsPositive_False)   { EXPECT_FALSE(Int32::IsPositive(-1)); }

// ---------------------------------------------------------------------------
// Int64::ToString(format)
// ---------------------------------------------------------------------------
TEST(Int64Tests, ToString_Hex) { EXPECT_EQ(Int64::ToString(255LL, std::string("X")), "FF"); }
TEST(Int64Tests, ToString_D_Padded) { EXPECT_EQ(Int64::ToString(7LL, std::string("D5")), "00007"); }
TEST(Int64Tests, ToString_MalformedWidth_ThrowsFormatException) {
    EXPECT_THROW(Int64::ToString(5LL, std::string("Xz")), System::FormatException);
    EXPECT_THROW(Int64::ToString(5LL, std::string("X99999999999999999999")), System::FormatException);
}
// Regression for ticket 337: real .NET's Int64.ToString supports the "B"/"b" binary format
// specifier (Number.Formatting.cs's FormatInt64, added in .NET 8) same as Int32; this port's
// Int64::ToString(value, format) was missing it entirely (silently falling through to plain
// decimal) even though Int32::ToString already implemented it correctly.
TEST(Int64Tests, ToString_B_Basic)  { EXPECT_EQ(Int64::ToString(5LL, std::string("B")), "101"); }
TEST(Int64Tests, ToString_B_Padded) { EXPECT_EQ(Int64::ToString(5LL, std::string("B8")), "00000101"); }
TEST(Int64Tests, ToString_B_Zero)   { EXPECT_EQ(Int64::ToString(0LL, std::string("B")), "0"); }
