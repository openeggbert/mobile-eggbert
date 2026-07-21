// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/NameValueHeaderValue.hpp"
#include "System/ArgumentException.hpp"
#include "System/FormatException.hpp"

using System::Net::Http::Headers::NameValueHeaderValue;

TEST(NameValueHeaderValueTests, NameOnlyConstructor) {
    NameValueHeaderValue v("charset");
    EXPECT_EQ(v.getNameProperty(), "charset");
    EXPECT_EQ(v.getValueProperty(), "");
    EXPECT_EQ(v.ToString(), "charset");
}

TEST(NameValueHeaderValueTests, NameValueConstructor) {
    NameValueHeaderValue v("charset", "utf-8");
    EXPECT_EQ(v.getNameProperty(), "charset");
    EXPECT_EQ(v.getValueProperty(), "utf-8");
    EXPECT_EQ(v.ToString(), "charset=utf-8");
}

TEST(NameValueHeaderValueTests, QuotedValue_Allowed) {
    NameValueHeaderValue v("boundary", "\"abc123\"");
    EXPECT_EQ(v.getValueProperty(), "\"abc123\"");
}

TEST(NameValueHeaderValueTests, Constructor_EmptyName_Throws) {
    EXPECT_THROW(NameValueHeaderValue(""), System::ArgumentException);
}

TEST(NameValueHeaderValueTests, Constructor_InvalidNameChars_Throws) {
    EXPECT_THROW(NameValueHeaderValue("bad name"), System::FormatException);
}

TEST(NameValueHeaderValueTests, Constructor_UnquotedInvalidValue_Throws) {
    EXPECT_THROW(NameValueHeaderValue("name", " leading space"), System::FormatException);
}

TEST(NameValueHeaderValueTests, Constructor_UnterminatedQuote_Throws) {
    EXPECT_THROW(NameValueHeaderValue("name", "\"unterminated"), System::FormatException);
}

TEST(NameValueHeaderValueTests, Equals_NameCaseInsensitive) {
    NameValueHeaderValue a("Charset", "utf-8");
    NameValueHeaderValue b("charset", "utf-8");
    EXPECT_TRUE(a == b);
}

TEST(NameValueHeaderValueTests, Equals_UnquotedValueCaseInsensitive) {
    NameValueHeaderValue a("charset", "UTF-8");
    NameValueHeaderValue b("charset", "utf-8");
    EXPECT_TRUE(a == b);
}

TEST(NameValueHeaderValueTests, Equals_QuotedValueCaseSensitive) {
    NameValueHeaderValue a("name", "\"ABC\"");
    NameValueHeaderValue b("name", "\"abc\"");
    EXPECT_FALSE(a == b);
}

TEST(NameValueHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    NameValueHeaderValue a("Charset", "UTF-8");
    NameValueHeaderValue b("charset", "utf-8");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(NameValueHeaderValueTests, Parse_NameOnly) {
    auto v = NameValueHeaderValue::Parse("charset");
    EXPECT_EQ(v.getNameProperty(), "charset");
    EXPECT_TRUE(v.getValueProperty().empty());
}

TEST(NameValueHeaderValueTests, Parse_NameValue) {
    auto v = NameValueHeaderValue::Parse("charset=utf-8");
    EXPECT_EQ(v.getNameProperty(), "charset");
    EXPECT_EQ(v.getValueProperty(), "utf-8");
}

TEST(NameValueHeaderValueTests, Parse_WithSpaces) {
    auto v = NameValueHeaderValue::Parse(" charset = utf-8 ");
    EXPECT_EQ(v.getNameProperty(), "charset");
    EXPECT_EQ(v.getValueProperty(), "utf-8");
}

TEST(NameValueHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    NameValueHeaderValue result("x");
    EXPECT_FALSE(NameValueHeaderValue::TryParse("", result));
}

// Real .NET's GetValueLength requires a non-empty token or quoted-string after '=' -- "name="
// with nothing (or only whitespace) after the delimiter is an invalid value, not a name-only value.
TEST(NameValueHeaderValueTests, TryParse_TrailingEqualsNoValue_ReturnsFalse) {
    NameValueHeaderValue result("x");
    EXPECT_FALSE(NameValueHeaderValue::TryParse("foo=", result));
}

TEST(NameValueHeaderValueTests, TryParse_TrailingEqualsWhitespaceOnly_ReturnsFalse) {
    NameValueHeaderValue result("x");
    EXPECT_FALSE(NameValueHeaderValue::TryParse("foo=  ", result));
}

// Real .NET's TryParse only accepts an unquoted value that is entirely token characters (or a
// well-formed quoted-string) -- an unquoted value containing '=' or other non-token characters
// must be rejected, unlike the looser check used by the public constructor/setValueProperty.
TEST(NameValueHeaderValueTests, TryParse_UnquotedValueContainingEquals_ReturnsFalse) {
    NameValueHeaderValue result("x");
    EXPECT_FALSE(NameValueHeaderValue::TryParse("a=b=c", result));
}

TEST(NameValueHeaderValueTests, TryParse_UnquotedValueWithEmbeddedSpace_ReturnsFalse) {
    NameValueHeaderValue result("x");
    EXPECT_FALSE(NameValueHeaderValue::TryParse("a=b c", result));
}

TEST(NameValueHeaderValueTests, TryParse_QuotedValueContainingEquals_Succeeds) {
    NameValueHeaderValue result("x");
    ASSERT_TRUE(NameValueHeaderValue::TryParse("a=\"b=c\"", result));
    EXPECT_EQ(result.getNameProperty(), "a");
    EXPECT_EQ(result.getValueProperty(), "\"b=c\"");
}

TEST(NameValueHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(NameValueHeaderValue::Parse(""), System::FormatException);
}

TEST(NameValueHeaderValueTests, SetValueProperty_UpdatesValue) {
    NameValueHeaderValue v("name");
    v.setValueProperty("newval");
    EXPECT_EQ(v.getValueProperty(), "newval");
}

TEST(NameValueHeaderValueTests, SetValueProperty_Invalid_Throws) {
    NameValueHeaderValue v("name");
    EXPECT_THROW(v.setValueProperty("trailing space "), System::FormatException);
}
