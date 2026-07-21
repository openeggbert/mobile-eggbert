// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/HttpResponseHeaders.hpp"

using namespace System::Net::Http::Headers;

TEST(HttpResponseHeadersTests, AcceptRanges_AddThenGet) {
    HttpResponseHeaders h;
    h.AddAcceptRanges("bytes");
    auto values = h.getAcceptRangesProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0], "bytes");
}

TEST(HttpResponseHeadersTests, Age_RoundTrips) {
    HttpResponseHeaders h;
    EXPECT_FALSE(h.getAgeProperty().has_value());
    h.setAgeProperty(System::TimeSpan::FromSeconds(120));
    auto result = h.getAgeProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result->getTotalSecondsProperty(), 120.0);
}

TEST(HttpResponseHeadersTests, Age_Invalid_ReturnsNullopt) {
    HttpResponseHeaders h;
    h.Add("Age", "not-a-number");
    EXPECT_FALSE(h.getAgeProperty().has_value());
}

// Regression test for ticket 318: real .NET's Age header uses TimeSpanHeaderParser, whose
// HttpRuleParser.GetNumberLength counts a run of pure DIGIT characters (delta-seconds =
// 1*DIGIT grammar, no sign allowed at all). getAgeProperty used std::stoll directly, whose
// leading-sign tolerance let "+5" through -- the same bug class already fixed in
// CacheControlHeaderValue::tryParseSeconds for max-age/s-maxage/min-fresh.
TEST(HttpResponseHeadersTests, Age_LeadingPlusSign_ReturnsNullopt) {
    HttpResponseHeaders h;
    h.Add("Age", "+5");
    EXPECT_FALSE(h.getAgeProperty().has_value());
}
TEST(HttpResponseHeadersTests, Age_LeadingMinusSign_ReturnsNullopt) {
    HttpResponseHeaders h;
    h.Add("Age", "-5");
    EXPECT_FALSE(h.getAgeProperty().has_value());
}

TEST(HttpResponseHeadersTests, ETag_RoundTrips) {
    HttpResponseHeaders h;
    h.setETagProperty(EntityTagHeaderValue("\"abc123\""));
    auto result = h.getETagProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getTagProperty(), "\"abc123\"");
}

TEST(HttpResponseHeadersTests, Location_RoundTrips) {
    HttpResponseHeaders h;
    h.setLocationProperty(System::Uri("http://example.com/new"));
    auto result = h.getLocationProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->ToString(), "http://example.com/new");
}

TEST(HttpResponseHeadersTests, ProxyAuthenticate_AddThenGet) {
    HttpResponseHeaders h;
    h.AddProxyAuthenticate(AuthenticationHeaderValue("Basic"));
    auto values = h.getProxyAuthenticateProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getSchemeProperty(), "Basic");
}

TEST(HttpResponseHeadersTests, RetryAfter_Date_RoundTrips) {
    HttpResponseHeaders h;
    System::DateTimeOffset dto(2015, 10, 21, 7, 28, 0, System::TimeSpan::Zero);
    h.setRetryAfterProperty(RetryConditionHeaderValue(dto));
    auto result = h.getRetryAfterProperty();
    ASSERT_TRUE(result.has_value());
}

TEST(HttpResponseHeadersTests, RetryAfter_Delta_RoundTrips) {
    HttpResponseHeaders h;
    h.setRetryAfterProperty(RetryConditionHeaderValue(System::TimeSpan::FromSeconds(120)));
    auto result = h.getRetryAfterProperty();
    ASSERT_TRUE(result.has_value());
}

TEST(HttpResponseHeadersTests, Server_AddThenGet) {
    HttpResponseHeaders h;
    h.AddServer(ProductInfoHeaderValue("MyServer", "2.0"));
    auto values = h.getServerProperty();
    ASSERT_EQ(values.size(), 1u);
    ASSERT_TRUE(values[0].getProductProperty().has_value());
    EXPECT_EQ(values[0].getProductProperty()->getNameProperty(), "MyServer");
}

TEST(HttpResponseHeadersTests, Vary_AddThenGet) {
    HttpResponseHeaders h;
    h.AddVary("Accept-Encoding");
    auto values = h.getVaryProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0], "Accept-Encoding");
}

TEST(HttpResponseHeadersTests, WwwAuthenticate_AddThenGet) {
    HttpResponseHeaders h;
    h.AddWwwAuthenticate(AuthenticationHeaderValue("Bearer", "realm=\"example\""));
    auto values = h.getWwwAuthenticateProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getSchemeProperty(), "Bearer");
}

// --- General headers ---------------------------------------------------------------------

TEST(HttpResponseHeadersTests, CacheControl_RoundTrips) {
    HttpResponseHeaders h;
    CacheControlHeaderValue cc;
    cc.setPublicProperty(true);
    h.setCacheControlProperty(cc);
    auto result = h.getCacheControlProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->getPublicProperty());
}

TEST(HttpResponseHeadersTests, Connection_AddThenGet) {
    HttpResponseHeaders h;
    h.AddConnection("keep-alive");
    auto values = h.getConnectionProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0], "keep-alive");
}

TEST(HttpResponseHeadersTests, ConnectionClose_SetTrue_AddsCloseToken) {
    HttpResponseHeaders h;
    EXPECT_FALSE(h.getConnectionCloseProperty().has_value());
    h.setConnectionCloseProperty(true);
    ASSERT_TRUE(h.getConnectionCloseProperty().has_value());
    EXPECT_TRUE(*h.getConnectionCloseProperty());
}

TEST(HttpResponseHeadersTests, ConnectionClose_SetFalse_RemovesCloseToken) {
    HttpResponseHeaders h;
    h.AddConnection("keep-alive");
    h.setConnectionCloseProperty(true);
    h.setConnectionCloseProperty(false);
    EXPECT_FALSE(*h.getConnectionCloseProperty());
    auto tokens = h.getConnectionProperty();
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], "keep-alive");
}

TEST(HttpResponseHeadersTests, Date_RoundTrips) {
    HttpResponseHeaders h;
    System::DateTimeOffset dto(2015, 10, 21, 7, 28, 0, System::TimeSpan::Zero);
    h.setDateProperty(dto);
    auto result = h.getDateProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->ToString("r"), "Wed, 21 Oct 2015 07:28:00 GMT");
}

TEST(HttpResponseHeadersTests, Pragma_AddThenGet) {
    HttpResponseHeaders h;
    h.AddPragma(NameValueHeaderValue("no-cache"));
    auto values = h.getPragmaProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getNameProperty(), "no-cache");
}

TEST(HttpResponseHeadersTests, Trailer_AddThenGet) {
    HttpResponseHeaders h;
    h.AddTrailer("X-Checksum");
    auto values = h.getTrailerProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0], "X-Checksum");
}

TEST(HttpResponseHeadersTests, TransferEncoding_AddThenGet) {
    HttpResponseHeaders h;
    h.AddTransferEncoding(TransferCodingHeaderValue("gzip"));
    auto values = h.getTransferEncodingProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getValueProperty(), "gzip");
}

TEST(HttpResponseHeadersTests, TransferEncodingChunked_SetTrue_AddsChunkedToken) {
    HttpResponseHeaders h;
    EXPECT_FALSE(h.getTransferEncodingChunkedProperty().has_value());
    h.setTransferEncodingChunkedProperty(true);
    ASSERT_TRUE(h.getTransferEncodingChunkedProperty().has_value());
    EXPECT_TRUE(*h.getTransferEncodingChunkedProperty());
}

TEST(HttpResponseHeadersTests, TransferEncodingChunked_SetFalse_RemovesChunkedToken) {
    HttpResponseHeaders h;
    h.AddTransferEncoding(TransferCodingHeaderValue("gzip"));
    h.setTransferEncodingChunkedProperty(true);
    h.setTransferEncodingChunkedProperty(false);
    EXPECT_FALSE(*h.getTransferEncodingChunkedProperty());
    auto values = h.getTransferEncodingProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getValueProperty(), "gzip");
}

TEST(HttpResponseHeadersTests, Upgrade_AddThenGet) {
    HttpResponseHeaders h;
    h.AddUpgrade(ProductHeaderValue("websocket"));
    auto values = h.getUpgradeProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getNameProperty(), "websocket");
}

TEST(HttpResponseHeadersTests, Via_AddThenGet) {
    HttpResponseHeaders h;
    h.AddVia(ViaHeaderValue("1.1", "proxy.example.com"));
    auto values = h.getViaProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getReceivedByProperty(), "proxy.example.com");
}

TEST(HttpResponseHeadersTests, Warning_AddThenGet) {
    HttpResponseHeaders h;
    h.AddWarning(WarningHeaderValue(110, "anderson", "\"Response is stale\""));
    auto values = h.getWarningProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getCodeProperty(), 110);
}
