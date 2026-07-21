// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/ViaHeaderValue.hpp"
#include "System/FormatException.hpp"
#include "System/ArgumentException.hpp"

using System::Net::Http::Headers::ViaHeaderValue;

TEST(ViaHeaderValueTests, Constructor_VersionAndReceivedBy_ToString) {
    ViaHeaderValue v("1.1", "proxy.example.com");
    EXPECT_EQ(v.getProtocolVersionProperty(), "1.1");
    EXPECT_EQ(v.getReceivedByProperty(), "proxy.example.com");
    EXPECT_TRUE(v.getProtocolNameProperty().empty());
    EXPECT_EQ(v.ToString(), "1.1 proxy.example.com");
}

TEST(ViaHeaderValueTests, Constructor_WithProtocolName_ToString) {
    ViaHeaderValue v("1.1", "proxy.example.com", "HTTP");
    EXPECT_EQ(v.ToString(), "HTTP/1.1 proxy.example.com");
}

TEST(ViaHeaderValueTests, Constructor_WithComment_ToString) {
    ViaHeaderValue v("1.1", "proxy.example.com", "HTTP", "(Apache/1.1)");
    EXPECT_EQ(v.ToString(), "HTTP/1.1 proxy.example.com (Apache/1.1)");
}

TEST(ViaHeaderValueTests, Constructor_ReceivedByWithPort) {
    ViaHeaderValue v("1.1", "proxy.example.com:8080");
    EXPECT_EQ(v.getReceivedByProperty(), "proxy.example.com:8080");
}

TEST(ViaHeaderValueTests, Constructor_EmptyReceivedBy_Throws) {
    EXPECT_THROW(ViaHeaderValue("1.1", ""), System::ArgumentException);
}

TEST(ViaHeaderValueTests, Constructor_InvalidProtocolVersion_Throws) {
    EXPECT_THROW(ViaHeaderValue("1 1", "proxy.example.com"), System::FormatException);
}

TEST(ViaHeaderValueTests, Constructor_ReceivedByWithSpace_Throws) {
    EXPECT_THROW(ViaHeaderValue("1.1", "bad host name"), System::FormatException);
}

TEST(ViaHeaderValueTests, Parse_Simple) {
    auto v = ViaHeaderValue::Parse("1.1 proxy.example.com");
    EXPECT_EQ(v.getProtocolVersionProperty(), "1.1");
    EXPECT_EQ(v.getReceivedByProperty(), "proxy.example.com");
}

TEST(ViaHeaderValueTests, Parse_WithProtocolNameAndComment) {
    auto v = ViaHeaderValue::Parse("HTTP/1.1 proxy.example.com (comment here)");
    EXPECT_EQ(v.getProtocolNameProperty(), "HTTP");
    EXPECT_EQ(v.getProtocolVersionProperty(), "1.1");
    EXPECT_EQ(v.getReceivedByProperty(), "proxy.example.com");
    EXPECT_EQ(v.getCommentProperty(), "(comment here)");
}

TEST(ViaHeaderValueTests, Parse_RoundTripsThroughToString) {
    auto v = ViaHeaderValue::Parse("HTTP/1.1 proxy.example.com (comment)");
    auto v2 = ViaHeaderValue::Parse(v.ToString());
    EXPECT_TRUE(v == v2);
}

TEST(ViaHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(ViaHeaderValue::Parse("1.1"), System::FormatException);
}

TEST(ViaHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    ViaHeaderValue result("1.0", "x");
    EXPECT_FALSE(ViaHeaderValue::TryParse("1.1", result));
}

TEST(ViaHeaderValueTests, Equals_CaseInsensitiveFields) {
    ViaHeaderValue a("1.1", "Proxy.Example.Com", "HTTP");
    ViaHeaderValue b("1.1", "proxy.example.com", "http");
    EXPECT_TRUE(a == b);
}

TEST(ViaHeaderValueTests, Equals_CommentCaseSensitive) {
    ViaHeaderValue a("1.1", "proxy.example.com", "HTTP", "(Comment)");
    ViaHeaderValue b("1.1", "proxy.example.com", "HTTP", "(comment)");
    EXPECT_FALSE(a == b);
}

TEST(ViaHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    ViaHeaderValue a("1.1", "Proxy.Example.Com", "HTTP");
    ViaHeaderValue b("1.1", "proxy.example.com", "http");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}
