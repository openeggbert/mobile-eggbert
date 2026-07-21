// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Buffers/Binary/BinaryPrimitives.hpp"
#include "System/Int128.hpp"
#include "System/UInt128.hpp"

using System::Buffers::Binary::BinaryPrimitives;
using System::ReadOnlySpan;
using System::Span;

TEST(BinaryPrimitivesTest, ReadInt32LittleEndian) {
    uint8_t buf[] = { 0x01, 0x00, 0x00, 0x00 };
    ReadOnlySpan<uint8_t> span(buf, 4);
    EXPECT_EQ(BinaryPrimitives::ReadInt32LittleEndian(span), 1);
}

TEST(BinaryPrimitivesTest, ReadInt32BigEndian) {
    uint8_t buf[] = { 0x00, 0x00, 0x00, 0x01 };
    ReadOnlySpan<uint8_t> span(buf, 4);
    EXPECT_EQ(BinaryPrimitives::ReadInt32BigEndian(span), 1);
}

TEST(BinaryPrimitivesTest, WriteAndReadInt32RoundTrip) {
    uint8_t buf[4] = {};
    Span<uint8_t> dst(buf, 4);
    BinaryPrimitives::WriteInt32LittleEndian(dst, 0x12345678);
    ReadOnlySpan<uint8_t> src(buf, 4);
    EXPECT_EQ(BinaryPrimitives::ReadInt32LittleEndian(src), 0x12345678);
}

TEST(BinaryPrimitivesTest, WriteInt32BigEndian) {
    uint8_t buf[4] = {};
    Span<uint8_t> dst(buf, 4);
    BinaryPrimitives::WriteInt32BigEndian(dst, 0x01020304);
    EXPECT_EQ(buf[0], 0x01);
    EXPECT_EQ(buf[3], 0x04);
}

TEST(BinaryPrimitivesTest, ReadUInt16LittleEndian) {
    uint8_t buf[] = { 0xFF, 0x00 };
    ReadOnlySpan<uint8_t> span(buf, 2);
    EXPECT_EQ(BinaryPrimitives::ReadUInt16LittleEndian(span), 0x00FFu);
}

TEST(BinaryPrimitivesTest, ReadInt64LittleEndian) {
    uint8_t buf[] = { 1,0,0,0, 0,0,0,0 };
    ReadOnlySpan<uint8_t> span(buf, 8);
    EXPECT_EQ(BinaryPrimitives::ReadInt64LittleEndian(span), 1LL);
}

TEST(BinaryPrimitivesTest, ReverseEndiannessUInt16) {
    EXPECT_EQ(BinaryPrimitives::ReverseEndianness(uint16_t(0x1234)), uint16_t(0x3412));
}

TEST(BinaryPrimitivesTest, ReverseEndiannessUInt32) {
    EXPECT_EQ(BinaryPrimitives::ReverseEndianness(uint32_t(0x12345678)), uint32_t(0x78563412));
}

TEST(BinaryPrimitivesTest, ReverseEndiannessInt16) {
    int16_t in = 0x0102;
    int16_t expected;
    uint16_t u = BinaryPrimitives::ReverseEndianness(uint16_t(0x0102));
    std::memcpy(&expected, &u, 2);
    EXPECT_EQ(BinaryPrimitives::ReverseEndianness(in), expected);
}

// Regression test for a wave-3 audit finding: threw std::out_of_range (an unrelated std::
// exception type invisible to code catching System::Exception&) instead of
// System::ArgumentOutOfRangeException, which is what real .NET's BinaryPrimitives Read*
// methods throw when the source span is too small.
TEST(BinaryPrimitivesTest, SpanTooSmallThrows) {
    uint8_t buf[2] = {};
    ReadOnlySpan<uint8_t> span(buf, 2);
    EXPECT_THROW(BinaryPrimitives::ReadInt32LittleEndian(span), System::ArgumentOutOfRangeException);
}

// ===========================================================================
// Single / Double
// ===========================================================================

TEST(BinaryPrimitivesTest, WriteAndReadSingleLittleEndianRoundTrip) {
    uint8_t buf[4] = {};
    Span<uint8_t> dst(buf, 4);
    BinaryPrimitives::WriteSingleLittleEndian(dst, 3.5f);
    ReadOnlySpan<uint8_t> src(buf, 4);
    EXPECT_FLOAT_EQ(BinaryPrimitives::ReadSingleLittleEndian(src), 3.5f);
}

TEST(BinaryPrimitivesTest, WriteAndReadSingleBigEndianRoundTrip) {
    uint8_t buf[4] = {};
    Span<uint8_t> dst(buf, 4);
    BinaryPrimitives::WriteSingleBigEndian(dst, -12.25f);
    ReadOnlySpan<uint8_t> src(buf, 4);
    EXPECT_FLOAT_EQ(BinaryPrimitives::ReadSingleBigEndian(src), -12.25f);
}

TEST(BinaryPrimitivesTest, WriteAndReadDoubleLittleEndianRoundTrip) {
    uint8_t buf[8] = {};
    Span<uint8_t> dst(buf, 8);
    BinaryPrimitives::WriteDoubleLittleEndian(dst, 3.14159265358979);
    ReadOnlySpan<uint8_t> src(buf, 8);
    EXPECT_DOUBLE_EQ(BinaryPrimitives::ReadDoubleLittleEndian(src), 3.14159265358979);
}

TEST(BinaryPrimitivesTest, WriteAndReadDoubleBigEndianRoundTrip) {
    uint8_t buf[8] = {};
    Span<uint8_t> dst(buf, 8);
    BinaryPrimitives::WriteDoubleBigEndian(dst, -42.5);
    ReadOnlySpan<uint8_t> src(buf, 8);
    EXPECT_DOUBLE_EQ(BinaryPrimitives::ReadDoubleBigEndian(src), -42.5);
}

// ===========================================================================
// TryRead / TryWrite
// ===========================================================================

TEST(BinaryPrimitivesTest, TryReadInt32LittleEndian_Success) {
    uint8_t buf[] = { 0x78, 0x56, 0x34, 0x12 };
    ReadOnlySpan<uint8_t> span(buf, 4);
    int32_t value = 0;
    EXPECT_TRUE(BinaryPrimitives::TryReadInt32LittleEndian(span, value));
    EXPECT_EQ(value, 0x12345678);
}

TEST(BinaryPrimitivesTest, TryReadInt32LittleEndian_TooSmall_ReturnsFalse) {
    uint8_t buf[] = { 0x01, 0x02 };
    ReadOnlySpan<uint8_t> span(buf, 2);
    int32_t value = 42;
    EXPECT_FALSE(BinaryPrimitives::TryReadInt32LittleEndian(span, value));
    EXPECT_EQ(value, 0);
}

TEST(BinaryPrimitivesTest, TryReadUInt16BigEndian_Success) {
    uint8_t buf[] = { 0x01, 0x02 };
    ReadOnlySpan<uint8_t> span(buf, 2);
    uint16_t value = 0;
    EXPECT_TRUE(BinaryPrimitives::TryReadUInt16BigEndian(span, value));
    EXPECT_EQ(value, 0x0102u);
}

TEST(BinaryPrimitivesTest, TryReadInt64LittleEndian_TooSmall_ReturnsFalse) {
    uint8_t buf[4] = {};
    ReadOnlySpan<uint8_t> span(buf, 4);
    int64_t value = 7;
    EXPECT_FALSE(BinaryPrimitives::TryReadInt64LittleEndian(span, value));
    EXPECT_EQ(value, 0);
}

TEST(BinaryPrimitivesTest, TryReadSingleLittleEndian_RoundTrip) {
    uint8_t buf[4] = {};
    Span<uint8_t> dst(buf, 4);
    BinaryPrimitives::WriteSingleLittleEndian(dst, 1.5f);
    ReadOnlySpan<uint8_t> src(buf, 4);
    float value = 0.0f;
    EXPECT_TRUE(BinaryPrimitives::TryReadSingleLittleEndian(src, value));
    EXPECT_FLOAT_EQ(value, 1.5f);
}

TEST(BinaryPrimitivesTest, TryReadSingleLittleEndian_TooSmall_ReturnsFalse) {
    uint8_t buf[2] = {};
    ReadOnlySpan<uint8_t> span(buf, 2);
    float value = 1.0f;
    EXPECT_FALSE(BinaryPrimitives::TryReadSingleLittleEndian(span, value));
    EXPECT_FLOAT_EQ(value, 0.0f);
}

TEST(BinaryPrimitivesTest, TryReadDoubleBigEndian_RoundTrip) {
    uint8_t buf[8] = {};
    Span<uint8_t> dst(buf, 8);
    BinaryPrimitives::WriteDoubleBigEndian(dst, 2.71828);
    ReadOnlySpan<uint8_t> src(buf, 8);
    double value = 0.0;
    EXPECT_TRUE(BinaryPrimitives::TryReadDoubleBigEndian(src, value));
    EXPECT_DOUBLE_EQ(value, 2.71828);
}

TEST(BinaryPrimitivesTest, TryWriteInt32LittleEndian_Success) {
    uint8_t buf[4] = {};
    Span<uint8_t> dst(buf, 4);
    EXPECT_TRUE(BinaryPrimitives::TryWriteInt32LittleEndian(dst, 0x12345678));
    ReadOnlySpan<uint8_t> src(buf, 4);
    EXPECT_EQ(BinaryPrimitives::ReadInt32LittleEndian(src), 0x12345678);
}

TEST(BinaryPrimitivesTest, TryWriteInt32LittleEndian_TooSmall_ReturnsFalse) {
    uint8_t buf[2] = {};
    Span<uint8_t> dst(buf, 2);
    EXPECT_FALSE(BinaryPrimitives::TryWriteInt32LittleEndian(dst, 1));
}

TEST(BinaryPrimitivesTest, TryWriteUInt64BigEndian_Success) {
    uint8_t buf[8] = {};
    Span<uint8_t> dst(buf, 8);
    EXPECT_TRUE(BinaryPrimitives::TryWriteUInt64BigEndian(dst, 0x0102030405060708ull));
    ReadOnlySpan<uint8_t> src(buf, 8);
    EXPECT_EQ(BinaryPrimitives::ReadUInt64BigEndian(src), 0x0102030405060708ull);
}

TEST(BinaryPrimitivesTest, TryWriteSingleLittleEndian_TooSmall_ReturnsFalse) {
    uint8_t buf[2] = {};
    Span<uint8_t> dst(buf, 2);
    EXPECT_FALSE(BinaryPrimitives::TryWriteSingleLittleEndian(dst, 1.0f));
}

TEST(BinaryPrimitivesTest, TryWriteDoubleBigEndian_Success) {
    uint8_t buf[8] = {};
    Span<uint8_t> dst(buf, 8);
    EXPECT_TRUE(BinaryPrimitives::TryWriteDoubleBigEndian(dst, 9.5));
    ReadOnlySpan<uint8_t> src(buf, 8);
    EXPECT_DOUBLE_EQ(BinaryPrimitives::ReadDoubleBigEndian(src), 9.5);
}

// ---------------------------------------------------------------------------
// Int128 / UInt128 -- System::Int128/UInt128 were fully ported elsewhere but this file had no
// Read/Write/TryRead/TryWrite*Endian or ReverseEndianness overloads for them at all, unlike
// real .NET (added in .NET 7). Verified byte layout against BinaryPrimitives.Read*Endian.cs by
// hand-tracing MemoryMarshal.Read<Int128> on a little-endian host and its full-16-byte-reversed
// counterpart for big-endian.
// ---------------------------------------------------------------------------

TEST(BinaryPrimitivesTest, ReadInt128LittleEndian_ByteLayout) {
    // LE layout: low 64 bits occupy the first 8 bytes, high 64 bits the last 8.
    uint8_t buf[16] = { 1,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0 };
    ReadOnlySpan<uint8_t> span(buf, 16);
    EXPECT_EQ(BinaryPrimitives::ReadInt128LittleEndian(span), System::Int128(1));
}

TEST(BinaryPrimitivesTest, ReadInt128BigEndian_ByteLayout) {
    // BE layout: high 64 bits occupy the first 8 bytes (each in BE order), low 64 bits the last 8.
    uint8_t buf[16] = { 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,1 };
    ReadOnlySpan<uint8_t> span(buf, 16);
    EXPECT_EQ(BinaryPrimitives::ReadInt128BigEndian(span), System::Int128(1));
}

TEST(BinaryPrimitivesTest, WriteAndReadInt128LittleEndian_RoundTrip) {
    uint8_t buf[16] = {};
    Span<uint8_t> dst(buf, 16);
    System::Int128 original(0x0102030405060708ULL, 0x1112131415161718ULL);
    BinaryPrimitives::WriteInt128LittleEndian(dst, original);
    ReadOnlySpan<uint8_t> src(buf, 16);
    EXPECT_EQ(BinaryPrimitives::ReadInt128LittleEndian(src), original);
}

TEST(BinaryPrimitivesTest, WriteAndReadInt128BigEndian_RoundTrip) {
    uint8_t buf[16] = {};
    Span<uint8_t> dst(buf, 16);
    System::Int128 original(0x0102030405060708ULL, 0x1112131415161718ULL);
    BinaryPrimitives::WriteInt128BigEndian(dst, original);
    ReadOnlySpan<uint8_t> src(buf, 16);
    EXPECT_EQ(BinaryPrimitives::ReadInt128BigEndian(src), original);
}

TEST(BinaryPrimitivesTest, WriteAndReadUInt128LittleEndian_RoundTrip) {
    uint8_t buf[16] = {};
    Span<uint8_t> dst(buf, 16);
    System::UInt128 original(0xFFEEDDCCBBAA9988ULL, 0x0011223344556677ULL);
    BinaryPrimitives::WriteUInt128LittleEndian(dst, original);
    ReadOnlySpan<uint8_t> src(buf, 16);
    EXPECT_EQ(BinaryPrimitives::ReadUInt128LittleEndian(src), original);
}

TEST(BinaryPrimitivesTest, WriteAndReadUInt128BigEndian_RoundTrip) {
    uint8_t buf[16] = {};
    Span<uint8_t> dst(buf, 16);
    System::UInt128 original(0xFFEEDDCCBBAA9988ULL, 0x0011223344556677ULL);
    BinaryPrimitives::WriteUInt128BigEndian(dst, original);
    ReadOnlySpan<uint8_t> src(buf, 16);
    EXPECT_EQ(BinaryPrimitives::ReadUInt128BigEndian(src), original);
}

TEST(BinaryPrimitivesTest, TryReadInt128LittleEndian_TooSmall_ReturnsFalse) {
    uint8_t buf[15] = {};
    ReadOnlySpan<uint8_t> src(buf, 15);
    System::Int128 value(123);
    EXPECT_FALSE(BinaryPrimitives::TryReadInt128LittleEndian(src, value));
    EXPECT_EQ(value, System::Int128());
}

TEST(BinaryPrimitivesTest, TryWriteUInt128BigEndian_TooSmall_ReturnsFalse) {
    uint8_t buf[15] = {};
    Span<uint8_t> dst(buf, 15);
    EXPECT_FALSE(BinaryPrimitives::TryWriteUInt128BigEndian(dst, System::UInt128(1)));
}

TEST(BinaryPrimitivesTest, ReadInt128LittleEndian_TooSmall_Throws) {
    uint8_t buf[15] = {};
    ReadOnlySpan<uint8_t> src(buf, 15);
    EXPECT_THROW(BinaryPrimitives::ReadInt128LittleEndian(src), System::ArgumentOutOfRangeException);
}

// The two 64-bit halves both swap position AND each reverses internally -- verified against
// real .NET's Int128 ReverseEndianness(value) = new Int128(ReverseEndianness(value.Lower),
// ReverseEndianness(value.Upper)).
TEST(BinaryPrimitivesTest, ReverseEndianness_Int128) {
    System::Int128 value(0x0102030405060708ULL, 0x1112131415161718ULL);
    System::Int128 reversed = BinaryPrimitives::ReverseEndianness(value);
    EXPECT_EQ(reversed.getUpperProperty(), 0x1817161514131211ULL);
    EXPECT_EQ(reversed.getLowerProperty(), 0x0807060504030201ULL);
}

TEST(BinaryPrimitivesTest, ReverseEndianness_UInt128_RoundTrips) {
    System::UInt128 value(0xFFEEDDCCBBAA9988ULL, 0x0011223344556677ULL);
    System::UInt128 reversed = BinaryPrimitives::ReverseEndianness(value);
    System::UInt128 roundTrip = BinaryPrimitives::ReverseEndianness(reversed);
    EXPECT_EQ(roundTrip, value);
}
