// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/WebHeaderCollection.hpp"
#include "System/Net/HttpRequestHeader.hpp"
#include "System/Net/HttpResponseHeader.hpp"
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"

using System::Net::WebHeaderCollection;
using System::Net::HttpRequestHeader;
using System::Net::HttpResponseHeader;

TEST(WebHeaderCollectionTests, Add_ThenGet_ByName) {
    WebHeaderCollection h;
    h.Add("X-Custom", "value1");
    EXPECT_EQ(h.Get("X-Custom"), "value1");
}

TEST(WebHeaderCollectionTests, Get_CaseInsensitive) {
    WebHeaderCollection h;
    h.Add("Content-Type", "text/plain");
    EXPECT_EQ(h.Get("content-type"), "text/plain");
}

TEST(WebHeaderCollectionTests, Add_MultipleValues_CommaJoined) {
    WebHeaderCollection h;
    h.Add("Accept", "text/html");
    h.Add("Accept", "application/json");
    EXPECT_EQ(h.Get("Accept"), "text/html,application/json");
}

TEST(WebHeaderCollectionTests, Set_ReplacesExistingValue) {
    WebHeaderCollection h;
    h.Add("X-Custom", "a");
    h.Set("X-Custom", "b");
    EXPECT_EQ(h.Get("X-Custom"), "b");
}

TEST(WebHeaderCollectionTests, Remove_DeletesHeader) {
    WebHeaderCollection h;
    h.Add("X-Custom", "a");
    h.Remove("X-Custom");
    EXPECT_EQ(h.getCountProperty(), 0);
}

TEST(WebHeaderCollectionTests, Add_CombinedHeaderString) {
    WebHeaderCollection h;
    h.Add("X-Custom: hello world");
    EXPECT_EQ(h.Get("X-Custom"), "hello world");
}

TEST(WebHeaderCollectionTests, Add_CombinedHeaderMissingColon_Throws) {
    WebHeaderCollection h;
    EXPECT_THROW(h.Add("no colon here"), System::ArgumentException);
}

TEST(WebHeaderCollectionTests, Add_InvalidNameChars_Throws) {
    WebHeaderCollection h;
    EXPECT_THROW(h.Add("Bad:Name", "value"), System::ArgumentException);
}

TEST(WebHeaderCollectionTests, Add_EmptyName_ThrowsArgumentException) {
    WebHeaderCollection h;
    EXPECT_THROW(h.Add("", "value"), System::ArgumentException);
}

TEST(WebHeaderCollectionTests, Add_InvalidValueCrlf_Throws) {
    WebHeaderCollection h;
    EXPECT_THROW(h.Add("X-Custom", "bad\r\nvalue"), System::ArgumentException);
}

TEST(WebHeaderCollectionTests, Add_ValidFoldedValue_Allowed) {
    WebHeaderCollection h;
    h.Add("X-Custom", "line1\r\n line2");
    EXPECT_FALSE(h.Get("X-Custom").empty());
}

TEST(WebHeaderCollectionTests, ToString_Format) {
    WebHeaderCollection h;
    h.Add("Host", "example.com");
    EXPECT_EQ(h.ToString(), "Host: example.com\r\n\r\n");
}

TEST(WebHeaderCollectionTests, ToString_Empty) {
    WebHeaderCollection h;
    EXPECT_EQ(h.ToString(), "\r\n");
}

TEST(WebHeaderCollectionTests, ToByteArray_MatchesToString) {
    WebHeaderCollection h;
    h.Add("Host", "example.com");
    auto bytes = h.ToByteArray();
    std::string fromBytes(bytes.begin(), bytes.end());
    EXPECT_EQ(fromBytes, h.ToString());
}

TEST(WebHeaderCollectionTests, RequestHeaderIndexer_AddAndGet) {
    WebHeaderCollection h;
    h.Add(HttpRequestHeader::Host, "example.com");
    EXPECT_EQ(h.Get(HttpRequestHeader::Host), "example.com");
}

TEST(WebHeaderCollectionTests, ResponseHeaderIndexer_AddAndGet) {
    WebHeaderCollection h;
    h.Add(HttpResponseHeader::Server, "nginx");
    EXPECT_EQ(h.Get(HttpResponseHeader::Server), "nginx");
}

TEST(WebHeaderCollectionTests, MixingRequestAndResponseHeaders_Throws) {
    WebHeaderCollection h;
    h.Add(HttpRequestHeader::Host, "example.com");
    EXPECT_THROW(h.Add(HttpResponseHeader::Server, "nginx"), System::InvalidOperationException);
}

TEST(WebHeaderCollectionTests, IsRestricted_RequestHeader_True) {
    EXPECT_TRUE(WebHeaderCollection::IsRestricted("Host"));
    EXPECT_TRUE(WebHeaderCollection::IsRestricted("Content-Length"));
}

TEST(WebHeaderCollectionTests, IsRestricted_CustomHeader_False) {
    EXPECT_FALSE(WebHeaderCollection::IsRestricted("X-Custom"));
}

TEST(WebHeaderCollectionTests, IsRestricted_ResponseHeader) {
    EXPECT_TRUE(WebHeaderCollection::IsRestricted("WWW-Authenticate", true));
    EXPECT_FALSE(WebHeaderCollection::IsRestricted("Host", true));
}

TEST(WebHeaderCollectionTests, Clear_RemovesAllHeaders) {
    WebHeaderCollection h;
    h.Add("X-Custom", "a");
    h.Clear();
    EXPECT_EQ(h.getCountProperty(), 0);
}
