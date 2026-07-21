// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/WarningHeaderValue.hpp"
#include "System/FormatException.hpp"
#include "System/ArgumentException.hpp"
#include "System/TimeSpan.hpp"

using System::Net::Http::Headers::WarningHeaderValue;

TEST(WarningHeaderValueTests, Constructor_NoDate_ToString) {
    WarningHeaderValue v(110, "anderson", "\"Response is stale\"");
    EXPECT_EQ(v.getCodeProperty(), 110);
    EXPECT_EQ(v.getAgentProperty(), "anderson");
    EXPECT_EQ(v.getTextProperty(), "\"Response is stale\"");
    EXPECT_FALSE(v.getDateProperty().has_value());
    EXPECT_EQ(v.ToString(), "110 anderson \"Response is stale\"");
}

TEST(WarningHeaderValueTests, ToString_CodePadding) {
    WarningHeaderValue v(5, "agent", "\"text\"");
    EXPECT_EQ(v.ToString(), "005 agent \"text\"");
}

TEST(WarningHeaderValueTests, Constructor_WithDate_ToString) {
    System::DateTimeOffset dt(2015, 10, 21, 7, 28, 0, System::TimeSpan::Zero);
    WarningHeaderValue v(110, "anderson", "\"stale\"", dt);
    ASSERT_TRUE(v.getDateProperty().has_value());
    EXPECT_EQ(v.ToString(), "110 anderson \"stale\" \"Wed, 21 Oct 2015 07:28:00 GMT\"");
}

TEST(WarningHeaderValueTests, Constructor_CodeOutOfRange_Throws) {
    EXPECT_THROW(WarningHeaderValue(-1, "agent", "\"text\""), System::ArgumentException);
    EXPECT_THROW(WarningHeaderValue(1000, "agent", "\"text\""), System::ArgumentException);
}

TEST(WarningHeaderValueTests, Constructor_EmptyAgent_Throws) {
    EXPECT_THROW(WarningHeaderValue(100, "", "\"text\""), System::ArgumentException);
}

TEST(WarningHeaderValueTests, Constructor_UnquotedText_Throws) {
    EXPECT_THROW(WarningHeaderValue(100, "agent", "text"), System::FormatException);
}

TEST(WarningHeaderValueTests, Parse_NoDate) {
    auto v = WarningHeaderValue::Parse("110 anderson \"Response is stale\"");
    EXPECT_EQ(v.getCodeProperty(), 110);
    EXPECT_EQ(v.getAgentProperty(), "anderson");
    EXPECT_EQ(v.getTextProperty(), "\"Response is stale\"");
}

TEST(WarningHeaderValueTests, Parse_WithDate) {
    auto v = WarningHeaderValue::Parse("110 anderson \"stale\" \"Wed, 21 Oct 2015 07:28:00 GMT\"");
    ASSERT_TRUE(v.getDateProperty().has_value());
    EXPECT_EQ(v.getDateProperty()->ToString("r"), "Wed, 21 Oct 2015 07:28:00 GMT");
}

TEST(WarningHeaderValueTests, Parse_RoundTripsThroughToString) {
    auto v = WarningHeaderValue::Parse("110 anderson \"stale\"");
    auto v2 = WarningHeaderValue::Parse(v.ToString());
    EXPECT_TRUE(v == v2);
}

TEST(WarningHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(WarningHeaderValue::Parse("bad"), System::FormatException);
}

TEST(WarningHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    WarningHeaderValue result(1, "x", "\"y\"");
    EXPECT_FALSE(WarningHeaderValue::TryParse("bad", result));
}

TEST(WarningHeaderValueTests, Equals_AgentCaseInsensitive) {
    WarningHeaderValue a(110, "Anderson", "\"stale\"");
    WarningHeaderValue b(110, "anderson", "\"stale\"");
    EXPECT_TRUE(a == b);
}

TEST(WarningHeaderValueTests, Equals_TextCaseSensitive) {
    WarningHeaderValue a(110, "anderson", "\"Stale\"");
    WarningHeaderValue b(110, "anderson", "\"stale\"");
    EXPECT_FALSE(a == b);
}

TEST(WarningHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    WarningHeaderValue a(110, "Anderson", "\"stale\"");
    WarningHeaderValue b(110, "anderson", "\"stale\"");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}
