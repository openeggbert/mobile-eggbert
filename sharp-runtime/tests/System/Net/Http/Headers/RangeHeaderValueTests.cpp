// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/RangeHeaderValue.hpp"
#include "System/Net/Http/Headers/RangeItemHeaderValue.hpp"
#include "System/FormatException.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using System::Net::Http::Headers::RangeHeaderValue;
using System::Net::Http::Headers::RangeItemHeaderValue;

// ---------------------------------------------------------------------------
// RangeItemHeaderValue
// ---------------------------------------------------------------------------

TEST(RangeItemHeaderValueTests, Constructor_FromTo_ToString) {
    RangeItemHeaderValue v(1, 2);
    EXPECT_EQ(*v.getFromProperty(), 1);
    EXPECT_EQ(*v.getToProperty(), 2);
    EXPECT_EQ(v.ToString(), "1-2");
}

TEST(RangeItemHeaderValueTests, Constructor_FromOnly_ToString) {
    RangeItemHeaderValue v(500, std::nullopt);
    EXPECT_EQ(v.ToString(), "500-");
}

TEST(RangeItemHeaderValueTests, Constructor_ToOnly_ToString) {
    RangeItemHeaderValue v(std::nullopt, 500);
    EXPECT_EQ(v.ToString(), "-500");
}

TEST(RangeItemHeaderValueTests, Constructor_NeitherSet_Throws) {
    EXPECT_THROW(RangeItemHeaderValue(std::nullopt, std::nullopt), System::ArgumentException);
}

// Regression test for a wave-3 audit finding: threw std::out_of_range (an unrelated std::
// exception type invisible to code catching System::Exception&) instead of
// System::ArgumentOutOfRangeException, which is what real .NET's RangeItemHeaderValue
// constructor throws (via ArgumentOutOfRangeException.ThrowIfGreaterThan) when from > to.
TEST(RangeItemHeaderValueTests, Constructor_FromGreaterThanTo_Throws) {
    EXPECT_THROW(RangeItemHeaderValue(5, 2), System::ArgumentOutOfRangeException);
}

TEST(RangeItemHeaderValueTests, Equals_SameValues) {
    RangeItemHeaderValue a(1, 2);
    RangeItemHeaderValue b(1, 2);
    EXPECT_TRUE(a == b);
}

// ---------------------------------------------------------------------------
// RangeHeaderValue
// ---------------------------------------------------------------------------

TEST(RangeHeaderValueTests, DefaultConstructor_UnitBytes) {
    RangeHeaderValue v;
    EXPECT_EQ(v.getUnitProperty(), "bytes");
    EXPECT_TRUE(v.getRangesProperty().empty());
}

TEST(RangeHeaderValueTests, ConvenienceConstructor_ToString) {
    RangeHeaderValue v(0, 499);
    EXPECT_EQ(v.ToString(), "bytes=0-499");
}

TEST(RangeHeaderValueTests, MultipleRanges_ToString) {
    RangeHeaderValue v;
    v.getRangesProperty().push_back(RangeItemHeaderValue(0, 499));
    v.getRangesProperty().push_back(RangeItemHeaderValue(1000, 1499));
    EXPECT_EQ(v.ToString(), "bytes=0-499, 1000-1499");
}

TEST(RangeHeaderValueTests, Parse_SingleRange) {
    auto v = RangeHeaderValue::Parse("bytes=0-499");
    EXPECT_EQ(v.getUnitProperty(), "bytes");
    ASSERT_EQ(v.getRangesProperty().size(), 1u);
    EXPECT_EQ(*v.getRangesProperty()[0].getFromProperty(), 0);
    EXPECT_EQ(*v.getRangesProperty()[0].getToProperty(), 499);
}

TEST(RangeHeaderValueTests, Parse_MultipleRanges) {
    auto v = RangeHeaderValue::Parse("bytes=0-499, 1000-1499");
    ASSERT_EQ(v.getRangesProperty().size(), 2u);
}

TEST(RangeHeaderValueTests, Parse_SuffixRange) {
    auto v = RangeHeaderValue::Parse("bytes=-500");
    ASSERT_EQ(v.getRangesProperty().size(), 1u);
    EXPECT_FALSE(v.getRangesProperty()[0].getFromProperty().has_value());
    EXPECT_EQ(*v.getRangesProperty()[0].getToProperty(), 500);
}

TEST(RangeHeaderValueTests, Parse_OpenEndedRange) {
    auto v = RangeHeaderValue::Parse("bytes=500-");
    ASSERT_EQ(v.getRangesProperty().size(), 1u);
    EXPECT_EQ(*v.getRangesProperty()[0].getFromProperty(), 500);
    EXPECT_FALSE(v.getRangesProperty()[0].getToProperty().has_value());
}

TEST(RangeHeaderValueTests, Parse_ToleratesEmptySegments) {
    auto v = RangeHeaderValue::Parse("bytes=,1-2, , 3-4,,");
    ASSERT_EQ(v.getRangesProperty().size(), 2u);
}

TEST(RangeHeaderValueTests, Parse_RoundTripsThroughToString) {
    auto v = RangeHeaderValue::Parse("bytes=0-499, 1000-1499");
    auto v2 = RangeHeaderValue::Parse(v.ToString());
    EXPECT_TRUE(v == v2);
}

TEST(RangeHeaderValueTests, Parse_MissingEquals_Throws) {
    EXPECT_THROW(RangeHeaderValue::Parse("bytes 0-499"), System::FormatException);
}

TEST(RangeHeaderValueTests, Parse_NoRanges_Throws) {
    EXPECT_THROW(RangeHeaderValue::Parse("bytes="), System::FormatException);
}

TEST(RangeHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    RangeHeaderValue result;
    EXPECT_FALSE(RangeHeaderValue::TryParse("garbage", result));
}

TEST(RangeHeaderValueTests, Equals_UnitCaseInsensitive) {
    RangeHeaderValue a(0, 499);
    RangeHeaderValue b(0, 499);
    b.setUnitProperty("BYTES");
    EXPECT_TRUE(a == b);
}

TEST(RangeHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    auto a = RangeHeaderValue::Parse("bytes=0-499");
    auto b = RangeHeaderValue::Parse("bytes=0-499");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}
