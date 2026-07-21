// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <vector>
#include "System/IndexOutOfRangeException.hpp"
#include "System/Text/DecoderFallback.hpp"
#include "System/Text/Encoding.hpp"
#include "System/Text/EncoderFallback.hpp"
#include "System/Text/RunePosition.hpp"
#include "System/Text/StringBuilder.hpp"
#include "System/Text/StringBuilderRuneEnumerator.hpp"
#include "System/Text/StringRuneEnumerator.hpp"
#include "System/Text/UTF32Encoding.hpp"
#include "System/Text/UTF8Encoding.hpp"
#include "System/Text/UnicodeEncoding.hpp"

using namespace System::Text;

// --- DecoderFallbackBuffer / EncoderFallbackBuffer -------------------------------------------

TEST(DecoderFallbackBufferTests, ReplacementBuffer_YieldsReplacementChars) {
    DecoderReplacementFallback fallback("??");
    auto buffer = fallback.CreateFallbackBuffer();
    std::vector<SharpRuntime::bytecs> bad{0xFF};
    EXPECT_TRUE(buffer->Fallback(bad, 0));
    EXPECT_EQ(buffer->GetNextChar(), '?');
    EXPECT_EQ(buffer->GetNextChar(), '?');
    EXPECT_EQ(buffer->GetNextChar(), '\0');
}

TEST(DecoderFallbackBufferTests, ReplacementBuffer_MovePrevious) {
    DecoderReplacementFallback fallback("AB");
    auto buffer = fallback.CreateFallbackBuffer();
    std::vector<SharpRuntime::bytecs> bad{0xFF};
    buffer->Fallback(bad, 0);
    EXPECT_EQ(buffer->GetNextChar(), 'A');
    EXPECT_TRUE(buffer->MovePrevious());
    EXPECT_EQ(buffer->GetNextChar(), 'A');
}

TEST(DecoderFallbackBufferTests, ExceptionBuffer_Throws) {
    DecoderExceptionFallback fallback;
    auto buffer = fallback.CreateFallbackBuffer();
    std::vector<SharpRuntime::bytecs> bad{0xFF, 0xFE};
    EXPECT_THROW(buffer->Fallback(bad, 3), DecoderFallbackException);
}

TEST(DecoderFallbackExceptionTests, PropertiesSet) {
    std::vector<SharpRuntime::bytecs> bytes{0x01, 0x02};
    DecoderFallbackException ex("bad bytes", bytes, 5);
    EXPECT_EQ(ex.getBytesUnknownProperty(), bytes);
    EXPECT_EQ(ex.getIndexProperty(), 5);
}

TEST(EncoderFallbackBufferTests, ReplacementBuffer_YieldsReplacementChars) {
    EncoderReplacementFallback fallback("XY");
    auto buffer = fallback.CreateFallbackBuffer();
    EXPECT_TRUE(buffer->Fallback('\x01', 0));
    EXPECT_EQ(buffer->GetNextChar(), 'X');
    EXPECT_EQ(buffer->GetNextChar(), 'Y');
    EXPECT_EQ(buffer->GetNextChar(), '\0');
}

TEST(EncoderFallbackBufferTests, ExceptionBuffer_Throws) {
    EncoderExceptionFallback fallback;
    auto buffer = fallback.CreateFallbackBuffer();
    EXPECT_THROW(buffer->Fallback('\x01', 2), EncoderFallbackException);
}

TEST(EncoderFallbackExceptionTests, PropertiesSet) {
    EncoderFallbackException ex("bad char", 'z', 7);
    EXPECT_EQ(ex.getCharUnknownProperty(), 'z');
    EXPECT_EQ(ex.getIndexProperty(), 7);
}

// --- Encoding (expanded surface) --------------------------------------------------------------

TEST(EncodingTests, GetStringVectorOverload) {
    auto utf8 = Encoding::UTF8();
    std::vector<SharpRuntime::bytecs> bytes{'h', 'i'};
    EXPECT_EQ(utf8->GetString(bytes), "hi");
}

TEST(EncodingTests, GetByteCount_MatchesGetBytesSize) {
    auto utf8 = Encoding::UTF8();
    EXPECT_EQ(utf8->GetByteCount("hello"), 5);
}

TEST(EncodingTests, Equals_SameCodePage) {
    auto a = Encoding::UTF8();
    auto b = Encoding::UTF8();
    EXPECT_TRUE(a->Equals(*b));
}

TEST(EncodingTests, Equals_DifferentCodePage) {
    auto utf8 = Encoding::UTF8();
    auto ascii = Encoding::ASCII();
    EXPECT_FALSE(utf8->Equals(*ascii));
}

TEST(EncodingTests, DecoderFallback_DefaultIsReplacement) {
    auto utf8 = Encoding::UTF8();
    EXPECT_NE(utf8->getDecoderFallbackProperty(), nullptr);
}

TEST(EncodingTests, SetDecoderFallback_Roundtrips) {
    auto utf8 = Encoding::UTF8();
    auto originalFallback = utf8->getDecoderFallbackProperty();
    auto exceptionFallback = DecoderFallback::ExceptionFallback();
    utf8->setDecoderFallbackProperty(exceptionFallback);
    EXPECT_EQ(utf8->getDecoderFallbackProperty(), exceptionFallback);
    utf8->setDecoderFallbackProperty(originalFallback);
}

TEST(EncodingTests, StaticFactories_ReturnNonNull) {
    EXPECT_NE(Encoding::Latin1(), nullptr);
    EXPECT_NE(Encoding::UTF32(), nullptr);
    EXPECT_NE(Encoding::UTF7(), nullptr);
    EXPECT_NE(Encoding::BigEndianUnicode(), nullptr);
    EXPECT_NE(Encoding::Default(), nullptr);
}

TEST(EncodingTests, Default_IsUtf8) {
    EXPECT_EQ(Encoding::Default()->getCodePageProperty(), Encoding::UTF8()->getCodePageProperty());
}

// --- UnicodeEncoding (now full Unicode range, not just ASCII) --------------------------------

TEST(UnicodeEncodingTests, RoundTrip_Ascii) {
    UnicodeEncoding enc;
    auto bytes = enc.GetBytes("Hi!");
    EXPECT_EQ(enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size())), "Hi!");
}

TEST(UnicodeEncodingTests, RoundTrip_NonAsciiBmp) {
    UnicodeEncoding enc;
    std::string original = "caf\xC3\xA9"; // "café" in UTF-8 (é = U+00E9)
    auto bytes = enc.GetBytes(original);
    EXPECT_EQ(enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size())), original);
}

TEST(UnicodeEncodingTests, RoundTrip_SupplementaryPlane) {
    UnicodeEncoding enc;
    // U+1F600 (😀) UTF-8: F0 9F 98 80 — requires surrogate pair handling in UTF-16.
    std::string original = "\xF0\x9F\x98\x80";
    auto bytes = enc.GetBytes(original);
    EXPECT_EQ(bytes.size(), 4u); // one surrogate pair = 4 bytes
    EXPECT_EQ(enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size())), original);
}

TEST(UnicodeEncodingTests, BigEndianRoundTrip) {
    UnicodeEncoding enc(true, false);
    std::string original = "test";
    auto bytes = enc.GetBytes(original);
    EXPECT_EQ(bytes[0], 0);
    EXPECT_EQ(bytes[1], 't');
    EXPECT_EQ(enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size())), original);
}

// GetBytes' internal UTF-8 decode loop used to accept ill-formed input (no continuation-byte
// validation, no overlong-encoding rejection) and silently produce a wrong-but-plausible code
// point. Verified with a compiled reproduction before fixing: an overlong encoding of U+0000,
// "\xC0\x80", decoded straight through to real U+0000 instead of being replaced with U+FFFD.
TEST(UnicodeEncodingTests, GetBytes_OverlongUtf8Input_ReplacesWithFFFD) {
    UnicodeEncoding enc(false, false);
    auto bytes = enc.GetBytes(std::string("\xC0\x80"));
    // Rejecting the overlong sequence resyncs one byte at a time, so both input bytes (each
    // individually ill-formed once the 2-byte overlong reading is rejected) become their own
    // U+FFFD: two replacement characters, not one.
    ASSERT_EQ(bytes.size(), 4u);
    EXPECT_EQ(static_cast<uint8_t>(bytes[0]), 0xFD);
    EXPECT_EQ(static_cast<uint8_t>(bytes[1]), 0xFF);
    EXPECT_EQ(static_cast<uint8_t>(bytes[2]), 0xFD);
    EXPECT_EQ(static_cast<uint8_t>(bytes[3]), 0xFF);
}

TEST(UnicodeEncodingTests, GetBytes_BadContinuationByte_ReplacesWithFFFD) {
    UnicodeEncoding enc(false, false);
    auto bytes = enc.GetBytes(std::string("\xC2\x41")); // 'A' is not a continuation byte
    ASSERT_EQ(bytes.size(), 4u); // U+FFFD (2 bytes) + 'A' (2 bytes), resuming after the bad lead byte
    EXPECT_EQ(static_cast<uint8_t>(bytes[0]), 0xFD);
    EXPECT_EQ(static_cast<uint8_t>(bytes[1]), 0xFF);
}

// An unpaired surrogate in the UTF-16 byte stream must be replaced with U+FFFD, not encoded
// directly as a "surrogate in UTF-8" (which is CESU-8/WTF-8, not valid UTF-8).
TEST(UnicodeEncodingTests, GetString_UnpairedHighSurrogate_ReplacesWithFFFD) {
    UnicodeEncoding enc(false, false);
    std::vector<SharpRuntime::bytecs> bytes = {
        static_cast<SharpRuntime::bytecs>(0x00), static_cast<SharpRuntime::bytecs>(0xD8), // lone high surrogate, LE
        static_cast<SharpRuntime::bytecs>('A'), static_cast<SharpRuntime::bytecs>(0x00),
    };
    std::string result = enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size()));
    EXPECT_EQ(result, "\xEF\xBF\xBD" "A"); // U+FFFD followed by 'A'
}

TEST(UnicodeEncodingTests, GetString_LoneLowSurrogate_ReplacesWithFFFD) {
    UnicodeEncoding enc(false, false);
    std::vector<SharpRuntime::bytecs> bytes = {
        static_cast<SharpRuntime::bytecs>(0x00), static_cast<SharpRuntime::bytecs>(0xDC), // lone low surrogate, LE
    };
    std::string result = enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size()));
    EXPECT_EQ(result, "\xEF\xBF\xBD"); // U+FFFD
}

// --- UTF32Encoding (now full Unicode range) ---------------------------------------------------

TEST(UTF32EncodingTests, RoundTrip_NonAscii) {
    UTF32Encoding enc(false, false);
    std::string original = "caf\xC3\xA9"; // "café"
    auto bytes = enc.GetBytes(original);
    EXPECT_EQ(enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size())), original);
}

TEST(UTF32EncodingTests, RoundTrip_SupplementaryPlane) {
    UTF32Encoding enc(false, false);
    std::string original = "\xF0\x9F\x98\x80"; // U+1F600
    auto bytes = enc.GetBytes(original);
    ASSERT_EQ(bytes.size(), 4u);
    EXPECT_EQ(enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size())), original);
}

// Same ill-formed-UTF-8-input gap as UnicodeEncoding, verified independently since
// UTF32Encoding has its own copy of the decode loop.
TEST(UTF32EncodingTests, GetBytes_OverlongUtf8Input_ReplacesWithFFFD) {
    UTF32Encoding enc(false, false);
    auto bytes = enc.GetBytes(std::string("\xC0\x80"));
    // Rejecting the overlong sequence resyncs one byte at a time, so both input bytes become
    // their own U+FFFD 4-byte code unit: two replacement characters, not one.
    ASSERT_EQ(bytes.size(), 8u);
    EXPECT_EQ(static_cast<uint8_t>(bytes[0]), 0xFD);
    EXPECT_EQ(static_cast<uint8_t>(bytes[1]), 0xFF);
    EXPECT_EQ(static_cast<uint8_t>(bytes[2]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(bytes[3]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(bytes[4]), 0xFD);
    EXPECT_EQ(static_cast<uint8_t>(bytes[5]), 0xFF);
}

// An out-of-range or surrogate 32-bit code unit must be replaced with U+FFFD, not passed
// straight to the UTF-8 encoder -- which would previously produce a structurally invalid
// byte sequence, not just the wrong code point.
TEST(UTF32EncodingTests, GetString_OutOfRangeCodeUnit_ReplacesWithFFFD) {
    UTF32Encoding enc(false, false);
    std::vector<SharpRuntime::bytecs> bytes = {
        static_cast<SharpRuntime::bytecs>(0xFF), static_cast<SharpRuntime::bytecs>(0xFF),
        static_cast<SharpRuntime::bytecs>(0xFF), static_cast<SharpRuntime::bytecs>(0xFF),
    };
    std::string result = enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size()));
    EXPECT_EQ(result, "\xEF\xBF\xBD"); // U+FFFD
}

TEST(UTF32EncodingTests, GetString_SurrogateCodeUnit_ReplacesWithFFFD) {
    UTF32Encoding enc(false, false);
    std::vector<SharpRuntime::bytecs> bytes = {
        static_cast<SharpRuntime::bytecs>(0x00), static_cast<SharpRuntime::bytecs>(0xD8),
        static_cast<SharpRuntime::bytecs>(0x00), static_cast<SharpRuntime::bytecs>(0x00),
    };
    std::string result = enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size()));
    EXPECT_EQ(result, "\xEF\xBF\xBD"); // U+FFFD
}

// --- UTF8Encoding (GetBytes/GetString now validate well-formedness via the fallback objects,
// instead of an unvalidated byte passthrough) --------------------------------------------------

TEST(UTF8EncodingTests, RoundTrip_Ascii) {
    UTF8Encoding enc;
    auto bytes = enc.GetBytes("Hi!");
    EXPECT_EQ(enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size())), "Hi!");
}

TEST(UTF8EncodingTests, RoundTrip_NonAscii) {
    UTF8Encoding enc;
    std::string original = "caf\xC3\xA9"; // "café"
    auto bytes = enc.GetBytes(original);
    ASSERT_EQ(bytes.size(), original.size());
    EXPECT_EQ(enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size())), original);
}

TEST(UTF8EncodingTests, RoundTrip_SupplementaryPlane) {
    UTF8Encoding enc;
    std::string original = "\xF0\x9F\x98\x80"; // U+1F600
    auto bytes = enc.GetBytes(original);
    EXPECT_EQ(enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size())), original);
}

// GetBytes()/GetString() used to be a raw byte passthrough with zero well-formedness
// validation in either direction. Verified before fixing: a bad continuation byte reached the
// output completely unexamined instead of triggering the (already-existing but previously
// unused anywhere in the codebase) EncoderFallback/DecoderFallback machinery.
TEST(UTF8EncodingTests, GetBytes_BadContinuationByte_ReplacesWithFFFD) {
    UTF8Encoding enc;
    // 0xC2 is a valid 2-byte lead but 'A' (0x41) is not a continuation byte.
    auto bytes = enc.GetBytes(std::string("\xC2\x41"));
    ASSERT_EQ(bytes.size(), 4u); // U+FFFD (3 bytes) + 'A' (1 byte), resuming after the bad lead byte
    EXPECT_EQ(static_cast<uint8_t>(bytes[0]), 0xEF);
    EXPECT_EQ(static_cast<uint8_t>(bytes[1]), 0xBF);
    EXPECT_EQ(static_cast<uint8_t>(bytes[2]), 0xBD);
    EXPECT_EQ(static_cast<uint8_t>(bytes[3]), 'A');
}

TEST(UTF8EncodingTests, GetBytes_OverlongEncoding_ReplacesWithFFFD) {
    UTF8Encoding enc;
    auto bytes = enc.GetBytes(std::string("\xC0\x80")); // overlong encoding of U+0000
    // Each ill-formed byte resyncs individually -> two U+FFFD (3 bytes each), not one.
    ASSERT_EQ(bytes.size(), 6u);
    EXPECT_EQ(static_cast<uint8_t>(bytes[0]), 0xEF);
    EXPECT_EQ(static_cast<uint8_t>(bytes[3]), 0xEF);
}

TEST(UTF8EncodingTests, GetString_TruncatedSequence_ReplacesWithFFFD) {
    UTF8Encoding enc;
    std::vector<SharpRuntime::bytecs> bytes = {0xE2, 0x82}; // truncated 3-byte sequence
    std::string result = enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size()));
    EXPECT_EQ(result, "\xEF\xBF\xBD\xEF\xBF\xBD"); // one U+FFFD per bad byte
}

TEST(UTF8EncodingTests, GetString_SurrogateEncodedDirectly_ReplacesWithFFFD) {
    UTF8Encoding enc;
    // ED A0 80 is the CESU-8/WTF-8 encoding of U+D800 -- structurally decodable but forbidden
    // in real UTF-8 since surrogate code points can't be encoded directly.
    std::vector<SharpRuntime::bytecs> bytes = {0xED, 0xA0, 0x80};
    std::string result = enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size()));
    EXPECT_EQ(result, "\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD"); // resyncs one byte at a time
}

TEST(UTF8EncodingTests, GetBytes_WithExceptionFallback_ThrowsEncoderFallbackException) {
    UTF8Encoding enc;
    enc.setEncoderFallbackProperty(EncoderFallback::ExceptionFallback());
    EXPECT_THROW(enc.GetBytes(std::string("\xC2\x41")), EncoderFallbackException);
}

TEST(UTF8EncodingTests, GetString_WithExceptionFallback_ThrowsDecoderFallbackException) {
    UTF8Encoding enc;
    enc.setDecoderFallbackProperty(DecoderFallback::ExceptionFallback());
    std::vector<SharpRuntime::bytecs> bytes = {0xC2, 0x41};
    EXPECT_THROW(enc.GetString(bytes.data(), 0, static_cast<SharpRuntime::intcs>(bytes.size())),
                 DecoderFallbackException);
}

// --- StringRuneEnumerator / RunePosition / StringBuilderRuneEnumerator ------------------------

// Regression test for a memory-safety bug found via AddressSanitizer (ticket 1485):
// StringRuneEnumerator's constructor used to store a raw `const std::string*` pointing at its
// reference parameter instead of owning a copy. Constructing from a string literal -- exactly
// what this test does, and the single most natural way to use this type -- binds the
// parameter to a temporary std::string that is destroyed at the end of the full expression,
// so every subsequent MoveNext() read through an already-dangling pointer. This test always
// happened to "pass" under a normal (non-ASan) build purely by luck of what garbage bytes
// were left on the stack; it is kept here specifically because it is the exact repro, now that
// the enumerator owns its own copy of the string instead.
TEST(StringRuneEnumeratorTests, EnumeratesAsciiRunes) {
    StringRuneEnumerator e("abc");
    std::vector<uint32_t> values;
    for (Rune r : e) values.push_back(r.getValueProperty());
    EXPECT_EQ(values, (std::vector<uint32_t>{'a', 'b', 'c'}));
}

TEST(StringRuneEnumeratorTests, EnumeratesMultiByteRune) {
    StringRuneEnumerator e("a\xC3\xA9z"); // 'a', 'é' (U+00E9), 'z'
    std::vector<uint32_t> values;
    for (Rune r : e) values.push_back(r.getValueProperty());
    ASSERT_EQ(values.size(), 3u);
    EXPECT_EQ(values[0], static_cast<uint32_t>('a'));
    EXPECT_EQ(values[1], 0xE9u);
    EXPECT_EQ(values[2], static_cast<uint32_t>('z'));
}

TEST(RunePositionTests, EnumerateReportsStartIndexAndLength) {
    std::string s = "a\xC3\xA9"; // 'a' (1 byte) + 'é' (2 bytes)
    std::vector<RunePosition> positions;
    for (RunePosition p : RunePosition::Enumerate(s)) positions.push_back(p);
    ASSERT_EQ(positions.size(), 2u);
    EXPECT_EQ(positions[0].getStartIndexProperty(), 0);
    EXPECT_EQ(positions[0].getLengthProperty(), 1);
    EXPECT_FALSE(positions[0].getWasReplacedProperty());
    EXPECT_EQ(positions[1].getStartIndexProperty(), 1);
    EXPECT_EQ(positions[1].getLengthProperty(), 2);
}

// Regression test for the same dangling-pointer bug class as StringRuneEnumeratorTests.
// EnumeratesAsciiRunes above (ticket 1485): RunePosition::Enumerator had the identical
// raw-pointer-to-reference-parameter defect. RunePosition::Enumerate("abc") binds the
// parameter to a temporary std::string, which is destroyed at the end of the full expression
// -- unlike EnumerateReportsStartIndexAndLength above (which uses a named local `s` that
// outlives the loop and so never actually exercised the bug), this test specifically
// constructs from a temporary to catch a regression here.
TEST(RunePositionTests, Enumerate_FromTemporary_DoesNotDangle) {
    std::vector<RunePosition> positions;
    for (RunePosition p : RunePosition::Enumerate("abc")) positions.push_back(p);
    ASSERT_EQ(positions.size(), 3u);
    EXPECT_EQ(positions[0].getRuneProperty().getValueProperty(), static_cast<uint32_t>('a'));
    EXPECT_EQ(positions[1].getRuneProperty().getValueProperty(), static_cast<uint32_t>('b'));
    EXPECT_EQ(positions[2].getRuneProperty().getValueProperty(), static_cast<uint32_t>('c'));
}

TEST(RunePositionTests, Equality) {
    RunePosition a(Rune('x'), 0, 1, false);
    RunePosition b(Rune('x'), 0, 1, false);
    RunePosition c(Rune('y'), 0, 1, false);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(StringBuilderRuneEnumeratorTests, EnumeratesBuilderContents) {
    StringBuilder sb;
    sb.Append("hi");
    std::vector<uint32_t> values;
    for (Rune r : sb.EnumerateRunes()) values.push_back(r.getValueProperty());
    EXPECT_EQ(values, (std::vector<uint32_t>{'h', 'i'}));
}

// --- StringBuilder::ChunkEnumerator / indexer -------------------------------------------------

TEST(StringBuilderTests, GetChunks_YieldsSingleChunkWithFullContent) {
    StringBuilder sb;
    sb.Append("hello world");
    int chunkCount = 0;
    for (const std::string& chunk : sb.GetChunks()) {
        EXPECT_EQ(chunk, "hello world");
        ++chunkCount;
    }
    EXPECT_EQ(chunkCount, 1);
}

TEST(StringBuilderTests, Indexer_ReadAndWrite) {
    StringBuilder sb("hello");
    EXPECT_EQ(sb[0], 'h');
    sb[0] = 'H';
    EXPECT_EQ(sb.ToString(), "Hello");
}

// .NET's StringBuilder indexer throws IndexOutOfRangeException for an out-of-range index
// (StringBuilder.cs); plain std::string::operator[] is undefined behavior in that case, so
// this must bounds-check explicitly rather than delegating straight through.
TEST(StringBuilderTests, Indexer_OutOfRange_Throws) {
    StringBuilder sb("hello");
    EXPECT_THROW((void)sb[5], System::IndexOutOfRangeException);
    EXPECT_THROW((void)sb[-1], System::IndexOutOfRangeException);
    const StringBuilder& csb = sb;
    EXPECT_THROW((void)csb[5], System::IndexOutOfRangeException);
}
