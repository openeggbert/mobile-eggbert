// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/RangeConditionHeaderValue.hpp"
#include "System/FormatException.hpp"
#include "System/TimeSpan.hpp"

using System::Net::Http::Headers::RangeConditionHeaderValue;
using System::Net::Http::Headers::EntityTagHeaderValue;

TEST(RangeConditionHeaderValueTests, Constructor_EntityTagString) {
    RangeConditionHeaderValue v("\"abc\"");
    ASSERT_TRUE(v.getEntityTagProperty().has_value());
    EXPECT_EQ(v.getEntityTagProperty()->getTagProperty(), "\"abc\"");
    EXPECT_FALSE(v.getDateProperty().has_value());
    EXPECT_EQ(v.ToString(), "\"abc\"");
}

TEST(RangeConditionHeaderValueTests, Constructor_EntityTagValue) {
    EntityTagHeaderValue tag("\"xyz\"", true);
    RangeConditionHeaderValue v(tag);
    EXPECT_EQ(v.ToString(), "W/\"xyz\"");
}

TEST(RangeConditionHeaderValueTests, Constructor_Date) {
    System::DateTimeOffset dt(2015, 10, 21, 7, 28, 0, System::TimeSpan::Zero);
    RangeConditionHeaderValue v(dt);
    ASSERT_TRUE(v.getDateProperty().has_value());
    EXPECT_FALSE(v.getEntityTagProperty().has_value());
    EXPECT_EQ(v.ToString(), "Wed, 21 Oct 2015 07:28:00 GMT");
}

TEST(RangeConditionHeaderValueTests, Parse_EntityTag) {
    auto v = RangeConditionHeaderValue::Parse("\"abc\"");
    ASSERT_TRUE(v.getEntityTagProperty().has_value());
    EXPECT_EQ(v.getEntityTagProperty()->getTagProperty(), "\"abc\"");
}

TEST(RangeConditionHeaderValueTests, Parse_WeakEntityTag) {
    auto v = RangeConditionHeaderValue::Parse("W/\"abc\"");
    ASSERT_TRUE(v.getEntityTagProperty().has_value());
    EXPECT_TRUE(v.getEntityTagProperty()->getIsWeakProperty());
}

TEST(RangeConditionHeaderValueTests, Parse_Date) {
    auto v = RangeConditionHeaderValue::Parse("Wed, 21 Oct 2015 07:28:00 GMT");
    ASSERT_TRUE(v.getDateProperty().has_value());
    EXPECT_EQ(v.getDateProperty()->ToString("r"), "Wed, 21 Oct 2015 07:28:00 GMT");
}

TEST(RangeConditionHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(RangeConditionHeaderValue::Parse("not valid"), System::FormatException);
}

TEST(RangeConditionHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    RangeConditionHeaderValue result("\"x\"");
    EXPECT_FALSE(RangeConditionHeaderValue::TryParse("not valid", result));
}

TEST(RangeConditionHeaderValueTests, Equals_SameEntityTag) {
    RangeConditionHeaderValue a("\"abc\"");
    RangeConditionHeaderValue b("\"abc\"");
    EXPECT_TRUE(a == b);
}

TEST(RangeConditionHeaderValueTests, Equals_EntityTagVsDate_False) {
    System::DateTimeOffset dt(2015, 10, 21, 7, 28, 0, System::TimeSpan::Zero);
    RangeConditionHeaderValue a("\"abc\"");
    RangeConditionHeaderValue b(dt);
    EXPECT_FALSE(a == b);
}

TEST(RangeConditionHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    RangeConditionHeaderValue a("\"abc\"");
    RangeConditionHeaderValue b("\"abc\"");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}
