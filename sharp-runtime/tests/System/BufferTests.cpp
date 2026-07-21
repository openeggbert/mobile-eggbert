// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <vector>
#include "System/Buffer.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using System::Buffer;
using SharpRuntime::bytecs;
using SharpRuntime::intcs;

// ---------------------------------------------------------------------------
// BlockCopy — raw pointer overload
// ---------------------------------------------------------------------------

TEST(BufferTests, BlockCopy_RawPointer_CopiesBytes) {
    uint8_t src[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t dst[4] = {0x00, 0x00, 0x00, 0x00};
    Buffer::BlockCopy(src, 0, dst, 0, 4);
    EXPECT_EQ(dst[0], 0xAA);
    EXPECT_EQ(dst[1], 0xBB);
    EXPECT_EQ(dst[2], 0xCC);
    EXPECT_EQ(dst[3], 0xDD);
}

TEST(BufferTests, BlockCopy_RawPointer_SrcOffset) {
    uint8_t src[4] = {0x01, 0x02, 0x03, 0x04};
    uint8_t dst[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    Buffer::BlockCopy(src, 2, dst, 0, 2);
    EXPECT_EQ(dst[0], 0x03);
    EXPECT_EQ(dst[1], 0x04);
    EXPECT_EQ(dst[2], 0xFF);  // untouched
}

TEST(BufferTests, BlockCopy_RawPointer_DstOffset) {
    uint8_t src[2] = {0xAB, 0xCD};
    uint8_t dst[4] = {0x00, 0x00, 0x00, 0x00};
    Buffer::BlockCopy(src, 0, dst, 2, 2);
    EXPECT_EQ(dst[0], 0x00);  // untouched
    EXPECT_EQ(dst[1], 0x00);  // untouched
    EXPECT_EQ(dst[2], 0xAB);
    EXPECT_EQ(dst[3], 0xCD);
}

TEST(BufferTests, BlockCopy_RawPointer_Int32Array) {
    int32_t src[2] = {0x12345678, static_cast<int32_t>(0x9ABCDEF0)};
    int32_t dst[2] = {0, 0};
    Buffer::BlockCopy(src, 0, dst, 0, 8);
    EXPECT_EQ(dst[0], 0x12345678);
    EXPECT_EQ(dst[1], static_cast<int32_t>(0x9ABCDEF0));
}

// ---------------------------------------------------------------------------
// BlockCopy — vector<bytecs> overload
// ---------------------------------------------------------------------------

TEST(BufferTests, BlockCopy_Vector_FullCopy) {
    std::vector<bytecs> src = {1, 2, 3, 4};
    std::vector<bytecs> dst(4, 0);
    Buffer::BlockCopy(src, 0, dst, 0, 4);
    EXPECT_EQ(dst, src);
}

TEST(BufferTests, BlockCopy_Vector_SrcOffset) {
    std::vector<bytecs> src = {10, 20, 30, 40};
    std::vector<bytecs> dst(4, 0);
    Buffer::BlockCopy(src, 1, dst, 0, 3);
    EXPECT_EQ(dst[0], 20);
    EXPECT_EQ(dst[1], 30);
    EXPECT_EQ(dst[2], 40);
}

TEST(BufferTests, BlockCopy_Vector_DstOffset) {
    std::vector<bytecs> src = {0xAA, 0xBB};
    std::vector<bytecs> dst = {0x00, 0x00, 0x00, 0x00};
    Buffer::BlockCopy(src, 0, dst, 2, 2);
    EXPECT_EQ(dst[0], 0x00);
    EXPECT_EQ(dst[1], 0x00);
    EXPECT_EQ(dst[2], 0xAA);
    EXPECT_EQ(dst[3], 0xBB);
}

// ---------------------------------------------------------------------------
// ByteLength
// ---------------------------------------------------------------------------

TEST(BufferTests, ByteLength_ByteVector_EqualsSizeInBytes) {
    std::vector<bytecs> v = {1, 2, 3, 4};
    EXPECT_EQ(Buffer::ByteLength(v), 4);
}

TEST(BufferTests, ByteLength_Int32Vector_IsFourTimesCount) {
    std::vector<int32_t> v = {0, 0, 0};
    EXPECT_EQ(Buffer::ByteLength(v), 12);
}

TEST(BufferTests, ByteLength_Int64Vector_IsEightTimesCount) {
    std::vector<int64_t> v = {0, 0};
    EXPECT_EQ(Buffer::ByteLength(v), 16);
}

TEST(BufferTests, ByteLength_EmptyVector_IsZero) {
    std::vector<int32_t> v;
    EXPECT_EQ(Buffer::ByteLength(v), 0);
}

// ---------------------------------------------------------------------------
// GetByte / SetByte
// ---------------------------------------------------------------------------

TEST(BufferTests, GetByte_LittleEndian_FirstByteOfInt32) {
    std::vector<int32_t> v = {0x01020304};
    // On a little-endian system byte 0 is the LSB
    bytecs b0 = Buffer::GetByte(v, 0);
    bytecs b3 = Buffer::GetByte(v, 3);
    EXPECT_EQ(b0 + (static_cast<int>(b3) << 24), (0x01020304 & 0xFF000000) | b0);
    // Just verify the round-trip rather than a platform-specific value:
    EXPECT_EQ(static_cast<int>(b0) | (static_cast<int>(Buffer::GetByte(v, 1)) << 8) |
              (static_cast<int>(Buffer::GetByte(v, 2)) << 16) |
              (static_cast<int>(Buffer::GetByte(v, 3)) << 24), 0x01020304);
}

TEST(BufferTests, SetByte_ModifiesCorrectByte) {
    std::vector<bytecs> v = {0xAA, 0xBB, 0xCC};
    Buffer::SetByte(v, 1, 0xFF);
    EXPECT_EQ(v[0], 0xAA);
    EXPECT_EQ(v[1], 0xFF);
    EXPECT_EQ(v[2], 0xCC);
}

TEST(BufferTests, GetByte_AfterSetByte_RoundTrip) {
    std::vector<bytecs> v = {0x00, 0x00, 0x00};
    Buffer::SetByte(v, 2, 0x42);
    EXPECT_EQ(Buffer::GetByte(v, 2), 0x42);
}

TEST(BufferTests, SetByte_FirstByte) {
    std::vector<bytecs> v = {0x01, 0x02};
    Buffer::SetByte(v, 0, 0xDE);
    EXPECT_EQ(Buffer::GetByte(v, 0), 0xDE);
}

// ---------------------------------------------------------------------------
// BlockCopy — edge cases
// ---------------------------------------------------------------------------

TEST(BufferTests, BlockCopy_ZeroCount_NoOp) {
    std::vector<bytecs> src = {1, 2, 3};
    std::vector<bytecs> dst = {9, 9, 9};
    Buffer::BlockCopy(src, 0, dst, 0, 0);
    EXPECT_EQ(dst[0], 9);
    EXPECT_EQ(dst[1], 9);
    EXPECT_EQ(dst[2], 9);
}

TEST(BufferTests, BlockCopy_RawPointer_ZeroCount_NoOp) {
    uint8_t src[2] = {0xAA, 0xBB};
    uint8_t dst[2] = {0x00, 0x00};
    Buffer::BlockCopy(src, 0, dst, 0, 0);
    EXPECT_EQ(dst[0], 0x00);
}

TEST(BufferTests, BlockCopy_RawPointer_OverlapSafe) {
    // .NET Buffer.BlockCopy copies via Memmove, so overlapping regions are safe.
    uint8_t buf[6] = {1, 2, 3, 4, 5, 6};
    // Shift bytes [0..3] one position to the right within the same buffer.
    Buffer::BlockCopy(buf, 0, buf, 1, 4);
    EXPECT_EQ(buf[1], 1);
    EXPECT_EQ(buf[2], 2);
    EXPECT_EQ(buf[3], 3);
    EXPECT_EQ(buf[4], 4);
}

// ---------------------------------------------------------------------------
// ByteLength — extra types
// ---------------------------------------------------------------------------

TEST(BufferTests, ByteLength_SingleByteElement) {
    std::vector<bytecs> v = {42};
    EXPECT_EQ(Buffer::ByteLength(v), 1);
}

TEST(BufferTests, ByteLength_FloatVector) {
    std::vector<float> v(3);
    EXPECT_EQ(Buffer::ByteLength(v), 12); // 3 * 4 bytes
}

TEST(BufferTests, ByteLength_DoubleVector) {
    std::vector<double> v(2);
    EXPECT_EQ(Buffer::ByteLength(v), 16); // 2 * 8 bytes
}

// ---------------------------------------------------------------------------
// GetByte — byte vector
// ---------------------------------------------------------------------------

TEST(BufferTests, GetByte_ByteVector_DirectRead) {
    std::vector<bytecs> v = {0x10, 0x20, 0x30};
    EXPECT_EQ(Buffer::GetByte(v, 0), 0x10);
    EXPECT_EQ(Buffer::GetByte(v, 1), 0x20);
    EXPECT_EQ(Buffer::GetByte(v, 2), 0x30);
}

// ---------------------------------------------------------------------------
// SetByte — extra cases
// ---------------------------------------------------------------------------

TEST(BufferTests, SetByte_LastByte) {
    std::vector<bytecs> v = {0, 0, 0};
    Buffer::SetByte(v, 2, 0x99);
    EXPECT_EQ(v[2], 0x99);
    EXPECT_EQ(v[0], 0);
    EXPECT_EQ(v[1], 0);
}

TEST(BufferTests, SetByte_AllBytes) {
    std::vector<bytecs> v(4, 0);
    for (intcs i = 0; i < 4; ++i)
        Buffer::SetByte(v, i, static_cast<bytecs>(i + 1));
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[3], 4);
}

// ---------------------------------------------------------------------------
// MemoryCopy
// ---------------------------------------------------------------------------

using SharpRuntime::longcs;

TEST(BufferTests, MemoryCopy_Basic) {
    uint8_t src[4] = {1, 2, 3, 4};
    uint8_t dst[4] = {0, 0, 0, 0};
    Buffer::MemoryCopy(src, dst, longcs{4}, longcs{4});
    EXPECT_EQ(dst[0], 1);
    EXPECT_EQ(dst[3], 4);
}

TEST(BufferTests, MemoryCopy_PartialCopy) {
    uint8_t src[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t dst[4] = {0x00, 0x00, 0x00, 0x00};
    Buffer::MemoryCopy(src, dst, longcs{4}, longcs{2});
    EXPECT_EQ(dst[0], 0xAA);
    EXPECT_EQ(dst[1], 0xBB);
    EXPECT_EQ(dst[2], 0x00); // untouched
}

TEST(BufferTests, MemoryCopy_ZeroBytes_NoOp) {
    uint8_t dst[2] = {0x11, 0x22};
    Buffer::MemoryCopy(dst, dst, longcs{2}, longcs{0});
    EXPECT_EQ(dst[0], 0x11);
}

TEST(BufferTests, MemoryCopy_OverlapSafe) {
    uint8_t buf[6] = {1, 2, 3, 4, 5, 6};
    // Shift [0..3] one byte right — overlapping move
    Buffer::MemoryCopy(buf, buf + 1, longcs{5}, longcs{4});
    EXPECT_EQ(buf[1], 1);
    EXPECT_EQ(buf[2], 2);
    EXPECT_EQ(buf[3], 3);
    EXPECT_EQ(buf[4], 4);
}

TEST(BufferTests, MemoryCopy_SizeExceeded_Throws) {
    uint8_t src[4] = {1, 2, 3, 4};
    uint8_t dst[2] = {0, 0};
    EXPECT_THROW(Buffer::MemoryCopy(src, dst, longcs{2}, longcs{4}),
                 System::ArgumentOutOfRangeException);
}

TEST(BufferTests, MemoryCopy_NegativeSourceBytesToCopy_Throws) {
    uint8_t src[4] = {1, 2, 3, 4};
    uint8_t dst[4] = {0, 0, 0, 0};
    EXPECT_THROW(Buffer::MemoryCopy(src, dst, longcs{4}, longcs{-1}),
                 System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// BlockCopy / GetByte / SetByte — bounds validation (regression: these
// previously did zero validation and reached memmove/pointer arithmetic
// directly with attacker/caller-controlled offsets, count, or index --
// confirmed exploitable via a standalone ASan repro before the fix).
// ---------------------------------------------------------------------------

TEST(BufferTests, BlockCopy_Vector_CountExceedsSrc_Throws) {
    std::vector<bytecs> src{1, 2, 3, 4};
    std::vector<bytecs> dst{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_THROW(Buffer::BlockCopy(src, 0, dst, 0, 1000), System::ArgumentException);
}

TEST(BufferTests, BlockCopy_Vector_CountExceedsDst_Throws) {
    std::vector<bytecs> src{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<bytecs> dst{0, 0};
    EXPECT_THROW(Buffer::BlockCopy(src, 0, dst, 0, 10), System::ArgumentException);
}

TEST(BufferTests, BlockCopy_Vector_NegativeSrcOffset_Throws) {
    std::vector<bytecs> src{1, 2, 3, 4};
    std::vector<bytecs> dst{0, 0, 0, 0};
    EXPECT_THROW(Buffer::BlockCopy(src, -1, dst, 0, 2), System::ArgumentOutOfRangeException);
}

TEST(BufferTests, BlockCopy_Vector_NegativeDstOffset_Throws) {
    std::vector<bytecs> src{1, 2, 3, 4};
    std::vector<bytecs> dst{0, 0, 0, 0};
    EXPECT_THROW(Buffer::BlockCopy(src, 0, dst, -1, 2), System::ArgumentOutOfRangeException);
}

TEST(BufferTests, BlockCopy_Vector_NegativeCount_Throws) {
    std::vector<bytecs> src{1, 2, 3, 4};
    std::vector<bytecs> dst{0, 0, 0, 0};
    EXPECT_THROW(Buffer::BlockCopy(src, 0, dst, 0, -1), System::ArgumentOutOfRangeException);
}

TEST(BufferTests, BlockCopy_TypedVector_CountExceedsBounds_Throws) {
    std::vector<int32_t> src{1, 2};
    std::vector<int32_t> dst{0, 0};
    // 8 bytes available per vector; ask for 100 bytes.
    EXPECT_THROW(Buffer::BlockCopy(src, 0, dst, 0, 100), System::ArgumentException);
}

TEST(BufferTests, GetByte_IndexOutOfRange_Throws) {
    std::vector<int32_t> v{0x01020304};
    EXPECT_THROW(Buffer::GetByte(v, 4), System::ArgumentOutOfRangeException);
    EXPECT_THROW(Buffer::GetByte(v, -1), System::ArgumentOutOfRangeException);
}

TEST(BufferTests, SetByte_IndexOutOfRange_Throws) {
    std::vector<int32_t> v{0x01020304};
    EXPECT_THROW(Buffer::SetByte(v, 4, 0xFF), System::ArgumentOutOfRangeException);
    EXPECT_THROW(Buffer::SetByte(v, -1, 0xFF), System::ArgumentOutOfRangeException);
}
