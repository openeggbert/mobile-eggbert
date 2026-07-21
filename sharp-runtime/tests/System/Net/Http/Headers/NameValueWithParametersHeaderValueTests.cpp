// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/NameValueWithParametersHeaderValue.hpp"
#include "System/FormatException.hpp"

using System::Net::Http::Headers::NameValueWithParametersHeaderValue;
using System::Net::Http::Headers::NameValueHeaderValue;

TEST(NameValueWithParametersHeaderValueTests, NameOnly_NoParameters) {
    NameValueWithParametersHeaderValue v("form-data");
    EXPECT_EQ(v.getNameProperty(), "form-data");
    EXPECT_TRUE(v.getParametersProperty().empty());
    EXPECT_EQ(v.ToString(), "form-data");
}

TEST(NameValueWithParametersHeaderValueTests, AddParameters_ToString) {
    NameValueWithParametersHeaderValue v("form-data");
    v.getParametersProperty().push_back(NameValueHeaderValue("charset", "utf-8"));
    EXPECT_EQ(v.ToString(), "form-data; charset=utf-8");
}

TEST(NameValueWithParametersHeaderValueTests, Parse_NoParameters) {
    auto v = NameValueWithParametersHeaderValue::Parse("form-data");
    EXPECT_EQ(v.getNameProperty(), "form-data");
    EXPECT_TRUE(v.getParametersProperty().empty());
}

TEST(NameValueWithParametersHeaderValueTests, Parse_OneParameter) {
    auto v = NameValueWithParametersHeaderValue::Parse("form-data; charset=utf-8");
    EXPECT_EQ(v.getNameProperty(), "form-data");
    ASSERT_EQ(v.getParametersProperty().size(), 1u);
    EXPECT_EQ(v.getParametersProperty()[0].getNameProperty(), "charset");
    EXPECT_EQ(v.getParametersProperty()[0].getValueProperty(), "utf-8");
}

TEST(NameValueWithParametersHeaderValueTests, Parse_MultipleParameters) {
    auto v = NameValueWithParametersHeaderValue::Parse("attachment; boundary=xyz; charset=utf-8");
    ASSERT_EQ(v.getParametersProperty().size(), 2u);
    EXPECT_EQ(v.getParametersProperty()[0].getNameProperty(), "boundary");
    EXPECT_EQ(v.getParametersProperty()[1].getNameProperty(), "charset");
}

TEST(NameValueWithParametersHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    NameValueWithParametersHeaderValue result("x");
    EXPECT_FALSE(NameValueWithParametersHeaderValue::TryParse("bad name; charset=utf-8", result));
}

// The head (name[=value]) portion must go through the same strict token-or-quoted-string
// grammar as a plain NameValueHeaderValue -- real .NET's GetNameValueWithParametersLength
// delegates to the identical GetNameValueLength parser for the head.
TEST(NameValueWithParametersHeaderValueTests, TryParse_TrailingEqualsNoValue_ReturnsFalse) {
    NameValueWithParametersHeaderValue result("x");
    EXPECT_FALSE(NameValueWithParametersHeaderValue::TryParse("text/plain=; charset=utf-8", result));
}

TEST(NameValueWithParametersHeaderValueTests, TryParse_UnquotedHeadValueContainingEquals_ReturnsFalse) {
    NameValueWithParametersHeaderValue result("x");
    EXPECT_FALSE(NameValueWithParametersHeaderValue::TryParse("a=b=c; charset=utf-8", result));
}

TEST(NameValueWithParametersHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(NameValueWithParametersHeaderValue::Parse("bad name"), System::FormatException);
}

TEST(NameValueWithParametersHeaderValueTests, Equals_SameNameAndParameters) {
    auto a = NameValueWithParametersHeaderValue::Parse("form-data; charset=utf-8");
    auto b = NameValueWithParametersHeaderValue::Parse("form-data; charset=utf-8");
    EXPECT_TRUE(a == b);
}

TEST(NameValueWithParametersHeaderValueTests, Equals_ParametersOrderIndependent) {
    auto a = NameValueWithParametersHeaderValue::Parse("attachment; a=1; b=2");
    auto b = NameValueWithParametersHeaderValue::Parse("attachment; b=2; a=1");
    EXPECT_TRUE(a == b);
}

TEST(NameValueWithParametersHeaderValueTests, Equals_DifferentParameters_False) {
    auto a = NameValueWithParametersHeaderValue::Parse("form-data; charset=utf-8");
    auto b = NameValueWithParametersHeaderValue::Parse("form-data; charset=ascii");
    EXPECT_FALSE(a == b);
}

TEST(NameValueWithParametersHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    auto a = NameValueWithParametersHeaderValue::Parse("form-data; charset=utf-8");
    auto b = NameValueWithParametersHeaderValue::Parse("form-data; charset=utf-8");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(NameValueWithParametersHeaderValueTests, IsA_NameValueHeaderValue) {
    NameValueWithParametersHeaderValue v("form-data");
    NameValueHeaderValue* base = &v;
    EXPECT_EQ(base->getNameProperty(), "form-data");
}
