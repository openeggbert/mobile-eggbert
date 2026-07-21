// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for System::Text::Unicode::UnicodeRange and UnicodeRanges.
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Buffers/OperationStatus.hpp"
#include "System/Text/Unicode/UnicodeRange.hpp"
#include "System/Text/Unicode/UnicodeRanges.hpp"
#include "System/Text/Unicode/Utf16.hpp"
#include "System/Text/Unicode/Utf8.hpp"

using System::ArgumentOutOfRangeException;
using System::Buffers::OperationStatus;
using System::Text::Unicode::UnicodeRange;
using System::Text::Unicode::UnicodeRanges;
using System::Text::Unicode::Utf16;
using System::Text::Unicode::Utf8;

// ===========================================================================
// UnicodeRange
// ===========================================================================

TEST(UnicodeRangeTests, Constructor_StoresFirstCodePointAndLength) {
    UnicodeRange r(0x0041, 26);
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0041);
    EXPECT_EQ(r.getLengthProperty(), 26);
}

TEST(UnicodeRangeTests, Constructor_ZeroLength_IsValid) {
    UnicodeRange r(0x0000, 0);
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0000);
    EXPECT_EQ(r.getLengthProperty(), 0);
}

TEST(UnicodeRangeTests, Constructor_MaxBMP_IsValid) {
    // 0xFFFF is last BMP code point; length 1 puts end at 0x10000 which is within bounds
    UnicodeRange r(0xFFFF, 1);
    EXPECT_EQ(r.getFirstCodePointProperty(), 0xFFFF);
    EXPECT_EQ(r.getLengthProperty(), 1);
}

TEST(UnicodeRangeTests, Constructor_NegativeFirstCodePoint_Throws) {
    EXPECT_THROW((UnicodeRange(-1, 10)), ArgumentOutOfRangeException);
}

TEST(UnicodeRangeTests, Constructor_FirstCodePointAboveBMP_Throws) {
    EXPECT_THROW((UnicodeRange(0x10000, 1)), ArgumentOutOfRangeException);
}

TEST(UnicodeRangeTests, Constructor_LengthOverflow_Throws) {
    EXPECT_THROW((UnicodeRange(0xFF00, 0x200)), ArgumentOutOfRangeException);
}

TEST(UnicodeRangeTests, Create_BasicLatin) {
    UnicodeRange r = UnicodeRange::Create(u'A', u'Z'); // A–Z
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0041);
    EXPECT_EQ(r.getLengthProperty(), 26);
}

TEST(UnicodeRangeTests, Create_SingleChar) {
    UnicodeRange r = UnicodeRange::Create(u' ', u' ');
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0020);
    EXPECT_EQ(r.getLengthProperty(), 1);
}

TEST(UnicodeRangeTests, Create_FirstGreaterThanLast_Throws) {
    EXPECT_THROW(UnicodeRange::Create(u'`', u'A'), ArgumentOutOfRangeException);
}

// ===========================================================================
// UnicodeRanges
// ===========================================================================

TEST(UnicodeRangesTests, All_StartsAtZero_LengthIs64K) {
    UnicodeRange r = UnicodeRanges::getAllProperty();
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0000);
    EXPECT_EQ(r.getLengthProperty(), 0x10000);
}

TEST(UnicodeRangesTests, None_LengthIsZero) {
    EXPECT_EQ(UnicodeRanges::getNoneProperty().getLengthProperty(), 0);
}

TEST(UnicodeRangesTests, BasicLatin_StartsAt0_Length128) {
    UnicodeRange r = UnicodeRanges::getBasicLatinProperty();
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0000);
    EXPECT_EQ(r.getLengthProperty(), 128);
}

TEST(UnicodeRangesTests, Latin1Supplement_StartsAt0x80_Length128) {
    UnicodeRange r = UnicodeRanges::getLatin1SupplementProperty();
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0080);
    EXPECT_EQ(r.getLengthProperty(), 128);
}

TEST(UnicodeRangesTests, Cyrillic_StartsAt0x0400_Length256) {
    UnicodeRange r = UnicodeRanges::getCyrillicProperty();
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0400);
    EXPECT_EQ(r.getLengthProperty(), 256);
}

TEST(UnicodeRangesTests, CJKUnifiedIdeographs_StartsAt0x4E00) {
    UnicodeRange r = UnicodeRanges::getCjkUnifiedIdeographsProperty();
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x4E00);
    EXPECT_GT(r.getLengthProperty(), 0);
}

TEST(UnicodeRangesTests, HangulSyllables_StartsAt0xAC00) {
    UnicodeRange r = UnicodeRanges::getHangulSyllablesProperty();
    EXPECT_EQ(r.getFirstCodePointProperty(), 0xAC00);
}

TEST(UnicodeRangesTests, Specials_StartsAt0xFFF0_Length16) {
    UnicodeRange r = UnicodeRanges::getSpecialsProperty();
    EXPECT_EQ(r.getFirstCodePointProperty(), 0xFFF0);
    EXPECT_EQ(r.getLengthProperty(), 16);
}

TEST(UnicodeRangesTests, Greek_StartsAt0x0370) {
    UnicodeRange r = UnicodeRanges::getGreekandCopticProperty();
    EXPECT_EQ(r.getFirstCodePointProperty(), 0x0370);
}

// ===========================================================================
// Utf16
// ===========================================================================

TEST(Utf16Tests, IsValid_AsciiText_IsValid) {
    EXPECT_TRUE(Utf16::IsValid(u"Hello, world!"));
}

TEST(Utf16Tests, IsValid_ValidSurrogatePair_IsValid) {
    // U+1F600 GRINNING FACE, encoded as the surrogate pair D83D DE00.
    std::u16string s = {0xD83D, 0xDE00};
    EXPECT_TRUE(Utf16::IsValid(s));
}

TEST(Utf16Tests, IsValid_UnpairedHighSurrogate_IsInvalid) {
    std::u16string s = {u'A', 0xD800, u'B'};
    EXPECT_FALSE(Utf16::IsValid(s));
}

TEST(Utf16Tests, IsValid_UnpairedLowSurrogate_IsInvalid) {
    std::u16string s = {u'A', 0xDC00, u'B'};
    EXPECT_FALSE(Utf16::IsValid(s));
}

TEST(Utf16Tests, IsValid_TrailingHighSurrogate_IsInvalid) {
    std::u16string s = {u'A', 0xD800};
    EXPECT_FALSE(Utf16::IsValid(s));
}

TEST(Utf16Tests, IndexOfInvalidSubsequence_ValidInput_ReturnsNegativeOne) {
    EXPECT_EQ(Utf16::IndexOfInvalidSubsequence(u"abc"), -1);
}

TEST(Utf16Tests, IndexOfInvalidSubsequence_UnpairedSurrogate_ReturnsItsIndex) {
    std::u16string s = {u'A', u'B', 0xD800, u'C'};
    EXPECT_EQ(Utf16::IndexOfInvalidSubsequence(s), 2);
}

// ===========================================================================
// Utf8
// ===========================================================================

TEST(Utf8Tests, IsValid_AsciiText_IsValid) {
    std::vector<SharpRuntime::bytecs> v{'H', 'i', '!'};
    EXPECT_TRUE(Utf8::IsValid(v));
}

TEST(Utf8Tests, IsValid_ValidThreeByteSequence_IsValid) {
    // U+20AC EURO SIGN = E2 82 AC in UTF-8.
    std::vector<SharpRuntime::bytecs> v{
        static_cast<SharpRuntime::bytecs>(0xE2), static_cast<SharpRuntime::bytecs>(0x82),
        static_cast<SharpRuntime::bytecs>(0xAC)};
    EXPECT_TRUE(Utf8::IsValid(v));
}

TEST(Utf8Tests, IsValid_OverlongEncoding_IsInvalid) {
    // C0 80 is an overlong (2-byte) encoding of NUL - forbidden by the UTF-8 spec.
    std::vector<SharpRuntime::bytecs> v{static_cast<SharpRuntime::bytecs>(0xC0), static_cast<SharpRuntime::bytecs>(0x80)};
    EXPECT_FALSE(Utf8::IsValid(v));
}

TEST(Utf8Tests, IsValid_SurrogateEncodedInUtf8_IsInvalid) {
    // ED A0 80 would decode to U+D800 (a surrogate) - not permitted in UTF-8.
    std::vector<SharpRuntime::bytecs> v{
        static_cast<SharpRuntime::bytecs>(0xED), static_cast<SharpRuntime::bytecs>(0xA0),
        static_cast<SharpRuntime::bytecs>(0x80)};
    EXPECT_FALSE(Utf8::IsValid(v));
}

TEST(Utf8Tests, IsValid_CodePointAboveMax_IsInvalid) {
    // F4 90 80 80 decodes to U+110000, one past the maximum valid code point U+10FFFF.
    std::vector<SharpRuntime::bytecs> v{
        static_cast<SharpRuntime::bytecs>(0xF4), static_cast<SharpRuntime::bytecs>(0x90),
        static_cast<SharpRuntime::bytecs>(0x80), static_cast<SharpRuntime::bytecs>(0x80)};
    EXPECT_FALSE(Utf8::IsValid(v));
}

TEST(Utf8Tests, IsValid_TruncatedSequence_IsInvalid) {
    std::vector<SharpRuntime::bytecs> v{static_cast<SharpRuntime::bytecs>(0xE2), static_cast<SharpRuntime::bytecs>(0x82)};
    EXPECT_FALSE(Utf8::IsValid(v));
}

TEST(Utf8Tests, IsValid_LoneContinuationByte_IsInvalid) {
    std::vector<SharpRuntime::bytecs> v{static_cast<SharpRuntime::bytecs>(0x80)};
    EXPECT_FALSE(Utf8::IsValid(v));
}

TEST(Utf8Tests, IndexOfInvalidSubsequence_ValidInput_ReturnsNegativeOne) {
    std::vector<SharpRuntime::bytecs> v{'a', 'b', 'c'};
    EXPECT_EQ(Utf8::IndexOfInvalidSubsequence(v), -1);
}

TEST(Utf8Tests, IndexOfInvalidSubsequence_InvalidByte_ReturnsItsIndex) {
    std::vector<SharpRuntime::bytecs> v{'a', static_cast<SharpRuntime::bytecs>(0x80), 'c'};
    EXPECT_EQ(Utf8::IndexOfInvalidSubsequence(v), 1);
}

TEST(Utf8Tests, FromUtf16_AsciiRoundTrip) {
    std::u16string source = u"Hello";
    std::vector<SharpRuntime::bytecs> destination(16, 0);
    SharpRuntime::intcs charsRead = 0, bytesWritten = 0;
    auto status = Utf8::FromUtf16(source, destination, charsRead, bytesWritten);
    EXPECT_EQ(status, OperationStatus::Done);
    EXPECT_EQ(charsRead, 5);
    EXPECT_EQ(bytesWritten, 5);
    for (int i = 0; i < 5; ++i) EXPECT_EQ(destination[static_cast<size_t>(i)], source[static_cast<size_t>(i)]);
}

TEST(Utf8Tests, FromUtf16_SupplementaryPlaneCharacter_EncodesFourBytes) {
    // U+1F600 GRINNING FACE = F0 9F 98 80 in UTF-8.
    std::u16string source = {0xD83D, 0xDE00};
    std::vector<SharpRuntime::bytecs> destination(4, 0);
    SharpRuntime::intcs charsRead = 0, bytesWritten = 0;
    auto status = Utf8::FromUtf16(source, destination, charsRead, bytesWritten);
    EXPECT_EQ(status, OperationStatus::Done);
    EXPECT_EQ(charsRead, 2);
    EXPECT_EQ(bytesWritten, 4);
    EXPECT_EQ(static_cast<unsigned char>(destination[0]), 0xF0);
    EXPECT_EQ(static_cast<unsigned char>(destination[1]), 0x9F);
    EXPECT_EQ(static_cast<unsigned char>(destination[2]), 0x98);
    EXPECT_EQ(static_cast<unsigned char>(destination[3]), 0x80);
}

TEST(Utf8Tests, FromUtf16_DestinationTooSmall_ReportsPartialProgress) {
    std::u16string source = u"Hello";
    std::vector<SharpRuntime::bytecs> destination(3, 0);
    SharpRuntime::intcs charsRead = 0, bytesWritten = 0;
    auto status = Utf8::FromUtf16(source, destination, charsRead, bytesWritten);
    EXPECT_EQ(status, OperationStatus::DestinationTooSmall);
    EXPECT_EQ(charsRead, 3);
    EXPECT_EQ(bytesWritten, 3);
}

TEST(Utf8Tests, FromUtf16_UnpairedSurrogate_ReplacedWithFFFDByDefault) {
    std::u16string source = {u'A', 0xD800, u'B'};
    std::vector<SharpRuntime::bytecs> destination(8, 0);
    SharpRuntime::intcs charsRead = 0, bytesWritten = 0;
    auto status = Utf8::FromUtf16(source, destination, charsRead, bytesWritten);
    EXPECT_EQ(status, OperationStatus::Done);
    // 'A' (1 byte) + U+FFFD (3 bytes: EF BF BD) + 'B' (1 byte) = 5 bytes.
    EXPECT_EQ(bytesWritten, 5);
    EXPECT_EQ(static_cast<unsigned char>(destination[1]), 0xEF);
    EXPECT_EQ(static_cast<unsigned char>(destination[2]), 0xBF);
    EXPECT_EQ(static_cast<unsigned char>(destination[3]), 0xBD);
}

TEST(Utf8Tests, FromUtf16_UnpairedSurrogate_NoReplace_ReturnsInvalidData) {
    std::u16string source = {u'A', 0xD800, u'B'};
    std::vector<SharpRuntime::bytecs> destination(8, 0);
    SharpRuntime::intcs charsRead = 0, bytesWritten = 0;
    auto status = Utf8::FromUtf16(source, destination, charsRead, bytesWritten, /*replaceInvalidSequences=*/false);
    EXPECT_EQ(status, OperationStatus::InvalidData);
    EXPECT_EQ(charsRead, 1);
    EXPECT_EQ(bytesWritten, 1);
}

TEST(Utf8Tests, FromUtf16_TrailingHighSurrogate_NotFinalBlock_ReturnsNeedMoreData) {
    std::u16string source = {u'A', 0xD800};
    std::vector<SharpRuntime::bytecs> destination(8, 0);
    SharpRuntime::intcs charsRead = 0, bytesWritten = 0;
    auto status = Utf8::FromUtf16(source, destination, charsRead, bytesWritten, true, /*isFinalBlock=*/false);
    EXPECT_EQ(status, OperationStatus::NeedMoreData);
    EXPECT_EQ(charsRead, 1);
    EXPECT_EQ(bytesWritten, 1);
}

TEST(Utf8Tests, ToUtf16_AsciiRoundTrip) {
    std::vector<SharpRuntime::bytecs> source{'H', 'i'};
    std::u16string destination(8, 0);
    SharpRuntime::intcs bytesRead = 0, charsWritten = 0;
    auto status = Utf8::ToUtf16(source, destination, bytesRead, charsWritten);
    EXPECT_EQ(status, OperationStatus::Done);
    EXPECT_EQ(bytesRead, 2);
    EXPECT_EQ(charsWritten, 2);
    EXPECT_EQ(destination[0], u'H');
    EXPECT_EQ(destination[1], u'i');
}

TEST(Utf8Tests, ToUtf16_FourByteSequence_ProducesSurrogatePair) {
    std::vector<SharpRuntime::bytecs> source{
        static_cast<SharpRuntime::bytecs>(0xF0), static_cast<SharpRuntime::bytecs>(0x9F),
        static_cast<SharpRuntime::bytecs>(0x98), static_cast<SharpRuntime::bytecs>(0x80)};
    std::u16string destination(4, 0);
    SharpRuntime::intcs bytesRead = 0, charsWritten = 0;
    auto status = Utf8::ToUtf16(source, destination, bytesRead, charsWritten);
    EXPECT_EQ(status, OperationStatus::Done);
    EXPECT_EQ(bytesRead, 4);
    EXPECT_EQ(charsWritten, 2);
    EXPECT_EQ(destination[0], 0xD83D);
    EXPECT_EQ(destination[1], 0xDE00);
}

TEST(Utf8Tests, ToUtf16_InvalidByte_ReplacedWithFFFDByDefault) {
    std::vector<SharpRuntime::bytecs> source{'A', static_cast<SharpRuntime::bytecs>(0x80), 'B'};
    std::u16string destination(8, 0);
    SharpRuntime::intcs bytesRead = 0, charsWritten = 0;
    auto status = Utf8::ToUtf16(source, destination, bytesRead, charsWritten);
    EXPECT_EQ(status, OperationStatus::Done);
    EXPECT_EQ(charsWritten, 3);
    EXPECT_EQ(destination[0], u'A');
    EXPECT_EQ(destination[1], 0xFFFD);
    EXPECT_EQ(destination[2], u'B');
}

TEST(Utf8Tests, ToUtf16_InvalidByte_NoReplace_ReturnsInvalidData) {
    std::vector<SharpRuntime::bytecs> source{'A', static_cast<SharpRuntime::bytecs>(0x80), 'B'};
    std::u16string destination(8, 0);
    SharpRuntime::intcs bytesRead = 0, charsWritten = 0;
    auto status = Utf8::ToUtf16(source, destination, bytesRead, charsWritten, /*replaceInvalidSequences=*/false);
    EXPECT_EQ(status, OperationStatus::InvalidData);
    EXPECT_EQ(bytesRead, 1);
    EXPECT_EQ(charsWritten, 1);
}

TEST(Utf8Tests, ToUtf16_TruncatedSequence_NotFinalBlock_ReturnsNeedMoreData) {
    std::vector<SharpRuntime::bytecs> source{'A', static_cast<SharpRuntime::bytecs>(0xE2), static_cast<SharpRuntime::bytecs>(0x82)};
    std::u16string destination(8, 0);
    SharpRuntime::intcs bytesRead = 0, charsWritten = 0;
    auto status = Utf8::ToUtf16(source, destination, bytesRead, charsWritten, true, /*isFinalBlock=*/false);
    EXPECT_EQ(status, OperationStatus::NeedMoreData);
    EXPECT_EQ(bytesRead, 1);
    EXPECT_EQ(charsWritten, 1);
}

// Regression tests for ticket 321: verified against real .NET's Rune.DecodeFromUtf8 (which
// Utf8.ToUtf16 delegates to for exactly this purpose), which implements the Unicode Standard's
// "maximal subpart" replacement algorithm (Ch. 3.9) -- a malformed/truncated sequence should
// consume ONLY the bytes that formed a valid prefix before hitting either an invalid byte or
// the end of the buffer, not every remaining byte in the buffer. The previous implementation's
// separate "not enough bytes for the promised length" branch consumed every remaining byte as
// one replacement even when some of them weren't continuation bytes at all -- confirmed via a
// standalone repro before the fix: a 4-byte lead byte followed by ordinary ASCII 'A','B'
// silently swallowed both characters, producing only U+FFFD instead of U+FFFD + "AB".
TEST(Utf8Tests, ToUtf16_TruncatedLeadByteFollowedByValidChars_PreservesThem) {
    std::vector<SharpRuntime::bytecs> source{static_cast<SharpRuntime::bytecs>(0xF0), 'A', 'B'};
    std::u16string destination(8, 0);
    SharpRuntime::intcs bytesRead = 0, charsWritten = 0;
    auto status = Utf8::ToUtf16(source, destination, bytesRead, charsWritten, /*replaceInvalidSequences=*/true, /*isFinalBlock=*/true);
    EXPECT_EQ(status, OperationStatus::Done);
    EXPECT_EQ(bytesRead, 3);
    ASSERT_EQ(charsWritten, 3);
    EXPECT_EQ(destination[0], 0xFFFD);
    EXPECT_EQ(destination[1], u'A');
    EXPECT_EQ(destination[2], u'B');
}
TEST(Utf8Tests, ToUtf16_LoneLeadByteAtEnd_ConsumesOnlyOneByte) {
    std::vector<SharpRuntime::bytecs> source{static_cast<SharpRuntime::bytecs>(0xE0)};
    std::u16string destination(8, 0);
    SharpRuntime::intcs bytesRead = 0, charsWritten = 0;
    auto status = Utf8::ToUtf16(source, destination, bytesRead, charsWritten, true, true);
    EXPECT_EQ(status, OperationStatus::Done);
    EXPECT_EQ(bytesRead, 1);
    ASSERT_EQ(charsWritten, 1);
    EXPECT_EQ(destination[0], 0xFFFD);
}
TEST(Utf8Tests, ToUtf16_TruncatedWithValidContinuationPrefix_ConsumesAllOfIt) {
    // 4-byte lead + 2 genuinely valid continuation bytes, but the buffer ends before the 4th
    // byte -- the whole 3-byte run IS the maximal valid subpart, so all 3 bytes are consumed
    // for one replacement (unlike the mixed-validity case above).
    std::vector<SharpRuntime::bytecs> source{static_cast<SharpRuntime::bytecs>(0xF0),
                                              static_cast<SharpRuntime::bytecs>(0x90),
                                              static_cast<SharpRuntime::bytecs>(0x80)};
    std::u16string destination(8, 0);
    SharpRuntime::intcs bytesRead = 0, charsWritten = 0;
    auto status = Utf8::ToUtf16(source, destination, bytesRead, charsWritten, true, true);
    EXPECT_EQ(status, OperationStatus::Done);
    EXPECT_EQ(bytesRead, 3);
    ASSERT_EQ(charsWritten, 1);
    EXPECT_EQ(destination[0], 0xFFFD);
}

TEST(Utf8Tests, ToUtf16_DestinationTooSmall_ReportsPartialProgress) {
    std::vector<SharpRuntime::bytecs> source{'H', 'i', '!'};
    std::u16string destination(2, 0);
    SharpRuntime::intcs bytesRead = 0, charsWritten = 0;
    auto status = Utf8::ToUtf16(source, destination, bytesRead, charsWritten);
    EXPECT_EQ(status, OperationStatus::DestinationTooSmall);
    EXPECT_EQ(bytesRead, 2);
    EXPECT_EQ(charsWritten, 2);
}
