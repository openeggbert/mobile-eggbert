// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/HttpContentHeaders.hpp"

using System::Net::Http::Headers::HttpContentHeaders;
using System::Net::Http::Headers::ContentDispositionHeaderValue;
using System::Net::Http::Headers::ContentRangeHeaderValue;
using System::Net::Http::Headers::MediaTypeHeaderValue;

TEST(HttpContentHeadersTests, Allow_AddThenGet) {
    HttpContentHeaders h;
    EXPECT_TRUE(h.getAllowProperty().empty());
    h.AddAllow("GET");
    h.AddAllow("POST");
    auto values = h.getAllowProperty();
    ASSERT_EQ(values.size(), 2u);
    EXPECT_EQ(values[0], "GET");
    EXPECT_EQ(values[1], "POST");
}

TEST(HttpContentHeadersTests, ContentDisposition_RoundTrips) {
    HttpContentHeaders h;
    EXPECT_FALSE(h.getContentDispositionProperty().has_value());
    ContentDispositionHeaderValue cd("attachment");
    cd.setFileNameProperty("test.txt");
    h.setContentDispositionProperty(cd);
    auto result = h.getContentDispositionProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getDispositionTypeProperty(), "attachment");
    EXPECT_EQ(result->getFileNameProperty(), "test.txt");
}

TEST(HttpContentHeadersTests, ContentDisposition_SetNullopt_Removes) {
    HttpContentHeaders h;
    ContentDispositionHeaderValue cd("attachment");
    h.setContentDispositionProperty(cd);
    EXPECT_TRUE(h.getContentDispositionProperty().has_value());
    h.setContentDispositionProperty(std::nullopt);
    EXPECT_FALSE(h.getContentDispositionProperty().has_value());
}

TEST(HttpContentHeadersTests, ContentEncoding_AddThenGet) {
    HttpContentHeaders h;
    h.AddContentEncoding("gzip");
    auto values = h.getContentEncodingProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0], "gzip");
}

TEST(HttpContentHeadersTests, ContentLanguage_AddThenGet) {
    HttpContentHeaders h;
    h.AddContentLanguage("en-US");
    auto values = h.getContentLanguageProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0], "en-US");
}

TEST(HttpContentHeadersTests, ContentLength_RoundTrips) {
    HttpContentHeaders h;
    EXPECT_FALSE(h.getContentLengthProperty().has_value());
    h.setContentLengthProperty(1234);
    ASSERT_TRUE(h.getContentLengthProperty().has_value());
    EXPECT_EQ(*h.getContentLengthProperty(), 1234);
}

TEST(HttpContentHeadersTests, ContentLength_Invalid_ReturnsNullopt) {
    HttpContentHeaders h;
    h.Add("Content-Length", "not-a-number");
    EXPECT_FALSE(h.getContentLengthProperty().has_value());
}

TEST(HttpContentHeadersTests, ContentLength_SetNullopt_Removes) {
    HttpContentHeaders h;
    h.setContentLengthProperty(100);
    EXPECT_TRUE(h.Contains("Content-Length"));
    h.setContentLengthProperty(std::nullopt);
    EXPECT_FALSE(h.Contains("Content-Length"));
}

TEST(HttpContentHeadersTests, ContentLocation_RoundTrips) {
    HttpContentHeaders h;
    h.setContentLocationProperty(System::Uri("http://example.com/foo"));
    auto result = h.getContentLocationProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->ToString(), "http://example.com/foo");
}

TEST(HttpContentHeadersTests, ContentMD5_RoundTrips) {
    HttpContentHeaders h;
    std::vector<SharpRuntime::bytecs> bytes{1, 2, 3, 4};
    h.setContentMD5Property(bytes);
    auto result = h.getContentMD5Property();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, bytes);
}

TEST(HttpContentHeadersTests, ContentRange_RoundTrips) {
    HttpContentHeaders h;
    ContentRangeHeaderValue range(0, 499, 1000);
    h.setContentRangeProperty(range);
    auto result = h.getContentRangeProperty();
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->getFromProperty().has_value());
    EXPECT_EQ(*result->getFromProperty(), 0);
    ASSERT_TRUE(result->getToProperty().has_value());
    EXPECT_EQ(*result->getToProperty(), 499);
    ASSERT_TRUE(result->getLengthProperty().has_value());
    EXPECT_EQ(*result->getLengthProperty(), 1000);
}

TEST(HttpContentHeadersTests, ContentType_RoundTrips) {
    HttpContentHeaders h;
    MediaTypeHeaderValue mt("text/plain");
    h.setContentTypeProperty(mt);
    auto result = h.getContentTypeProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getMediaTypeProperty(), "text/plain");
}

TEST(HttpContentHeadersTests, ContentType_Invalid_ReturnsNullopt) {
    HttpContentHeaders h;
    h.Add("Content-Type", "not a media type/");
    EXPECT_FALSE(h.getContentTypeProperty().has_value());
}

TEST(HttpContentHeadersTests, Expires_RoundTrips) {
    HttpContentHeaders h;
    System::DateTimeOffset dto(2015, 10, 21, 7, 28, 0, System::TimeSpan::Zero);
    h.setExpiresProperty(dto);
    auto result = h.getExpiresProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->ToString("r"), "Wed, 21 Oct 2015 07:28:00 GMT");
}

TEST(HttpContentHeadersTests, Expires_Invalid_ReturnsNullopt) {
    HttpContentHeaders h;
    h.Add("Expires", "not a date");
    EXPECT_FALSE(h.getExpiresProperty().has_value());
}

TEST(HttpContentHeadersTests, LastModified_RoundTrips) {
    HttpContentHeaders h;
    System::DateTimeOffset dto(2020, 1, 2, 3, 4, 5, System::TimeSpan::Zero);
    h.setLastModifiedProperty(dto);
    auto result = h.getLastModifiedProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->ToString("r"), "Thu, 02 Jan 2020 03:04:05 GMT");
}

TEST(HttpContentHeadersTests, LastModified_SetNullopt_Removes) {
    HttpContentHeaders h;
    System::DateTimeOffset dto(2020, 1, 2, 3, 4, 5, System::TimeSpan::Zero);
    h.setLastModifiedProperty(dto);
    EXPECT_TRUE(h.Contains("Last-Modified"));
    h.setLastModifiedProperty(std::nullopt);
    EXPECT_FALSE(h.Contains("Last-Modified"));
}
