// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/TransferCodingHeaderValue.hpp"
#include "System/Net/Http/Headers/TransferCodingWithQualityHeaderValue.hpp"
#include "System/FormatException.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using System::Net::Http::Headers::TransferCodingHeaderValue;
using System::Net::Http::Headers::TransferCodingWithQualityHeaderValue;

TEST(TransferCodingHeaderValueTests, Constructor_ValueOnly_ToString) {
    TransferCodingHeaderValue v("chunked");
    EXPECT_EQ(v.getValueProperty(), "chunked");
    EXPECT_TRUE(v.getParametersProperty().empty());
    EXPECT_EQ(v.ToString(), "chunked");
}

TEST(TransferCodingHeaderValueTests, Constructor_Empty_Throws) {
    EXPECT_THROW(TransferCodingHeaderValue(""), System::ArgumentException);
}

TEST(TransferCodingHeaderValueTests, Constructor_InvalidToken_Throws) {
    EXPECT_THROW(TransferCodingHeaderValue("bad value"), System::FormatException);
}

TEST(TransferCodingHeaderValueTests, Parse_WithParameters) {
    auto v = TransferCodingHeaderValue::Parse("gzip; level=5");
    EXPECT_EQ(v.getValueProperty(), "gzip");
    ASSERT_EQ(v.getParametersProperty().size(), 1u);
}

TEST(TransferCodingHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(TransferCodingHeaderValue::Parse("bad value"), System::FormatException);
}

TEST(TransferCodingHeaderValueTests, Equals_ValueCaseInsensitive) {
    auto a = TransferCodingHeaderValue::Parse("Gzip");
    auto b = TransferCodingHeaderValue::Parse("gzip");
    EXPECT_TRUE(a == b);
}

TEST(TransferCodingHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    auto a = TransferCodingHeaderValue::Parse("Gzip");
    auto b = TransferCodingHeaderValue::Parse("gzip");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

// ---------------------------------------------------------------------------
// TransferCodingWithQualityHeaderValue
// ---------------------------------------------------------------------------

TEST(TransferCodingWithQualityHeaderValueTests, NoQuality_ToString) {
    TransferCodingWithQualityHeaderValue v("gzip");
    EXPECT_FALSE(v.getQualityProperty().has_value());
    EXPECT_EQ(v.ToString(), "gzip");
}

TEST(TransferCodingWithQualityHeaderValueTests, WithQuality_ToString) {
    TransferCodingWithQualityHeaderValue v("gzip", 0.5);
    ASSERT_TRUE(v.getQualityProperty().has_value());
    EXPECT_DOUBLE_EQ(*v.getQualityProperty(), 0.5);
    EXPECT_EQ(v.ToString(), "gzip; q=0.5");
}

// Regression test for a wave-3 audit finding: threw std::out_of_range (an unrelated std::
// exception type invisible to code catching System::Exception&) instead of
// System::ArgumentOutOfRangeException, which is what real .NET's HeaderUtilities.SetQuality
// throws for an out-of-[0,1]-range quality value.
TEST(TransferCodingWithQualityHeaderValueTests, Constructor_QualityOutOfRange_Throws) {
    EXPECT_THROW(TransferCodingWithQualityHeaderValue("gzip", 1.5), System::ArgumentOutOfRangeException);
}

TEST(TransferCodingWithQualityHeaderValueTests, Parse_WithQuality) {
    auto v = TransferCodingWithQualityHeaderValue::Parse("gzip; q=0.5");
    EXPECT_EQ(v.getValueProperty(), "gzip");
    ASSERT_TRUE(v.getQualityProperty().has_value());
    EXPECT_DOUBLE_EQ(*v.getQualityProperty(), 0.5);
}

TEST(TransferCodingWithQualityHeaderValueTests, TryParse_InvalidQuality_ReturnsFalse) {
    TransferCodingWithQualityHeaderValue result("x");
    EXPECT_FALSE(TransferCodingWithQualityHeaderValue::TryParse("gzip; q=2.0", result));
}

TEST(TransferCodingWithQualityHeaderValueTests, IsA_TransferCodingHeaderValue) {
    TransferCodingWithQualityHeaderValue v("gzip", 0.5);
    TransferCodingHeaderValue* base = &v;
    EXPECT_EQ(base->getValueProperty(), "gzip");
}
