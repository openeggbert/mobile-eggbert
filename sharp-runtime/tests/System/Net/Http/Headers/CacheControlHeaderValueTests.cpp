// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/CacheControlHeaderValue.hpp"
#include "System/FormatException.hpp"

using System::Net::Http::Headers::CacheControlHeaderValue;

TEST(CacheControlHeaderValueTests, DefaultConstructor_AllFalseAndEmpty) {
    CacheControlHeaderValue v;
    EXPECT_FALSE(v.getNoCacheProperty());
    EXPECT_FALSE(v.getNoStoreProperty());
    EXPECT_FALSE(v.getPublicProperty());
    EXPECT_FALSE(v.getMaxAgeProperty().has_value());
    EXPECT_TRUE(v.getNoCacheHeadersProperty().empty());
    EXPECT_TRUE(v.getExtensionsProperty().empty());
    EXPECT_EQ(v.ToString(), "");
}

TEST(CacheControlHeaderValueTests, ToString_SimpleFlags) {
    CacheControlHeaderValue v;
    v.setNoStoreProperty(true);
    v.setPublicProperty(true);
    EXPECT_EQ(v.ToString(), "no-store, public");
}

TEST(CacheControlHeaderValueTests, ToString_MaxAge) {
    CacheControlHeaderValue v;
    v.setMaxAgeProperty(3600);
    EXPECT_EQ(v.ToString(), "max-age=3600");
}

TEST(CacheControlHeaderValueTests, ToString_NoCacheWithHeaders) {
    CacheControlHeaderValue v;
    v.setNoCacheProperty(true);
    v.getNoCacheHeadersProperty().push_back("Set-Cookie");
    EXPECT_EQ(v.ToString(), "no-cache=\"Set-Cookie\"");
}

TEST(CacheControlHeaderValueTests, ToString_MaxStaleNoLimit) {
    CacheControlHeaderValue v;
    v.setMaxStaleProperty(true);
    EXPECT_EQ(v.ToString(), "max-stale");
}

TEST(CacheControlHeaderValueTests, ToString_MaxStaleWithLimit) {
    CacheControlHeaderValue v;
    v.setMaxStaleProperty(true);
    v.setMaxStaleLimitProperty(60);
    EXPECT_EQ(v.ToString(), "max-stale=60");
}

TEST(CacheControlHeaderValueTests, ToString_FullOrdering) {
    CacheControlHeaderValue v;
    v.setNoStoreProperty(true);
    v.setNoTransformProperty(true);
    v.setOnlyIfCachedProperty(true);
    v.setPublicProperty(true);
    v.setMustRevalidateProperty(true);
    v.setProxyRevalidateProperty(true);
    v.setNoCacheProperty(true);
    v.setMaxAgeProperty(100);
    v.setSharedMaxAgeProperty(200);
    v.setMaxStaleProperty(true);
    v.setMinFreshProperty(50);
    v.setPrivateProperty(true);
    EXPECT_EQ(v.ToString(),
        "no-store, no-transform, only-if-cached, public, must-revalidate, proxy-revalidate, "
        "no-cache, max-age=100, s-maxage=200, max-stale, min-fresh=50, private");
}

TEST(CacheControlHeaderValueTests, Parse_SimpleFlags) {
    auto v = CacheControlHeaderValue::Parse("no-store, public");
    EXPECT_TRUE(v.getNoStoreProperty());
    EXPECT_TRUE(v.getPublicProperty());
    EXPECT_FALSE(v.getNoCacheProperty());
}

TEST(CacheControlHeaderValueTests, Parse_MaxAge) {
    auto v = CacheControlHeaderValue::Parse("max-age=3600");
    ASSERT_TRUE(v.getMaxAgeProperty().has_value());
    EXPECT_EQ(*v.getMaxAgeProperty(), 3600);
}

TEST(CacheControlHeaderValueTests, Parse_NoCacheWithQuotedHeaderList) {
    auto v = CacheControlHeaderValue::Parse("no-cache=\"Set-Cookie, X-Foo\", max-age=3600");
    EXPECT_TRUE(v.getNoCacheProperty());
    ASSERT_EQ(v.getNoCacheHeadersProperty().size(), 2u);
    EXPECT_EQ(v.getNoCacheHeadersProperty()[0], "Set-Cookie");
    EXPECT_EQ(v.getNoCacheHeadersProperty()[1], "X-Foo");
    ASSERT_TRUE(v.getMaxAgeProperty().has_value());
    EXPECT_EQ(*v.getMaxAgeProperty(), 3600);
}

TEST(CacheControlHeaderValueTests, Parse_UnknownDirective_BecomesExtension) {
    auto v = CacheControlHeaderValue::Parse("community=\"UCI\"");
    ASSERT_EQ(v.getExtensionsProperty().size(), 1u);
    EXPECT_EQ(v.getExtensionsProperty()[0].getNameProperty(), "community");
    EXPECT_EQ(v.getExtensionsProperty()[0].getValueProperty(), "\"UCI\"");
}

TEST(CacheControlHeaderValueTests, Parse_CaseInsensitiveDirectiveNames) {
    auto v = CacheControlHeaderValue::Parse("NO-STORE, MAX-AGE=10");
    EXPECT_TRUE(v.getNoStoreProperty());
    ASSERT_TRUE(v.getMaxAgeProperty().has_value());
    EXPECT_EQ(*v.getMaxAgeProperty(), 10);
}

TEST(CacheControlHeaderValueTests, Parse_RoundTripsThroughToString) {
    auto v = CacheControlHeaderValue::Parse("no-store, public, max-age=100");
    auto v2 = CacheControlHeaderValue::Parse(v.ToString());
    EXPECT_TRUE(v == v2);
}

TEST(CacheControlHeaderValueTests, Parse_Empty_ReturnsAllDefaults) {
    auto v = CacheControlHeaderValue::Parse("");
    EXPECT_FALSE(v.getNoStoreProperty());
    EXPECT_TRUE(v.getExtensionsProperty().empty());
}

TEST(CacheControlHeaderValueTests, Parse_MaxAgeMissingValue_Throws) {
    EXPECT_THROW(CacheControlHeaderValue::Parse("max-age"), System::FormatException);
}

TEST(CacheControlHeaderValueTests, Parse_MaxAgeNonNumeric_Throws) {
    EXPECT_THROW(CacheControlHeaderValue::Parse("max-age=abc"), System::FormatException);
}

// Regression tests for a wave-3 audit finding: tryParseSeconds used std::stol (accepting a
// leading '-') then narrowed the result to intcs with no range check, silently wrapping an
// out-of-range value instead of failing to parse. Verified against HeaderUtilities.cs's
// TryParseInt32, which delegates to int.TryParse(value, NumberStyles.None, ...) --
// NumberStyles.None requires an unsigned, sign-free digit string and fails on overflow.
TEST(CacheControlHeaderValueTests, Parse_MaxAgeOverflowsIntcs_Throws) {
    EXPECT_THROW(CacheControlHeaderValue::Parse("max-age=99999999999"), System::FormatException);
}

TEST(CacheControlHeaderValueTests, TryParse_MaxAgeOverflowsIntcs_ReturnsFalse) {
    CacheControlHeaderValue result;
    EXPECT_FALSE(CacheControlHeaderValue::TryParse("max-age=99999999999", result));
}

TEST(CacheControlHeaderValueTests, Parse_MaxAgeNegative_Throws) {
    EXPECT_THROW(CacheControlHeaderValue::Parse("max-age=-5"), System::FormatException);
}

TEST(CacheControlHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    CacheControlHeaderValue result;
    EXPECT_FALSE(CacheControlHeaderValue::TryParse("max-age=abc", result));
}

// Regression tests for ticket 314: TryParse rejected any empty list element (leading, trailing,
// or a run of consecutive commas between real directives) as a hard parse failure. Verified
// against real .NET's actual parsing chain (CacheControlHeaderValue.cs's
// MultipleValueNameValueParser -> BaseHeaderParser.TryParseValue ->
// HeaderUtilities.GetNextNonEmptyOrWhitespaceIndex, which has supportsMultipleValues=true and
// therefore loops to silently skip any run of ,-separated empty elements) -- matches RFC 7230's
// #rule ABNF, where empty list elements are explicitly ignorable, not an error.
TEST(CacheControlHeaderValueTests, TryParse_TrailingComma_Succeeds) {
    CacheControlHeaderValue result;
    ASSERT_TRUE(CacheControlHeaderValue::TryParse("no-cache,", result));
    EXPECT_TRUE(result.getNoCacheProperty());
}
TEST(CacheControlHeaderValueTests, TryParse_LeadingComma_Succeeds) {
    CacheControlHeaderValue result;
    ASSERT_TRUE(CacheControlHeaderValue::TryParse(",no-cache", result));
    EXPECT_TRUE(result.getNoCacheProperty());
}
TEST(CacheControlHeaderValueTests, TryParse_DoubleCommaBetweenDirectives_Succeeds) {
    CacheControlHeaderValue result;
    ASSERT_TRUE(CacheControlHeaderValue::TryParse("no-cache,,no-store", result));
    EXPECT_TRUE(result.getNoCacheProperty());
    EXPECT_TRUE(result.getNoStoreProperty());
}
TEST(CacheControlHeaderValueTests, TryParse_AllCommasNoDirectives_ReturnsDefaults) {
    CacheControlHeaderValue result;
    ASSERT_TRUE(CacheControlHeaderValue::TryParse(",,,", result));
    EXPECT_FALSE(result.getNoCacheProperty());
    EXPECT_FALSE(result.getNoStoreProperty());
}

TEST(CacheControlHeaderValueTests, Equals_SameFlags) {
    auto a = CacheControlHeaderValue::Parse("no-store, max-age=100");
    auto b = CacheControlHeaderValue::Parse("no-store, max-age=100");
    EXPECT_TRUE(a == b);
}

TEST(CacheControlHeaderValueTests, Equals_DifferentMaxAge_False) {
    auto a = CacheControlHeaderValue::Parse("max-age=100");
    auto b = CacheControlHeaderValue::Parse("max-age=200");
    EXPECT_FALSE(a == b);
}

TEST(CacheControlHeaderValueTests, Equals_TokenListsOrderIndependent) {
    auto a = CacheControlHeaderValue::Parse("no-cache=\"A, B\"");
    auto b = CacheControlHeaderValue::Parse("no-cache=\"B, A\"");
    EXPECT_TRUE(a == b);
}

TEST(CacheControlHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    auto a = CacheControlHeaderValue::Parse("no-store, max-age=100");
    auto b = CacheControlHeaderValue::Parse("no-store, max-age=100");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}
