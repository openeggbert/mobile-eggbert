// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/RetryConditionHeaderValue.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include "System/TimeSpan.hpp"

using System::Net::Http::Headers::RetryConditionHeaderValue;

TEST(RetryConditionHeaderValueTests, Constructor_Delta) {
    RetryConditionHeaderValue v(System::TimeSpan::FromSeconds(120));
    ASSERT_TRUE(v.getDeltaProperty().has_value());
    EXPECT_FALSE(v.getDateProperty().has_value());
    EXPECT_EQ(v.ToString(), "120");
}

// Regression test for a wave-3 audit finding: threw std::out_of_range (an unrelated std::
// exception type invisible to code catching System::Exception&) instead of
// System::ArgumentOutOfRangeException, which is what real .NET's
// RetryConditionHeaderValue(TimeSpan) constructor throws (via
// ArgumentOutOfRangeException.ThrowIfGreaterThan) when delta exceeds int.MaxValue seconds.
TEST(RetryConditionHeaderValueTests, Constructor_DeltaExceedsInt32Max_Throws) {
    // int.MaxValue seconds is ~68 years; 1e10 seconds (~317 years) exceeds it while still
    // being well within TimeSpan's own much larger representable range.
    EXPECT_THROW(RetryConditionHeaderValue(System::TimeSpan::FromSeconds(1e10)), System::ArgumentOutOfRangeException);
}

TEST(RetryConditionHeaderValueTests, Constructor_Date) {
    System::DateTimeOffset dt(2015, 10, 21, 7, 28, 0, System::TimeSpan::Zero);
    RetryConditionHeaderValue v(dt);
    ASSERT_TRUE(v.getDateProperty().has_value());
    EXPECT_FALSE(v.getDeltaProperty().has_value());
    EXPECT_EQ(v.ToString(), "Wed, 21 Oct 2015 07:28:00 GMT");
}

TEST(RetryConditionHeaderValueTests, Parse_Delta) {
    auto v = RetryConditionHeaderValue::Parse("120");
    ASSERT_TRUE(v.getDeltaProperty().has_value());
    EXPECT_DOUBLE_EQ(v.getDeltaProperty()->getTotalSecondsProperty(), 120.0);
}

TEST(RetryConditionHeaderValueTests, Parse_Date) {
    auto v = RetryConditionHeaderValue::Parse("Wed, 21 Oct 2015 07:28:00 GMT");
    ASSERT_TRUE(v.getDateProperty().has_value());
    EXPECT_EQ(v.getDateProperty()->ToString("r"), "Wed, 21 Oct 2015 07:28:00 GMT");
}

TEST(RetryConditionHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(RetryConditionHeaderValue::Parse("not valid"), System::FormatException);
}

TEST(RetryConditionHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    RetryConditionHeaderValue result(System::TimeSpan::FromSeconds(1));
    EXPECT_FALSE(RetryConditionHeaderValue::TryParse("not valid", result));
}

TEST(RetryConditionHeaderValueTests, Equals_SameDelta) {
    RetryConditionHeaderValue a(System::TimeSpan::FromSeconds(60));
    RetryConditionHeaderValue b(System::TimeSpan::FromSeconds(60));
    EXPECT_TRUE(a == b);
}

TEST(RetryConditionHeaderValueTests, Equals_DeltaVsDate_False) {
    System::DateTimeOffset dt(2015, 10, 21, 7, 28, 0, System::TimeSpan::Zero);
    RetryConditionHeaderValue a(System::TimeSpan::FromSeconds(60));
    RetryConditionHeaderValue b(dt);
    EXPECT_FALSE(a == b);
}

TEST(RetryConditionHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    RetryConditionHeaderValue a(System::TimeSpan::FromSeconds(60));
    RetryConditionHeaderValue b(System::TimeSpan::FromSeconds(60));
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}
