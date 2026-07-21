// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <vector>
#include "System/Buffer.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using System::Buffer;
using SharpRuntime::bytecs;
using SharpRuntime::intcs;
using SharpRuntime::longcs;
using SharpRuntime::ulongcs;

// ===========================================================================
// BlockCopy — generic vector<T> overload (treats elements as raw bytes)
// ===========================================================================

TEST(BufferBlockCopyGenericTests, BlockCopy_Int32Vector_CopiesRawBytes) {
    std::vector<int32_t> src = {1000, 2000};
    std::vector<int32_t> dst(2, 0);
    Buffer::BlockCopy(src, 0, dst, 0, 8);
    EXPECT_EQ(dst[0], 1000);
    EXPECT_EQ(dst[1], 2000);
}

TEST(BufferBlockCopyGenericTests, BlockCopy_Int32Vector_PartialBytes) {
    std::vector<int32_t> src = {0x12345678, 0x00BCDEF0};
    std::vector<int32_t> dst(2, 0);
    // Copy only the first 4 bytes (one int32)
    Buffer::BlockCopy(src, 0, dst, 0, 4);
    EXPECT_EQ(dst[0], 0x12345678);
    EXPECT_EQ(dst[1], 0);  // untouched
}

TEST(BufferBlockCopyGenericTests, BlockCopy_Int32Vector_DstByteOffset) {
    std::vector<int32_t> src = {0x000000FF, 0};
    std::vector<int32_t> dst(2, 0);
    // Copy 4 bytes starting at dst byte offset 4 (i.e., into dst[1])
    Buffer::BlockCopy(src, 0, dst, 4, 4);
    EXPECT_EQ(dst[0], 0);
    EXPECT_EQ(dst[1], 0x000000FF);
}

TEST(BufferBlockCopyGenericTests, BlockCopy_FloatVector_RoundTrip) {
    std::vector<float> src = {1.0f, 2.0f, 3.0f};
    std::vector<float> dst(3, 0.0f);
    Buffer::BlockCopy(src, 0, dst, 0, static_cast<intcs>(src.size() * sizeof(float)));
    EXPECT_FLOAT_EQ(dst[0], 1.0f);
    EXPECT_FLOAT_EQ(dst[1], 2.0f);
    EXPECT_FLOAT_EQ(dst[2], 3.0f);
}

TEST(BufferBlockCopyGenericTests, BlockCopy_ZeroCount_NoChange) {
    std::vector<int32_t> src = {99};
    std::vector<int32_t> dst = {0};
    Buffer::BlockCopy(src, 0, dst, 0, 0);
    EXPECT_EQ(dst[0], 0);
}

// ===========================================================================
// MemoryCopy — ulongcs overload
// ===========================================================================

TEST(BufferMemoryCopyUlongTests, MemoryCopy_UlongSize_Basic) {
    uint8_t src[4] = {10, 20, 30, 40};
    uint8_t dst[4] = {0, 0, 0, 0};
    Buffer::MemoryCopy(src, dst, ulongcs{4}, ulongcs{4});
    EXPECT_EQ(dst[0], 10);
    EXPECT_EQ(dst[3], 40);
}

TEST(BufferMemoryCopyUlongTests, MemoryCopy_UlongSize_PartialCopy) {
    uint8_t src[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t dst[4] = {0x00, 0x00, 0x00, 0x00};
    Buffer::MemoryCopy(src, dst, ulongcs{4}, ulongcs{2});
    EXPECT_EQ(dst[0], 0xAA);
    EXPECT_EQ(dst[1], 0xBB);
    EXPECT_EQ(dst[2], 0x00);
}

TEST(BufferMemoryCopyUlongTests, MemoryCopy_UlongSize_ZeroBytes_NoOp) {
    uint8_t dst[2] = {0x11, 0x22};
    Buffer::MemoryCopy(dst, dst, ulongcs{2}, ulongcs{0});
    EXPECT_EQ(dst[0], 0x11);
}

TEST(BufferMemoryCopyUlongTests, MemoryCopy_UlongSize_OverlapSafe) {
    uint8_t buf[5] = {1, 2, 3, 4, 5};
    Buffer::MemoryCopy(buf, buf + 1, ulongcs{4}, ulongcs{4});
    EXPECT_EQ(buf[1], 1);
    EXPECT_EQ(buf[2], 2);
    EXPECT_EQ(buf[3], 3);
    EXPECT_EQ(buf[4], 4);
}

TEST(BufferMemoryCopyUlongTests, MemoryCopy_UlongSize_ExceedsDst_Throws) {
    uint8_t src[4] = {1, 2, 3, 4};
    uint8_t dst[2] = {0, 0};
    EXPECT_THROW(Buffer::MemoryCopy(src, dst, ulongcs{2}, ulongcs{4}),
                 System::ArgumentOutOfRangeException);
}
