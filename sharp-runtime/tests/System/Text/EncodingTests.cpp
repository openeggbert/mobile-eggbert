// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)

#include <gtest/gtest.h>
#include "System/Text/Encoding.hpp"

using System::Text::Encoding;
using SharpRuntime::bytecs;

TEST(EncodingTests, UTF8ReturnsNonNull) {
    EXPECT_NE(nullptr, Encoding::UTF8());
}

TEST(EncodingTests, ASCIIReturnsNonNull) {
    EXPECT_NE(nullptr, Encoding::ASCII());
}

TEST(EncodingTests, UnicodeReturnsNonNull) {
    EXPECT_NE(nullptr, Encoding::Unicode());
}

TEST(EncodingTests, UTF8EncodingName) {
    EXPECT_EQ("utf-8", Encoding::UTF8()->getEncodingNameProperty());
}

TEST(EncodingTests, ASCIIEncodingNameContainsAscii) {
    std::string name = Encoding::ASCII()->getEncodingNameProperty();
    EXPECT_NE(std::string::npos, name.find("ascii"));
}

TEST(EncodingTests, ASCIIGetBytesABC) {
    auto enc = Encoding::ASCII();
    auto bytes = enc->GetBytes("ABC");
    ASSERT_EQ(3u, bytes.size());
    EXPECT_EQ(65, bytes[0]);
    EXPECT_EQ(66, bytes[1]);
    EXPECT_EQ(67, bytes[2]);
}

TEST(EncodingTests, UTF8GetBytesHello) {
    auto enc = Encoding::UTF8();
    auto bytes = enc->GetBytes("hello");
    ASSERT_EQ(5u, bytes.size());
    EXPECT_EQ(104, bytes[0]);
    EXPECT_EQ(101, bytes[1]);
    EXPECT_EQ(108, bytes[2]);
    EXPECT_EQ(108, bytes[3]);
    EXPECT_EQ(111, bytes[4]);
}

TEST(EncodingTests, UTF8RoundTrip) {
    auto enc = Encoding::UTF8();
    std::string original = "hello world";
    auto bytes = enc->GetBytes(original);
    std::string decoded = enc->GetString(bytes.data(), 0, static_cast<int>(bytes.size()));
    EXPECT_EQ(original, decoded);
}

TEST(EncodingTests, ASCIIRoundTrip) {
    auto enc = Encoding::ASCII();
    std::string original = "hello world";
    auto bytes = enc->GetBytes(original);
    std::string decoded = enc->GetString(bytes.data(), 0, static_cast<int>(bytes.size()));
    EXPECT_EQ(original, decoded);
}

TEST(EncodingTests, UTF8EmptyStringGetBytesIsEmpty) {
    auto bytes = Encoding::UTF8()->GetBytes("");
    EXPECT_TRUE(bytes.empty());
}

TEST(EncodingTests, ASCIIEmptyStringGetBytesIsEmpty) {
    auto bytes = Encoding::ASCII()->GetBytes("");
    EXPECT_TRUE(bytes.empty());
}

TEST(EncodingTests, UTF8GetStringEmptyRange) {
    auto enc = Encoding::UTF8();
    std::string result = enc->GetString(nullptr, 0, 0);
    EXPECT_EQ("", result);
}

TEST(EncodingTests, GetStringWithOffset) {
    auto enc = Encoding::UTF8();
    std::string original = "abcdef";
    auto bytes = enc->GetBytes(original);
    // decode bytes[2..4] => "cd"
    std::string decoded = enc->GetString(bytes.data(), 2, 2);
    EXPECT_EQ("cd", decoded);
}

TEST(EncodingTests, UTF8GetBytesNumbers) {
    auto enc = Encoding::UTF8();
    auto bytes = enc->GetBytes("123");
    ASSERT_EQ(3u, bytes.size());
    EXPECT_EQ('1', bytes[0]);
    EXPECT_EQ('2', bytes[1]);
    EXPECT_EQ('3', bytes[2]);
}
