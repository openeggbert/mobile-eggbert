// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/HttpRequestHeaders.hpp"
#include "System/FormatException.hpp"

using namespace System::Net::Http::Headers;

TEST(HttpRequestHeadersTests, Accept_AddThenGet) {
    HttpRequestHeaders h;
    h.AddAccept(MediaTypeWithQualityHeaderValue("text/html", 0.9));
    h.AddAccept(MediaTypeWithQualityHeaderValue("application/json"));
    auto values = h.getAcceptProperty();
    ASSERT_EQ(values.size(), 2u);
    EXPECT_EQ(values[0].getMediaTypeProperty(), "text/html");
    ASSERT_TRUE(values[0].getQualityProperty().has_value());
    EXPECT_DOUBLE_EQ(*values[0].getQualityProperty(), 0.9);
    EXPECT_EQ(values[1].getMediaTypeProperty(), "application/json");
}

TEST(HttpRequestHeadersTests, AcceptCharset_AddThenGet) {
    HttpRequestHeaders h;
    h.AddAcceptCharset(StringWithQualityHeaderValue("utf-8"));
    auto values = h.getAcceptCharsetProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getValueProperty(), "utf-8");
}

TEST(HttpRequestHeadersTests, AcceptEncoding_AddThenGet) {
    HttpRequestHeaders h;
    h.AddAcceptEncoding(StringWithQualityHeaderValue("gzip"));
    h.AddAcceptEncoding(StringWithQualityHeaderValue("deflate"));
    auto values = h.getAcceptEncodingProperty();
    ASSERT_EQ(values.size(), 2u);
}

TEST(HttpRequestHeadersTests, AcceptLanguage_AddThenGet) {
    HttpRequestHeaders h;
    h.AddAcceptLanguage(StringWithQualityHeaderValue("en-US"));
    auto values = h.getAcceptLanguageProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getValueProperty(), "en-US");
}

TEST(HttpRequestHeadersTests, Authorization_RoundTrips) {
    HttpRequestHeaders h;
    EXPECT_FALSE(h.getAuthorizationProperty().has_value());
    h.setAuthorizationProperty(AuthenticationHeaderValue("Bearer", "abc123"));
    auto result = h.getAuthorizationProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getSchemeProperty(), "Bearer");
    EXPECT_EQ(result->getParameterProperty(), "abc123");
}

TEST(HttpRequestHeadersTests, ExpectContinue_NeverSet_ReturnsNullopt) {
    HttpRequestHeaders h;
    EXPECT_FALSE(h.getExpectContinueProperty().has_value());
}

TEST(HttpRequestHeadersTests, ExpectContinue_SetTrue_AddsToExpect) {
    HttpRequestHeaders h;
    h.setExpectContinueProperty(true);
    ASSERT_TRUE(h.getExpectContinueProperty().has_value());
    EXPECT_TRUE(*h.getExpectContinueProperty());
}

TEST(HttpRequestHeadersTests, ExpectContinue_SetFalse_ReturnsFalse) {
    HttpRequestHeaders h;
    h.setExpectContinueProperty(true);
    h.setExpectContinueProperty(false);
    ASSERT_TRUE(h.getExpectContinueProperty().has_value());
    EXPECT_FALSE(*h.getExpectContinueProperty());
}

TEST(HttpRequestHeadersTests, Expect_AddThenGet) {
    HttpRequestHeaders h;
    h.AddExpect(NameValueWithParametersHeaderValue("trailers"));
    auto values = h.getExpectProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getNameProperty(), "trailers");
}

TEST(HttpRequestHeadersTests, From_RoundTrips) {
    HttpRequestHeaders h;
    h.setFromProperty(std::string("user@example.com"));
    ASSERT_TRUE(h.getFromProperty().has_value());
    EXPECT_EQ(*h.getFromProperty(), "user@example.com");
}

TEST(HttpRequestHeadersTests, From_ContainsNewline_Throws) {
    HttpRequestHeaders h;
    EXPECT_THROW(h.setFromProperty(std::string("bad\r\nvalue")), System::FormatException);
}

TEST(HttpRequestHeadersTests, Host_RoundTrips) {
    HttpRequestHeaders h;
    h.setHostProperty(std::string("example.com:8080"));
    ASSERT_TRUE(h.getHostProperty().has_value());
    EXPECT_EQ(*h.getHostProperty(), "example.com:8080");
}

TEST(HttpRequestHeadersTests, Host_ContainsWhitespace_Throws) {
    HttpRequestHeaders h;
    EXPECT_THROW(h.setHostProperty(std::string("bad host")), System::FormatException);
}

// Regression test (ticket 266): isValidHost() didn't check for an embedded NUL byte, unlike
// the checkNoNewlineOrNul() helper this same file uses for From/Protocol -- an inconsistency,
// not a deliberate choice, since a literal NUL is never valid in any HTTP header value.
TEST(HttpRequestHeadersTests, Host_ContainsNulByte_Throws) {
    HttpRequestHeaders h;
    std::string badHost("exa\0mple.com", 12);
    EXPECT_THROW(h.setHostProperty(badHost), System::FormatException);
}

TEST(HttpRequestHeadersTests, IfMatch_AddThenGet) {
    HttpRequestHeaders h;
    h.AddIfMatch(EntityTagHeaderValue("\"abc\""));
    auto values = h.getIfMatchProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getTagProperty(), "\"abc\"");
}

TEST(HttpRequestHeadersTests, IfModifiedSince_RoundTrips) {
    HttpRequestHeaders h;
    System::DateTimeOffset dto(2015, 10, 21, 7, 28, 0, System::TimeSpan::Zero);
    h.setIfModifiedSinceProperty(dto);
    auto result = h.getIfModifiedSinceProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->ToString("r"), "Wed, 21 Oct 2015 07:28:00 GMT");
}

TEST(HttpRequestHeadersTests, IfNoneMatch_AddThenGet) {
    HttpRequestHeaders h;
    h.AddIfNoneMatch(EntityTagHeaderValue::Any());
    auto values = h.getIfNoneMatchProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].ToString(), "*");
}

TEST(HttpRequestHeadersTests, IfRange_RoundTrips) {
    HttpRequestHeaders h;
    h.setIfRangeProperty(RangeConditionHeaderValue(EntityTagHeaderValue("\"abc\"")));
    auto result = h.getIfRangeProperty();
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->getEntityTagProperty().has_value());
    EXPECT_EQ(result->getEntityTagProperty()->getTagProperty(), "\"abc\"");
}

TEST(HttpRequestHeadersTests, IfUnmodifiedSince_RoundTrips) {
    HttpRequestHeaders h;
    System::DateTimeOffset dto(2020, 1, 2, 3, 4, 5, System::TimeSpan::Zero);
    h.setIfUnmodifiedSinceProperty(dto);
    auto result = h.getIfUnmodifiedSinceProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->ToString("r"), "Thu, 02 Jan 2020 03:04:05 GMT");
}

TEST(HttpRequestHeadersTests, MaxForwards_RoundTrips) {
    HttpRequestHeaders h;
    h.setMaxForwardsProperty(10);
    ASSERT_TRUE(h.getMaxForwardsProperty().has_value());
    EXPECT_EQ(*h.getMaxForwardsProperty(), 10);
}

TEST(HttpRequestHeadersTests, Protocol_RoundTrips) {
    HttpRequestHeaders h;
    EXPECT_FALSE(h.getProtocolProperty().has_value());
    h.setProtocolProperty(std::string("websocket"));
    ASSERT_TRUE(h.getProtocolProperty().has_value());
    EXPECT_EQ(*h.getProtocolProperty(), "websocket");
    EXPECT_FALSE(h.Contains("Protocol"));
}

TEST(HttpRequestHeadersTests, ProxyAuthorization_RoundTrips) {
    HttpRequestHeaders h;
    h.setProxyAuthorizationProperty(AuthenticationHeaderValue("Basic", "xyz"));
    auto result = h.getProxyAuthorizationProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getSchemeProperty(), "Basic");
}

TEST(HttpRequestHeadersTests, Range_RoundTrips) {
    HttpRequestHeaders h;
    h.setRangeProperty(RangeHeaderValue(0, 499));
    auto result = h.getRangeProperty();
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->getRangesProperty().size(), 1u);
}

TEST(HttpRequestHeadersTests, Referrer_RoundTrips) {
    HttpRequestHeaders h;
    h.setReferrerProperty(System::Uri("http://example.com/page"));
    auto result = h.getReferrerProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->ToString(), "http://example.com/page");
}

TEST(HttpRequestHeadersTests, TE_AddThenGet) {
    HttpRequestHeaders h;
    h.AddTE(TransferCodingWithQualityHeaderValue("trailers"));
    auto values = h.getTEProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getValueProperty(), "trailers");
}

TEST(HttpRequestHeadersTests, UserAgent_AddThenGet) {
    HttpRequestHeaders h;
    h.AddUserAgent(ProductInfoHeaderValue("MyClient", "1.0"));
    h.AddUserAgent(ProductInfoHeaderValue("(compatible)"));
    auto values = h.getUserAgentProperty();
    ASSERT_EQ(values.size(), 2u);
    ASSERT_TRUE(values[0].getProductProperty().has_value());
    EXPECT_EQ(values[0].getProductProperty()->getNameProperty(), "MyClient");
    EXPECT_EQ(values[1].getCommentProperty(), "(compatible)");
}

// --- General headers ---------------------------------------------------------------------

TEST(HttpRequestHeadersTests, CacheControl_RoundTrips) {
    HttpRequestHeaders h;
    CacheControlHeaderValue cc;
    cc.setNoCacheProperty(true);
    h.setCacheControlProperty(cc);
    auto result = h.getCacheControlProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->getNoCacheProperty());
}

TEST(HttpRequestHeadersTests, Connection_AddThenGet) {
    HttpRequestHeaders h;
    h.AddConnection("keep-alive");
    auto values = h.getConnectionProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0], "keep-alive");
}

TEST(HttpRequestHeadersTests, ConnectionClose_SetTrue_AddsCloseToken) {
    HttpRequestHeaders h;
    EXPECT_FALSE(h.getConnectionCloseProperty().has_value());
    h.setConnectionCloseProperty(true);
    ASSERT_TRUE(h.getConnectionCloseProperty().has_value());
    EXPECT_TRUE(*h.getConnectionCloseProperty());
    auto tokens = h.getConnectionProperty();
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], "close");
}

TEST(HttpRequestHeadersTests, ConnectionClose_SetFalse_RemovesCloseToken) {
    HttpRequestHeaders h;
    h.AddConnection("keep-alive");
    h.setConnectionCloseProperty(true);
    h.setConnectionCloseProperty(false);
    EXPECT_FALSE(*h.getConnectionCloseProperty());
    auto tokens = h.getConnectionProperty();
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], "keep-alive");
}

TEST(HttpRequestHeadersTests, Date_RoundTrips) {
    HttpRequestHeaders h;
    System::DateTimeOffset dto(2015, 10, 21, 7, 28, 0, System::TimeSpan::Zero);
    h.setDateProperty(dto);
    auto result = h.getDateProperty();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->ToString("r"), "Wed, 21 Oct 2015 07:28:00 GMT");
}

TEST(HttpRequestHeadersTests, Pragma_AddThenGet) {
    HttpRequestHeaders h;
    h.AddPragma(NameValueHeaderValue("no-cache"));
    auto values = h.getPragmaProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getNameProperty(), "no-cache");
}

TEST(HttpRequestHeadersTests, Trailer_AddThenGet) {
    HttpRequestHeaders h;
    h.AddTrailer("X-Checksum");
    auto values = h.getTrailerProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0], "X-Checksum");
}

TEST(HttpRequestHeadersTests, TransferEncoding_AddThenGet) {
    HttpRequestHeaders h;
    h.AddTransferEncoding(TransferCodingHeaderValue("gzip"));
    auto values = h.getTransferEncodingProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getValueProperty(), "gzip");
}

TEST(HttpRequestHeadersTests, TransferEncodingChunked_SetTrue_AddsChunkedToken) {
    HttpRequestHeaders h;
    EXPECT_FALSE(h.getTransferEncodingChunkedProperty().has_value());
    h.setTransferEncodingChunkedProperty(true);
    ASSERT_TRUE(h.getTransferEncodingChunkedProperty().has_value());
    EXPECT_TRUE(*h.getTransferEncodingChunkedProperty());
}

TEST(HttpRequestHeadersTests, TransferEncodingChunked_SetFalse_RemovesChunkedToken) {
    HttpRequestHeaders h;
    h.AddTransferEncoding(TransferCodingHeaderValue("gzip"));
    h.setTransferEncodingChunkedProperty(true);
    h.setTransferEncodingChunkedProperty(false);
    EXPECT_FALSE(*h.getTransferEncodingChunkedProperty());
    auto values = h.getTransferEncodingProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getValueProperty(), "gzip");
}

TEST(HttpRequestHeadersTests, Upgrade_AddThenGet) {
    HttpRequestHeaders h;
    h.AddUpgrade(ProductHeaderValue("websocket"));
    auto values = h.getUpgradeProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getNameProperty(), "websocket");
}

TEST(HttpRequestHeadersTests, Via_AddThenGet) {
    HttpRequestHeaders h;
    h.AddVia(ViaHeaderValue("1.1", "proxy.example.com"));
    auto values = h.getViaProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getReceivedByProperty(), "proxy.example.com");
}

TEST(HttpRequestHeadersTests, Warning_AddThenGet) {
    HttpRequestHeaders h;
    h.AddWarning(WarningHeaderValue(110, "anderson", "\"Response is stale\""));
    auto values = h.getWarningProperty();
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].getCodeProperty(), 110);
}
