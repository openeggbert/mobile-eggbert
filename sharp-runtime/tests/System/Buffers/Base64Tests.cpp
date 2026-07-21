// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Buffers/Text/Base64.hpp"

using System::Buffers::Text::Base64;
using System::Buffers::OperationStatus;
using System::ReadOnlySpan;
using System::Span;
using System::FormatException;

TEST(Base64Test, EncodeHello) {
    std::vector<uint8_t> input = {'H','e','l','l','o'};
    std::string encoded = Base64::EncodeToString(input);
    EXPECT_EQ(encoded, "SGVsbG8=");
}

TEST(Base64Test, EncodeEmpty) {
    std::vector<uint8_t> input;
    EXPECT_EQ(Base64::EncodeToString(input), "");
}

TEST(Base64Test, DecodeHello) {
    std::string b64 = "SGVsbG8=";
    ReadOnlySpan<uint8_t> src(reinterpret_cast<const uint8_t*>(b64.data()), static_cast<int>(b64.size()));
    std::vector<uint8_t> out(10);
    Span<uint8_t> dst(out.data(), static_cast<int>(out.size()));
    int consumed = 0, written = 0;
    auto status = Base64::DecodeFromUtf8(src, dst, consumed, written, true);
    EXPECT_EQ(status, OperationStatus::Done);
    EXPECT_EQ(written, 5);
    EXPECT_EQ(out[0], 'H');
    EXPECT_EQ(out[4], 'o');
}

TEST(Base64Test, GetMaxDecodedLength) {
    EXPECT_EQ(Base64::GetMaxDecodedFromUtf8Length(4), 3);
    EXPECT_EQ(Base64::GetMaxDecodedFromUtf8Length(8), 6);
}

TEST(Base64Test, GetMaxEncodedLength) {
    EXPECT_EQ(Base64::GetMaxEncodedToUtf8Length(3), 4);
    EXPECT_EQ(Base64::GetMaxEncodedToUtf8Length(4), 8);
}

TEST(Base64Test, IsValidTrue) {
    std::string valid = "SGVsbG8=";
    ReadOnlySpan<uint8_t> span(reinterpret_cast<const uint8_t*>(valid.data()), static_cast<int>(valid.size()));
    EXPECT_TRUE(Base64::IsValid(span));
}

TEST(Base64Test, IsValidFalse) {
    std::string invalid = "SGVs!G8=";
    ReadOnlySpan<uint8_t> span(reinterpret_cast<const uint8_t*>(invalid.data()), static_cast<int>(invalid.size()));
    EXPECT_FALSE(Base64::IsValid(span));
}

TEST(Base64Test, DestinationTooSmall) {
    std::vector<uint8_t> input = {'A','B','C'};
    ReadOnlySpan<uint8_t> src(input.data(), static_cast<int>(input.size()));
    uint8_t dstBuf[2] = {};
    Span<uint8_t> dst(dstBuf, 2);
    int consumed = 0, written = 0;
    auto status = Base64::EncodeToUtf8(src, dst, consumed, written, true);
    EXPECT_EQ(status, OperationStatus::DestinationTooSmall);
}

TEST(Base64Test, RoundTripBytes) {
    std::vector<uint8_t> original = {0x00, 0xFF, 0x42, 0xAB};
    std::string encoded = Base64::EncodeToString(original);
    ReadOnlySpan<uint8_t> srcSpan(reinterpret_cast<const uint8_t*>(encoded.data()),
                                   static_cast<int>(encoded.size()));
    std::vector<uint8_t> decoded(10);
    Span<uint8_t> dstSpan(decoded.data(), static_cast<int>(decoded.size()));
    int consumed = 0, written = 0;
    Base64::DecodeFromUtf8(srcSpan, dstSpan, consumed, written, true);
    ASSERT_EQ(written, 4);
    for (int i = 0; i < 4; ++i) EXPECT_EQ(decoded[i], original[i]);
}

// ===========================================================================
// Whitespace handling / incomplete-group correctness (parity fixes)
// ===========================================================================

TEST(Base64Test, DecodeFromUtf8_WithEmbeddedWhitespace_Succeeds) {
    std::string b64 = "SGVs\r\nbG8=";
    ReadOnlySpan<uint8_t> src(reinterpret_cast<const uint8_t*>(b64.data()), static_cast<int>(b64.size()));
    std::vector<uint8_t> out(10);
    Span<uint8_t> dst(out.data(), static_cast<int>(out.size()));
    int consumed = 0, written = 0;
    auto status = Base64::DecodeFromUtf8(src, dst, consumed, written, true);
    EXPECT_EQ(status, OperationStatus::Done);
    ASSERT_EQ(written, 5);
    EXPECT_EQ(out[0], 'H');
    EXPECT_EQ(out[4], 'o');
}

TEST(Base64Test, DecodeFromUtf8_TrailingDataAfterPadding_IsInvalid) {
    std::string b64 = "SGVsbG8=XX";
    ReadOnlySpan<uint8_t> src(reinterpret_cast<const uint8_t*>(b64.data()), static_cast<int>(b64.size()));
    std::vector<uint8_t> out(10);
    Span<uint8_t> dst(out.data(), static_cast<int>(out.size()));
    int consumed = 0, written = 0;
    EXPECT_EQ(Base64::DecodeFromUtf8(src, dst, consumed, written, true), OperationStatus::InvalidData);
}

TEST(Base64Test, DecodeFromUtf8_IncompleteFinalGroup_IsInvalid) {
    std::string b64 = "QQQ"; // 3 symbols, not a multiple of 4, no padding
    ReadOnlySpan<uint8_t> src(reinterpret_cast<const uint8_t*>(b64.data()), static_cast<int>(b64.size()));
    std::vector<uint8_t> out(10);
    Span<uint8_t> dst(out.data(), static_cast<int>(out.size()));
    int consumed = 0, written = 0;
    EXPECT_EQ(Base64::DecodeFromUtf8(src, dst, consumed, written, true), OperationStatus::InvalidData);
}

// Regression tests for a code-audit finding (ticket 247): the isFinalBlock=false streaming
// path (NeedMoreData) had zero test coverage for either DecodeFromUtf8 or EncodeToChars, despite
// being explicit, documented OperationStatus outcomes used by the streaming (chunked) overloads.
TEST(Base64Test, DecodeFromUtf8_IncompleteFinalGroup_NotFinalBlock_NeedsMoreData) {
    std::string b64 = "QQQ"; // 3 symbols -- incomplete group, but more data may follow
    ReadOnlySpan<uint8_t> src(reinterpret_cast<const uint8_t*>(b64.data()), static_cast<int>(b64.size()));
    std::vector<uint8_t> out(10);
    Span<uint8_t> dst(out.data(), static_cast<int>(out.size()));
    int consumed = 0, written = 0;
    EXPECT_EQ(Base64::DecodeFromUtf8(src, dst, consumed, written, false), OperationStatus::NeedMoreData);
    EXPECT_EQ(consumed, 0);
    EXPECT_EQ(written, 0);
}

TEST(Base64Test, EncodeToChars_NotFinalBlock_RemainderBytes_NeedsMoreData) {
    std::vector<uint8_t> input = {'H', 'e'}; // 2 bytes -- not a full 3-byte group
    ReadOnlySpan<uint8_t> src(input.data(), static_cast<int>(input.size()));
    std::vector<char> out(10);
    Span<char> dst(out.data(), static_cast<int>(out.size()));
    int consumed = 0, written = 0;
    EXPECT_EQ(Base64::EncodeToChars(src, dst, consumed, written, false), OperationStatus::NeedMoreData);
    EXPECT_EQ(consumed, 0);
    EXPECT_EQ(written, 0);
}

TEST(Base64Test, IsValid_AllowsEmbeddedWhitespace) {
    std::string valid = "SGVs\r\nbG8=";
    ReadOnlySpan<uint8_t> span(reinterpret_cast<const uint8_t*>(valid.data()), static_cast<int>(valid.size()));
    EXPECT_TRUE(Base64::IsValid(span));
}

TEST(Base64Test, IsValid_WithDecodedLength_CountsWhitespaceFree) {
    std::string valid = "SGVs\r\nbG8=";
    ReadOnlySpan<uint8_t> span(reinterpret_cast<const uint8_t*>(valid.data()), static_cast<int>(valid.size()));
    int decodedLength = 0;
    EXPECT_TRUE(Base64::IsValid(span, decodedLength));
    EXPECT_EQ(decodedLength, 5);
}

// ===========================================================================
// New convenience overloads
// ===========================================================================

TEST(Base64Test, GetEncodedLength_MatchesGetMaxEncodedToUtf8Length) {
    EXPECT_EQ(Base64::GetEncodedLength(3), Base64::GetMaxEncodedToUtf8Length(3));
}

TEST(Base64Test, GetMaxDecodedLength_MatchesGetMaxDecodedFromUtf8Length) {
    EXPECT_EQ(Base64::GetMaxDecodedLength(8), Base64::GetMaxDecodedFromUtf8Length(8));
}

TEST(Base64Test, EncodeToUtf8_TwoArg_ReturnsBytesWritten) {
    std::vector<uint8_t> input = {'H','i'};
    ReadOnlySpan<uint8_t> src(input.data(), static_cast<int>(input.size()));
    std::vector<uint8_t> out(8);
    Span<uint8_t> dst(out.data(), static_cast<int>(out.size()));
    int written = Base64::EncodeToUtf8(src, dst);
    EXPECT_EQ(written, 4);
}

TEST(Base64Test, EncodeToUtf8_TwoArg_TooSmall_Throws) {
    std::vector<uint8_t> input = {'H','i'};
    ReadOnlySpan<uint8_t> src(input.data(), static_cast<int>(input.size()));
    uint8_t buf[2] = {};
    Span<uint8_t> dst(buf, 2);
    EXPECT_THROW(Base64::EncodeToUtf8(src, dst), System::ArgumentException);
}

TEST(Base64Test, EncodeToUtf8_ReturnsVector) {
    std::vector<uint8_t> input = {'H','i'};
    ReadOnlySpan<uint8_t> src(input.data(), static_cast<int>(input.size()));
    std::vector<uint8_t> encoded = Base64::EncodeToUtf8(src);
    std::string s(encoded.begin(), encoded.end());
    EXPECT_EQ(s, "SGk=");
}

TEST(Base64Test, TryEncodeToUtf8_Success) {
    std::vector<uint8_t> input = {'H','i'};
    ReadOnlySpan<uint8_t> src(input.data(), static_cast<int>(input.size()));
    std::vector<uint8_t> out(8);
    Span<uint8_t> dst(out.data(), static_cast<int>(out.size()));
    int written = 0;
    EXPECT_TRUE(Base64::TryEncodeToUtf8(src, dst, written));
    EXPECT_EQ(written, 4);
}

TEST(Base64Test, TryEncodeToUtf8_TooSmall_ReturnsFalse) {
    std::vector<uint8_t> input = {'H','i'};
    ReadOnlySpan<uint8_t> src(input.data(), static_cast<int>(input.size()));
    uint8_t buf[2] = {};
    Span<uint8_t> dst(buf, 2);
    int written = 0;
    EXPECT_FALSE(Base64::TryEncodeToUtf8(src, dst, written));
}

TEST(Base64Test, EncodeToUtf8InPlace_RoundTrip) {
    uint8_t buf[8] = {'M','a','n', 0,0,0,0,0};
    Span<uint8_t> span(buf, 8);
    int written = 0;
    auto status = Base64::EncodeToUtf8InPlace(span, 3, written);
    EXPECT_EQ(status, OperationStatus::Done);
    EXPECT_EQ(written, 4);
    EXPECT_EQ(std::string(reinterpret_cast<char*>(buf), 4), "TWFu");
}

TEST(Base64Test, EncodeToUtf8InPlace_TooSmall_ReturnsDestinationTooSmall) {
    uint8_t buf[3] = {'M','a','n'};
    Span<uint8_t> span(buf, 3);
    int written = 0;
    EXPECT_EQ(Base64::EncodeToUtf8InPlace(span, 3, written), OperationStatus::DestinationTooSmall);
}

TEST(Base64Test, TryEncodeToUtf8InPlace_Success) {
    uint8_t buf[8] = {'M','a', 0,0,0,0,0,0};
    Span<uint8_t> span(buf, 8);
    int written = 0;
    EXPECT_TRUE(Base64::TryEncodeToUtf8InPlace(span, 2, written));
    EXPECT_EQ(std::string(reinterpret_cast<char*>(buf), written), "TWE=");
}

TEST(Base64Test, DecodeFromUtf8_TwoArg_ReturnsBytesWritten) {
    std::string b64 = "SGk=";
    ReadOnlySpan<uint8_t> src(reinterpret_cast<const uint8_t*>(b64.data()), static_cast<int>(b64.size()));
    std::vector<uint8_t> out(8);
    Span<uint8_t> dst(out.data(), static_cast<int>(out.size()));
    int written = Base64::DecodeFromUtf8(src, dst);
    EXPECT_EQ(written, 2);
    EXPECT_EQ(out[0], 'H');
    EXPECT_EQ(out[1], 'i');
}

TEST(Base64Test, DecodeFromUtf8_TwoArg_InvalidData_ThrowsFormatException) {
    std::string b64 = "SG!=";
    ReadOnlySpan<uint8_t> src(reinterpret_cast<const uint8_t*>(b64.data()), static_cast<int>(b64.size()));
    std::vector<uint8_t> out(8);
    Span<uint8_t> dst(out.data(), static_cast<int>(out.size()));
    EXPECT_THROW(Base64::DecodeFromUtf8(src, dst), FormatException);
}

TEST(Base64Test, DecodeFromUtf8_ReturnsVector) {
    std::string b64 = "SGk=";
    ReadOnlySpan<uint8_t> src(reinterpret_cast<const uint8_t*>(b64.data()), static_cast<int>(b64.size()));
    std::vector<uint8_t> decoded = Base64::DecodeFromUtf8(src);
    ASSERT_EQ(decoded.size(), 2u);
    EXPECT_EQ(decoded[0], 'H');
    EXPECT_EQ(decoded[1], 'i');
}

TEST(Base64Test, TryDecodeFromUtf8_Success) {
    std::string b64 = "SGk=";
    ReadOnlySpan<uint8_t> src(reinterpret_cast<const uint8_t*>(b64.data()), static_cast<int>(b64.size()));
    std::vector<uint8_t> out(8);
    Span<uint8_t> dst(out.data(), static_cast<int>(out.size()));
    int written = 0;
    EXPECT_TRUE(Base64::TryDecodeFromUtf8(src, dst, written));
    EXPECT_EQ(written, 2);
}

TEST(Base64Test, TryDecodeFromUtf8_InvalidData_Throws) {
    std::string b64 = "SG!=";
    ReadOnlySpan<uint8_t> src(reinterpret_cast<const uint8_t*>(b64.data()), static_cast<int>(b64.size()));
    std::vector<uint8_t> out(8);
    Span<uint8_t> dst(out.data(), static_cast<int>(out.size()));
    int written = 0;
    EXPECT_THROW(Base64::TryDecodeFromUtf8(src, dst, written), FormatException);
}

TEST(Base64Test, DecodeFromUtf8InPlace_RoundTrip) {
    // The entire span is treated as base64 text to decode in place (matches .NET semantics).
    uint8_t buf[4] = {'T','W','F','u'};
    Span<uint8_t> span(buf, 4);
    int written = 0;
    auto status = Base64::DecodeFromUtf8InPlace(span, written);
    EXPECT_EQ(status, OperationStatus::Done);
    ASSERT_EQ(written, 3);
    EXPECT_EQ(std::string(reinterpret_cast<char*>(buf), 3), "Man");
}

TEST(Base64Test, EncodeToChars_RoundTrip) {
    std::vector<uint8_t> input = {'H','i'};
    ReadOnlySpan<uint8_t> src(input.data(), static_cast<int>(input.size()));
    char buf[8] = {};
    Span<char> dst(buf, 8);
    int consumed = 0, written = 0;
    auto status = Base64::EncodeToChars(src, dst, consumed, written, true);
    EXPECT_EQ(status, OperationStatus::Done);
    EXPECT_EQ(std::string(buf, written), "SGk=");
}

TEST(Base64Test, EncodeToChars_TwoArg_ThrowsWhenTooSmall) {
    std::vector<uint8_t> input = {'H','i'};
    ReadOnlySpan<uint8_t> src(input.data(), static_cast<int>(input.size()));
    char buf[2] = {};
    Span<char> dst(buf, 2);
    EXPECT_THROW(Base64::EncodeToChars(src, dst), System::ArgumentException);
}

TEST(Base64Test, DecodeFromChars_RoundTrip) {
    std::string b64 = "SGk=";
    ReadOnlySpan<char> src(b64.data(), static_cast<int>(b64.size()));
    std::vector<uint8_t> out(8);
    Span<uint8_t> dst(out.data(), static_cast<int>(out.size()));
    int consumed = 0, written = 0;
    auto status = Base64::DecodeFromChars(src, dst, consumed, written, true);
    EXPECT_EQ(status, OperationStatus::Done);
    ASSERT_EQ(written, 2);
    EXPECT_EQ(out[0], 'H');
    EXPECT_EQ(out[1], 'i');
}

TEST(Base64Test, TryDecodeFromChars_Success) {
    std::string b64 = "SGk=";
    ReadOnlySpan<char> src(b64.data(), static_cast<int>(b64.size()));
    std::vector<uint8_t> out(8);
    Span<uint8_t> dst(out.data(), static_cast<int>(out.size()));
    int written = 0;
    EXPECT_TRUE(Base64::TryDecodeFromChars(src, dst, written));
    EXPECT_EQ(written, 2);
}

TEST(Base64Test, IsValid_Char_WithDecodedLength) {
    std::string valid = "SGk=";
    ReadOnlySpan<char> span(valid.data(), static_cast<int>(valid.size()));
    int decodedLength = 0;
    EXPECT_TRUE(Base64::IsValid(span, decodedLength));
    EXPECT_EQ(decodedLength, 2);
}

TEST(Base64Test, GetMaxEncodedLength_Negative_Throws) {
    EXPECT_THROW(Base64::GetMaxEncodedToUtf8Length(-1), System::ArgumentOutOfRangeException);
}

// Regression test for a code-audit finding (ticket 247): the negative-input bound of
// GetMaxEncodedToUtf8Length was tested above, but the upper bound (bytesLength >
// kMaximumEncodeLength, i.e. > 1610612733, matching .NET's Base64Helper.MaximumEncodeLength)
// had no test coverage at all.
TEST(Base64Test, GetMaxEncodedLength_AboveMaximum_Throws) {
    EXPECT_THROW(Base64::GetMaxEncodedToUtf8Length(1610612734), System::ArgumentOutOfRangeException);
    EXPECT_NO_THROW(Base64::GetMaxEncodedToUtf8Length(1610612733));
}

TEST(Base64Test, GetMaxDecodedLength_Negative_Throws) {
    EXPECT_THROW(Base64::GetMaxDecodedFromUtf8Length(-1), System::ArgumentOutOfRangeException);
}
