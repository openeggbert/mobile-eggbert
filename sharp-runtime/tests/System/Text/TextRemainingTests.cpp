// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for remaining Text types: NormalizationForm, CompositeFormat, Rune,
// UTF7/UTF32/Latin1Encoding, Encoder, Decoder, Regex/Match/MatchCollection.
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include "System/FormatException.hpp"
#include "System/NotImplementedException.hpp"
#include "System/Text/NormalizationForm.hpp"
#include "System/Text/CompositeFormat.hpp"
#include "System/Text/Rune.hpp"
#include "System/Text/UTF7Encoding.hpp"
#include "System/Text/UTF32Encoding.hpp"
#include "System/Text/Latin1Encoding.hpp"
#include "System/Text/Encoder.hpp"
#include "System/Text/Decoder.hpp"
#include "System/Text/Encoding.hpp"
#include "System/Text/RegularExpressions/Regex.hpp"
#include "System/Text/RegularExpressions/Match.hpp"
#include "System/Text/RegularExpressions/MatchCollection.hpp"

using System::Text::NormalizationForm;
using System::Text::CompositeFormat;
using System::Text::Rune;
using System::Text::UTF7Encoding;
using System::Text::UTF32Encoding;
using System::Text::Latin1Encoding;
using System::Text::Encoder;
using System::Text::Decoder;
using System::Text::Encoding;
using System::Text::RegularExpressions::Regex;
using System::Text::RegularExpressions::Match;
using System::Text::RegularExpressions::MatchCollection;
using System::Text::RegularExpressions::RegexParseException;
using System::Text::RegularExpressions::RegexParseError;

// ===========================================================================
// NormalizationForm
// ===========================================================================

TEST(NormalizationFormTests, FormC_IsOne) {
    EXPECT_EQ(static_cast<int>(NormalizationForm::FormC), 1);
}

TEST(NormalizationFormTests, FormD_IsTwo) {
    EXPECT_EQ(static_cast<int>(NormalizationForm::FormD), 2);
}

TEST(NormalizationFormTests, FormKC_IsFive) {
    EXPECT_EQ(static_cast<int>(NormalizationForm::FormKC), 5);
}

TEST(NormalizationFormTests, FormKD_IsSix) {
    EXPECT_EQ(static_cast<int>(NormalizationForm::FormKD), 6);
}

// ===========================================================================
// CompositeFormat
// ===========================================================================

TEST(CompositeFormatTests, Parse_SinglePlaceholder_MinArgCountOne) {
    auto cf = CompositeFormat::Parse("Hello, {0}!");
    EXPECT_EQ(cf.getMinimumArgumentCountProperty(), 1);
}

TEST(CompositeFormatTests, Parse_ThreePlaceholders_MinArgCountThree) {
    auto cf = CompositeFormat::Parse("{0} {1} {2}");
    EXPECT_EQ(cf.getMinimumArgumentCountProperty(), 3);
}

TEST(CompositeFormatTests, Parse_NoPlaceholders_MinArgCountZero) {
    auto cf = CompositeFormat::Parse("no placeholders here");
    EXPECT_EQ(cf.getMinimumArgumentCountProperty(), 0);
}

TEST(CompositeFormatTests, getFormatProperty_ReturnsOriginal) {
    auto cf = CompositeFormat::Parse("{0} world");
    EXPECT_EQ(cf.getFormatProperty(), "{0} world");
}

TEST(CompositeFormatTests, Parse_RepeatedIndex_MinArgCountOne) {
    auto cf = CompositeFormat::Parse("{0} and {0} again");
    EXPECT_EQ(cf.getMinimumArgumentCountProperty(), 1);
}

TEST(CompositeFormatTests, Parse_AlignmentAndFormatSpec_MinArgCountCorrect) {
    auto cf = CompositeFormat::Parse("{0,-5:F2} {1}");
    EXPECT_EQ(cf.getMinimumArgumentCountProperty(), 2);
}

TEST(CompositeFormatTests, Parse_EscapedBraces_NotTreatedAsPlaceholder) {
    auto cf = CompositeFormat::Parse("{{literal}} {0}");
    EXPECT_EQ(cf.getMinimumArgumentCountProperty(), 1);
}

TEST(CompositeFormatTests, Parse_UnterminatedPlaceholder_Throws) {
    EXPECT_THROW(CompositeFormat::Parse("Hello {"), System::FormatException);
}

TEST(CompositeFormatTests, Parse_StrayClosingBrace_Throws) {
    EXPECT_THROW(CompositeFormat::Parse("Hello } world"), System::FormatException);
}

TEST(CompositeFormatTests, Parse_NonNumericIndex_Throws) {
    EXPECT_THROW(CompositeFormat::Parse("{abc}"), System::FormatException);
}

TEST(CompositeFormatTests, Parse_EmptyIndex_Throws) {
    EXPECT_THROW(CompositeFormat::Parse("{}"), System::FormatException);
}

// ===========================================================================
// Rune
// ===========================================================================

TEST(RuneTests, Constructor_ValidASCII_StoresValue) {
    Rune r(0x41u);
    EXPECT_EQ(r.getValueProperty(), 0x41u);
}

TEST(RuneTests, CharConstructor_Works) {
    Rune r('A');
    EXPECT_EQ(r.getValueProperty(), 65u);
}

// Regression tests for a wave-3 audit finding: the Rune(uint32_t) constructor threw
// std::out_of_range (an unrelated std:: exception type invisible to code catching
// System::Exception&) instead of System::ArgumentOutOfRangeException, which is what real
// .NET's Rune(int)/Rune(uint) constructors throw for an invalid Unicode scalar value.
TEST(RuneTests, Constructor_SurrogateCodePoint_Throws) {
    EXPECT_THROW(Rune{0xD800u}, System::ArgumentOutOfRangeException);
}

TEST(RuneTests, Constructor_BeyondMaxCodePoint_Throws) {
    EXPECT_THROW(Rune{0x200000u}, System::ArgumentOutOfRangeException);
}

TEST(RuneTests, IsAscii_True_ForLatinLetter) {
    Rune r('Z');
    EXPECT_TRUE(r.getIsAsciiProperty());
    EXPECT_TRUE(Rune::IsAscii(r));
}

TEST(RuneTests, IsAscii_False_ForHighCodePoint) {
    Rune r(0x00E9u); // é
    EXPECT_FALSE(r.getIsAsciiProperty());
}

TEST(RuneTests, IsBmp_True_ForBmpChar) {
    Rune r(0x20ACu); // €
    EXPECT_TRUE(r.getIsBmpProperty());
}

TEST(RuneTests, IsBmp_False_ForSupplementaryChar) {
    Rune r(0x1F600u); // 😀
    EXPECT_FALSE(r.getIsBmpProperty());
}

TEST(RuneTests, Utf8SequenceLength_1_ForASCII) {
    EXPECT_EQ(Rune(0x41u).getUtf8SequenceLengthProperty(), 1);
}

TEST(RuneTests, Utf8SequenceLength_2_ForLatin1Ext) {
    EXPECT_EQ(Rune(0x00E9u).getUtf8SequenceLengthProperty(), 2);
}

TEST(RuneTests, Utf8SequenceLength_3_ForBMP) {
    EXPECT_EQ(Rune(0x20ACu).getUtf8SequenceLengthProperty(), 3);
}

TEST(RuneTests, Utf8SequenceLength_4_ForSMP) {
    EXPECT_EQ(Rune(0x1F600u).getUtf8SequenceLengthProperty(), 4);
}

TEST(RuneTests, Utf16SequenceLength_1_ForBMP) {
    EXPECT_EQ(Rune(0x20ACu).getUtf16SequenceLengthProperty(), 1);
}

TEST(RuneTests, Utf16SequenceLength_2_ForSMP) {
    EXPECT_EQ(Rune(0x1F600u).getUtf16SequenceLengthProperty(), 2);
}

TEST(RuneTests, IsValid_True_False) {
    EXPECT_TRUE(Rune::IsValid(0x41u));
    EXPECT_FALSE(Rune::IsValid(0xD800u));
    EXPECT_FALSE(Rune::IsValid(0x200000u));
}

TEST(RuneTests, IsLetter_True_False) {
    EXPECT_TRUE(Rune::IsLetter(Rune('A')));
    EXPECT_FALSE(Rune::IsLetter(Rune('1')));
}

TEST(RuneTests, IsDigit_True_False) {
    EXPECT_TRUE(Rune::IsDigit(Rune('5')));
    EXPECT_FALSE(Rune::IsDigit(Rune('A')));
}

TEST(RuneTests, IsWhiteSpace_True_False) {
    EXPECT_TRUE(Rune::IsWhiteSpace(Rune(' ')));
    EXPECT_FALSE(Rune::IsWhiteSpace(Rune('A')));
}

TEST(RuneTests, ToUpper_LowercaseToUpper) {
    EXPECT_EQ(Rune::ToUpper(Rune('a')).getValueProperty(), static_cast<uint32_t>('A'));
}

TEST(RuneTests, ToLower_UppercaseToLower) {
    EXPECT_EQ(Rune::ToLower(Rune('Z')).getValueProperty(), static_cast<uint32_t>('z'));
}

TEST(RuneTests, ToString_ASCIIChar) {
    EXPECT_EQ(Rune('A').ToString(), "A");
}

TEST(RuneTests, Equality_SameCodePoint) {
    EXPECT_TRUE(Rune('X') == Rune(0x58u));
}

TEST(RuneTests, ReplacementChar_ValueIsFFFD) {
    EXPECT_EQ(Rune::ReplacementChar.getValueProperty(), 0xFFFDu);
}

TEST(RuneTests, TryGetRuneAt_ValidSequences) {
    Rune r(static_cast<uint32_t>(0)); std::size_t consumed = 0;
    EXPECT_TRUE(Rune::TryGetRuneAt(std::string("A"), 0, r, consumed));
    EXPECT_EQ(r.getValueProperty(), 0x41u);
    EXPECT_EQ(consumed, 1u);

    EXPECT_TRUE(Rune::TryGetRuneAt(std::string("\xC2\xA9"), 0, r, consumed)); // U+00A9 COPYRIGHT SIGN
    EXPECT_EQ(r.getValueProperty(), 0xA9u);
    EXPECT_EQ(consumed, 2u);

    EXPECT_TRUE(Rune::TryGetRuneAt(std::string("\xE2\x82\xAC"), 0, r, consumed)); // U+20AC EURO SIGN
    EXPECT_EQ(r.getValueProperty(), 0x20ACu);
    EXPECT_EQ(consumed, 3u);

    EXPECT_TRUE(Rune::TryGetRuneAt(std::string("\xF0\x9F\x98\x80"), 0, r, consumed)); // U+1F600 GRINNING FACE
    EXPECT_EQ(r.getValueProperty(), 0x1F600u);
    EXPECT_EQ(consumed, 4u);
}

// RFC 3629 requires the shortest possible encoding; a longer-than-necessary sequence for a
// code point is "overlong" and must be rejected as ill-formed, not silently decoded.
TEST(RuneTests, TryGetRuneAt_RejectsOverlongEncodings) {
    Rune r(static_cast<uint32_t>(0)); std::size_t consumed = 0;
    EXPECT_FALSE(Rune::TryGetRuneAt(std::string("\xC0\x80"), 0, r, consumed));       // overlong U+0000 (2 bytes)
    EXPECT_FALSE(Rune::TryGetRuneAt(std::string("\xE0\x80\x80"), 0, r, consumed));   // overlong U+0000 (3 bytes)
    EXPECT_FALSE(Rune::TryGetRuneAt(std::string("\xF0\x80\x80\x80"), 0, r, consumed)); // overlong U+0000 (4 bytes)
}

// Every continuation byte must match the 10xxxxxx bit pattern; a lead byte followed by
// something else (e.g. an ASCII byte) is ill-formed UTF-8, not a valid sequence to decode.
TEST(RuneTests, TryGetRuneAt_RejectsInvalidContinuationByte) {
    Rune r(static_cast<uint32_t>(0)); std::size_t consumed = 0;
    EXPECT_FALSE(Rune::TryGetRuneAt(std::string("\xC2\x41"), 0, r, consumed));         // 'A' is not a continuation byte
    EXPECT_FALSE(Rune::TryGetRuneAt(std::string("\xE2\x82\x41"), 0, r, consumed));
    EXPECT_FALSE(Rune::TryGetRuneAt(std::string("\xF0\x9F\x98\x41"), 0, r, consumed));
}

// ===========================================================================
// UTF7Encoding
// ===========================================================================

TEST(UTF7EncodingTests, EncodingName_IsUtf7) {
    UTF7Encoding enc;
    EXPECT_EQ(enc.getEncodingNameProperty(), "utf-7");
}

TEST(UTF7EncodingTests, CodePage_Is65000) {
    UTF7Encoding enc;
    EXPECT_EQ(enc.getCodePageProperty(), 65000);
}

TEST(UTF7EncodingTests, GetBytes_ASCII_SameBytes) {
    UTF7Encoding enc;
    auto bytes = enc.GetBytes("ABC");
    ASSERT_EQ(bytes.size(), 3u);
    EXPECT_EQ(bytes[0], uint8_t('A'));
    EXPECT_EQ(bytes[2], uint8_t('C'));
}

TEST(UTF7EncodingTests, AllowOptionals_DefaultFalse) {
    UTF7Encoding enc;
    EXPECT_FALSE(enc.getAllowOptionals());
}

// Real UTF-7 (RFC 2152) encodes non-ASCII text via modified-Base64 shift sequences, which
// this port does not implement. Per CLAUDE.md's rule against silently returning a wrong
// value for something that can't be meaningfully implemented, GetBytes/GetString must throw
// for non-ASCII input rather than substituting '?' (this port's old, silently-corrupting
// behavior).
TEST(UTF7EncodingTests, GetBytes_NonAscii_Throws) {
    UTF7Encoding enc;
    EXPECT_THROW(enc.GetBytes("caf\xC3\xA9"), System::NotImplementedException);
}

TEST(UTF7EncodingTests, GetString_NonAsciiByte_Throws) {
    UTF7Encoding enc;
    std::vector<SharpRuntime::bytecs> bytes = {'A', 'B', 0x80};
    EXPECT_THROW(enc.GetString(bytes.data(), 0, 3), System::NotImplementedException);
}

// ===========================================================================
// UTF32Encoding
// ===========================================================================

TEST(UTF32EncodingTests, EncodingName_IsUtf32) {
    UTF32Encoding enc;
    EXPECT_EQ(enc.getEncodingNameProperty(), "utf-32");
}

TEST(UTF32EncodingTests, CodePage_Is12000) {
    UTF32Encoding enc;
    EXPECT_EQ(enc.getCodePageProperty(), 12000);
}

TEST(UTF32EncodingTests, GetBytes_WithBOM_SizeIs8ForSingleChar) {
    UTF32Encoding enc(false, true);
    auto bytes = enc.GetBytes("A");
    EXPECT_EQ(bytes.size(), 8u);
}

TEST(UTF32EncodingTests, GetBytes_NoBOM_SizeIs4ForSingleChar) {
    UTF32Encoding enc(false, false);
    auto bytes = enc.GetBytes("A");
    ASSERT_EQ(bytes.size(), 4u);
    EXPECT_EQ(bytes[0], uint8_t('A'));
    EXPECT_EQ(bytes[1], 0u);
    EXPECT_EQ(bytes[2], 0u);
    EXPECT_EQ(bytes[3], 0u);
}

// ===========================================================================
// Latin1Encoding
// ===========================================================================

TEST(Latin1EncodingTests, EncodingName_IsIso88591) {
    Latin1Encoding enc;
    EXPECT_EQ(enc.getEncodingNameProperty(), "iso-8859-1");
}

TEST(Latin1EncodingTests, CodePage_Is28591) {
    Latin1Encoding enc;
    EXPECT_EQ(enc.getCodePageProperty(), 28591);
}

TEST(Latin1EncodingTests, GetBytes_ASCIIString) {
    Latin1Encoding enc;
    auto bytes = enc.GetBytes("ABC");
    ASSERT_EQ(bytes.size(), 3u);
    EXPECT_EQ(bytes[0], uint8_t('A'));
}

TEST(Latin1EncodingTests, GetString_Roundtrip) {
    Latin1Encoding enc;
    auto bytes = enc.GetBytes("hello");
    auto result = enc.GetString(bytes.data(), 0, static_cast<int32_t>(bytes.size()));
    EXPECT_EQ(result, "hello");
}

// ===========================================================================
// Encoder
// ===========================================================================

TEST(EncoderTests, GetBytes_UTF8_MatchesEncoding) {
    Encoder enc(Encoding::UTF8());
    auto bytes = enc.GetBytes("hi");
    EXPECT_EQ(bytes.size(), 2u);
    EXPECT_EQ(bytes[0], uint8_t('h'));
}

TEST(EncoderTests, GetByteCount_MatchesByteCount) {
    Encoder enc(Encoding::UTF8());
    EXPECT_EQ(enc.GetByteCount("hello"), 5);
}

TEST(EncoderTests, Reset_NoThrow) {
    Encoder enc(Encoding::UTF8());
    EXPECT_NO_THROW(enc.Reset());
}

// ===========================================================================
// Decoder
// ===========================================================================

TEST(DecoderTests, GetString_FromBytes_RoundtripASCII) {
    Decoder dec(Encoding::UTF8());
    std::string original = "sharp";
    auto bytes = Encoding::UTF8()->GetBytes(original);
    auto result = dec.GetString(bytes.data(), 0, static_cast<int32_t>(bytes.size()));
    EXPECT_EQ(result, original);
}

TEST(DecoderTests, GetString_FromVector) {
    Decoder dec(Encoding::UTF8());
    std::string original = "test";
    auto bytes = Encoding::UTF8()->GetBytes(original);
    auto result = dec.GetString(bytes);
    EXPECT_EQ(result, original);
}

TEST(DecoderTests, Reset_NoThrow) {
    Decoder dec(Encoding::UTF8());
    EXPECT_NO_THROW(dec.Reset());
}

// ===========================================================================
// Regex
// ===========================================================================

TEST(RegexTests, IsMatch_MatchingPattern_True) {
    Regex r("\\d+");
    EXPECT_TRUE(r.IsMatch("abc123"));
}

TEST(RegexTests, IsMatch_NoMatch_False) {
    Regex r("\\d+");
    EXPECT_FALSE(r.IsMatch("abcdef"));
}

TEST(RegexTests, Match__Success_ReturnsMatchedValue) {
    Regex r("\\d+");
    Match m = r.Match_("abc 42 xyz");
    EXPECT_TRUE(m.getSuccessProperty());
    EXPECT_EQ(m.getValueProperty(), "42");
}

TEST(RegexTests, Match__NoMatch_SuccessFalse) {
    Regex r("\\d+");
    Match m = r.Match_("no digits here");
    EXPECT_FALSE(m.getSuccessProperty());
}

TEST(RegexTests, Matches_ReturnsAllMatches) {
    Regex r("\\d+");
    MatchCollection mc = r.Matches("1 22 333");
    EXPECT_EQ(mc.getCountProperty(), 3);
}

TEST(RegexTests, Replace_ReplacesAll) {
    Regex r("world");
    std::string result = r.Replace("hello world world", "C++");
    EXPECT_EQ(result.find("world"), std::string::npos);
    EXPECT_NE(result.find("C++"), std::string::npos);
}

TEST(RegexTests, Static_IsMatch_True) {
    EXPECT_TRUE(Regex::IsMatch("foo bar", "bar"));
}

TEST(RegexTests, Static_Replace) {
    std::string result = Regex::Replace("hello world", "world", "regex");
    EXPECT_EQ(result, "hello regex");
}

TEST(RegexTests, Static_Split) {
    auto parts = Regex::Split("a,b,c", ",");
    ASSERT_EQ(parts.size(), 3u);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[2], "c");
}

// matchFrom() (used by Match()/NextMatch() chains and this MatchEvaluator overload) used to
// search a fresh substr(offset) each call, so std::smatch::position() was relative to that
// substring's start rather than the true start of the original input -- corrupting every
// reported Index after the first match. Confirmed with a compiled reproduction before fixing:
// "abc 123 def 456" produced "abc [123] def 456[456]def 456" instead of "abc [123] def [456]".
TEST(RegexTests, Replace_WithMatchEvaluator_ReportsCorrectIndices) {
    Regex re("\\d+");
    std::string result = re.Replace("abc 123 def 456", [](const Match& m) {
        return "[" + m.getValueProperty() + "]";
    });
    EXPECT_EQ(result, "abc [123] def [456]");
}

// Same root cause as above: searching a fresh substring each call also made '^' incorrectly
// match at every resumption offset, not just the true start of input. Fixed by searching an
// iterator range into the original string with match_prev_avail instead of a re-materialized
// substring, so '^' correctly evaluates against the real preceding character.
TEST(RegexTests, NextMatch_AnchorDoesNotFalsePositiveOnResumedSearch) {
    Regex re("^\\d");
    Match m = re.Match("123456");
    int count = 0;
    while (m.getSuccessProperty()) {
        ++count;
        m = m.NextMatch();
    }
    EXPECT_EQ(count, 1);
}

// RegexParseException previously had no Offset property, unlike real .NET's
// RegexParseException.Offset (the zero-based character offset in the pattern where parsing
// failed). std::regex_error doesn't expose a comparable position, so it defaults to 0.
TEST(RegexParseExceptionTests, Offset_DefaultsToZero) {
    RegexParseException ex(RegexParseError::Unknown, "bad pattern");
    EXPECT_EQ(ex.getOffsetProperty(), 0);
    EXPECT_EQ(ex.getErrorProperty(), RegexParseError::Unknown);
}

TEST(RegexParseExceptionTests, Offset_ExplicitValue_IsStored) {
    RegexParseException ex(RegexParseError::Unknown, "bad pattern", 7);
    EXPECT_EQ(ex.getOffsetProperty(), 7);
}

TEST(RegexTests, InvalidPattern_Throws) {
    EXPECT_THROW(Regex("["), RegexParseException);
}

// ===========================================================================
// Match
// ===========================================================================

TEST(MatchTests, Empty_SuccessFalse) {
    const Match& m = Match::Empty();
    EXPECT_FALSE(m.getSuccessProperty());
    EXPECT_EQ(m.getValueProperty(), "");
    EXPECT_EQ(m.getIndexProperty(), -1);
    EXPECT_EQ(m.getLengthProperty(), 0);
}

TEST(MatchTests, FromRegex_IndexAndLength) {
    Regex r("\\d+");
    Match m = r.Match_("abc123def");
    EXPECT_TRUE(m.getSuccessProperty());
    EXPECT_EQ(m.getIndexProperty(), 3);
    EXPECT_EQ(m.getLengthProperty(), 3);
}

// Groups()'s per-Group Name previously always used the numeric index as a string, even for
// named groups -- the name-based indexer (Groups()["name"]) already correctly resolved by
// name, but Group.Name itself never reflected the parsed (?<name>...) name. Real .NET's
// convention: named groups report their parsed name; unnamed groups report the numeric
// index as a string (e.g. Regex.Match("a1","(\d)").Groups[1].Name == "1").
TEST(MatchTests, Groups_NamedGroup_ReportsParsedName) {
    Regex r(R"((?<year>\d{4})-(?<month>\d{2}))");
    Match m = r.Match("2024-06");
    ASSERT_TRUE(m.getSuccessProperty());
    auto groups = m.Groups();
    EXPECT_EQ(groups[0].getNameProperty(), "0");
    EXPECT_EQ(groups[1].getNameProperty(), "year");
    EXPECT_EQ(groups[2].getNameProperty(), "month");
    EXPECT_EQ(groups["year"].getValueProperty(), "2024");
    EXPECT_EQ(groups["month"].getValueProperty(), "06");
}

TEST(MatchTests, Groups_UnnamedGroup_ReportsNumericIndexAsName) {
    Regex r(R"((\d+)-(\w+))");
    Match m = r.Match("42-abc");
    ASSERT_TRUE(m.getSuccessProperty());
    auto groups = m.Groups();
    EXPECT_EQ(groups[1].getNameProperty(), "1");
    EXPECT_EQ(groups[2].getNameProperty(), "2");
}

// ===========================================================================
// MatchCollection
// ===========================================================================

TEST(MatchCollectionTests, DefaultCtor_CountZero) {
    MatchCollection mc;
    EXPECT_EQ(mc.getCountProperty(), 0);
}

TEST(MatchCollectionTests, OperatorBracket_ReturnsMatch) {
    Regex r("[a-z]");
    MatchCollection mc = r.Matches("a1b2");
    EXPECT_EQ(mc.getCountProperty(), 2);
    EXPECT_EQ(mc[0].getValueProperty(), "a");
    EXPECT_EQ(mc[1].getValueProperty(), "b");
}

// operator[] used raw vector::operator[] with no bounds check (UB for out-of-range index);
// real .NET's indexer throws ArgumentOutOfRangeException (MatchCollection.cs), matching the
// already-correct behavior of the sibling GroupCollection/CaptureCollection indexers.
TEST(MatchCollectionTests, OperatorBracket_OutOfRange_Throws) {
    Regex r("[a-z]");
    MatchCollection mc = r.Matches("a1b2");
    EXPECT_THROW(mc[100], System::ArgumentOutOfRangeException);
    EXPECT_THROW(mc[-1], System::ArgumentOutOfRangeException);
}
