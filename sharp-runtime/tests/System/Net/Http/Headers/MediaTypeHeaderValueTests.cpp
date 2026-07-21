// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/MediaTypeHeaderValue.hpp"
#include "System/Net/Http/Headers/MediaTypeWithQualityHeaderValue.hpp"
#include "System/FormatException.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using System::Net::Http::Headers::MediaTypeHeaderValue;
using System::Net::Http::Headers::MediaTypeWithQualityHeaderValue;

TEST(MediaTypeHeaderValueTests, Constructor_MediaTypeOnly_ToString) {
    MediaTypeHeaderValue v("text/plain");
    EXPECT_EQ(v.getMediaTypeProperty(), "text/plain");
    EXPECT_TRUE(v.getParametersProperty().empty());
    EXPECT_EQ(v.ToString(), "text/plain");
}

TEST(MediaTypeHeaderValueTests, Constructor_WithCharSet) {
    MediaTypeHeaderValue v("text/plain", "utf-8");
    EXPECT_EQ(v.getCharSetProperty(), "utf-8");
    EXPECT_EQ(v.ToString(), "text/plain; charset=utf-8");
}

TEST(MediaTypeHeaderValueTests, Constructor_Empty_Throws) {
    EXPECT_THROW(MediaTypeHeaderValue(""), System::ArgumentException);
}

TEST(MediaTypeHeaderValueTests, Constructor_MissingSlash_Throws) {
    EXPECT_THROW(MediaTypeHeaderValue("textplain"), System::FormatException);
}

TEST(MediaTypeHeaderValueTests, Constructor_WhitespaceInMediaType_Throws) {
    EXPECT_THROW(MediaTypeHeaderValue("text /plain"), System::FormatException);
    EXPECT_THROW(MediaTypeHeaderValue(" text/plain"), System::FormatException);
}

TEST(MediaTypeHeaderValueTests, CharSet_SetThenRemove) {
    MediaTypeHeaderValue v("text/plain");
    v.setCharSetProperty("utf-8");
    EXPECT_EQ(v.getCharSetProperty(), "utf-8");
    v.setCharSetProperty("");
    EXPECT_TRUE(v.getCharSetProperty().empty());
    EXPECT_TRUE(v.getParametersProperty().empty());
}

TEST(MediaTypeHeaderValueTests, Parse_WithParameters) {
    auto v = MediaTypeHeaderValue::Parse("text/plain; charset=utf-8; boundary=xyz");
    EXPECT_EQ(v.getMediaTypeProperty(), "text/plain");
    EXPECT_EQ(v.getCharSetProperty(), "utf-8");
    ASSERT_EQ(v.getParametersProperty().size(), 2u);
}

TEST(MediaTypeHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(MediaTypeHeaderValue::Parse("textplain"), System::FormatException);
}

TEST(MediaTypeHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    MediaTypeHeaderValue result("x/x");
    EXPECT_FALSE(MediaTypeHeaderValue::TryParse("textplain", result));
}

TEST(MediaTypeHeaderValueTests, Equals_MediaTypeCaseInsensitive) {
    auto a = MediaTypeHeaderValue::Parse("Text/Plain; charset=utf-8");
    auto b = MediaTypeHeaderValue::Parse("text/plain; charset=utf-8");
    EXPECT_TRUE(a == b);
}

TEST(MediaTypeHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    auto a = MediaTypeHeaderValue::Parse("text/plain; charset=utf-8");
    auto b = MediaTypeHeaderValue::Parse("text/plain; charset=utf-8");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

// ---------------------------------------------------------------------------
// MediaTypeWithQualityHeaderValue
// ---------------------------------------------------------------------------

TEST(MediaTypeWithQualityHeaderValueTests, NoQuality_ToString) {
    MediaTypeWithQualityHeaderValue v("text/html");
    EXPECT_FALSE(v.getQualityProperty().has_value());
    EXPECT_EQ(v.ToString(), "text/html");
}

TEST(MediaTypeWithQualityHeaderValueTests, WithQuality_ToString) {
    MediaTypeWithQualityHeaderValue v("text/html", 0.8);
    ASSERT_TRUE(v.getQualityProperty().has_value());
    EXPECT_DOUBLE_EQ(*v.getQualityProperty(), 0.8);
    EXPECT_EQ(v.ToString(), "text/html; q=0.8");
}

// Regression test for a wave-3 audit finding: threw std::out_of_range (an unrelated std::
// exception type invisible to code catching System::Exception&) instead of
// System::ArgumentOutOfRangeException, which is what real .NET's HeaderUtilities.SetQuality
// throws (via ArgumentOutOfRangeException.ThrowIfNegative/ThrowIfGreaterThan) for an
// out-of-[0,1]-range quality value.
TEST(MediaTypeWithQualityHeaderValueTests, Constructor_QualityOutOfRange_Throws) {
    EXPECT_THROW(MediaTypeWithQualityHeaderValue("text/html", 1.5), System::ArgumentOutOfRangeException);
    EXPECT_THROW(MediaTypeWithQualityHeaderValue("text/html", -0.1), System::ArgumentOutOfRangeException);
}

TEST(MediaTypeWithQualityHeaderValueTests, Parse_WithQualityAndParameters) {
    auto v = MediaTypeWithQualityHeaderValue::Parse("text/html; charset=utf-8; q=0.8");
    EXPECT_EQ(v.getMediaTypeProperty(), "text/html");
    EXPECT_EQ(v.getCharSetProperty(), "utf-8");
    ASSERT_TRUE(v.getQualityProperty().has_value());
    EXPECT_DOUBLE_EQ(*v.getQualityProperty(), 0.8);
}

TEST(MediaTypeWithQualityHeaderValueTests, TryParse_InvalidQuality_ReturnsFalse) {
    MediaTypeWithQualityHeaderValue result("x/x");
    EXPECT_FALSE(MediaTypeWithQualityHeaderValue::TryParse("text/html; q=2.0", result));
}

TEST(MediaTypeWithQualityHeaderValueTests, IsA_MediaTypeHeaderValue) {
    MediaTypeWithQualityHeaderValue v("text/html", 0.5);
    MediaTypeHeaderValue* base = &v;
    EXPECT_EQ(base->getMediaTypeProperty(), "text/html");
}
