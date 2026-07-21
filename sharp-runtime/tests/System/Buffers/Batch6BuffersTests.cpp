// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <cstdint>
#include <vector>
#include "System/SequencePosition.hpp"
#include "System/Buffers/IBufferWriter.hpp"
#include "System/Buffers/ArrayBufferWriter.hpp"
#include "System/Buffers/MemoryPool.hpp"
#include "System/Buffers/ReadOnlySequence.hpp"
#include "System/Buffers/SequenceReader.hpp"
#include "System/Buffers/Binary/BinaryPrimitives.hpp"

using System::SequencePosition;
using System::Buffers::ArrayBufferWriter;
using System::Buffers::MemoryPool;
using System::Buffers::ReadOnlySequence;
using System::Buffers::SequenceReader;
using System::Buffers::Binary::BinaryPrimitives;

// ===========================================================================
// SequencePosition
// ===========================================================================

TEST(SequencePositionTests, DefaultConstructor_ZeroObjectAndInteger) {
    SequencePosition sp;
    EXPECT_EQ(sp.GetObject(), nullptr);
    EXPECT_EQ(sp.GetInteger(), 0);
}

TEST(SequencePositionTests, Constructor_StoresValues) {
    int dummy = 0;
    SequencePosition sp(&dummy, 42);
    EXPECT_EQ(sp.GetObject(), &dummy);
    EXPECT_EQ(sp.GetInteger(), 42);
}

TEST(SequencePositionTests, Equality_SameValues) {
    SequencePosition a(nullptr, 5);
    SequencePosition b(nullptr, 5);
    EXPECT_TRUE(a == b);
}

TEST(SequencePositionTests, Equality_DifferentInteger) {
    SequencePosition a(nullptr, 1);
    SequencePosition b(nullptr, 2);
    EXPECT_FALSE(a == b);
}

TEST(SequencePositionTests, Inequality_DifferentValues) {
    SequencePosition a(nullptr, 3);
    SequencePosition b(nullptr, 7);
    EXPECT_TRUE(a != b);
}

TEST(SequencePositionTests, Inequality_EqualValues_ReturnsFalse) {
    SequencePosition a(nullptr, 0);
    SequencePosition b(nullptr, 0);
    EXPECT_FALSE(a != b);
}

// ===========================================================================
// ArrayBufferWriter
// ===========================================================================

TEST(ArrayBufferWriterTests, DefaultConstructor_HasDefaultCapacity) {
    // .NET's ArrayBufferWriter() starts empty (Capacity == 0); DefaultInitialBufferSize
    // only kicks in lazily once the buffer needs to grow from empty.
    ArrayBufferWriter<uint8_t> writer;
    EXPECT_EQ(writer.getCapacityProperty(), 0);
    EXPECT_EQ(writer.getWrittenCountProperty(), 0);
    writer.GetSpan(1);
    EXPECT_GE(writer.getCapacityProperty(), ArrayBufferWriter<uint8_t>::DefaultInitialBufferSize);
}

TEST(ArrayBufferWriterTests, CustomCapacity_SetCorrectly) {
    ArrayBufferWriter<int> writer(64);
    EXPECT_EQ(writer.getCapacityProperty(), 64);
}

TEST(ArrayBufferWriterTests, InvalidCapacity_Throws) {
    EXPECT_THROW(ArrayBufferWriter<int>(0), System::ArgumentException);
    EXPECT_THROW(ArrayBufferWriter<int>(-1), System::ArgumentException);
}

TEST(ArrayBufferWriterTests, GetSpan_ReturnsWritableSpan) {
    ArrayBufferWriter<uint8_t> writer(8);
    auto span = writer.GetSpan(4);
    EXPECT_GE(span.getLengthProperty(), 4);
}

TEST(ArrayBufferWriterTests, Advance_IncreasesWrittenCount) {
    ArrayBufferWriter<uint8_t> writer(8);
    auto span = writer.GetSpan(3);
    span[0] = 1; span[1] = 2; span[2] = 3;
    writer.Advance(3);
    EXPECT_EQ(writer.getWrittenCountProperty(), 3);
}

TEST(ArrayBufferWriterTests, GetWrittenMemory_ReflectsWrittenData) {
    ArrayBufferWriter<uint8_t> writer(8);
    auto span = writer.GetSpan(2);
    span[0] = 0xAB; span[1] = 0xCD;
    writer.Advance(2);
    auto mem = writer.getWrittenMemoryProperty();
    EXPECT_EQ(mem.getLengthProperty(), 2);
    EXPECT_EQ(mem[0], 0xAB);
    EXPECT_EQ(mem[1], 0xCD);
}

TEST(ArrayBufferWriterTests, Clear_ResetsWrittenCount) {
    ArrayBufferWriter<int> writer(8);
    auto span = writer.GetSpan(2);
    span[0] = 1; span[1] = 2;
    writer.Advance(2);
    writer.Clear();
    EXPECT_EQ(writer.getWrittenCountProperty(), 0);
}

TEST(ArrayBufferWriterTests, FreeCapacity_DecreasesAfterAdvance) {
    ArrayBufferWriter<int> writer(8);
    int initial = writer.getFreeCapacityProperty();
    auto span = writer.GetSpan(3);
    (void)span;
    writer.Advance(3);
    EXPECT_EQ(writer.getFreeCapacityProperty(), initial - 3);
}

TEST(ArrayBufferWriterTests, GetSpan_GrowsBufferWhenNeeded) {
    ArrayBufferWriter<uint8_t> writer(4);
    auto span = writer.GetSpan(4);
    (void)span;
    writer.Advance(4);
    // Request more than remaining capacity — should grow
    auto span2 = writer.GetSpan(4);
    EXPECT_GE(span2.getLengthProperty(), 4);
}

TEST(ArrayBufferWriterTests, Advance_NegativeCount_Throws) {
    ArrayBufferWriter<uint8_t> writer(8);
    EXPECT_THROW(writer.Advance(-1), System::ArgumentException);
}

// ===========================================================================
// MemoryPool
// ===========================================================================

TEST(MemoryPoolTests, Shared_ReturnsSameInstance) {
    MemoryPool<uint8_t>& a = MemoryPool<uint8_t>::Shared();
    MemoryPool<uint8_t>& b = MemoryPool<uint8_t>::Shared();
    EXPECT_EQ(&a, &b);
}

TEST(MemoryPoolTests, Rent_ReturnsNonNull) {
    auto owner = MemoryPool<uint8_t>::Shared().Rent(16);
    EXPECT_NE(owner.get(), nullptr);
}

TEST(MemoryPoolTests, Rent_BufferHasAtLeastRequestedSize) {
    auto owner = MemoryPool<int>::Shared().Rent(32);
    EXPECT_GE(owner->getMemoryProperty().getLengthProperty(), 32);
}

TEST(MemoryPoolTests, Rent_DefaultSize_ReturnsUsableBuffer) {
    auto owner = MemoryPool<uint8_t>::Shared().Rent();
    EXPECT_GT(owner->getMemoryProperty().getLengthProperty(), 0);
}

TEST(MemoryPoolTests, Dispose_ClearsBuffer) {
    auto owner = MemoryPool<uint8_t>::Shared().Rent(8);
    owner->Dispose();
    EXPECT_EQ(owner->getMemoryProperty().getLengthProperty(), 0);
}

TEST(MemoryPoolTests, Rent_DefaultSize_AccountsForSizeofT) {
    // Regression: the default size used to be a flat 4096 *elements* regardless of T,
    // matching neither .NET's "~4096 bytes" default (ArrayMemoryPool.cs:
    // 1 + (4095 / sizeof(T))) nor any documented sharp-runtime behavior. For double
    // (8 bytes), .NET's default is 512 elements, not 4096.
    auto owner = MemoryPool<double>::Shared().Rent();
    auto len = owner->getMemoryProperty().getLengthProperty();
    EXPECT_EQ(len, 1 + (4095 / static_cast<decltype(len)>(sizeof(double))));
}

TEST(MemoryPoolTests, Rent_ExactlyMinusOne_UsesDefault) {
    auto owner = MemoryPool<uint8_t>::Shared().Rent(-1);
    EXPECT_EQ(owner->getMemoryProperty().getLengthProperty(), 1 + (4095 / 1));
}

TEST(MemoryPoolTests, Rent_ZeroSize_Allowed) {
    // Regression: 0 was previously (incorrectly) treated the same as "use default";
    // .NET only special-cases exactly -1.
    auto owner = MemoryPool<uint8_t>::Shared().Rent(0);
    EXPECT_EQ(owner->getMemoryProperty().getLengthProperty(), 0);
}

TEST(MemoryPoolTests, Rent_NegativeOtherThanMinusOne_Throws) {
    EXPECT_THROW(MemoryPool<uint8_t>::Shared().Rent(-2), System::ArgumentOutOfRangeException);
}

TEST(MemoryPoolTests, Rent_ExceedsMaxBufferSize_Throws) {
    EXPECT_THROW(MemoryPool<uint8_t>::Shared().Rent(MemoryPool<uint8_t>::MaxArrayLength + 1),
                 System::ArgumentOutOfRangeException);
}

TEST(MemoryPoolTests, MaxBufferSize_MatchesArrayMaxLength) {
    EXPECT_EQ(MemoryPool<uint8_t>::Shared().getMaxBufferSizeProperty(), 0x7FFFFFC7);
}

// ===========================================================================
// ReadOnlySequence
// ===========================================================================

TEST(ReadOnlySequenceTests, DefaultConstructor_IsEmpty) {
    ReadOnlySequence<uint8_t> seq;
    EXPECT_TRUE(seq.getIsEmptyProperty());
    EXPECT_EQ(seq.getLengthProperty(), 0LL);
}

TEST(ReadOnlySequenceTests, VectorConstructor_CorrectLength) {
    std::vector<int> v = {1, 2, 3, 4};
    ReadOnlySequence<int> seq(std::move(v));
    EXPECT_EQ(seq.getLengthProperty(), 4LL);
    EXPECT_FALSE(seq.getIsEmptyProperty());
}

TEST(ReadOnlySequenceTests, PointerConstructor_CorrectLength) {
    uint8_t data[] = {10, 20, 30};
    ReadOnlySequence<uint8_t> seq(data, 3);
    EXPECT_EQ(seq.getLengthProperty(), 3LL);
}

TEST(ReadOnlySequenceTests, Start_IntegerIsZero) {
    std::vector<int> v = {1, 2};
    ReadOnlySequence<int> seq(std::move(v));
    EXPECT_EQ(seq.getStartProperty().GetInteger(), 0);
}

TEST(ReadOnlySequenceTests, End_IntegerEqualsLength) {
    std::vector<int> v = {1, 2, 3};
    ReadOnlySequence<int> seq(std::move(v));
    EXPECT_EQ(seq.getEndProperty().GetInteger(), 3);
}

TEST(ReadOnlySequenceTests, First_ReturnsCorrectData) {
    uint8_t data[] = {5, 6, 7};
    ReadOnlySequence<uint8_t> seq(data, 3);
    auto mem = seq.First();
    EXPECT_EQ(mem.getLengthProperty(), 3);
    EXPECT_EQ(mem[0], 5);
    EXPECT_EQ(mem[2], 7);
}

TEST(ReadOnlySequenceTests, Slice_ByPositions_CorrectLength) {
    uint8_t data[] = {1, 2, 3, 4, 5};
    ReadOnlySequence<uint8_t> seq(data, 5);
    auto start = seq.GetPosition(1);
    auto end   = seq.GetPosition(4);
    auto sub   = seq.Slice(start, end);
    EXPECT_EQ(sub.getLengthProperty(), 3LL);
}

TEST(ReadOnlySequenceTests, Slice_FromStart_CorrectLength) {
    uint8_t data[] = {10, 20, 30, 40};
    ReadOnlySequence<uint8_t> seq(data, 4);
    auto pos = seq.GetPosition(2);
    auto sub = seq.Slice(pos);
    EXPECT_EQ(sub.getLengthProperty(), 2LL);
}

TEST(ReadOnlySequenceTests, GetPosition_OutOfRange_Throws) {
    uint8_t data[] = {1, 2};
    ReadOnlySequence<uint8_t> seq(data, 2);
    EXPECT_THROW(seq.GetPosition(10), System::ArgumentOutOfRangeException);
}

// ===========================================================================
// SequenceReader
// ===========================================================================

TEST(SequenceReaderTests, InitialState_ConsumedIsZero) {
    uint8_t data[] = {1, 2, 3};
    ReadOnlySequence<uint8_t> seq(data, 3);
    SequenceReader<uint8_t> reader(seq);
    EXPECT_EQ(reader.getConsumedProperty(), 0LL);
    EXPECT_EQ(reader.getRemainingProperty(), 3LL);
}

TEST(SequenceReaderTests, TryRead_ReturnsElements) {
    uint8_t data[] = {10, 20, 30};
    ReadOnlySequence<uint8_t> seq(data, 3);
    SequenceReader<uint8_t> reader(seq);
    uint8_t v = 0;
    EXPECT_TRUE(reader.TryRead(v)); EXPECT_EQ(v, 10);
    EXPECT_TRUE(reader.TryRead(v)); EXPECT_EQ(v, 20);
    EXPECT_TRUE(reader.TryRead(v)); EXPECT_EQ(v, 30);
    EXPECT_FALSE(reader.TryRead(v));
}

TEST(SequenceReaderTests, End_TrueWhenExhausted) {
    uint8_t data[] = {1};
    ReadOnlySequence<uint8_t> seq(data, 1);
    SequenceReader<uint8_t> reader(seq);
    EXPECT_FALSE(reader.getEndProperty());
    uint8_t v = 0;
    reader.TryRead(v);
    EXPECT_TRUE(reader.getEndProperty());
}

TEST(SequenceReaderTests, IsNext_MatchesWithoutAdvancing) {
    uint8_t data[] = {42, 99};
    ReadOnlySequence<uint8_t> seq(data, 2);
    SequenceReader<uint8_t> reader(seq);
    EXPECT_TRUE(reader.IsNext(42));
    EXPECT_EQ(reader.getConsumedProperty(), 0LL);
}

TEST(SequenceReaderTests, Advance_SkipsElements) {
    uint8_t data[] = {1, 2, 3, 4};
    ReadOnlySequence<uint8_t> seq(data, 4);
    SequenceReader<uint8_t> reader(seq);
    reader.Advance(2);
    EXPECT_EQ(reader.getConsumedProperty(), 2LL);
    uint8_t v = 0;
    reader.TryRead(v);
    EXPECT_EQ(v, 3);
}

// Regression test for a wave-3 audit finding: Advance() threw std::out_of_range (an unrelated
// std:: exception type invisible to code catching System::Exception&) instead of
// System::ArgumentOutOfRangeException, which is what real .NET's SequenceReader<T>.Advance
// throws when count is negative or exceeds the remaining elements.
TEST(SequenceReaderTests, Advance_PastRemainingElements_Throws) {
    uint8_t data[] = {1, 2, 3};
    ReadOnlySequence<uint8_t> seq(data, 3);
    SequenceReader<uint8_t> reader(seq);
    EXPECT_THROW(reader.Advance(4), System::ArgumentOutOfRangeException);
}

TEST(SequenceReaderTests, TryAdvancePast_MatchingElement) {
    uint8_t data[] = {7, 8};
    ReadOnlySequence<uint8_t> seq(data, 2);
    SequenceReader<uint8_t> reader(seq);
    EXPECT_TRUE(reader.TryAdvancePast(7));
    EXPECT_EQ(reader.getConsumedProperty(), 1LL);
}

TEST(SequenceReaderTests, TryAdvancePast_NoMatchDoesNotAdvance) {
    uint8_t data[] = {7, 8};
    ReadOnlySequence<uint8_t> seq(data, 2);
    SequenceReader<uint8_t> reader(seq);
    EXPECT_FALSE(reader.TryAdvancePast(99));
    EXPECT_EQ(reader.getConsumedProperty(), 0LL);
}

// Regression tests: Rewind() previously took no argument and unconditionally reset the
// reader to the absolute start of the sequence -- a completely different API shape from real
// .NET's SequenceReader<T>.Rewind(long count), which moves the reader back by a *relative*
// count and throws ArgumentOutOfRangeException for a negative count or one exceeding Consumed.
// Ported C# code calling reader.Rewind(n) would not even compile against the old signature.
TEST(SequenceReaderTests, Rewind_MovesBackByCount) {
    uint8_t data[] = {1, 2, 3, 4};
    ReadOnlySequence<uint8_t> seq(data, 4);
    SequenceReader<uint8_t> reader(seq);
    reader.Advance(4);
    reader.Rewind(1);
    EXPECT_EQ(reader.getConsumedProperty(), 3LL);
    uint8_t v = 0;
    reader.TryRead(v);
    EXPECT_EQ(v, 4);
}

TEST(SequenceReaderTests, Rewind_ByConsumedCount_ResetsToStart) {
    uint8_t data[] = {1, 2, 3};
    ReadOnlySequence<uint8_t> seq(data, 3);
    SequenceReader<uint8_t> reader(seq);
    reader.Advance(3);
    reader.Rewind(reader.getConsumedProperty());
    EXPECT_EQ(reader.getConsumedProperty(), 0LL);
}

TEST(SequenceReaderTests, Rewind_Zero_IsNoOp) {
    uint8_t data[] = {1, 2, 3};
    ReadOnlySequence<uint8_t> seq(data, 3);
    SequenceReader<uint8_t> reader(seq);
    reader.Advance(2);
    reader.Rewind(0);
    EXPECT_EQ(reader.getConsumedProperty(), 2LL);
}

TEST(SequenceReaderTests, Rewind_NegativeCount_Throws) {
    uint8_t data[] = {1, 2, 3};
    ReadOnlySequence<uint8_t> seq(data, 3);
    SequenceReader<uint8_t> reader(seq);
    reader.Advance(2);
    EXPECT_THROW(reader.Rewind(-1), System::ArgumentOutOfRangeException);
}

TEST(SequenceReaderTests, Rewind_MoreThanConsumed_Throws) {
    uint8_t data[] = {1, 2, 3};
    ReadOnlySequence<uint8_t> seq(data, 3);
    SequenceReader<uint8_t> reader(seq);
    reader.Advance(2);
    EXPECT_THROW(reader.Rewind(3), System::ArgumentOutOfRangeException);
}

// ===========================================================================
// BinaryPrimitives — little-endian
// ===========================================================================

TEST(BinaryPrimitivesTests, ReadInt16LittleEndian_CorrectValue) {
    uint8_t data[] = {0x01, 0x00};
    auto span = System::ReadOnlySpan<uint8_t>(data, 2);
    EXPECT_EQ(BinaryPrimitives::ReadInt16LittleEndian(span), int16_t(1));
}

TEST(BinaryPrimitivesTests, ReadUInt16LittleEndian_CorrectValue) {
    uint8_t data[] = {0xFF, 0x00};
    auto span = System::ReadOnlySpan<uint8_t>(data, 2);
    EXPECT_EQ(BinaryPrimitives::ReadUInt16LittleEndian(span), uint16_t(0x00FF));
}

TEST(BinaryPrimitivesTests, ReadInt32LittleEndian_CorrectValue) {
    uint8_t data[] = {0x78, 0x56, 0x34, 0x12};
    auto span = System::ReadOnlySpan<uint8_t>(data, 4);
    EXPECT_EQ(BinaryPrimitives::ReadInt32LittleEndian(span), int32_t(0x12345678));
}

TEST(BinaryPrimitivesTests, ReadUInt32LittleEndian_CorrectValue) {
    uint8_t data[] = {0xEF, 0xCD, 0xAB, 0x89};
    auto span = System::ReadOnlySpan<uint8_t>(data, 4);
    EXPECT_EQ(BinaryPrimitives::ReadUInt32LittleEndian(span), uint32_t(0x89ABCDEFu));
}

TEST(BinaryPrimitivesTests, ReadInt64LittleEndian_CorrectValue) {
    uint8_t data[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    auto span = System::ReadOnlySpan<uint8_t>(data, 8);
    EXPECT_EQ(BinaryPrimitives::ReadInt64LittleEndian(span), int64_t(1));
}

TEST(BinaryPrimitivesTests, ReadUInt64LittleEndian_CorrectValue) {
    uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    auto span = System::ReadOnlySpan<uint8_t>(data, 8);
    EXPECT_EQ(BinaryPrimitives::ReadUInt64LittleEndian(span), UINT64_MAX);
}

TEST(BinaryPrimitivesTests, WriteInt32LittleEndian_ProducesCorrectBytes) {
    uint8_t buf[4] = {};
    auto span = System::Span<uint8_t>(buf, 4);
    BinaryPrimitives::WriteInt32LittleEndian(span, 0x12345678);
    EXPECT_EQ(buf[0], 0x78);
    EXPECT_EQ(buf[1], 0x56);
    EXPECT_EQ(buf[2], 0x34);
    EXPECT_EQ(buf[3], 0x12);
}

TEST(BinaryPrimitivesTests, WriteInt16LittleEndian_RoundTrip) {
    uint8_t buf[2] = {};
    auto wspan = System::Span<uint8_t>(buf, 2);
    BinaryPrimitives::WriteInt16LittleEndian(wspan, int16_t(0x0102));
    auto rspan = System::ReadOnlySpan<uint8_t>(buf, 2);
    EXPECT_EQ(BinaryPrimitives::ReadInt16LittleEndian(rspan), int16_t(0x0102));
}

// ===========================================================================
// BinaryPrimitives — big-endian
// ===========================================================================

TEST(BinaryPrimitivesTests, ReadInt32BigEndian_CorrectValue) {
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
    auto span = System::ReadOnlySpan<uint8_t>(data, 4);
    EXPECT_EQ(BinaryPrimitives::ReadInt32BigEndian(span), int32_t(0x12345678));
}

TEST(BinaryPrimitivesTests, ReadUInt16BigEndian_CorrectValue) {
    uint8_t data[] = {0x00, 0xFF};
    auto span = System::ReadOnlySpan<uint8_t>(data, 2);
    EXPECT_EQ(BinaryPrimitives::ReadUInt16BigEndian(span), uint16_t(0x00FF));
}

TEST(BinaryPrimitivesTests, WriteInt32BigEndian_ProducesCorrectBytes) {
    uint8_t buf[4] = {};
    auto span = System::Span<uint8_t>(buf, 4);
    BinaryPrimitives::WriteInt32BigEndian(span, 0x12345678);
    EXPECT_EQ(buf[0], 0x12);
    EXPECT_EQ(buf[1], 0x34);
    EXPECT_EQ(buf[2], 0x56);
    EXPECT_EQ(buf[3], 0x78);
}

TEST(BinaryPrimitivesTests, WriteUInt64BigEndian_RoundTrip) {
    uint8_t buf[8] = {};
    auto wspan = System::Span<uint8_t>(buf, 8);
    BinaryPrimitives::WriteUInt64BigEndian(wspan, uint64_t(0x0102030405060708ULL));
    auto rspan = System::ReadOnlySpan<uint8_t>(buf, 8);
    EXPECT_EQ(BinaryPrimitives::ReadUInt64BigEndian(rspan), uint64_t(0x0102030405060708ULL));
}

// Regression tests for a wave-3 audit finding: Read*/Write* threw std::out_of_range (an
// unrelated std:: exception type invisible to code catching System::Exception&) instead of
// System::ArgumentOutOfRangeException, which is what real .NET's BinaryPrimitives Read*/Write*
// methods throw when the source/destination span is too small.
TEST(BinaryPrimitivesTests, ReadInt32LittleEndian_SpanTooSmall_Throws) {
    uint8_t data[] = {0x01, 0x02};
    auto span = System::ReadOnlySpan<uint8_t>(data, 2);
    EXPECT_THROW(BinaryPrimitives::ReadInt32LittleEndian(span), System::ArgumentOutOfRangeException);
}

TEST(BinaryPrimitivesTests, WriteInt64BigEndian_SpanTooSmall_Throws) {
    uint8_t buf[4] = {};
    auto span = System::Span<uint8_t>(buf, 4);
    EXPECT_THROW(BinaryPrimitives::WriteInt64BigEndian(span, 0LL), System::ArgumentOutOfRangeException);
}
