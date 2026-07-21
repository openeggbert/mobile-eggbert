// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/ContentRangeHeaderValue.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"

using System::Net::Http::Headers::ContentRangeHeaderValue;

TEST(ContentRangeHeaderValueTests, Constructor_FromToLength_ToString) {
    ContentRangeHeaderValue v(12, 34, 5678);
    EXPECT_EQ(v.getUnitProperty(), "bytes");
    EXPECT_TRUE(v.getHasRangeProperty());
    EXPECT_TRUE(v.getHasLengthProperty());
    EXPECT_EQ(*v.getFromProperty(), 12);
    EXPECT_EQ(*v.getToProperty(), 34);
    EXPECT_EQ(*v.getLengthProperty(), 5678);
    EXPECT_EQ(v.ToString(), "bytes 12-34/5678");
}

TEST(ContentRangeHeaderValueTests, Constructor_LengthOnly_ToString) {
    ContentRangeHeaderValue v(1234);
    EXPECT_FALSE(v.getHasRangeProperty());
    EXPECT_TRUE(v.getHasLengthProperty());
    EXPECT_EQ(v.ToString(), "bytes */1234");
}

TEST(ContentRangeHeaderValueTests, Constructor_FromToOnly_ToString) {
    ContentRangeHeaderValue v(12, 34);
    EXPECT_TRUE(v.getHasRangeProperty());
    EXPECT_FALSE(v.getHasLengthProperty());
    EXPECT_EQ(v.ToString(), "bytes 12-34/*");
}

// Regression tests for a wave-3 audit finding: these threw std::out_of_range (an unrelated
// std:: exception type invisible to code catching System::Exception&) instead of
// System::ArgumentOutOfRangeException, which is what real .NET's ContentRangeHeaderValue
// constructors throw (via ArgumentOutOfRangeException.ThrowIfNegative/ThrowIfGreaterThan) for
// these exact validation failures.
TEST(ContentRangeHeaderValueTests, Constructor_ToGreaterThanLength_Throws) {
    EXPECT_THROW(ContentRangeHeaderValue(12, 5000, 100), System::ArgumentOutOfRangeException);
}

TEST(ContentRangeHeaderValueTests, Constructor_FromGreaterThanTo_Throws) {
    EXPECT_THROW(ContentRangeHeaderValue(34, 12, 100), System::ArgumentOutOfRangeException);
    EXPECT_THROW(ContentRangeHeaderValue(34, 12), System::ArgumentOutOfRangeException);
}

TEST(ContentRangeHeaderValueTests, Constructor_NegativeLength_Throws) {
    EXPECT_THROW(ContentRangeHeaderValue(-1), System::ArgumentOutOfRangeException);
}

TEST(ContentRangeHeaderValueTests, Parse_FullForm) {
    auto v = ContentRangeHeaderValue::Parse("bytes 12-34/5678");
    EXPECT_EQ(*v.getFromProperty(), 12);
    EXPECT_EQ(*v.getToProperty(), 34);
    EXPECT_EQ(*v.getLengthProperty(), 5678);
}

TEST(ContentRangeHeaderValueTests, Parse_UnknownLength) {
    auto v = ContentRangeHeaderValue::Parse("bytes 12-34/*");
    EXPECT_TRUE(v.getHasRangeProperty());
    EXPECT_FALSE(v.getHasLengthProperty());
}

TEST(ContentRangeHeaderValueTests, Parse_UnknownRange) {
    auto v = ContentRangeHeaderValue::Parse("bytes */1234");
    EXPECT_FALSE(v.getHasRangeProperty());
    EXPECT_TRUE(v.getHasLengthProperty());
    EXPECT_EQ(*v.getLengthProperty(), 1234);
}

TEST(ContentRangeHeaderValueTests, Parse_RoundTripsThroughToString) {
    auto v = ContentRangeHeaderValue::Parse("bytes 12-34/5678");
    auto v2 = ContentRangeHeaderValue::Parse(v.ToString());
    EXPECT_TRUE(v == v2);
}

TEST(ContentRangeHeaderValueTests, Parse_ToExceedsLength_Throws) {
    EXPECT_THROW(ContentRangeHeaderValue::Parse("bytes 12-9999/100"), System::FormatException);
}

TEST(ContentRangeHeaderValueTests, Parse_MissingSlash_Throws) {
    EXPECT_THROW(ContentRangeHeaderValue::Parse("bytes 12-34"), System::FormatException);
}

TEST(ContentRangeHeaderValueTests, Parse_InvalidUnit_Throws) {
    EXPECT_THROW(ContentRangeHeaderValue::Parse("bad unit 12-34/5678"), System::FormatException);
}

TEST(ContentRangeHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    ContentRangeHeaderValue result(100);
    EXPECT_FALSE(ContentRangeHeaderValue::TryParse("garbage", result));
}

TEST(ContentRangeHeaderValueTests, Equals_SameValues) {
    auto a = ContentRangeHeaderValue::Parse("bytes 12-34/5678");
    auto b = ContentRangeHeaderValue::Parse("bytes 12-34/5678");
    EXPECT_TRUE(a == b);
}

TEST(ContentRangeHeaderValueTests, Equals_UnitCaseInsensitive) {
    ContentRangeHeaderValue a(1, 2, 100);
    ContentRangeHeaderValue b(1, 2, 100);
    b.setUnitProperty("BYTES");
    EXPECT_TRUE(a == b);
}

TEST(ContentRangeHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    auto a = ContentRangeHeaderValue::Parse("bytes 12-34/5678");
    auto b = ContentRangeHeaderValue::Parse("bytes 12-34/5678");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}
