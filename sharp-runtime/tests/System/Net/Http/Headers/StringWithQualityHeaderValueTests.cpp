// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/StringWithQualityHeaderValue.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"

using System::Net::Http::Headers::StringWithQualityHeaderValue;

TEST(StringWithQualityHeaderValueTests, NoQuality_ToString) {
    StringWithQualityHeaderValue v("gzip");
    EXPECT_EQ(v.getValueProperty(), "gzip");
    EXPECT_FALSE(v.getQualityProperty().has_value());
    EXPECT_EQ(v.ToString(), "gzip");
}

TEST(StringWithQualityHeaderValueTests, WithQuality_ToString) {
    StringWithQualityHeaderValue v("gzip", 0.8);
    ASSERT_TRUE(v.getQualityProperty().has_value());
    EXPECT_DOUBLE_EQ(*v.getQualityProperty(), 0.8);
    EXPECT_EQ(v.ToString(), "gzip; q=0.8");
}

TEST(StringWithQualityHeaderValueTests, Quality_One_ToString) {
    StringWithQualityHeaderValue v("gzip", 1.0);
    EXPECT_EQ(v.ToString(), "gzip; q=1.0");
}

// Regression test for a wave-3 audit finding: threw std::out_of_range (an unrelated std::
// exception type invisible to code catching System::Exception&) instead of
// System::ArgumentOutOfRangeException, which is what real .NET's StringWithQualityHeaderValue
// constructor throws (via ArgumentOutOfRangeException.ThrowIfNegative/ThrowIfGreaterThan) for
// an out-of-[0,1]-range quality value.
TEST(StringWithQualityHeaderValueTests, Constructor_QualityOutOfRange_Throws) {
    EXPECT_THROW(StringWithQualityHeaderValue("gzip", -0.1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(StringWithQualityHeaderValue("gzip", 1.1), System::ArgumentOutOfRangeException);
}

TEST(StringWithQualityHeaderValueTests, Constructor_InvalidToken_Throws) {
    EXPECT_THROW(StringWithQualityHeaderValue("bad value"), System::FormatException);
}

TEST(StringWithQualityHeaderValueTests, Equals_CaseInsensitiveValue) {
    StringWithQualityHeaderValue a("GZIP", 0.5);
    StringWithQualityHeaderValue b("gzip", 0.5);
    EXPECT_TRUE(a == b);
}

TEST(StringWithQualityHeaderValueTests, Equals_DifferentQuality_False) {
    StringWithQualityHeaderValue a("gzip", 0.5);
    StringWithQualityHeaderValue b("gzip", 0.9);
    EXPECT_FALSE(a == b);
}

TEST(StringWithQualityHeaderValueTests, Parse_NoQuality) {
    auto v = StringWithQualityHeaderValue::Parse("deflate");
    EXPECT_EQ(v.getValueProperty(), "deflate");
    EXPECT_FALSE(v.getQualityProperty().has_value());
}

TEST(StringWithQualityHeaderValueTests, Parse_WithQuality) {
    auto v = StringWithQualityHeaderValue::Parse("gzip; q=0.8");
    EXPECT_EQ(v.getValueProperty(), "gzip");
    ASSERT_TRUE(v.getQualityProperty().has_value());
    EXPECT_DOUBLE_EQ(*v.getQualityProperty(), 0.8);
}

TEST(StringWithQualityHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    StringWithQualityHeaderValue result("x");
    EXPECT_FALSE(StringWithQualityHeaderValue::TryParse("gzip; q=2.0", result));
}

// Real .NET's HttpRuleParser.GetNumberLength rejects a leading dot, a sign, and scientific
// notation for the quality number -- std::stod alone accepts all three, which would let
// TryParse silently accept malformed quality values real .NET rejects.
TEST(StringWithQualityHeaderValueTests, TryParse_LeadingDotQuality_ReturnsFalse) {
    StringWithQualityHeaderValue result("x");
    EXPECT_FALSE(StringWithQualityHeaderValue::TryParse("gzip; q=.5", result));
}

TEST(StringWithQualityHeaderValueTests, TryParse_SignedQuality_ReturnsFalse) {
    StringWithQualityHeaderValue result("x");
    EXPECT_FALSE(StringWithQualityHeaderValue::TryParse("gzip; q=+0.5", result));
}

TEST(StringWithQualityHeaderValueTests, TryParse_ScientificNotationQuality_ReturnsFalse) {
    StringWithQualityHeaderValue result("x");
    EXPECT_FALSE(StringWithQualityHeaderValue::TryParse("gzip; q=1e0", result));
}

TEST(StringWithQualityHeaderValueTests, TryParse_TrailingDotQuality_Valid) {
    StringWithQualityHeaderValue result("x");
    ASSERT_TRUE(StringWithQualityHeaderValue::TryParse("gzip; q=1.", result));
    ASSERT_TRUE(result.getQualityProperty().has_value());
    EXPECT_DOUBLE_EQ(*result.getQualityProperty(), 1.0);
}

TEST(StringWithQualityHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(StringWithQualityHeaderValue::Parse("bad value"), System::FormatException);
}

TEST(StringWithQualityHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    StringWithQualityHeaderValue a("GZIP", 0.5);
    StringWithQualityHeaderValue b("gzip", 0.5);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}
