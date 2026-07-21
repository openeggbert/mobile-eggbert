// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/BinaryData.hpp"
#include "System/IO/MemoryStream.hpp"

using System::BinaryData;

TEST(BinaryDataTests, Equals_SameContent_True) {
    auto a = BinaryData::FromString("abc");
    auto b = BinaryData::FromString("abc");
    EXPECT_TRUE(a.Equals(b));
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(BinaryDataTests, Equals_DifferentContent_False) {
    auto a = BinaryData::FromString("abc");
    auto b = BinaryData::FromString("xyz");
    EXPECT_FALSE(a.Equals(b));
    EXPECT_TRUE(a != b);
}

TEST(BinaryDataTests, Equals_DifferentMediaType_False) {
    std::vector<uint8_t> v{1, 2, 3};
    auto a = BinaryData::FromBytes(v, "text/plain");
    auto b = BinaryData::FromBytes(v, "application/octet-stream");
    EXPECT_FALSE(a.Equals(b));
}

TEST(BinaryDataTests, GetHashCode_SameContent_SameHash) {
    auto a = BinaryData::FromString("hello");
    auto b = BinaryData::FromString("hello");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(BinaryDataTests, GetHashCode_NonNegative) {
    auto bd = BinaryData::FromString("test");
    EXPECT_GE(bd.GetHashCode(), 0);
}

TEST(BinaryDataTests, FromBytes_VectorWithMediaType_Stored) {
    std::vector<uint8_t> v{1, 2, 3};
    auto bd = BinaryData::FromBytes(v, "image/png");
    EXPECT_EQ(bd.getMediaTypeProperty(), "image/png");
    EXPECT_EQ(bd.getLengthProperty(), 3);
}

TEST(BinaryDataTests, FromStream_ReadsAllBytes) {
    std::vector<uint8_t> v{10, 20, 30, 40};
    System::IO::MemoryStream ms(v.data(), static_cast<int>(v.size()));
    auto bd = BinaryData::FromStream(ms);
    EXPECT_EQ(bd.getLengthProperty(), 4);
    EXPECT_EQ(bd[0], 10);
    EXPECT_EQ(bd[3], 40);
}

TEST(BinaryDataTests, Indexer_NegativeIndex_Throws) {
    // Unchecked `bytes_[static_cast<size_t>(index)]` for a negative index wraps to a huge
    // unsigned value -- a real out-of-bounds read (confirmed via ASan heap-buffer-overflow),
    // not a caught exception, prior to this bounds check being added.
    auto bd = BinaryData::FromString("abc");
    EXPECT_THROW(bd[-1], System::ArgumentOutOfRangeException);
}

TEST(BinaryDataTests, Indexer_IndexEqualsLength_Throws) {
    auto bd = BinaryData::FromString("abc");
    EXPECT_THROW(bd[3], System::ArgumentOutOfRangeException);
}

TEST(BinaryDataTests, Indexer_ValidIndex_ReturnsExpectedByte) {
    auto bd = BinaryData::FromString("abc");
    EXPECT_EQ(bd[0], static_cast<uint8_t>('a'));
    EXPECT_EQ(bd[2], static_cast<uint8_t>('c'));
}

TEST(BinaryDataTests, FromStream_WithMediaType) {
    std::vector<uint8_t> v{1, 2};
    System::IO::MemoryStream ms(v.data(), static_cast<int>(v.size()));
    auto bd = BinaryData::FromStream(ms, "application/octet-stream");
    EXPECT_EQ(bd.getMediaTypeProperty(), "application/octet-stream");
    EXPECT_EQ(bd.getLengthProperty(), 2);
}

TEST(BinaryDataTests, ToStream_CorrectBytes) {
    auto bd = BinaryData::FromString("hello");
    auto ms = bd.ToStream();
    std::vector<uint8_t> buf(5);
    ms.Read(buf.data(), 0, 5);
    EXPECT_EQ(buf[0], static_cast<uint8_t>('h'));
    EXPECT_EQ(buf[4], static_cast<uint8_t>('o'));
}

TEST(BinaryDataTests, ImplicitConversion_ToReadOnlyMemory) {
    auto bd = BinaryData::FromString("abc");
    System::ReadOnlyMemory<uint8_t> rom = bd;
    EXPECT_EQ(rom.getLengthProperty(), 3);
}

TEST(BinaryDataTests, ImplicitConversion_ToReadOnlySpan) {
    auto bd = BinaryData::FromString("xyz");
    System::ReadOnlySpan<uint8_t> ros = bd;
    EXPECT_EQ(ros.getLengthProperty(), 3);
}
