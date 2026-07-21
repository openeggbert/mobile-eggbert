// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <limits>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Buffers/Binary/BinaryPrimitives.hpp"
#include "System/Buffers/SequenceReader.hpp"
#include "System/Buffers/ReadOnlySequenceSegment.hpp"
#include "System/Buffers/ReadOnlySequence.hpp"
#include "System/Buffers/BuffersExtensions.hpp"
#include "System/Buffers/ArrayBufferWriter.hpp"
#include "System/Buffers/Text/Base64.hpp"
#include "System/Buffers/Text/Base64Url.hpp"

using BP = System::Buffers::Binary::BinaryPrimitives;
using OS = System::Buffers::OperationStatus;

// ===========================================================================
// BinaryPrimitives — ReverseEndianness
// ===========================================================================

TEST(BinaryPrimitivesReverseTests, ReverseEndianness_Sbyte_NoOp) {
    EXPECT_EQ(BP::ReverseEndianness(int8_t(42)), int8_t(42));
}
TEST(BinaryPrimitivesReverseTests, ReverseEndianness_Byte_NoOp) {
    EXPECT_EQ(BP::ReverseEndianness(uint8_t(0xAB)), uint8_t(0xAB));
}
TEST(BinaryPrimitivesReverseTests, ReverseEndianness_Int16) {
    EXPECT_EQ(BP::ReverseEndianness(int16_t(0x0102)), int16_t(0x0201));
}
TEST(BinaryPrimitivesReverseTests, ReverseEndianness_UInt16) {
    EXPECT_EQ(BP::ReverseEndianness(uint16_t(0x0102u)), uint16_t(0x0201u));
}
TEST(BinaryPrimitivesReverseTests, ReverseEndianness_Int32) {
    EXPECT_EQ(BP::ReverseEndianness(int32_t(0x01020304)), int32_t(0x04030201));
}
TEST(BinaryPrimitivesReverseTests, ReverseEndianness_UInt32) {
    EXPECT_EQ(BP::ReverseEndianness(uint32_t(0x01020304u)), uint32_t(0x04030201u));
}
TEST(BinaryPrimitivesReverseTests, ReverseEndianness_Int64) {
    EXPECT_EQ(BP::ReverseEndianness(int64_t(0x0102030405060708LL)),
              int64_t(0x0807060504030201LL));
}
TEST(BinaryPrimitivesReverseTests, ReverseEndianness_UInt64) {
    EXPECT_EQ(BP::ReverseEndianness(uint64_t(0x0102030405060708ULL)),
              uint64_t(0x0807060504030201ULL));
}
TEST(BinaryPrimitivesReverseTests, ReverseEndianness_Twice_IsIdentity) {
    uint32_t v = 0xDEADBEEFu;
    EXPECT_EQ(BP::ReverseEndianness(BP::ReverseEndianness(v)), v);
}

// ===========================================================================
// SequenceReader — new methods
// ===========================================================================

TEST(SequenceReaderNewTests, GetLength) {
    std::vector<uint8_t> data{1, 2, 3, 4, 5};
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    EXPECT_EQ(reader.getLengthProperty(), 5LL);
}

TEST(SequenceReaderNewTests, GetSequence) {
    std::vector<uint8_t> data{10, 20};
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    EXPECT_EQ(reader.getSequenceProperty().getLengthProperty(), 2LL);
}

TEST(SequenceReaderNewTests, TryPeek_Success) {
    std::vector<uint8_t> data{42};
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    uint8_t val = 0;
    EXPECT_TRUE(reader.TryPeek(val));
    EXPECT_EQ(val, 42u);
    // Peek should not consume
    EXPECT_EQ(reader.getRemainingProperty(), 1LL);
}

TEST(SequenceReaderNewTests, TryPeek_AtEnd_ReturnsFalse) {
    std::vector<uint8_t> data{};
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    uint8_t val = 0;
    EXPECT_FALSE(reader.TryPeek(val));
}

TEST(SequenceReaderNewTests, TryReadTo_FindsDelimiter) {
    std::vector<uint8_t> data{'H','e','l','l','o','\n','W','o','r','l','d'};
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    std::vector<uint8_t> result;
    EXPECT_TRUE(reader.TryReadTo(result, uint8_t('\n'), true));
    EXPECT_EQ(result.size(), 5u);  // "Hello"
    EXPECT_EQ(reader.getRemainingProperty(), 5LL);  // "World"
}

TEST(SequenceReaderNewTests, TryReadTo_DelimiterNotFound) {
    std::vector<uint8_t> data{1, 2, 3};
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    std::vector<uint8_t> result;
    EXPECT_FALSE(reader.TryReadTo(result, uint8_t(9)));
    // Position should be restored
    EXPECT_EQ(reader.getConsumedProperty(), 0LL);
}

TEST(SequenceReaderNewTests, AdvancePast_SkipsMatching) {
    std::vector<uint8_t> data{0, 0, 0, 1, 2};
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    int skipped = reader.AdvancePast(uint8_t(0));
    EXPECT_EQ(skipped, 3);
    EXPECT_EQ(reader.getConsumedProperty(), 3LL);
}

TEST(SequenceReaderNewTests, IsNext_Vector_Match) {
    std::vector<uint8_t> data{1, 2, 3, 4};
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    std::vector<uint8_t> needle{1, 2};
    EXPECT_TRUE(reader.IsNext(needle));
    EXPECT_EQ(reader.getConsumedProperty(), 0LL);  // not consumed
}

TEST(SequenceReaderNewTests, IsNext_Vector_AdvancePast) {
    std::vector<uint8_t> data{1, 2, 3};
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::SequenceReader<uint8_t> reader(seq);
    std::vector<uint8_t> needle{1, 2};
    EXPECT_TRUE(reader.IsNext(needle, true));
    EXPECT_EQ(reader.getConsumedProperty(), 2LL);
}

// ===========================================================================
// ReadOnlySequenceSegment
// ===========================================================================

namespace {
struct IntSegment : System::Buffers::ReadOnlySequenceSegment<int> {
    std::vector<int> data_;
    explicit IntSegment(std::vector<int> d) : data_(std::move(d)) {
        setMemoryProperty(System::ReadOnlyMemory<int>(data_.data(), static_cast<int>(data_.size())));
    }
    void linkTo(IntSegment* next, long long runningIndex) {
        setNextProperty(next);
        if (next) next->setRunningIndexProperty(runningIndex);
    }
};
} // anonymous namespace

TEST(ReadOnlySequenceSegmentTests, GetMemory_NonEmpty) {
    IntSegment seg({10, 20, 30});
    EXPECT_EQ(seg.getMemoryProperty().getLengthProperty(), 3);
}

TEST(ReadOnlySequenceSegmentTests, GetNext_NullByDefault) {
    IntSegment seg({1, 2});
    EXPECT_EQ(seg.getNextProperty(), nullptr);
}

TEST(ReadOnlySequenceSegmentTests, RunningIndex_Default_Zero) {
    IntSegment seg({1});
    EXPECT_EQ(seg.getRunningIndexProperty(), 0LL);
}

TEST(ReadOnlySequenceSegmentTests, Link_SetsNext) {
    IntSegment a({1, 2});
    IntSegment b({3, 4});
    a.linkTo(&b, 2LL);
    EXPECT_EQ(a.getNextProperty(), &b);
    EXPECT_EQ(b.getRunningIndexProperty(), 2LL);
}

// ===========================================================================
// ReadOnlySequence — new methods
// ===========================================================================

TEST(ReadOnlySequenceNewTests, IsSingleSegment_AlwaysTrue) {
    std::vector<int> data{1, 2, 3};
    System::Buffers::ReadOnlySequence<int> seq(data);
    EXPECT_TRUE(seq.getIsSingleSegmentProperty());
}

TEST(ReadOnlySequenceNewTests, ToArray_ReturnsAllElements) {
    std::vector<int> data{10, 20, 30};
    System::Buffers::ReadOnlySequence<int> seq(data);
    auto arr = seq.ToArray();
    EXPECT_EQ(arr.size(), 3u);
    EXPECT_EQ(arr[0], 10);
    EXPECT_EQ(arr[2], 30);
}

TEST(ReadOnlySequenceNewTests, CopyTo_CopiesData) {
    std::vector<uint8_t> data{1, 2, 3, 4};
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    std::vector<uint8_t> dst(4, 0);
    System::Span<uint8_t> span(dst.data(), 4);
    seq.CopyTo(span);
    EXPECT_EQ(dst[0], 1u);
    EXPECT_EQ(dst[3], 4u);
}

TEST(ReadOnlySequenceNewTests, Empty_IsEmpty) {
    auto seq = System::Buffers::ReadOnlySequence<int>::getEmpty();
    EXPECT_TRUE(seq.getIsEmptyProperty());
    EXPECT_EQ(seq.getLengthProperty(), 0LL);
}

TEST(ReadOnlySequenceNewTests, Slice_LongLong_ReturnsSubrange) {
    std::vector<int> data{10, 20, 30, 40, 50};
    System::Buffers::ReadOnlySequence<int> seq(data);
    auto sliced = seq.Slice(1LL, 3LL);
    EXPECT_EQ(sliced.getLengthProperty(), 3LL);
    auto arr = sliced.ToArray();
    EXPECT_EQ(arr[0], 20);
    EXPECT_EQ(arr[2], 40);
}

TEST(ReadOnlySequenceNewTests, Slice_IntInt_ReturnsSubrange) {
    std::vector<int> data{10, 20, 30, 40};
    System::Buffers::ReadOnlySequence<int> seq(data);
    auto sliced = seq.Slice(1, 2);
    auto arr = sliced.ToArray();
    EXPECT_EQ(arr.size(), 2u);
    EXPECT_EQ(arr[0], 20);
    EXPECT_EQ(arr[1], 30);
}

TEST(ReadOnlySequenceNewTests, Slice_LongOnly_ToEnd) {
    std::vector<int> data{1, 2, 3, 4};
    System::Buffers::ReadOnlySequence<int> seq(data);
    auto sliced = seq.Slice(2LL);
    EXPECT_EQ(sliced.getLengthProperty(), 2LL);
}

TEST(ReadOnlySequenceNewTests, GetPosition_WithOrigin) {
    std::vector<int> data{1, 2, 3, 4, 5};
    System::Buffers::ReadOnlySequence<int> seq(data);
    auto mid = seq.GetPosition(2);
    auto fromMid = seq.GetPosition(1, mid);
    auto sliced = seq.Slice(fromMid);
    EXPECT_EQ(sliced.getLengthProperty(), 2LL); // elements at index 3,4
}

TEST(ReadOnlySequenceNewTests, GetPosition_NegativeOffset_Throws) {
    std::vector<int> data{1, 2, 3};
    System::Buffers::ReadOnlySequence<int> seq(data);
    EXPECT_THROW(seq.GetPosition(-1), System::ArgumentOutOfRangeException);
}

// Regression test for ticket 305: GetPosition narrowed `offset` (longcs) to intcs BEFORE
// bounds-checking it, so a huge offset (e.g. 2^32 + 2) silently truncated to a small value
// (2) that happened to land inside the valid range, instead of throwing
// ArgumentOutOfRangeException -- confirmed via a standalone repro returning a bogus position
// with no throw at all.
TEST(ReadOnlySequenceNewTests, GetPosition_HugeOffsetThatWouldTruncateInRange_Throws) {
    std::vector<int> data{1, 2, 3, 4, 5};
    System::Buffers::ReadOnlySequence<int> seq(data);
    SharpRuntime::longcs hugeOffset = (1LL << 32) + 2; // truncates to 2 as intcs
    EXPECT_THROW(seq.GetPosition(hugeOffset), System::ArgumentOutOfRangeException);
}

TEST(ReadOnlySequenceNewTests, GetPosition_MaxLongOffset_ThrowsInsteadOfOverflowing) {
    std::vector<int> data{1, 2, 3};
    System::Buffers::ReadOnlySequence<int> seq(data);
    EXPECT_THROW(seq.GetPosition(std::numeric_limits<SharpRuntime::longcs>::max()),
                 System::ArgumentOutOfRangeException);
}

// Regression test for ticket 305: Slice(longcs start, longcs length) computed `start + length`
// directly before validating either argument, which is signed-integer-overflow UB (confirmed
// via UBSan) for a huge start near LONGCS_MAX combined with any positive length.
TEST(ReadOnlySequenceNewTests, Slice_LongLong_NearMaxStartDoesNotOverflow_ThrowsOutOfRange) {
    std::vector<int> data{1, 2, 3};
    System::Buffers::ReadOnlySequence<int> seq(data);
    SharpRuntime::longcs nearMaxStart = std::numeric_limits<SharpRuntime::longcs>::max() - 2;
    EXPECT_THROW(seq.Slice(nearMaxStart, 10), System::ArgumentOutOfRangeException);
}

TEST(ReadOnlySequenceNewTests, TryGet_ReturnsDataThenFalse) {
    std::vector<int> data{7, 8, 9};
    System::Buffers::ReadOnlySequence<int> seq(data);
    System::SequencePosition pos = seq.getStartProperty();
    System::ReadOnlyMemory<int> mem;
    bool first = seq.TryGet(pos, mem, true);
    EXPECT_TRUE(first);
    EXPECT_EQ(mem.getLengthProperty(), 3);
    bool second = seq.TryGet(pos, mem, true);
    EXPECT_FALSE(second);
}

// ===========================================================================
// BuffersExtensions
// ===========================================================================

TEST(BuffersExtensionsTests, PositionOf_Found) {
    std::vector<int> data{5, 10, 15, 20};
    System::Buffers::ReadOnlySequence<int> seq(data);
    auto pos = System::Buffers::BuffersExtensions::PositionOf(seq, 15);
    EXPECT_TRUE(pos.has_value());
}

TEST(BuffersExtensionsTests, PositionOf_NotFound) {
    std::vector<int> data{1, 2, 3};
    System::Buffers::ReadOnlySequence<int> seq(data);
    auto pos = System::Buffers::BuffersExtensions::PositionOf(seq, 99);
    EXPECT_FALSE(pos.has_value());
}

TEST(BuffersExtensionsTests, ToArray_Extension) {
    std::vector<uint8_t> data{7, 8, 9};
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    auto arr = System::Buffers::BuffersExtensions::ToArray(seq);
    EXPECT_EQ(arr.size(), 3u);
    EXPECT_EQ(arr[0], 7u);
}

TEST(BuffersExtensionsTests, CopyTo_Extension) {
    std::vector<uint8_t> data{100, 200};
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    std::vector<uint8_t> dst(2, 0);
    System::Span<uint8_t> span(dst.data(), 2);
    System::Buffers::BuffersExtensions::CopyTo(seq, span);
    EXPECT_EQ(dst[0], 100u);
    EXPECT_EQ(dst[1], 200u);
}

TEST(BuffersExtensionsTests, Write_ReadOnlySpan_FastPath) {
    // Matches .NET's real BuffersExtensions.Write<T>(IBufferWriter<T>, ReadOnlySpan<T>).
    System::Buffers::ArrayBufferWriter<uint8_t> writer;
    std::vector<uint8_t> data{1, 2, 3};
    System::Buffers::BuffersExtensions::Write(
        writer, System::ReadOnlySpan<uint8_t>(data.data(), static_cast<int>(data.size())));
    EXPECT_EQ(writer.getWrittenCountProperty(), 3);
    auto written = writer.getWrittenSpanProperty();
    EXPECT_EQ(written[0], 1u);
    EXPECT_EQ(written[2], 3u);
}

TEST(BuffersExtensionsTests, Write_ReadOnlySpan_MultiSegment) {
    // Force the writer's buffer to be smaller than the input so Write must loop.
    System::Buffers::ArrayBufferWriter<uint8_t> writer(2);
    std::vector<uint8_t> data{1, 2, 3, 4, 5};
    System::Buffers::BuffersExtensions::Write(
        writer, System::ReadOnlySpan<uint8_t>(data.data(), static_cast<int>(data.size())));
    EXPECT_EQ(writer.getWrittenCountProperty(), 5);
    auto written = writer.getWrittenSpanProperty();
    for (int i = 0; i < 5; ++i) EXPECT_EQ(written[i], data[static_cast<size_t>(i)]);
}

TEST(BuffersExtensionsTests, Write_ReadOnlySequence_Convenience) {
    System::Buffers::ArrayBufferWriter<uint8_t> writer;
    std::vector<uint8_t> data{9, 8, 7};
    System::Buffers::ReadOnlySequence<uint8_t> seq(data);
    System::Buffers::BuffersExtensions::Write(writer, seq);
    EXPECT_EQ(writer.getWrittenCountProperty(), 3);
    auto written = writer.getWrittenSpanProperty();
    EXPECT_EQ(written[0], 9u);
    EXPECT_EQ(written[2], 7u);
}

// ===========================================================================
// Base64 — Encode / Decode / Validate
// ===========================================================================

TEST(Base64Tests, GetMaxEncodedLength_Zero) {
    EXPECT_EQ(System::Buffers::Text::Base64::GetMaxEncodedToUtf8Length(0), 0);
}
TEST(Base64Tests, GetMaxEncodedLength_3bytes_is4) {
    EXPECT_EQ(System::Buffers::Text::Base64::GetMaxEncodedToUtf8Length(3), 4);
}
TEST(Base64Tests, GetMaxEncodedLength_4bytes_is8) {
    EXPECT_EQ(System::Buffers::Text::Base64::GetMaxEncodedToUtf8Length(4), 8);
}

TEST(Base64Tests, EncodeToString_Empty) {
    std::vector<uint8_t> data{};
    EXPECT_EQ(System::Buffers::Text::Base64::EncodeToString(data), "");
}

TEST(Base64Tests, EncodeToString_Man) {
    // "Man" → "TWFu"
    std::vector<uint8_t> data{'M', 'a', 'n'};
    EXPECT_EQ(System::Buffers::Text::Base64::EncodeToString(data), "TWFu");
}

TEST(Base64Tests, EncodeToString_M) {
    // "M" → "TQ=="
    std::vector<uint8_t> data{'M'};
    EXPECT_EQ(System::Buffers::Text::Base64::EncodeToString(data), "TQ==");
}

TEST(Base64Tests, EncodeToString_Ma) {
    // "Ma" → "TWE="
    std::vector<uint8_t> data{'M', 'a'};
    EXPECT_EQ(System::Buffers::Text::Base64::EncodeToString(data), "TWE=");
}

TEST(Base64Tests, Decode_Man) {
    const char* b64 = "TWFu";
    std::vector<uint8_t> src(b64, b64 + 4);
    std::vector<uint8_t> dst(3);
    int consumed = 0, written = 0;
    auto status = System::Buffers::Text::Base64::DecodeFromUtf8(
        System::ReadOnlySpan<uint8_t>(src.data(), 4),
        System::Span<uint8_t>(dst.data(), 3),
        consumed, written);
    EXPECT_EQ(status, OS::Done);
    EXPECT_EQ(written, 3);
    EXPECT_EQ(dst[0], uint8_t('M'));
    EXPECT_EQ(dst[1], uint8_t('a'));
    EXPECT_EQ(dst[2], uint8_t('n'));
}

TEST(Base64Tests, Decode_WithPadding_TQ) {
    const char* b64 = "TQ==";
    std::vector<uint8_t> src(b64, b64 + 4);
    std::vector<uint8_t> dst(3);
    int consumed = 0, written = 0;
    auto status = System::Buffers::Text::Base64::DecodeFromUtf8(
        System::ReadOnlySpan<uint8_t>(src.data(), 4),
        System::Span<uint8_t>(dst.data(), 3),
        consumed, written);
    EXPECT_EQ(status, OS::Done);
    EXPECT_EQ(written, 1);
    EXPECT_EQ(dst[0], uint8_t('M'));
}

TEST(Base64Tests, IsValid_ValidString) {
    const char* s = "TWFu";
    System::ReadOnlySpan<char> span(s, 4);
    EXPECT_TRUE(System::Buffers::Text::Base64::IsValid(span));
}
TEST(Base64Tests, IsValid_InvalidChar) {
    const char* s = "TW!u";
    System::ReadOnlySpan<char> span(s, 4);
    EXPECT_FALSE(System::Buffers::Text::Base64::IsValid(span));
}
TEST(Base64Tests, IsValid_WrongLength) {
    const char* s = "TWF";
    System::ReadOnlySpan<char> span(s, 3);
    EXPECT_FALSE(System::Buffers::Text::Base64::IsValid(span));
}
TEST(Base64Tests, IsValid_WithDecodedLength) {
    const char* s = "TWFu";
    System::ReadOnlySpan<uint8_t> span(reinterpret_cast<const uint8_t*>(s), 4);
    int decodedLen = 0;
    EXPECT_TRUE(System::Buffers::Text::Base64::IsValid(span, decodedLen));
    EXPECT_EQ(decodedLen, 3);
}

TEST(Base64Tests, RoundTrip_HelloWorld) {
    std::string input = "Hello, World!";
    std::vector<uint8_t> srcBytes(input.begin(), input.end());
    std::string encoded = System::Buffers::Text::Base64::EncodeToString(srcBytes);

    std::vector<uint8_t> encBytes(encoded.begin(), encoded.end());
    std::vector<uint8_t> decoded(srcBytes.size());
    int consumed = 0, written = 0;
    System::Buffers::Text::Base64::DecodeFromUtf8(
        System::ReadOnlySpan<uint8_t>(encBytes.data(), static_cast<int>(encBytes.size())),
        System::Span<uint8_t>(decoded.data(), static_cast<int>(decoded.size())),
        consumed, written);
    EXPECT_EQ(written, static_cast<int>(srcBytes.size()));
    EXPECT_EQ(std::string(decoded.begin(), decoded.begin() + written), input);
}

// ===========================================================================
// Base64Url — Encode / Decode / Validate
// ===========================================================================

TEST(Base64UrlTests, GetEncodedLength_3bytes_is4) {
    EXPECT_EQ(System::Buffers::Text::Base64Url::GetEncodedLength(3), 4);
}
TEST(Base64UrlTests, GetEncodedLength_1byte_is2) {
    EXPECT_EQ(System::Buffers::Text::Base64Url::GetEncodedLength(1), 2);
}
TEST(Base64UrlTests, GetEncodedLength_2bytes_is3) {
    EXPECT_EQ(System::Buffers::Text::Base64Url::GetEncodedLength(2), 3);
}

TEST(Base64UrlTests, EncodeToString_Man) {
    std::vector<uint8_t> data{'M', 'a', 'n'};
    EXPECT_EQ(System::Buffers::Text::Base64Url::EncodeToString(data), "TWFu");
}

TEST(Base64UrlTests, EncodeToString_M_NoPadding) {
    std::vector<uint8_t> data{'M'};
    // "M" in base64url (no padding) = "TQ"
    EXPECT_EQ(System::Buffers::Text::Base64Url::EncodeToString(data), "TQ");
}

TEST(Base64UrlTests, EncodeToString_Ma_NoPadding) {
    std::vector<uint8_t> data{'M', 'a'};
    // "Ma" in base64url = "TWE"
    EXPECT_EQ(System::Buffers::Text::Base64Url::EncodeToString(data), "TWE");
}

TEST(Base64UrlTests, IsValid_ValidString) {
    const char* s = "TWFu";
    System::ReadOnlySpan<char> span(s, 4);
    EXPECT_TRUE(System::Buffers::Text::Base64Url::IsValid(span));
}
TEST(Base64UrlTests, IsValid_NoPaddingTwoChars) {
    const char* s = "TQ";
    System::ReadOnlySpan<char> span(s, 2);
    EXPECT_TRUE(System::Buffers::Text::Base64Url::IsValid(span));
}
TEST(Base64UrlTests, IsValid_InvalidChar_Plus) {
    const char* s = "TW+u";  // '+' not in base64url
    System::ReadOnlySpan<char> span(s, 4);
    EXPECT_FALSE(System::Buffers::Text::Base64Url::IsValid(span));
}
TEST(Base64UrlTests, IsValid_DashUnderscore_Valid) {
    // base64url uses '-' and '_'
    const char* s = "aa-_";
    System::ReadOnlySpan<char> span(s, 4);
    EXPECT_TRUE(System::Buffers::Text::Base64Url::IsValid(span));
}

TEST(Base64UrlTests, RoundTrip_HelloWorld) {
    std::string input = "Hello, World!";
    std::vector<uint8_t> srcBytes(input.begin(), input.end());
    std::string encoded = System::Buffers::Text::Base64Url::EncodeToString(srcBytes);

    std::vector<uint8_t> encBytes(encoded.begin(), encoded.end());
    std::vector<uint8_t> decoded(srcBytes.size());
    int consumed = 0, written = 0;
    System::Buffers::Text::Base64Url::DecodeFromUtf8(
        System::ReadOnlySpan<uint8_t>(encBytes.data(), static_cast<int>(encBytes.size())),
        System::Span<uint8_t>(decoded.data(), static_cast<int>(decoded.size())),
        consumed, written);
    EXPECT_EQ(written, static_cast<int>(srcBytes.size()));
    EXPECT_EQ(std::string(decoded.begin(), decoded.begin() + written), input);
}
