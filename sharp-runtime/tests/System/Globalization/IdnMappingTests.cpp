// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/Globalization/IdnMapping.hpp"

using namespace System::Globalization;

// RFC 3492 test vectors + common examples

TEST(IdnMappingTests, AsciiPassthrough) {
    IdnMapping idn;
    EXPECT_EQ(idn.GetAscii("example.com"), "example.com");
}

TEST(IdnMappingTests, AsciiPassthroughUpperLowered) {
    IdnMapping idn;
    EXPECT_EQ(idn.GetAscii("EXAMPLE.COM"), "example.com");
}

TEST(IdnMappingTests, PunycodeEncodeGerman) {
    // München → xn--mnchen-3ya
    IdnMapping idn;
    std::string result = idn.GetAscii("m\xC3\xBCnchen"); // "münchen" in UTF-8
    EXPECT_EQ(result, "xn--mnchen-3ya");
}

TEST(IdnMappingTests, PunycodeEncodeFullDomain) {
    // münchen.de → xn--mnchen-3ya.de
    IdnMapping idn;
    std::string result = idn.GetAscii("m\xC3\xBCnchen.de");
    EXPECT_EQ(result, "xn--mnchen-3ya.de");
}

TEST(IdnMappingTests, PunycodeDecodeMunchen) {
    IdnMapping idn;
    std::string result = idn.GetUnicode("xn--mnchen-3ya");
    EXPECT_EQ(result, "m\xC3\xBCnchen"); // "münchen" in UTF-8
}

TEST(IdnMappingTests, PunycodeDecodeFullDomain) {
    IdnMapping idn;
    std::string result = idn.GetUnicode("xn--mnchen-3ya.de");
    EXPECT_EQ(result, "m\xC3\xBCnchen.de");
}

TEST(IdnMappingTests, PunycodeEncodeChinese) {
    // "中文.com" (Chinese characters)
    IdnMapping idn;
    std::string input = "\xE4\xB8\xAD\xE6\x96\x87.com"; // 中文.com
    std::string result = idn.GetAscii(input);
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.substr(0, 4), "xn--");
}

TEST(IdnMappingTests, PunycodeDecodeChinese) {
    IdnMapping idn;
    // Encode then decode — round trip
    std::string original = "\xE4\xB8\xAD\xE6\x96\x87.com";
    std::string ascii = idn.GetAscii(original);
    std::string decoded = idn.GetUnicode(ascii);
    EXPECT_EQ(decoded, original);
}

TEST(IdnMappingTests, RoundTripArabic) {
    IdnMapping idn;
    // مثال.إختبار — example.test in Arabic
    std::string original = "\xD9\x85\xD8\xAB\xD8\xA7\xD9\x84.\xD8\xA5\xD8\xAE\xD8\xAA\xD8\xA8\xD8\xA7\xD8\xB1";
    std::string ascii = idn.GetAscii(original);
    std::string decoded = idn.GetUnicode(ascii);
    EXPECT_EQ(decoded, original);
}

TEST(IdnMappingTests, MultipleLabels) {
    IdnMapping idn;
    std::string original = "m\xC3\xBCnchen.m\xC3\xBCnchen.de";
    std::string ascii = idn.GetAscii(original);
    EXPECT_EQ(ascii, "xn--mnchen-3ya.xn--mnchen-3ya.de");
}

TEST(IdnMappingTests, TrailingDot) {
    IdnMapping idn;
    std::string result = idn.GetAscii("example.com.");
    EXPECT_EQ(result, "example.com.");
}

TEST(IdnMappingTests, EmptyThrows) {
    IdnMapping idn;
    EXPECT_THROW(idn.GetAscii(""), System::ArgumentException);
}

// Regression tests for ticket 312: the UTF-8 decoder previously trusted continuation bytes'
// low 6 bits without checking their top 2 bits are actually 10xxxxxx (and never rejected
// overlong encodings or out-of-range/surrogate code points), so malformed UTF-8 was silently
// misinterpreted as some other, unrelated code point instead of being rejected -- a real concern
// for IdnMapping specifically, since domain names routinely originate from untrusted input.
TEST(IdnMappingTests, MalformedUtf8_InvalidContinuationByte_Throws) {
    IdnMapping idn;
    // 0xC2 is a valid 2-byte lead byte, but 'A' (0x41) is not a continuation byte.
    EXPECT_THROW(idn.GetAscii("\xC2\x41"), System::ArgumentException);
}
TEST(IdnMappingTests, MalformedUtf8_TruncatedSequence_Throws) {
    IdnMapping idn;
    EXPECT_THROW(idn.GetAscii("\xE2\x82"), System::ArgumentException); // 3-byte lead, only 1 continuation byte
}
TEST(IdnMappingTests, MalformedUtf8_OverlongEncoding_Throws) {
    IdnMapping idn;
    // 0xC0 0x80 is an overlong encoding of U+0000 -- forbidden by RFC 3629.
    EXPECT_THROW(idn.GetAscii(std::string("\xC0\x80")), System::ArgumentException);
}
TEST(IdnMappingTests, MalformedUtf8_SurrogateCodePoint_Throws) {
    IdnMapping idn;
    // 0xED 0xA0 0x80 encodes U+D800, a surrogate -- never a valid standalone UTF-8 code point.
    EXPECT_THROW(idn.GetAscii("\xED\xA0\x80"), System::ArgumentException);
}
TEST(IdnMappingTests, ValidMultiByteUtf8_StillAccepted) {
    IdnMapping idn;
    // Sanity check the fix didn't over-reject well-formed multi-byte UTF-8.
    EXPECT_NO_THROW(idn.GetAscii("m\xC3\xBCnchen")); // "münchen"
}

TEST(IdnMappingTests, GetHashCode_MatchesForEqualInstances) {
    IdnMapping a, b;
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    b.setAllowUnassignedProperty(true);
    EXPECT_NE(a.GetHashCode(), b.GetHashCode());
}

TEST(IdnMappingTests, Equality) {
    IdnMapping a, b;
    EXPECT_TRUE(a == b);
    b.setAllowUnassignedProperty(true);
    EXPECT_FALSE(a == b);
}

// ===========================================================================
// GetAscii/GetUnicode (string, int) and (string, int, int) overloads
// (verified against IdnMapping.cs; index/count are byte offsets into the
// UTF-8 string in this port, not UTF-16 code-unit offsets)
// ===========================================================================

TEST(IdnMappingTests, GetAscii_IndexOverload_SkipsPrefix) {
    IdnMapping idn;
    EXPECT_EQ(idn.GetAscii("XXexample.com", 2), "example.com");
}

TEST(IdnMappingTests, GetAscii_IndexCountOverload_SelectsSubstring) {
    IdnMapping idn;
    EXPECT_EQ(idn.GetAscii("XXexample.comYY", 2, 11), "example.com");
}

TEST(IdnMappingTests, GetAscii_IndexOverload_NegativeIndex_Throws) {
    IdnMapping idn;
    EXPECT_THROW(idn.GetAscii("example.com", -1), System::ArgumentOutOfRangeException);
}

TEST(IdnMappingTests, GetAscii_IndexOverload_IndexTooLarge_Throws) {
    IdnMapping idn;
    EXPECT_THROW(idn.GetAscii("example.com", 100), System::ArgumentOutOfRangeException);
}

TEST(IdnMappingTests, GetAscii_IndexCountOverload_CountPastEnd_Throws) {
    IdnMapping idn;
    EXPECT_THROW(idn.GetAscii("example.com", 0, 100), System::ArgumentOutOfRangeException);
}

TEST(IdnMappingTests, GetUnicode_IndexOverload_SkipsPrefix) {
    IdnMapping idn;
    EXPECT_EQ(idn.GetUnicode("XXxn--mnchen-3ya", 2), "m\xC3\xBCnchen");
}

TEST(IdnMappingTests, GetUnicode_IndexCountOverload_SelectsSubstring) {
    IdnMapping idn;
    EXPECT_EQ(idn.GetUnicode("XXxn--mnchen-3yaYY", 2, 14), "m\xC3\xBCnchen");
}

TEST(IdnMappingTests, GetUnicode_IndexOverload_NegativeIndex_Throws) {
    IdnMapping idn;
    EXPECT_THROW(idn.GetUnicode("xn--mnchen-3ya", -1), System::ArgumentOutOfRangeException);
}

// ===========================================================================
// UseStd3AsciiRules enforcement (verified against IdnMapping.cs's ValidateStd3)
// ===========================================================================

TEST(IdnMappingTests, UseStd3AsciiRules_Disabled_AllowsIllegalChar) {
    IdnMapping idn;
    EXPECT_NO_THROW(idn.GetAscii("exa_mple.com"));
}

TEST(IdnMappingTests, UseStd3AsciiRules_Enabled_RejectsIllegalChar) {
    IdnMapping idn;
    idn.setUseStd3AsciiRulesProperty(true);
    EXPECT_THROW(idn.GetAscii("exa_mple.com"), System::ArgumentException);
}

TEST(IdnMappingTests, UseStd3AsciiRules_Enabled_RejectsLeadingHyphen) {
    IdnMapping idn;
    idn.setUseStd3AsciiRulesProperty(true);
    EXPECT_THROW(idn.GetAscii("-example.com"), System::ArgumentException);
}

TEST(IdnMappingTests, UseStd3AsciiRules_Enabled_RejectsTrailingHyphen) {
    IdnMapping idn;
    idn.setUseStd3AsciiRulesProperty(true);
    EXPECT_THROW(idn.GetAscii("example-.com"), System::ArgumentException);
}

TEST(IdnMappingTests, UseStd3AsciiRules_Enabled_AllowsInteriorHyphen) {
    IdnMapping idn;
    idn.setUseStd3AsciiRulesProperty(true);
    EXPECT_NO_THROW(idn.GetAscii("exa-mple.com"));
}

TEST(IdnMappingTests, UseStd3AsciiRules_Enabled_AllowsNonAsciiUnrestricted) {
    IdnMapping idn;
    idn.setUseStd3AsciiRulesProperty(true);
    EXPECT_NO_THROW(idn.GetAscii("m\xC3\xBCnchen.de"));
}

// ===========================================================================
// LabelMax (63-octet-per-label limit) enforcement
// ===========================================================================

TEST(IdnMappingTests, LabelMax_ExactLimit_Succeeds) {
    IdnMapping idn;
    std::string label(63, 'a');
    EXPECT_NO_THROW(idn.GetAscii(label + ".com"));
}

TEST(IdnMappingTests, LabelMax_OverLimit_Throws) {
    IdnMapping idn;
    std::string label(64, 'a');
    EXPECT_THROW(idn.GetAscii(label + ".com"), System::ArgumentException);
}

TEST(IdnMappingTests, LabelMax_GetUnicode_OverLimit_Throws) {
    IdnMapping idn;
    std::string label(64, 'a');
    EXPECT_THROW(idn.GetUnicode(label + ".com"), System::ArgumentException);
}

// ===========================================================================
// decodeLabel() trailing-delimiter bug (verified against IdnMapping.cs's
// PunycodeDecode: "Trailing - not allowed")
// ===========================================================================

TEST(IdnMappingTests, GetUnicode_BarePrefix_Throws) {
    IdnMapping idn;
    EXPECT_THROW(idn.GetUnicode("xn--"), System::ArgumentException);
}

TEST(IdnMappingTests, GetUnicode_TrailingDelimiterOnly_Throws) {
    IdnMapping idn;
    // "xn--a-" decodes to basic code point 'a' followed by an empty extended
    // string -- a delimiter with nothing after it, which real .NET rejects.
    EXPECT_THROW(idn.GetUnicode("xn--a-"), System::ArgumentException);
}

// ===========================================================================
// GetUnicode canonical round-trip check (verified against IdnMapping.cs's
// GetUnicodeInvariant)
// ===========================================================================

TEST(IdnMappingTests, GetUnicode_ValidPunycode_RoundTripSucceeds) {
    IdnMapping idn;
    EXPECT_NO_THROW(idn.GetUnicode("xn--mnchen-3ya"));
}

TEST(IdnMappingTests, GetUnicode_RoundTripReencodeFails_Throws) {
    // "xn--exa_mple-e6a" is GetAscii("exa_mpleü") encoded WITHOUT UseStd3AsciiRules --
    // a syntactically valid Punycode label whose decoded basic portion ("exa_mple")
    // contains an underscore. Decoding it under UseStd3AsciiRules must fail: the
    // round-trip check's internal re-encode (GetAscii, which does enforce Std3 on this
    // instance) rejects the underscore, so GetUnicode must propagate the failure rather
    // than silently accept a name that violates the very rules requested.
    IdnMapping idn;
    idn.setUseStd3AsciiRulesProperty(true);
    EXPECT_THROW(idn.GetUnicode("xn--exa_mple-e6a"), System::ArgumentException);
}
