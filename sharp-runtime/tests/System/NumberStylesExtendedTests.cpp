// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Regression tests for the NumberStyles.AllowThousands/AllowDecimalPoint/AllowCurrencySymbol/
// AllowParentheses/AllowTrailingSign extension to include/System/detail/IntegerNumberStylesParser.hpp
// (NEXT.md 2026-07-13 task: extend ticket 1717's NumberStyles-aware integer parser to also
// support NumberStyles.Number and NumberStyles.Currency, not just .Integer/.HexNumber).
#include <gtest/gtest.h>
#include "System/Byte.hpp"
#include "System/Int16.hpp"
#include "System/Int32.hpp"
#include "System/Int64.hpp"
#include "System/SByte.hpp"
#include "System/UInt16.hpp"
#include "System/UInt32.hpp"
#include "System/UInt64.hpp"
#include "System/FormatException.hpp"
#include "System/OverflowException.hpp"

using System::Byte;
using System::Int16;
using System::Int32;
using System::Int64;
using System::SByte;
using System::UInt16;
using System::UInt32;
using System::UInt64;
using System::Globalization::NumberStyles;

namespace {
// UTF-8 encoding of U+00A4 (international currency sign), matching
// NumberFormatInfo.InvariantInfo.CurrencySymbol -- see IntegerNumberStylesParser's doc-comment.
constexpr const char* kCurrency = "\xC2\xA4";
} // namespace

// -----------------------------------------------------------------------
// AllowThousands
// -----------------------------------------------------------------------

TEST(NumberStylesExtendedTests, AllowThousands_SingleGroupSeparator) {
    EXPECT_EQ(Int32::Parse("1,234", NumberStyles::Number, nullptr), 1234);
}

TEST(NumberStylesExtendedTests, AllowThousands_MultipleGroupSeparators) {
    EXPECT_EQ(Int32::Parse("1,234,567", NumberStyles::Number, nullptr), 1234567);
}

TEST(NumberStylesExtendedTests, AllowThousands_NegativeWithGrouping) {
    EXPECT_EQ(Int32::Parse("-1,234", NumberStyles::Number, nullptr), -1234);
}

TEST(NumberStylesExtendedTests, AllowThousands_LeadingSeparatorIsInvalid) {
    SharpRuntime::intcs out;
    EXPECT_FALSE(Int32::TryParse(",1234", NumberStyles::Number, nullptr, out));
}

TEST(NumberStylesExtendedTests, AllowThousands_NotSetRejectsGroupedInput) {
    // Integer style (the pre-existing default) does NOT include AllowThousands -- confirms the
    // new grammar is gated by the flag, not unconditionally applied.
    SharpRuntime::intcs out;
    EXPECT_FALSE(Int32::TryParse("1,234", NumberStyles::Integer, nullptr, out));
}

// -----------------------------------------------------------------------
// AllowDecimalPoint
// -----------------------------------------------------------------------

TEST(NumberStylesExtendedTests, AllowDecimalPoint_TrailingZerosParsesAsInteger) {
    EXPECT_EQ(Int32::Parse("123.00", NumberStyles::Number, nullptr), 123);
    EXPECT_EQ(Int32::Parse("123.", NumberStyles::Number, nullptr), 123);
}

TEST(NumberStylesExtendedTests, AllowDecimalPoint_NonZeroFraction_ThrowsOverflowNotFormat) {
    // Faithful to real .NET's own quirk: TryNumberBufferToBinaryInteger rejects any nonzero
    // fractional digit with Overflow, not a format failure -- Int32.Parse("123.5",
    // NumberStyles.Number) throws OverflowException in real .NET, not FormatException.
    EXPECT_THROW(Int32::Parse("123.5", NumberStyles::Number, nullptr), System::OverflowException);
    SharpRuntime::intcs out;
    EXPECT_FALSE(Int32::TryParse("123.5", NumberStyles::Number, nullptr, out));
}

TEST(NumberStylesExtendedTests, AllowDecimalPoint_TrailingGarbageTakesPrecedenceOverOverflow) {
    // Format-grammar violations outrank the fractional-overflow quirk (matches real .NET's own
    // documented precedence) -- "123.5x" has trailing garbage, so it's a FormatException.
    EXPECT_THROW(Int32::Parse("123.5x", NumberStyles::Number, nullptr), System::FormatException);
}

TEST(NumberStylesExtendedTests, AllowDecimalPoint_OnlyFractionalDigits) {
    EXPECT_EQ(Int32::Parse(".00", NumberStyles::Number, nullptr), 0);
}

// -----------------------------------------------------------------------
// AllowCurrencySymbol / NumberStyles.Currency
// -----------------------------------------------------------------------

TEST(NumberStylesExtendedTests, Currency_LeadingSymbolWithGroupingAndDecimal) {
    std::string input = std::string(kCurrency) + "1,234.00";
    EXPECT_EQ(Int32::Parse(input, NumberStyles::Currency, nullptr), 1234);
}

TEST(NumberStylesExtendedTests, Currency_TrailingSymbol) {
    std::string input = "1234" + std::string(kCurrency);
    EXPECT_EQ(Int32::Parse(input, NumberStyles::Currency, nullptr), 1234);
}

TEST(NumberStylesExtendedTests, Currency_NegativeWithSymbol) {
    std::string input = "-" + std::string(kCurrency) + "1234";
    EXPECT_EQ(Int32::Parse(input, NumberStyles::Currency, nullptr), -1234);
}

TEST(NumberStylesExtendedTests, Currency_DuplicateSymbolIsInvalid) {
    std::string input = std::string(kCurrency) + "12" + std::string(kCurrency) + "34";
    SharpRuntime::intcs out;
    EXPECT_FALSE(Int32::TryParse(input, NumberStyles::Currency, nullptr, out));
}

// -----------------------------------------------------------------------
// AllowParentheses
// -----------------------------------------------------------------------

TEST(NumberStylesExtendedTests, AllowParentheses_WrapsNegativeValue) {
    EXPECT_EQ(Int32::Parse("(42)", NumberStyles::Integer | NumberStyles::AllowParentheses, nullptr), -42);
}

TEST(NumberStylesExtendedTests, AllowParentheses_UnclosedIsInvalid) {
    SharpRuntime::intcs out;
    EXPECT_FALSE(Int32::TryParse("(42", NumberStyles::Integer | NumberStyles::AllowParentheses, nullptr, out));
}

TEST(NumberStylesExtendedTests, AllowParentheses_UnsignedTypeRejectsParens) {
    // Parens always indicate a negative value; this port rejects that outright for unsigned
    // types rather than importing real .NET's negative-unsigned-throws-Overflow quirk -- see
    // TryParseUnsignedCore's doc-comment.
    SharpRuntime::uintcs out;
    EXPECT_FALSE(UInt32::TryParse("(5)", NumberStyles::Integer | NumberStyles::AllowParentheses, nullptr, out));
}

// -----------------------------------------------------------------------
// AllowTrailingSign
// -----------------------------------------------------------------------

TEST(NumberStylesExtendedTests, AllowTrailingSign_NegativeAfterDigits) {
    EXPECT_EQ(Int32::Parse("42-", NumberStyles::Integer | NumberStyles::AllowTrailingSign, nullptr), -42);
}

TEST(NumberStylesExtendedTests, AllowTrailingSign_PositiveAfterDigits) {
    EXPECT_EQ(Int32::Parse("42+", NumberStyles::Integer | NumberStyles::AllowTrailingSign, nullptr), 42);
}

TEST(NumberStylesExtendedTests, AllowTrailingSign_UnsignedRejectsTrailingMinus) {
    SharpRuntime::uintcs out;
    EXPECT_FALSE(UInt32::TryParse("42-", NumberStyles::Integer | NumberStyles::AllowTrailingSign, nullptr, out));
}

TEST(NumberStylesExtendedTests, AllowTrailingSign_UnsignedAcceptsTrailingPlus) {
    SharpRuntime::uintcs out;
    EXPECT_TRUE(UInt32::TryParse("42+", NumberStyles::Integer | NumberStyles::AllowTrailingSign, nullptr, out));
    EXPECT_EQ(out, 42u);
}

// -----------------------------------------------------------------------
// Coverage across the other 6 integer types (Int16/Int64/SByte/UInt16/UInt64/Byte), spot-checked
// with one representative case each rather than the full matrix (already exercised for Int32).
// -----------------------------------------------------------------------

TEST(NumberStylesExtendedTests, Int16_ThousandsAndDecimal) {
    EXPECT_EQ(Int16::Parse("1,234.00", NumberStyles::Number, nullptr), 1234);
}

TEST(NumberStylesExtendedTests, Int64_ThousandsAndDecimal) {
    EXPECT_EQ(Int64::Parse("1,234,567.00", NumberStyles::Number, nullptr), 1234567);
}

TEST(NumberStylesExtendedTests, SByte_Parentheses) {
    EXPECT_EQ(SByte::Parse("(12)", NumberStyles::Integer | NumberStyles::AllowParentheses, nullptr), -12);
}

TEST(NumberStylesExtendedTests, UInt16_ThousandsAndDecimal) {
    EXPECT_EQ(UInt16::Parse("1,234.00", NumberStyles::Number, nullptr), 1234);
}

TEST(NumberStylesExtendedTests, UInt64_Currency) {
    std::string input = std::string(kCurrency) + "1,234,567.00";
    EXPECT_EQ(UInt64::Parse(input, NumberStyles::Currency, nullptr), 1234567u);
}

TEST(NumberStylesExtendedTests, Byte_ThousandsSeparatorRejectedWhenTooLarge) {
    EXPECT_THROW(Byte::Parse("1,234", NumberStyles::Number, nullptr), System::OverflowException);
}

// -----------------------------------------------------------------------
// Whitespace interleaved with leading/trailing tokens (fixed 2026-07-14): AllowLeadingWhite/
// AllowTrailingWhite previously only skipped whitespace once, strictly before/after the
// sign/parentheses/currency-symbol matching loops -- real .NET tolerates whitespace BETWEEN a
// matched token and the digits (or end of string) too. Confirmed via source trace against
// Number.Parsing.Common.cs and empirical cross-check against real .NET/Mono.
// -----------------------------------------------------------------------

TEST(NumberStylesExtendedTests, TrailingWhitespace_AfterTrailingSign) {
    EXPECT_EQ(Int32::Parse("123-  ", NumberStyles::Number, nullptr), -123);
    EXPECT_EQ(Int32::Parse("123+  ", NumberStyles::Number, nullptr), 123);
}

TEST(NumberStylesExtendedTests, TrailingWhitespace_AfterClosingParenthesis) {
    EXPECT_EQ(Int32::Parse(" (123)  ", NumberStyles::Currency, nullptr), -123);
}

TEST(NumberStylesExtendedTests, TrailingWhitespace_AfterTrailingCurrencySymbol) {
    std::string input = "123" + std::string(kCurrency) + "  ";
    EXPECT_EQ(Int32::Parse(input, NumberStyles::Currency, nullptr), 123);
}

TEST(NumberStylesExtendedTests, LeadingWhitespace_BetweenCurrencySymbolAndDigits) {
    std::string input = std::string(kCurrency) + "  123";
    EXPECT_EQ(UInt32::Parse(input, NumberStyles::Currency, nullptr), 123u);
    EXPECT_EQ(Int32::Parse(input, NumberStyles::Currency, nullptr), 123);
}

TEST(NumberStylesExtendedTests, TrailingWhitespace_StillRejectedWhenNotAllowed) {
    // AllowLeadingSign | AllowTrailingSign WITHOUT AllowTrailingWhite (NumberStyles::Integer
    // would already bundle AllowTrailingWhite in, so it can't be used here to test this): the
    // fix must not make whitespace tolerance unconditional -- it stays gated by the flag.
    SharpRuntime::intcs out;
    EXPECT_FALSE(Int32::TryParse("123-  ", NumberStyles::AllowLeadingSign | NumberStyles::AllowTrailingSign, nullptr, out));
    EXPECT_TRUE(Int32::TryParse("123-", NumberStyles::AllowLeadingSign | NumberStyles::AllowTrailingSign, nullptr, out));
    EXPECT_EQ(out, -123);
}

// -----------------------------------------------------------------------
// HexNumber leading zeros (fixed 2026-07-14): digitCount previously included leading zeros when
// checking against maxDigits, so a harmlessly zero-padded hex string longer than the type's
// native width incorrectly overflowed even though its significant digit count fit. Confirmed
// against real .NET's TryParseBinaryIntegerHexOrBinaryNumberStyle, which skips leading zeros
// before counting.
// -----------------------------------------------------------------------

TEST(NumberStylesExtendedTests, HexNumber_LeadingZeros_DoNotCountTowardOverflow) {
    EXPECT_EQ(UInt32::Parse("000000001", NumberStyles::HexNumber, nullptr), 1u);
    EXPECT_EQ(UInt32::Parse("00000000FFFFFFFF", NumberStyles::HexNumber, nullptr), 4294967295u);
    EXPECT_EQ(Int32::Parse("00000000FFFFFFFF", NumberStyles::HexNumber, nullptr), -1);
}

TEST(NumberStylesExtendedTests, HexNumber_GenuineOverflow_StillThrows) {
    // Sanity check: 9 significant (non-zero-leading) digits still correctly overflows a 32-bit type.
    EXPECT_THROW(UInt32::Parse("100000000", NumberStyles::HexNumber, nullptr), System::OverflowException);
}

TEST(NumberStylesExtendedTests, HexNumber_AllZeros_ParsesAsZero) {
    EXPECT_EQ(UInt32::Parse("00000000", NumberStyles::HexNumber, nullptr), 0u);
    EXPECT_EQ(UInt32::Parse("0", NumberStyles::HexNumber, nullptr), 0u);
}

// -----------------------------------------------------------------------
// NumberStyles.BinaryNumber / AllowBinarySpecifier (added 2026-07-14): previously defined in
// NumberStyles.hpp but never checked by any Parse/TryParse overload, so it silently fell through
// to decimal parsing instead of throwing FormatException or parsing as binary -- e.g.
// Int32::TryParse("101", NumberStyles::BinaryNumber, ...) used to return true with result 101
// (a decimal reinterpretation), not 5. Mirrors the existing, already-correct HexNumber support.
// -----------------------------------------------------------------------

TEST(NumberStylesExtendedTests, BinaryNumber_Int32_RoundTrips) {
    EXPECT_EQ(Int32::Parse("101", NumberStyles::BinaryNumber, nullptr), 5);
    EXPECT_EQ(Int32::Parse("0", NumberStyles::BinaryNumber, nullptr), 0);
    EXPECT_EQ(Int32::Parse(std::string(32, '1'), NumberStyles::BinaryNumber, nullptr), -1);
    EXPECT_EQ(Int32::Parse(" 101 ", NumberStyles::BinaryNumber, nullptr), 5);
}

TEST(NumberStylesExtendedTests, BinaryNumber_LeadingZeros_DoNotCountTowardOverflow) {
    EXPECT_EQ(UInt32::Parse(std::string(20, '0') + "101", NumberStyles::BinaryNumber, nullptr), 5u);
}

TEST(NumberStylesExtendedTests, BinaryNumber_TooManyDigits_ThrowsOverflow) {
    EXPECT_THROW(UInt32::Parse(std::string(33, '1'), NumberStyles::BinaryNumber, nullptr), System::OverflowException);
    SharpRuntime::uintcs out;
    EXPECT_FALSE(UInt32::TryParse(std::string(33, '1'), NumberStyles::BinaryNumber, nullptr, out));
}

TEST(NumberStylesExtendedTests, BinaryNumber_InvalidDigit_ThrowsFormat) {
    EXPECT_THROW(Int32::Parse("102", NumberStyles::BinaryNumber, nullptr), System::FormatException);
}

TEST(NumberStylesExtendedTests, BinaryNumber_CoverageAcrossOtherTypes) {
    EXPECT_EQ(Int16::Parse("101", NumberStyles::BinaryNumber, nullptr), 5);
    EXPECT_EQ(Int64::Parse("101", NumberStyles::BinaryNumber, nullptr), 5);
    EXPECT_EQ(SByte::Parse("101", NumberStyles::BinaryNumber, nullptr), 5);
    EXPECT_EQ(UInt16::Parse("101", NumberStyles::BinaryNumber, nullptr), 5u);
    EXPECT_EQ(UInt64::Parse("101", NumberStyles::BinaryNumber, nullptr), 5u);
    EXPECT_EQ(Byte::Parse("101", NumberStyles::BinaryNumber, nullptr), 5u);
}

// -----------------------------------------------------------------------
// Unsigned-parser repeated-sign rejection (fixed 2026-07-14, duplicated-implementation audit
// finding): TryParseUnsignedCore's leading/trailing '+' matching had no "already consumed a
// sign" guard, unlike TryParseSignedCore's shared `haveSign` flag -- so multiple '+' tokens
// (leading, trailing, or both) were silently accepted instead of rejected as a format error.
// -----------------------------------------------------------------------

TEST(NumberStylesExtendedTests, UnsignedParse_RepeatedLeadingSign_Rejected) {
    SharpRuntime::uintcs out;
    EXPECT_FALSE(UInt32::TryParse("++5", NumberStyles::Integer, nullptr, out));
    EXPECT_FALSE(UInt32::TryParse("+++5", NumberStyles::Integer, nullptr, out));
}

TEST(NumberStylesExtendedTests, UnsignedParse_RepeatedTrailingSign_Rejected) {
    SharpRuntime::uintcs out;
    NumberStyles style = NumberStyles::Integer | NumberStyles::AllowTrailingSign;
    EXPECT_FALSE(UInt32::TryParse("5++", style, nullptr, out));
}

TEST(NumberStylesExtendedTests, UnsignedParse_LeadingAndTrailingSignTogether_Rejected) {
    SharpRuntime::uintcs out;
    NumberStyles style = NumberStyles::Integer | NumberStyles::AllowTrailingSign;
    EXPECT_FALSE(UInt32::TryParse("+5+", style, nullptr, out));
}

TEST(NumberStylesExtendedTests, UnsignedParse_SingleSign_StillAccepted) {
    // Sanity check: the fix must not reject a single, legitimate '+'.
    EXPECT_EQ(UInt32::Parse("+5", NumberStyles::Integer, nullptr), 5u);
    EXPECT_EQ(UInt32::Parse("5+", NumberStyles::Integer | NumberStyles::AllowTrailingSign, nullptr), 5u);
    EXPECT_EQ(Byte::Parse("+5", NumberStyles::Integer, nullptr), 5u);
}
