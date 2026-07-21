// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Char.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/ArgumentException.hpp"

using System::Char;
using SharpRuntime::charcs;

// ---------------------------------------------------------------------------
// ToUpperInvariant / ToLowerInvariant
// ---------------------------------------------------------------------------

TEST(CharTests2, ToUpperInvariant_Lower_ReturnsUpper) {
    EXPECT_EQ(Char::ToUpperInvariant(u'a'), u'A');
    EXPECT_EQ(Char::ToUpperInvariant(u'z'), u'Z');
}

TEST(CharTests2, ToUpperInvariant_AlreadyUpper_Unchanged) {
    EXPECT_EQ(Char::ToUpperInvariant(u'A'), u'A');
}

TEST(CharTests2, ToUpperInvariant_NonLetter_Unchanged) {
    EXPECT_EQ(Char::ToUpperInvariant(u'5'), u'5');
}

TEST(CharTests2, ToLowerInvariant_Upper_ReturnsLower) {
    EXPECT_EQ(Char::ToLowerInvariant(u'A'), u'a');
    EXPECT_EQ(Char::ToLowerInvariant(u'Z'), u'z');
}

TEST(CharTests2, ToLowerInvariant_AlreadyLower_Unchanged) {
    EXPECT_EQ(Char::ToLowerInvariant(u'a'), u'a');
}

TEST(CharTests2, ToLowerInvariant_NonLetter_Unchanged) {
    EXPECT_EQ(Char::ToLowerInvariant(u'9'), u'9');
}

// ---------------------------------------------------------------------------
// CompareTo / Equals
// ---------------------------------------------------------------------------

TEST(CharTests2, CompareTo_Equal_Zero) {
    EXPECT_EQ(Char::CompareTo(u'A', u'A'), 0);
}

TEST(CharTests2, CompareTo_Less_Negative) {
    EXPECT_LT(Char::CompareTo(u'A', u'B'), 0);
}

TEST(CharTests2, CompareTo_Greater_Positive) {
    EXPECT_GT(Char::CompareTo(u'B', u'A'), 0);
}

TEST(CharTests2, Equals_SameChar_True) {
    EXPECT_TRUE(Char::Equals(u'X', u'X'));
}

TEST(CharTests2, Equals_DiffChar_False) {
    EXPECT_FALSE(Char::Equals(u'X', u'Y'));
}

// ---------------------------------------------------------------------------
// GetHashCode
// ---------------------------------------------------------------------------

TEST(CharTests2, GetHashCode_EqualsCodeUnit) {
    auto v = static_cast<int>(u'A');
    EXPECT_EQ(Char::GetHashCode(u'A'), v | (v << 16));
}

TEST(CharTests2, GetHashCode_NullChar_IsZero) {
    EXPECT_EQ(Char::GetHashCode(u'\0'), 0);
}

// ---------------------------------------------------------------------------
// TryParse
// ---------------------------------------------------------------------------

TEST(CharTests2, TryParse_Valid_ReturnsTrue) {
    charcs r = u'\0';
    EXPECT_TRUE(Char::TryParse("A", r));
    EXPECT_EQ(r, u'A');
}

TEST(CharTests2, TryParse_Empty_ReturnsFalse) {
    charcs r = u'X';
    EXPECT_FALSE(Char::TryParse("", r));
    EXPECT_EQ(r, u'\0');
}

TEST(CharTests2, TryParse_MultiChar_ReturnsFalse) {
    charcs r = u'\0';
    EXPECT_FALSE(Char::TryParse("AB", r));
}

// ---------------------------------------------------------------------------
// IsBetween
// ---------------------------------------------------------------------------

TEST(CharTests2, IsBetween_InRange_True) {
    EXPECT_TRUE(Char::IsBetween(u'C', u'A', u'Z'));
    EXPECT_TRUE(Char::IsBetween(u'A', u'A', u'Z'));
    EXPECT_TRUE(Char::IsBetween(u'Z', u'A', u'Z'));
}

TEST(CharTests2, IsBetween_OutOfRange_False) {
    EXPECT_FALSE(Char::IsBetween(u'a', u'A', u'Z'));
    EXPECT_FALSE(Char::IsBetween(u'0', u'A', u'Z'));
}

// ---------------------------------------------------------------------------
// IsControl — new formula: (((uint)c + 1) & ~0x80) <= 0x20
// ---------------------------------------------------------------------------

TEST(CharTests2, IsControl_NUL_True) {
    EXPECT_TRUE(Char::IsControl(u'\0'));
}

TEST(CharTests2, IsControl_Tab_True) {
    EXPECT_TRUE(Char::IsControl(u'\t'));
}

TEST(CharTests2, IsControl_DEL_True) {
    EXPECT_TRUE(Char::IsControl(static_cast<charcs>(0x7F)));
}

TEST(CharTests2, IsControl_Space_False) {
    EXPECT_FALSE(Char::IsControl(u' '));
}

TEST(CharTests2, IsControl_Letter_False) {
    EXPECT_FALSE(Char::IsControl(u'A'));
}

// ---------------------------------------------------------------------------
// IsSeparator — tab/newline are NOT separators; space IS
// ---------------------------------------------------------------------------

TEST(CharTests2, IsSeparator_Space_True) {
    EXPECT_TRUE(Char::IsSeparator(u' '));
}

TEST(CharTests2, IsSeparator_NoBreakSpace_True) {
    EXPECT_TRUE(Char::IsSeparator(static_cast<charcs>(0x00A0)));
}

TEST(CharTests2, IsSeparator_Tab_False) {
    EXPECT_FALSE(Char::IsSeparator(u'\t'));
}

TEST(CharTests2, IsSeparator_Newline_False) {
    EXPECT_FALSE(Char::IsSeparator(u'\n'));
}

TEST(CharTests2, IsSeparator_Letter_False) {
    EXPECT_FALSE(Char::IsSeparator(u'A'));
}

// ---------------------------------------------------------------------------
// IsNumber — broader than IsDigit
// ---------------------------------------------------------------------------

TEST(CharTests2, IsNumber_Digit_True) {
    EXPECT_TRUE(Char::IsNumber(u'5'));
}

TEST(CharTests2, IsNumber_Letter_False) {
    EXPECT_FALSE(Char::IsNumber(u'A'));
}

// ---------------------------------------------------------------------------
// GetNumericValue — returns double
// ---------------------------------------------------------------------------

TEST(CharTests2, GetNumericValue_Digit_ReturnsValue) {
    EXPECT_DOUBLE_EQ(Char::GetNumericValue(u'7'), 7.0);
}

TEST(CharTests2, GetNumericValue_NonNumeric_ReturnsNeg1) {
    EXPECT_DOUBLE_EQ(Char::GetNumericValue(u'A'), -1.0);
}

TEST(CharTests2, GetNumericValue_Zero_Returns0) {
    EXPECT_DOUBLE_EQ(Char::GetNumericValue(u'0'), 0.0);
}

// ---------------------------------------------------------------------------
// GetUnicodeCategory
// ---------------------------------------------------------------------------

TEST(CharTests2, GetUnicodeCategory_UppercaseLetter) {
    EXPECT_EQ(Char::GetUnicodeCategory(u'A'),
              System::Globalization::UnicodeCategory::UppercaseLetter);
}

TEST(CharTests2, GetUnicodeCategory_LowercaseLetter) {
    EXPECT_EQ(Char::GetUnicodeCategory(u'z'),
              System::Globalization::UnicodeCategory::LowercaseLetter);
}

TEST(CharTests2, GetUnicodeCategory_Digit) {
    EXPECT_EQ(Char::GetUnicodeCategory(u'3'),
              System::Globalization::UnicodeCategory::DecimalDigitNumber);
}

// ---------------------------------------------------------------------------
// ConvertFromUtf32
// ---------------------------------------------------------------------------

TEST(CharTests2, ConvertFromUtf32_ASCII_SingleByte) {
    EXPECT_EQ(Char::ConvertFromUtf32(65), "A");
}

TEST(CharTests2, ConvertFromUtf32_BMP_UTF8) {
    // U+00E9 (é) → UTF-8 0xC3 0xA9
    std::string s = Char::ConvertFromUtf32(0x00E9);
    EXPECT_EQ(s.size(), 2u);
    EXPECT_EQ(static_cast<unsigned char>(s[0]), 0xC3u);
    EXPECT_EQ(static_cast<unsigned char>(s[1]), 0xA9u);
}

TEST(CharTests2, ConvertFromUtf32_Supplementary_FourBytes) {
    // U+1F600 → UTF-8 F0 9F 98 80
    std::string s = Char::ConvertFromUtf32(0x1F600);
    EXPECT_EQ(s.size(), 4u);
    EXPECT_EQ(static_cast<unsigned char>(s[0]), 0xF0u);
}

TEST(CharTests2, ConvertFromUtf32_Surrogate_Throws) {
    EXPECT_THROW(Char::ConvertFromUtf32(0xD800), System::ArgumentOutOfRangeException);
}

TEST(CharTests2, ConvertFromUtf32_NegativeValue_Throws) {
    EXPECT_THROW(Char::ConvertFromUtf32(-1), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// ConvertToUtf32(char, char)
// ---------------------------------------------------------------------------

TEST(CharTests2, ConvertToUtf32_ValidPair_CorrectValue) {
    // U+1F600 = (D83D, DE00)
    charcs high = static_cast<charcs>(0xD83D);
    charcs low  = static_cast<charcs>(0xDE00);
    EXPECT_EQ(Char::ConvertToUtf32(high, low), 0x1F600);
}

TEST(CharTests2, ConvertToUtf32_InvalidHigh_Throws) {
    EXPECT_THROW(Char::ConvertToUtf32(u'A', static_cast<charcs>(0xDC00)),
                 System::ArgumentOutOfRangeException);
}

TEST(CharTests2, ConvertToUtf32_InvalidLow_Throws) {
    EXPECT_THROW(Char::ConvertToUtf32(static_cast<charcs>(0xD800), u'A'),
                 System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// String-indexed overloads
// ---------------------------------------------------------------------------

TEST(CharTests2, StringIndex_IsDigit_True) {
    EXPECT_TRUE(Char::IsDigit("abc3xyz", 3));
}

TEST(CharTests2, StringIndex_IsDigit_False) {
    EXPECT_FALSE(Char::IsDigit("abc3xyz", 0));
}

TEST(CharTests2, StringIndex_IsDigit_OutOfRange_Throws) {
    EXPECT_THROW(Char::IsDigit("abc", 5), System::ArgumentOutOfRangeException);
}

TEST(CharTests2, StringIndex_IsLetter_True) {
    EXPECT_TRUE(Char::IsLetter("a1", 0));
}

TEST(CharTests2, StringIndex_IsLetter_False) {
    EXPECT_FALSE(Char::IsLetter("a1", 1));
}

TEST(CharTests2, StringIndex_IsUpper_True) {
    EXPECT_TRUE(Char::IsUpper("Ab", 0));
}

TEST(CharTests2, StringIndex_IsUpper_False) {
    EXPECT_FALSE(Char::IsUpper("Ab", 1));
}

TEST(CharTests2, StringIndex_IsLower_True) {
    EXPECT_TRUE(Char::IsLower("Ab", 1));
}

TEST(CharTests2, StringIndex_IsWhiteSpace_True) {
    EXPECT_TRUE(Char::IsWhiteSpace("a b", 1));
}

TEST(CharTests2, StringIndex_IsWhiteSpace_False) {
    EXPECT_FALSE(Char::IsWhiteSpace("a b", 0));
}

TEST(CharTests2, StringIndex_IsControl_True) {
    std::string s = "a\tb";
    EXPECT_TRUE(Char::IsControl(s, 1));
}

TEST(CharTests2, StringIndex_IsSeparator_Space_True) {
    EXPECT_TRUE(Char::IsSeparator("a b", 1));
}

TEST(CharTests2, StringIndex_IsSeparator_Tab_False) {
    std::string s = "a\tb";
    EXPECT_FALSE(Char::IsSeparator(s, 1));
}

TEST(CharTests2, StringIndex_GetNumericValue_Digit) {
    EXPECT_DOUBLE_EQ(Char::GetNumericValue("a5b", 1), 5.0);
}

TEST(CharTests2, StringIndex_GetUnicodeCategory_Letter) {
    EXPECT_EQ(Char::GetUnicodeCategory("Ab", 0),
              System::Globalization::UnicodeCategory::UppercaseLetter);
}

TEST(CharTests2, StringIndex_ConvertToUtf32_BMP) {
    EXPECT_EQ(Char::ConvertToUtf32("ABC", 1), static_cast<int>(u'B'));
}

TEST(CharTests2, StringIndex_IsSurrogatePair_False_ForASCII) {
    EXPECT_FALSE(Char::IsSurrogatePair("AB", 0));
}

// Regression tests for a code-audit finding (ticket 242): the string-indexed overloads read
// through charAt(), which returns the raw *byte* at an index rather than decoding UTF-8 --
// consistent with this project's existing single-byte-per-char std::string convention. One
// consequence (now documented on the "String-indexed overloads" section) is that a raw byte can
// never fall in the UTF-16 surrogate range (0xD800-0xDFFF), so IsHighSurrogate(s,index)/
// IsLowSurrogate(s,index)/IsSurrogate(s,index) can never return true through these overloads,
// even when s holds the UTF-8 encoding of an actual supplementary-plane character. These tests
// lock in that documented (not fixed -- see the section note) behavior so it isn't silently
// changed by an unrelated future edit.
TEST(CharTests2, StringIndex_IsHighSurrogate_NeverTrue_EvenForUtf8EncodedSupplementaryChar) {
    // U+1F600 (an emoji outside the BMP) UTF-8-encodes to F0 9F 98 80 -- none of those raw
    // bytes fall in the char16_t high-surrogate range, since a byte tops out at 0xFF.
    std::string s = Char::ConvertFromUtf32(0x1F600);
    for (SharpRuntime::intcs i = 0; i < static_cast<SharpRuntime::intcs>(s.size()); ++i) {
        EXPECT_FALSE(Char::IsHighSurrogate(s, i));
        EXPECT_FALSE(Char::IsLowSurrogate(s, i));
        EXPECT_FALSE(Char::IsSurrogate(s, i));
    }
}

TEST(CharTests2, StringIndex_ConvertToUtf32_NeverTakesSurrogateCombiningPath) {
    // ConvertToUtf32(s, index)'s surrogate-pair-combining branch is unreachable via the
    // string-indexed overload for the same reason -- it returns the raw first byte as-is.
    std::string s = Char::ConvertFromUtf32(0x1F600);
    EXPECT_EQ(Char::ConvertToUtf32(s, 0), static_cast<SharpRuntime::intcs>(static_cast<unsigned char>(s[0])));
}
