// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/EntityTagHeaderValue.hpp"
#include "System/FormatException.hpp"

using System::Net::Http::Headers::EntityTagHeaderValue;

TEST(EntityTagHeaderValueTests, StrongTag_ToString) {
    EntityTagHeaderValue v("\"abc\"");
    EXPECT_EQ(v.getTagProperty(), "\"abc\"");
    EXPECT_FALSE(v.getIsWeakProperty());
    EXPECT_EQ(v.ToString(), "\"abc\"");
}

TEST(EntityTagHeaderValueTests, WeakTag_ToString) {
    EntityTagHeaderValue v("\"abc\"", true);
    EXPECT_TRUE(v.getIsWeakProperty());
    EXPECT_EQ(v.ToString(), "W/\"abc\"");
}

TEST(EntityTagHeaderValueTests, Constructor_InvalidTag_Throws) {
    EXPECT_THROW(EntityTagHeaderValue("abc"), System::FormatException);
    EXPECT_THROW(EntityTagHeaderValue("\"unterminated"), System::FormatException);
}

TEST(EntityTagHeaderValueTests, Any_IsStar) {
    EXPECT_EQ(EntityTagHeaderValue::Any().getTagProperty(), "*");
    EXPECT_FALSE(EntityTagHeaderValue::Any().getIsWeakProperty());
}

TEST(EntityTagHeaderValueTests, Equals_SameTagAndWeakness) {
    EntityTagHeaderValue a("\"abc\"", true);
    EntityTagHeaderValue b("\"abc\"", true);
    EXPECT_TRUE(a == b);
}

TEST(EntityTagHeaderValueTests, Equals_CaseSensitiveTag) {
    EntityTagHeaderValue a("\"ABC\"");
    EntityTagHeaderValue b("\"abc\"");
    EXPECT_FALSE(a == b);
}

TEST(EntityTagHeaderValueTests, Equals_DifferentWeakness) {
    EntityTagHeaderValue a("\"abc\"", true);
    EntityTagHeaderValue b("\"abc\"", false);
    EXPECT_FALSE(a == b);
}

TEST(EntityTagHeaderValueTests, Parse_Strong) {
    auto v = EntityTagHeaderValue::Parse("\"xyz\"");
    EXPECT_EQ(v.getTagProperty(), "\"xyz\"");
    EXPECT_FALSE(v.getIsWeakProperty());
}

TEST(EntityTagHeaderValueTests, Parse_Weak) {
    auto v = EntityTagHeaderValue::Parse("W/\"xyz\"");
    EXPECT_EQ(v.getTagProperty(), "\"xyz\"");
    EXPECT_TRUE(v.getIsWeakProperty());
}

TEST(EntityTagHeaderValueTests, Parse_Any) {
    auto v = EntityTagHeaderValue::Parse("*");
    EXPECT_EQ(v.getTagProperty(), "*");
}

TEST(EntityTagHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    EntityTagHeaderValue result("\"x\"");
    EXPECT_FALSE(EntityTagHeaderValue::TryParse("not-quoted", result));
}

// Real .NET's GetEntityTagLength skips whitespace between the "W/" prefix and the quoted-string
// tag (HttpRuleParser.GetWhitespaceLength after the '/').
TEST(EntityTagHeaderValueTests, TryParse_WeakTagWithSpaceAfterSlash_Succeeds) {
    EntityTagHeaderValue result("\"x\"");
    ASSERT_TRUE(EntityTagHeaderValue::TryParse("W/ \"abc\"", result));
    EXPECT_TRUE(result.getIsWeakProperty());
    EXPECT_EQ(result.getTagProperty(), "\"abc\"");
}

TEST(EntityTagHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(EntityTagHeaderValue::Parse("not-quoted"), System::FormatException);
}

TEST(EntityTagHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    EntityTagHeaderValue a("\"abc\"", true);
    EntityTagHeaderValue b("\"abc\"", true);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}
