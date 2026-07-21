// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/Buffers/ArrayBufferWriter.hpp"
#include "System/InvalidOperationException.hpp"

using System::Buffers::ArrayBufferWriter;
using System::InvalidOperationException;

TEST(ArrayBufferWriterTest, DefaultCtor) {
    // .NET's ArrayBufferWriter() starts with an empty backing buffer (Capacity == 0);
    // DefaultInitialBufferSize is only applied lazily on the first growth (GetSpan/GetMemory).
    ArrayBufferWriter<int> w;
    EXPECT_EQ(w.getWrittenCountProperty(), 0);
    EXPECT_EQ(w.getCapacityProperty(), 0);
    w.GetSpan(1);
    EXPECT_GE(w.getCapacityProperty(), ArrayBufferWriter<int>::DefaultInitialBufferSize);
}

TEST(ArrayBufferWriterTest, CtorWithCapacity) {
    ArrayBufferWriter<int> w(128);
    EXPECT_GE(w.getCapacityProperty(), 128);
}

TEST(ArrayBufferWriterTest, CtorZeroCapacityThrows) {
    EXPECT_THROW(ArrayBufferWriter<int>(0), System::ArgumentException);
}

TEST(ArrayBufferWriterTest, AdvanceIncreasesWrittenCount) {
    ArrayBufferWriter<int> w;
    w.GetSpan(10);
    w.Advance(5);
    EXPECT_EQ(w.getWrittenCountProperty(), 5);
}

TEST(ArrayBufferWriterTest, FreeCapacityDecreases) {
    ArrayBufferWriter<int> w;
    w.GetSpan(4); // establishes the backing buffer
    int before = w.getFreeCapacityProperty();
    w.Advance(4);
    EXPECT_EQ(w.getFreeCapacityProperty(), before - 4);
}

TEST(ArrayBufferWriterTest, Clear) {
    ArrayBufferWriter<int> w;
    w.GetSpan(3);
    w.Advance(3);
    w.Clear();
    EXPECT_EQ(w.getWrittenCountProperty(), 0);
}

TEST(ArrayBufferWriterTest, WrittenMemorySize) {
    ArrayBufferWriter<char> w;
    auto sp = w.GetSpan(5);
    sp[0] = 'A'; sp[1] = 'B';
    w.Advance(2);
    EXPECT_EQ(w.getWrittenMemoryProperty().getLengthProperty(), 2);
}

TEST(ArrayBufferWriterTest, WrittenSpan_ReflectsWrittenData) {
    ArrayBufferWriter<char> w;
    auto sp = w.GetSpan(5);
    sp[0] = 'A'; sp[1] = 'B';
    w.Advance(2);
    auto written = w.getWrittenSpanProperty();
    EXPECT_EQ(written.getLengthProperty(), 2);
    EXPECT_EQ(written[0], 'A');
    EXPECT_EQ(written[1], 'B');
}

TEST(ArrayBufferWriterTest, GetMemory_ReturnsWritableMemory) {
    ArrayBufferWriter<int> w;
    auto mem = w.GetMemory(4);
    EXPECT_GE(mem.getLengthProperty(), 4);
}

TEST(ArrayBufferWriterTest, Clear_ZeroesBufferContent) {
    ArrayBufferWriter<int> w;
    auto sp = w.GetSpan(3);
    sp[0] = 42;
    w.Advance(1);
    w.Clear();
    // After Clear(), the previously-written region must be zeroed (matches .NET's
    // ArrayBufferWriter<T>.Clear(), which zeroes content in addition to resetting the index).
    auto sp2 = w.GetSpan(1);
    EXPECT_EQ(sp2[0], 0);
}

TEST(ArrayBufferWriterTest, ResetWrittenCount_DoesNotZeroBuffer) {
    ArrayBufferWriter<int> w;
    auto sp = w.GetSpan(3);
    sp[0] = 42;
    w.Advance(1);
    w.ResetWrittenCount();
    EXPECT_EQ(w.getWrittenCountProperty(), 0);
    // Content is left untouched by ResetWrittenCount (unlike Clear()).
    auto sp2 = w.GetSpan(1);
    EXPECT_EQ(sp2[0], 42);
}

TEST(ArrayBufferWriterTest, Advance_PastEnd_ThrowsInvalidOperationException) {
    ArrayBufferWriter<int> w(4);
    EXPECT_THROW(w.Advance(5), InvalidOperationException);
}

TEST(ArrayBufferWriterTest, Advance_NegativeCount_ThrowsInvalidArgument) {
    ArrayBufferWriter<int> w(4);
    EXPECT_THROW(w.Advance(-1), System::ArgumentException);
}
