// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <cstring>
#include <limits>
#include "System/ArgumentException.hpp"
#include "System/OutOfMemoryException.hpp"
#include "System/Runtime/InteropServices/NativeMemory.hpp"

using namespace System::Runtime::InteropServices;

TEST(NativeMemoryTests, Alloc_Free_RoundTrip) {
    void* p = NativeMemory::Alloc(64);
    ASSERT_NE(p, nullptr);
    std::memset(p, 0xAB, 64);
    NativeMemory::Free(p);
}

TEST(NativeMemoryTests, Alloc_ElementCountElementSize_ComputesByteCount) {
    void* p = NativeMemory::Alloc(size_t(10), size_t(4));
    ASSERT_NE(p, nullptr);
    std::memset(p, 0, 40);
    NativeMemory::Free(p);
}

TEST(NativeMemoryTests, Alloc_ElementCountElementSize_OverflowThrows) {
    size_t maxVal = (std::numeric_limits<size_t>::max)();
    EXPECT_THROW(NativeMemory::Alloc(maxVal, size_t(2)), System::OutOfMemoryException);
}

TEST(NativeMemoryTests, Alloc_ZeroByteCount_ReturnsValidNonNullPointer) {
    void* p = NativeMemory::Alloc(size_t(0));
    ASSERT_NE(p, nullptr);
    NativeMemory::Free(p);
}

TEST(NativeMemoryTests, AllocZeroed_ByteCount_ZeroesMemory) {
    auto* p = static_cast<uint8_t*>(NativeMemory::AllocZeroed(size_t(32)));
    ASSERT_NE(p, nullptr);
    for (size_t i = 0; i < 32; ++i) EXPECT_EQ(p[i], 0);
    NativeMemory::Free(p);
}

TEST(NativeMemoryTests, AllocZeroed_ElementCountElementSize_ZeroesMemory) {
    auto* p = static_cast<uint8_t*>(NativeMemory::AllocZeroed(size_t(8), size_t(4)));
    ASSERT_NE(p, nullptr);
    for (size_t i = 0; i < 32; ++i) EXPECT_EQ(p[i], 0);
    NativeMemory::Free(p);
}

TEST(NativeMemoryTests, Realloc_GrowsAndPreservesContent) {
    auto* p = static_cast<uint8_t*>(NativeMemory::Alloc(size_t(16)));
    std::memset(p, 0x7F, 16);
    p = static_cast<uint8_t*>(NativeMemory::Realloc(p, size_t(64)));
    ASSERT_NE(p, nullptr);
    for (size_t i = 0; i < 16; ++i) EXPECT_EQ(p[i], 0x7F);
    NativeMemory::Free(p);
}

TEST(NativeMemoryTests, Realloc_NullPtr_ActsAsAlloc) {
    void* p = NativeMemory::Realloc(nullptr, size_t(32));
    ASSERT_NE(p, nullptr);
    NativeMemory::Free(p);
}

TEST(NativeMemoryTests, Free_Null_IsNoOp) {
    NativeMemory::Free(nullptr);
}

TEST(NativeMemoryTests, AlignedAlloc_ReturnsAlignedPointer) {
    void* p = NativeMemory::AlignedAlloc(size_t(128), size_t(64));
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(p) % 64, 0u);
    NativeMemory::AlignedFree(p);
}

TEST(NativeMemoryTests, AlignedAlloc_NonPow2Alignment_Throws) {
    EXPECT_THROW(NativeMemory::AlignedAlloc(size_t(64), size_t(3)), System::ArgumentException);
}

TEST(NativeMemoryTests, AlignedAlloc_ZeroAlignment_Throws) {
    EXPECT_THROW(NativeMemory::AlignedAlloc(size_t(64), size_t(0)), System::ArgumentException);
}

TEST(NativeMemoryTests, AlignedFree_Null_IsNoOp) {
    NativeMemory::AlignedFree(nullptr);
}

TEST(NativeMemoryTests, AlignedRealloc_NullPtr_ActsAsAlignedAlloc) {
    void* p = NativeMemory::AlignedRealloc(nullptr, size_t(64), size_t(32));
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(p) % 32, 0u);
    NativeMemory::AlignedFree(p);
}

TEST(NativeMemoryTests, AlignedRealloc_GrowsAndPreservesContent_StaysAligned) {
    auto* p = static_cast<uint8_t*>(NativeMemory::AlignedAlloc(size_t(32), size_t(32)));
    std::memset(p, 0x5A, 32);
    p = static_cast<uint8_t*>(NativeMemory::AlignedRealloc(p, size_t(128), size_t(32)));
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(p) % 32, 0u);
    for (size_t i = 0; i < 32; ++i) EXPECT_EQ(p[i], 0x5A);
    NativeMemory::AlignedFree(p);
}

TEST(NativeMemoryTests, AlignedRealloc_NonPow2Alignment_Throws) {
    void* p = NativeMemory::AlignedAlloc(size_t(16), size_t(16));
    EXPECT_THROW(NativeMemory::AlignedRealloc(p, size_t(32), size_t(5)), System::ArgumentException);
    NativeMemory::AlignedFree(p);
}

TEST(NativeMemoryTests, Clear_ZeroesMemory) {
    auto* p = static_cast<uint8_t*>(NativeMemory::Alloc(size_t(16)));
    std::memset(p, 0xFF, 16);
    NativeMemory::Clear(p, 16);
    for (size_t i = 0; i < 16; ++i) EXPECT_EQ(p[i], 0);
    NativeMemory::Free(p);
}

TEST(NativeMemoryTests, Copy_CopiesBytes) {
    uint8_t src[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t dst[8] = {0};
    NativeMemory::Copy(src, dst, 8);
    EXPECT_EQ(std::memcmp(src, dst, 8), 0);
}

TEST(NativeMemoryTests, Copy_OverlappingRegions_HandledCorrectly) {
    uint8_t buf[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    NativeMemory::Copy(buf, buf + 4, 8);
    uint8_t expected[16] = {0, 1, 2, 3, 0, 1, 2, 3, 4, 5, 6, 7, 12, 13, 14, 15};
    EXPECT_EQ(std::memcmp(buf, expected, 16), 0);
}

TEST(NativeMemoryTests, Fill_SetsAllBytes) {
    uint8_t buf[16] = {0};
    NativeMemory::Fill(buf, 16, 0x42);
    for (uint8_t b : buf) EXPECT_EQ(b, 0x42);
}
