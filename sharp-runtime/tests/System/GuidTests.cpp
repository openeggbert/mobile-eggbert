// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include "System/Guid.hpp"
#include "System/FormatException.hpp"
#include "System/ArgumentException.hpp"
#include "System/OverflowException.hpp"
#include "System/Span.hpp"

using System::Guid;

// ---------------------------------------------------------------------------
// Empty
// ---------------------------------------------------------------------------

TEST(GuidTests, EmptyIsAllZeros) {
    EXPECT_EQ(Guid::Empty.ToString(), "00000000-0000-0000-0000-000000000000");
}

TEST(GuidTests, DefaultConstructorEqualsEmpty) {
    Guid g;
    EXPECT_EQ(g, Guid::Empty);
}

TEST(GuidTests, EmptyByteArrayIsAllZeros) {
    for (uint8_t b : Guid::Empty.ToByteArray())
        EXPECT_EQ(b, 0u);
}

// ---------------------------------------------------------------------------
// String construction and ToString roundtrip
// ---------------------------------------------------------------------------

TEST(GuidTests, ParseRoundtrip) {
    // Official .NET test vector from GuidTests.cs: s_testGuid
    const std::string s = "a8a110d5-fc49-43c5-bf46-802db8f843ff";
    Guid g(s);
    EXPECT_EQ(g.ToString(), s);
}

TEST(GuidTests, ParseAllFFs) {
    // Guid with all bits set
    const std::string s = "ffffffff-ffff-ffff-ffff-ffffffffffff";
    Guid g(s);
    EXPECT_EQ(g.ToString(), s);
}

TEST(GuidTests, ParseZeroGuid) {
    Guid g("00000000-0000-0000-0000-000000000000");
    EXPECT_EQ(g, Guid::Empty);
}

TEST(GuidTests, ToStringIsLowercase) {
    Guid g("a0b1c2d3-e4f5-6789-abcd-ef0123456789");
    std::string s = g.ToString();
    for (char c : s)
        EXPECT_TRUE(c == '-' || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
            << "Character '" << c << "' is not lowercase hex or dash";
}

TEST(GuidTests, ToStringHas36Chars) {
    Guid g("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    EXPECT_EQ(g.ToString().size(), 36u);
}

TEST(GuidTests, ToStringDashPositions) {
    std::string s = Guid("a8a110d5-fc49-43c5-bf46-802db8f843ff").ToString();
    EXPECT_EQ(s[8],  '-');
    EXPECT_EQ(s[13], '-');
    EXPECT_EQ(s[18], '-');
    EXPECT_EQ(s[23], '-');
}

TEST(GuidTests, ParseBracedFormat) {
    // {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    Guid g("{a8a110d5-fc49-43c5-bf46-802db8f843ff}");
    EXPECT_EQ(g.ToString(), "a8a110d5-fc49-43c5-bf46-802db8f843ff");
}

TEST(GuidTests, ParseParenthesisFormat) {
    // (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx)
    Guid g("(a8a110d5-fc49-43c5-bf46-802db8f843ff)");
    EXPECT_EQ(g.ToString(), "a8a110d5-fc49-43c5-bf46-802db8f843ff");
}

TEST(GuidTests, ParseInvalidThrows) {
    EXPECT_THROW(Guid("not-a-guid"), std::exception);
    EXPECT_THROW(Guid("a8a110d5-fc49-43c5-bf46"), std::exception);
    EXPECT_THROW(Guid(""), std::exception);
}

// ---------------------------------------------------------------------------
// Byte-array constructor
// ---------------------------------------------------------------------------

TEST(GuidTests, ByteArrayConstructorRoundtrip) {
    Guid original("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    Guid copy(original.ToByteArray());
    EXPECT_EQ(copy, original);
}

TEST(GuidTests, ByteArrayAllZerosEqualsEmpty) {
    std::array<uint8_t, 16> zeros{};
    Guid g(zeros);
    EXPECT_EQ(g, Guid::Empty);
}

// ---------------------------------------------------------------------------
// Equality and ordering
// ---------------------------------------------------------------------------

TEST(GuidTests, EqualitySameString) {
    Guid a("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    Guid b("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    EXPECT_EQ(a, b);
    EXPECT_FALSE(a != b);
}

TEST(GuidTests, InequalityDifferentString) {
    Guid a("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    Guid b("00000000-0000-0000-0000-000000000001");
    EXPECT_NE(a, b);
}

TEST(GuidTests, LessThanOrdering) {
    Guid lo("00000000-0000-0000-0000-000000000001");
    Guid hi("00000000-0000-0000-0000-000000000002");
    EXPECT_LT(lo, hi);
    EXPECT_FALSE(hi < lo);
}

TEST(GuidTests, NotLessThanEqual) {
    Guid a("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    EXPECT_FALSE(a < a);
}

// ---------------------------------------------------------------------------
// NewGuid — RFC 4122 version 4
// ---------------------------------------------------------------------------

TEST(GuidTests, NewGuidNotEqualEmpty) {
    // From .NET GuidTests.cs: Assert.NotEqual(Guid.Empty, guid1)
    Guid g = Guid::NewGuid();
    EXPECT_NE(g, Guid::Empty);
}

TEST(GuidTests, NewGuidTwoCallsDiffer) {
    // From .NET GuidTests.cs: Assert.NotEqual(guid1, guid2)
    Guid g1 = Guid::NewGuid();
    Guid g2 = Guid::NewGuid();
    EXPECT_NE(g1, g2);
}

TEST(GuidTests, NewGuidVersion4Bits) {
    // RFC 4122: byte[6] upper nibble must be 0x4 (version 4). ToByteArray(true) is
    // the RFC-4122/network-byte-order layout; the default ToByteArray() is little-endian
    // for the first three components (matching .NET) and does not put the version
    // nibble at a fixed index.
    Guid g = Guid::NewGuid();
    const auto b = g.ToByteArray(true);
    EXPECT_EQ(b[6] & 0xF0u, 0x40u);
    EXPECT_EQ(g.getVersionProperty(), 4);
}

TEST(GuidTests, NewGuidVariantBits) {
    // RFC 4122: byte[8] top two bits must be 0b10xxxxxx (unaffected by byte order,
    // since it is one of the trailing 8 bytes that are never reordered).
    Guid g = Guid::NewGuid();
    const auto b = g.ToByteArray(true);
    EXPECT_EQ(b[8] & 0xC0u, 0x80u);
}

TEST(GuidTests, NewGuidToStringFormat) {
    // NewGuid().ToString() must be 36 chars in canonical form
    std::string s = Guid::NewGuid().ToString();
    ASSERT_EQ(s.size(), 36u);
    EXPECT_EQ(s[8],  '-');
    EXPECT_EQ(s[13], '-');
    EXPECT_EQ(s[18], '-');
    EXPECT_EQ(s[23], '-');
}

TEST(GuidTests, NewGuidMany_AllDiffer) {
    // Statistical check: 20 consecutive GUIDs should all be unique
    std::vector<Guid> guids;
    for (int i = 0; i < 20; ++i) guids.push_back(Guid::NewGuid());
    for (int i = 0; i < 20; ++i)
        for (int j = i + 1; j < 20; ++j)
            EXPECT_NE(guids[i], guids[j]) << "Duplicate at " << i << " and " << j;
}

// ---------------------------------------------------------------------------
// Parse / TryParse
// ---------------------------------------------------------------------------
TEST(GuidTests, Parse_ValidD_RoundTrips) {
    std::string s = "550e8400-e29b-41d4-a716-446655440000";
    Guid g = Guid::Parse(s);
    EXPECT_EQ(g.ToString(), s);
}
TEST(GuidTests, Parse_ValidN_Parses) {
    std::string n = "550e8400e29b41d4a716446655440000";
    Guid g = Guid::Parse(n);
    EXPECT_EQ(g.ToString(), "550e8400-e29b-41d4-a716-446655440000");
}
TEST(GuidTests, Parse_ValidB_Parses) {
    Guid g = Guid::Parse("{550e8400-e29b-41d4-a716-446655440000}");
    EXPECT_EQ(g.ToString(), "550e8400-e29b-41d4-a716-446655440000");
}
TEST(GuidTests, Parse_Invalid_Throws) {
    EXPECT_THROW(Guid::Parse("not-a-guid"), System::FormatException);
}
TEST(GuidTests, TryParse_Valid) {
    Guid g;
    bool ok = Guid::TryParse("550e8400-e29b-41d4-a716-446655440000", g);
    EXPECT_TRUE(ok);
    EXPECT_EQ(g.ToString(), "550e8400-e29b-41d4-a716-446655440000");
}
TEST(GuidTests, TryParse_Invalid_ReturnsFalse) {
    Guid g;
    bool ok = Guid::TryParse("bad-guid", g);
    EXPECT_FALSE(ok);
}

// ---------------------------------------------------------------------------
// ToString(format)
// ---------------------------------------------------------------------------
TEST(GuidTests, ToString_D_HasHyphens) {
    Guid g = Guid::Parse("550e8400-e29b-41d4-a716-446655440000");
    EXPECT_EQ(g.ToString("D"), "550e8400-e29b-41d4-a716-446655440000");
}
TEST(GuidTests, ToString_N_NoHyphens) {
    Guid g = Guid::Parse("550e8400-e29b-41d4-a716-446655440000");
    EXPECT_EQ(g.ToString("N"), "550e8400e29b41d4a716446655440000");
}
TEST(GuidTests, ToString_B_HasBraces) {
    Guid g = Guid::Parse("550e8400-e29b-41d4-a716-446655440000");
    EXPECT_EQ(g.ToString("B"), "{550e8400-e29b-41d4-a716-446655440000}");
}
TEST(GuidTests, ToString_P_HasParens) {
    Guid g = Guid::Parse("550e8400-e29b-41d4-a716-446655440000");
    EXPECT_EQ(g.ToString("P"), "(550e8400-e29b-41d4-a716-446655440000)");
}
TEST(GuidTests, ToString_InvalidFormat_Throws) {
    EXPECT_THROW(Guid::NewGuid().ToString("Z"), System::FormatException);
}

TEST(GuidTests, ToString_X_HasHexLiteralForm) {
    // Official .NET test vector (s_testGuid, GuidTests.cs ToStringTestData)
    Guid g("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    EXPECT_EQ(g.ToString("X"), "{0xa8a110d5,0xfc49,0x43c5,{0xbf,0x46,0x80,0x2d,0xb8,0xf8,0x43,0xff}}");
    EXPECT_EQ(g.ToString("x"), g.ToString("X"));
}

// ---------------------------------------------------------------------------
// Equals / CompareTo / additional operators
// ---------------------------------------------------------------------------

TEST(GuidTests, Equals_SameGuid_ReturnsTrue) {
    Guid a("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    Guid b("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    EXPECT_TRUE(a.Equals(b));
}

TEST(GuidTests, Equals_DifferentGuid_ReturnsFalse) {
    Guid a("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    Guid b("00000000-0000-0000-0000-000000000001");
    EXPECT_FALSE(a.Equals(b));
}

TEST(GuidTests, Equals_Self_ReturnsTrue) {
    Guid a = Guid::NewGuid();
    EXPECT_TRUE(a.Equals(a));
}

TEST(GuidTests, CompareTo_Equal_ReturnsZero) {
    Guid a("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    Guid b("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    EXPECT_EQ(a.CompareTo(b), 0);
}

TEST(GuidTests, CompareTo_Less_ReturnsNegative) {
    Guid lo("00000000-0000-0000-0000-000000000001");
    Guid hi("00000000-0000-0000-0000-000000000002");
    EXPECT_LT(lo.CompareTo(hi), 0);
}

TEST(GuidTests, CompareTo_Greater_ReturnsPositive) {
    Guid lo("00000000-0000-0000-0000-000000000001");
    Guid hi("00000000-0000-0000-0000-000000000002");
    EXPECT_GT(hi.CompareTo(lo), 0);
}

TEST(GuidTests, OperatorLessOrEqual_Equal) {
    Guid a("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    Guid b("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(b <= a);
}

TEST(GuidTests, OperatorLessOrEqual_Less) {
    Guid lo("00000000-0000-0000-0000-000000000001");
    Guid hi("00000000-0000-0000-0000-000000000002");
    EXPECT_TRUE(lo <= hi);
    EXPECT_FALSE(hi <= lo);
}

TEST(GuidTests, OperatorGreater) {
    Guid lo("00000000-0000-0000-0000-000000000001");
    Guid hi("00000000-0000-0000-0000-000000000002");
    EXPECT_TRUE(hi > lo);
    EXPECT_FALSE(lo > hi);
}

TEST(GuidTests, OperatorGreaterOrEqual_Equal) {
    Guid a("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    Guid b("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    EXPECT_TRUE(a >= b);
    EXPECT_TRUE(b >= a);
}

TEST(GuidTests, OperatorGreaterOrEqual_Greater) {
    Guid lo("00000000-0000-0000-0000-000000000001");
    Guid hi("00000000-0000-0000-0000-000000000002");
    EXPECT_TRUE(hi >= lo);
    EXPECT_FALSE(lo >= hi);
}

// ---------------------------------------------------------------------------
// AllBitsSet
// ---------------------------------------------------------------------------

TEST(GuidTests, AllBitsSetIsAllFs) {
    EXPECT_EQ(Guid::getAllBitsSetProperty().ToString(), "ffffffff-ffff-ffff-ffff-ffffffffffff");
}

// ---------------------------------------------------------------------------
// ToByteArray(bigEndian) — official .NET test vectors from GuidTests.cs ToByteArray()
// ---------------------------------------------------------------------------

TEST(GuidTests, ToByteArray_DefaultIsNativeLittleEndianForFirstThreeComponents) {
    Guid g("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    std::array<uint8_t, 16> expected = {
        0xd5, 0x10, 0xa1, 0xa8, 0x49, 0xfc, 0xc5, 0x43,
        0xbf, 0x46, 0x80, 0x2d, 0xb8, 0xf8, 0x43, 0xff
    };
    EXPECT_EQ(g.ToByteArray(), expected);
    EXPECT_EQ(g.ToByteArray(false), expected);
}

TEST(GuidTests, ToByteArray_BigEndianMatchesStringOrder) {
    Guid g("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    std::array<uint8_t, 16> expected = {
        0xa8, 0xa1, 0x10, 0xd5, 0xfc, 0x49, 0x43, 0xc5,
        0xbf, 0x46, 0x80, 0x2d, 0xb8, 0xf8, 0x43, 0xff
    };
    EXPECT_EQ(g.ToByteArray(true), expected);
}

TEST(GuidTests, ByteArrayCtor_NativeOrderRoundTripsThroughToByteArray) {
    // .NET GuidTests.cs GuidCtorArrayTestData: byte[]{0x44,0x33,0x22,0x11,0x66,0x55,0x88,0x77,
    // 0x99,0x00,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF} -> Guid("11223344-5566-7788-9900-aabbccddeeff")
    std::array<uint8_t, 16> nativeOrder = {
        0x44, 0x33, 0x22, 0x11, 0x66, 0x55, 0x88, 0x77,
        0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    };
    Guid g(nativeOrder);
    EXPECT_EQ(g, Guid("11223344-5566-7788-9900-aabbccddeeff"));
}

TEST(GuidTests, ByteArrayCtor_BigEndianOrder) {
    std::array<uint8_t, 16> bigEndianOrder = {
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
        0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    };
    Guid g(bigEndianOrder, true);
    EXPECT_EQ(g, Guid("11223344-5566-7788-9900-aabbccddeeff"));
}

TEST(GuidTests, ReadOnlySpanCtor_WrongLengthThrows) {
    std::array<uint8_t, 4> shortArr = {1, 2, 3, 4};
    System::ReadOnlySpan<uint8_t> span(shortArr.data(), static_cast<SharpRuntime::intcs>(shortArr.size()));
    EXPECT_THROW(Guid{span}, System::ArgumentException);
}

// ---------------------------------------------------------------------------
// Component constructors — official .NET test vectors from GuidTests.cs
// ---------------------------------------------------------------------------

TEST(GuidTests, ComponentCtor_UintUshortUshortBytes_MatchesTestGuid) {
    // s_testGuid = "a8a110d5-fc49-43c5-bf46-802db8f843ff"
    Guid g(0xa8a110d5u, static_cast<uint16_t>(0xfc49), static_cast<uint16_t>(0x43c5),
           0xbf, 0x46, 0x80, 0x2d, 0xb8, 0xf8, 0x43, 0xff);
    EXPECT_EQ(g, Guid("a8a110d5-fc49-43c5-bf46-802db8f843ff"));
}

TEST(GuidTests, ComponentCtor_IntShortShortByteArray) {
    std::array<uint8_t, 8> d = {0, 1, 2, 3, 4, 5, 6, 7};
    Guid g(1, 2, 3, d);
    EXPECT_EQ(g, Guid("00000001-0002-0003-0001-020304050607"));
}

TEST(GuidTests, ComponentCtor_IntShortShortBytes) {
    Guid g(2147483647, static_cast<int16_t>(32767), static_cast<int16_t>(32767),
           0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0xAA, 0xBB);
    EXPECT_EQ(g, Guid("7fffffff-7fff-7fff-0a0b-0c0d0e0faabb"));
}

// ---------------------------------------------------------------------------
// Variant / Version
// ---------------------------------------------------------------------------

TEST(GuidTests, Variant_ForRfc4122Guid) {
    Guid g = Guid::NewGuid();
    // Variant is the top 4 bits of _d; RFC 4122 variant is 0b10xx -> value in [8, 11].
    EXPECT_GE(g.getVariantProperty(), 8);
    EXPECT_LE(g.getVariantProperty(), 11);
}

TEST(GuidTests, Version_ForNewGuidIsFour) {
    Guid g = Guid::NewGuid();
    EXPECT_EQ(g.getVersionProperty(), 4);
}

// ---------------------------------------------------------------------------
// CreateVersion7
// ---------------------------------------------------------------------------

TEST(GuidTests, CreateVersion7_HasVersionSevenAndRfc4122Variant) {
    Guid g = Guid::CreateVersion7();
    EXPECT_EQ(g.getVersionProperty(), 7);
    EXPECT_GE(g.getVariantProperty(), 8);
    EXPECT_LE(g.getVariantProperty(), 11);
}

TEST(GuidTests, CreateVersion7_TwoCallsDiffer) {
    Guid g1 = Guid::CreateVersion7();
    Guid g2 = Guid::CreateVersion7();
    EXPECT_NE(g1, g2);
}

// ---------------------------------------------------------------------------
// ParseExact / TryParseExact
// ---------------------------------------------------------------------------

TEST(GuidTests, ParseExact_D) {
    Guid g = Guid::ParseExact("a8a110d5-fc49-43c5-bf46-802db8f843ff", "D");
    EXPECT_EQ(g, Guid("a8a110d5-fc49-43c5-bf46-802db8f843ff"));
}

TEST(GuidTests, ParseExact_N) {
    Guid g = Guid::ParseExact("a8a110d5fc4943c5bf46802db8f843ff", "N");
    EXPECT_EQ(g, Guid("a8a110d5-fc49-43c5-bf46-802db8f843ff"));
}

TEST(GuidTests, ParseExact_X) {
    Guid g = Guid::ParseExact(
        "{0xa8a110d5,0xfc49,0x43c5,{0xbf,0x46,0x80,0x2d,0xb8,0xf8,0x43,0xff}}", "X");
    EXPECT_EQ(g, Guid("a8a110d5-fc49-43c5-bf46-802db8f843ff"));
}

TEST(GuidTests, ParseExact_WrongFormatThrows) {
    // Valid "D" text parsed with "N" exact format must fail.
    EXPECT_THROW(Guid::ParseExact("a8a110d5-fc49-43c5-bf46-802db8f843ff", "N"), System::FormatException);
}

TEST(GuidTests, ParseExact_InvalidFormatSpecifierThrows) {
    EXPECT_THROW(Guid::ParseExact("a8a110d5-fc49-43c5-bf46-802db8f843ff", "Q"), System::FormatException);
}

TEST(GuidTests, TryParseExact_Valid) {
    Guid g;
    EXPECT_TRUE(Guid::TryParseExact("{a8a110d5-fc49-43c5-bf46-802db8f843ff}", "B", g));
    EXPECT_EQ(g, Guid("a8a110d5-fc49-43c5-bf46-802db8f843ff"));
}

TEST(GuidTests, TryParseExact_WrongFormatReturnsFalse) {
    Guid g;
    EXPECT_FALSE(Guid::TryParseExact("a8a110d5-fc49-43c5-bf46-802db8f843ff", "N", g));
}

// ---------------------------------------------------------------------------
// Parse auto-detection of "X" format
// ---------------------------------------------------------------------------

TEST(GuidTests, Parse_ValidX_Parses) {
    Guid g = Guid::Parse("{0xa8a110d5,0xfc49,0x43c5,{0xbf,0x46,0x80,0x2d,0xb8,0xf8,0x43,0xff}}");
    EXPECT_EQ(g, Guid("a8a110d5-fc49-43c5-bf46-802db8f843ff"));
}

TEST(GuidTests, Parse_X_OverflowingComponentThrowsOverflow) {
    // 9-digit hex component overflows a 32-bit value.
    EXPECT_THROW(
        Guid::Parse("{0x1a8a110d5,0xfc49,0x43c5,{0xbf,0x46,0x80,0x2d,0xb8,0xf8,0x43,0xff}}"),
        System::OverflowException);
}

// Regression test for a code-audit finding (ticket 252): the X format's byte-component-specific
// overflow path (a component that fits in 32 bits -- so it doesn't trip TryParseHexRun's
// >8-significant-hex-digit overflow -- but exceeds a byte's 0xFF range) had zero test coverage,
// despite being a distinct branch with its own error message (MSG_OVERFLOW_BYTE) separate from
// the 32-bit-overflow path already covered above. Verified against real .NET's own
// TryParseExactX, whose comment explicitly documents this as an intentional "odd inconsistency":
// a byte component is parsed as a full uint32 and only afterward checked against byte.MaxValue,
// so "0x1ff" (511, 3 significant hex digits) throws OverflowException here just as it does in
// real .NET, distinct from a component with >8 hex digits.
TEST(GuidTests, Parse_X_OverflowingByteComponentThrowsOverflow) {
    EXPECT_THROW(
        Guid::Parse("{0xa8a110d5,0xfc49,0x43c5,{0xbf,0x46,0x80,0x2d,0xb8,0xf8,0x43,0x1ff}}"),
        System::OverflowException);
}

// Regression test for ticket 302: real .NET's Guid TryParseHex counts significant digits as
// it scans and, upon hitting an invalid character, reports Overflow_UInt32 (not a format
// error) if more than 8 significant digits were already consumed -- overflow takes precedence
// over a trailing invalid character. The port previously bailed out with FormatException as
// soon as it saw the invalid character, without ever checking whether the digit count already
// exceeded 8, so a 9-significant-digit component followed by garbage threw the wrong exception
// type (FormatException instead of OverflowException).
TEST(GuidTests, Parse_X_OverflowBeforeInvalidChar_ThrowsOverflowNotFormat) {
    EXPECT_THROW(
        Guid::Parse("{0x123456789z,0x351d,0x4694,{0x93,0x92,0x03,0xac,0xc5,0x87,0x0e,0xb1}}"),
        System::OverflowException);
}

// Boundary check: exactly 8 significant digits (not >8) followed by an invalid character is
// still a plain format error, not overflow -- confirms the fix didn't over-widen the precedence.
TEST(GuidTests, Parse_X_ExactlyEightDigitsThenInvalidChar_ThrowsFormat) {
    EXPECT_THROW(
        Guid::Parse("{0x12345678z,0x351d,0x4694,{0x93,0x92,0x03,0xac,0xc5,0x87,0x0e,0xb1}}"),
        System::FormatException);
}

// ---------------------------------------------------------------------------
// TryFormat / TryFormatUtf8
// ---------------------------------------------------------------------------

TEST(GuidTests, TryFormat_FitsBuffer) {
    Guid g("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    char buf[36];
    SharpRuntime::intcs written = 0;
    System::Span<char> dest(buf, 36);
    EXPECT_TRUE(g.TryFormat(dest, written));
    EXPECT_EQ(written, 36);
    EXPECT_EQ(std::string(buf, 36), "a8a110d5-fc49-43c5-bf46-802db8f843ff");
}

TEST(GuidTests, TryFormat_TooSmallReturnsFalse) {
    Guid g("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    char buf[10];
    SharpRuntime::intcs written = -1;
    System::Span<char> dest(buf, 10);
    EXPECT_FALSE(g.TryFormat(dest, written));
    EXPECT_EQ(written, 0);
}

TEST(GuidTests, TryFormatUtf8_FitsBuffer) {
    Guid g("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    uint8_t buf[32];
    SharpRuntime::intcs written = 0;
    System::Span<uint8_t> dest(buf, 32);
    EXPECT_TRUE(g.TryFormatUtf8(dest, written, "N"));
    EXPECT_EQ(written, 32);
    EXPECT_EQ(std::string(reinterpret_cast<char*>(buf), 32), "a8a110d5fc4943c5bf46802db8f843ff");
}

// ---------------------------------------------------------------------------
// TryWriteBytes
// ---------------------------------------------------------------------------

TEST(GuidTests, TryWriteBytes_TooSmallReturnsFalse) {
    Guid g = Guid::NewGuid();
    uint8_t buf[8];
    System::Span<uint8_t> dest(buf, 8);
    EXPECT_FALSE(g.TryWriteBytes(dest));
}

TEST(GuidTests, TryWriteBytes_WritesNativeOrder) {
    Guid g("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    uint8_t buf[16] = {};
    System::Span<uint8_t> dest(buf, 16);
    EXPECT_TRUE(g.TryWriteBytes(dest));
    auto expected = g.ToByteArray();
    for (int i = 0; i < 16; ++i) EXPECT_EQ(buf[i], expected[static_cast<size_t>(i)]);
}

TEST(GuidTests, TryWriteBytes_BigEndian) {
    Guid g("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    uint8_t buf[16] = {};
    System::Span<uint8_t> dest(buf, 16);
    SharpRuntime::intcs written = 0;
    EXPECT_TRUE(g.TryWriteBytes(dest, true, written));
    EXPECT_EQ(written, 16);
    auto expected = g.ToByteArray(true);
    for (int i = 0; i < 16; ++i) EXPECT_EQ(buf[i], expected[static_cast<size_t>(i)]);
}

// ---------------------------------------------------------------------------
// GetHashCode
// ---------------------------------------------------------------------------

TEST(GuidTests, GetHashCode_EqualGuidsHaveEqualHashCodes) {
    Guid a("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    Guid b("a8a110d5-fc49-43c5-bf46-802db8f843ff");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(GuidTests, GetHashCode_EmptyIsZero) {
    // All 16 bytes zero XORs to zero regardless of byte order.
    EXPECT_EQ(Guid::Empty.GetHashCode(), 0);
}
