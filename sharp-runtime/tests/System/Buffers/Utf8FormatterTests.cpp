// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Buffers/Text/Utf8Formatter.hpp"
#include "System/FormatException.hpp"
#include "System/NotSupportedException.hpp"

using System::Buffers::Text::Utf8Formatter;
using System::Buffers::StandardFormat;
using System::Span;
using System::FormatException;
using System::NotSupportedException;

static std::string spanToString(const uint8_t* p, int len) {
    return std::string(reinterpret_cast<const char*>(p), static_cast<size_t>(len));
}

TEST(Utf8FormatterTest, FormatBoolTrue) {
    uint8_t buf[10] = {};
    Span<uint8_t> span(buf, 10);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(true, span, written));
    EXPECT_EQ(spanToString(buf, written), "True");
}

TEST(Utf8FormatterTest, FormatBoolFalse) {
    uint8_t buf[10] = {};
    Span<uint8_t> span(buf, 10);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(false, span, written));
    EXPECT_EQ(spanToString(buf, written), "False");
}

TEST(Utf8FormatterTest, FormatBoolLowercase) {
    uint8_t buf[10] = {};
    Span<uint8_t> span(buf, 10);
    int written = 0;
    System::Buffers::StandardFormat fmt('l');
    EXPECT_TRUE(Utf8Formatter::TryFormat(true, span, written, fmt));
    EXPECT_EQ(spanToString(buf, written), "true");
}

TEST(Utf8FormatterTest, FormatBoolDestinationTooSmall) {
    uint8_t buf[2] = {};
    Span<uint8_t> span(buf, 2);
    int written = 0;
    EXPECT_FALSE(Utf8Formatter::TryFormat(true, span, written));
    EXPECT_EQ(written, 0);
}

TEST(Utf8FormatterTest, FormatInt32Zero) {
    uint8_t buf[10] = {};
    Span<uint8_t> span(buf, 10);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(int32_t(0), span, written));
    EXPECT_EQ(spanToString(buf, written), "0");
}

TEST(Utf8FormatterTest, FormatInt32Positive) {
    uint8_t buf[20] = {};
    Span<uint8_t> span(buf, 20);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(int32_t(12345), span, written));
    EXPECT_EQ(spanToString(buf, written), "12345");
}

TEST(Utf8FormatterTest, FormatInt32Negative) {
    uint8_t buf[20] = {};
    Span<uint8_t> span(buf, 20);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(int32_t(-42), span, written));
    EXPECT_EQ(spanToString(buf, written), "-42");
}

TEST(Utf8FormatterTest, FormatUInt64Large) {
    uint8_t buf[25] = {};
    Span<uint8_t> span(buf, 25);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(uint64_t(18446744073709551615ULL), span, written));
    EXPECT_EQ(spanToString(buf, written), "18446744073709551615");
}

TEST(Utf8FormatterTest, FormatUInt8) {
    uint8_t buf[10] = {};
    Span<uint8_t> span(buf, 10);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(uint8_t(255), span, written));
    EXPECT_EQ(spanToString(buf, written), "255");
}

TEST(Utf8FormatterTest, FormatDestinationTooSmallInt) {
    uint8_t buf[2] = {};
    Span<uint8_t> span(buf, 2);
    int written = 0;
    EXPECT_FALSE(Utf8Formatter::TryFormat(int32_t(12345), span, written));
    EXPECT_EQ(written, 0);
}

// ===========================================================================
// Format specifiers: D (precision), X (hex), N (grouping), bad format
// ===========================================================================

TEST(Utf8FormatterTest, FormatBool_BadSpecifier_Throws) {
    uint8_t buf[10] = {};
    Span<uint8_t> span(buf, 10);
    int written = 0;
    EXPECT_THROW(Utf8Formatter::TryFormat(true, span, written, StandardFormat('X')), FormatException);
}

TEST(Utf8FormatterTest, FormatInt32_D_WithPrecision_ZeroPads) {
    uint8_t buf[20] = {};
    Span<uint8_t> span(buf, 20);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(int32_t(42), span, written, StandardFormat('D', 6)));
    EXPECT_EQ(spanToString(buf, written), "000042");
}

TEST(Utf8FormatterTest, FormatInt32Negative_D_WithPrecision_PadsAfterSign) {
    uint8_t buf[20] = {};
    Span<uint8_t> span(buf, 20);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(int32_t(-42), span, written, StandardFormat('D', 6)));
    EXPECT_EQ(spanToString(buf, written), "-000042");
}

TEST(Utf8FormatterTest, FormatUInt32_X_Lowercase) {
    uint8_t buf[20] = {};
    Span<uint8_t> span(buf, 20);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(uint32_t(0xABCD), span, written, StandardFormat('x')));
    EXPECT_EQ(spanToString(buf, written), "abcd");
}

TEST(Utf8FormatterTest, FormatUInt32_X_UppercaseWithPrecision) {
    uint8_t buf[20] = {};
    Span<uint8_t> span(buf, 20);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(uint32_t(0xAB), span, written, StandardFormat('X', 8)));
    EXPECT_EQ(spanToString(buf, written), "000000AB");
}

TEST(Utf8FormatterTest, FormatInt8Negative_X_ShowsWidthBitPattern) {
    uint8_t buf[20] = {};
    Span<uint8_t> span(buf, 20);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(int8_t(-1), span, written, StandardFormat('X')));
    EXPECT_EQ(spanToString(buf, written), "FF");
}

TEST(Utf8FormatterTest, FormatInt32_N_DefaultTwoDecimals) {
    uint8_t buf[20] = {};
    Span<uint8_t> span(buf, 20);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(int32_t(32767), span, written, StandardFormat('N')));
    EXPECT_EQ(spanToString(buf, written), "32,767.00");
}

TEST(Utf8FormatterTest, FormatInt32_N0_NoDecimals) {
    uint8_t buf[20] = {};
    Span<uint8_t> span(buf, 20);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(int32_t(32767), span, written, StandardFormat('N', 0)));
    EXPECT_EQ(spanToString(buf, written), "32,767");
}

TEST(Utf8FormatterTest, FormatInt32Negative_N_GroupsAfterSign) {
    uint8_t buf[20] = {};
    Span<uint8_t> span(buf, 20);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(int32_t(-1234567), span, written, StandardFormat('N', 0)));
    EXPECT_EQ(spanToString(buf, written), "-1,234,567");
}

TEST(Utf8FormatterTest, FormatInt32_G_WithPrecision_Throws) {
    uint8_t buf[20] = {};
    Span<uint8_t> span(buf, 20);
    int written = 0;
    EXPECT_THROW(Utf8Formatter::TryFormat(int32_t(42), span, written, StandardFormat('G', 3)), NotSupportedException);
}

TEST(Utf8FormatterTest, FormatInt32_BadSpecifier_Throws) {
    uint8_t buf[20] = {};
    Span<uint8_t> span(buf, 20);
    int written = 0;
    EXPECT_THROW(Utf8Formatter::TryFormat(int32_t(42), span, written, StandardFormat('Q')), FormatException);
}

// Regression tests for ticket 323: StandardFormat's precision (used as minDigits/decimalDigits)
// is caller-controlled up to MaxPrecision=99, but the internal formatting functions' stack
// buffers were sized only for each type's natural max width, not for a high requested
// precision -- confirmed via standalone ASan repros as genuine stack-buffer-overflows in all
// four of: tryFormatUnsignedDecimal ('D'/unsigned), tryFormatSignedDecimal ('D'/signed),
// tryFormatUnsignedHex ('X'), and tryFormatGrouped ('N').
TEST(Utf8FormatterTest, FormatUInt64_D_HighPrecision_DoesNotOverflow) {
    uint8_t buf[150] = {};
    Span<uint8_t> span(buf, 150);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(uint64_t(5), span, written, StandardFormat('D', 99)));
    EXPECT_EQ(written, 99);
    EXPECT_EQ(spanToString(buf, written), std::string(98, '0') + "5");
}

TEST(Utf8FormatterTest, FormatInt64_D_HighPrecision_DoesNotOverflow) {
    uint8_t buf[150] = {};
    Span<uint8_t> span(buf, 150);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(int64_t(-5), span, written, StandardFormat('D', 99)));
    EXPECT_EQ(written, 100); // 99 digits + sign
    EXPECT_EQ(spanToString(buf, written), "-" + std::string(98, '0') + "5");
}

TEST(Utf8FormatterTest, FormatUInt64_X_HighPrecision_DoesNotOverflow) {
    uint8_t buf[150] = {};
    Span<uint8_t> span(buf, 150);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(uint64_t(5), span, written, StandardFormat('X', 99)));
    EXPECT_EQ(written, 99);
    EXPECT_EQ(spanToString(buf, written), std::string(98, '0') + "5");
}

TEST(Utf8FormatterTest, FormatUInt64_N_HighPrecision_DoesNotOverflow) {
    uint8_t buf[200] = {};
    Span<uint8_t> span(buf, 200);
    int written = 0;
    EXPECT_TRUE(Utf8Formatter::TryFormat(uint64_t(123456789012345ULL), span, written, StandardFormat('N', 99)));
    EXPECT_EQ(written, 119); // "123,456,789,012,345" (19 chars) + '.' + 99 zeros
    EXPECT_EQ(spanToString(buf, written), "123,456,789,012,345." + std::string(99, '0'));
}
