// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/AuthenticationHeaderValue.hpp"
#include "System/ArgumentException.hpp"
#include "System/FormatException.hpp"

using System::Net::Http::Headers::AuthenticationHeaderValue;

TEST(AuthenticationHeaderValueTests, SchemeOnly_ToString) {
    AuthenticationHeaderValue v("NTLM");
    EXPECT_EQ(v.getSchemeProperty(), "NTLM");
    EXPECT_TRUE(v.getParameterProperty().empty());
    EXPECT_EQ(v.ToString(), "NTLM");
}

TEST(AuthenticationHeaderValueTests, SchemeAndParameter_ToString) {
    AuthenticationHeaderValue v("Basic", "QWxhZGRpbjpvcGVu");
    EXPECT_EQ(v.ToString(), "Basic QWxhZGRpbjpvcGVu");
}

TEST(AuthenticationHeaderValueTests, Constructor_EmptyScheme_Throws) {
    EXPECT_THROW(AuthenticationHeaderValue(""), System::ArgumentException);
}

TEST(AuthenticationHeaderValueTests, Constructor_InvalidSchemeChars_Throws) {
    EXPECT_THROW(AuthenticationHeaderValue("bad scheme"), System::FormatException);
}

TEST(AuthenticationHeaderValueTests, Constructor_EmbeddedNulInScheme_Throws) {
    EXPECT_THROW(AuthenticationHeaderValue(std::string("Ba\0sic", 6)), System::FormatException);
}

TEST(AuthenticationHeaderValueTests, Constructor_EmbeddedNewlineInParameter_Throws) {
    EXPECT_THROW(AuthenticationHeaderValue("Basic", "abc\r\nX-Evil: 1"), System::FormatException);
}

TEST(AuthenticationHeaderValueTests, Constructor_EmbeddedNulInParameter_Throws) {
    EXPECT_THROW(AuthenticationHeaderValue("Basic", std::string("ab\0cd", 5)), System::FormatException);
}

TEST(AuthenticationHeaderValueTests, Equals_SchemeCaseInsensitive_NoParameter) {
    AuthenticationHeaderValue a("NTLM");
    AuthenticationHeaderValue b("ntlm");
    EXPECT_TRUE(a == b);
}

TEST(AuthenticationHeaderValueTests, Equals_ParameterCaseSensitive) {
    AuthenticationHeaderValue a("Basic", "ABC=");
    AuthenticationHeaderValue b("Basic", "abc=");
    EXPECT_FALSE(a == b);
}

TEST(AuthenticationHeaderValueTests, Equals_DifferentParameter_False) {
    AuthenticationHeaderValue a("Basic", "abc");
    AuthenticationHeaderValue b("Basic", "def");
    EXPECT_FALSE(a == b);
}

TEST(AuthenticationHeaderValueTests, Parse_SchemeOnly) {
    auto v = AuthenticationHeaderValue::Parse("NTLM");
    EXPECT_EQ(v.getSchemeProperty(), "NTLM");
    EXPECT_TRUE(v.getParameterProperty().empty());
}

TEST(AuthenticationHeaderValueTests, Parse_SchemeAndParameter) {
    auto v = AuthenticationHeaderValue::Parse("Basic QWxhZGRpbjpvcGVu");
    EXPECT_EQ(v.getSchemeProperty(), "Basic");
    EXPECT_EQ(v.getParameterProperty(), "QWxhZGRpbjpvcGVu");
}

TEST(AuthenticationHeaderValueTests, Parse_ExtraWhitespaceTrimmed) {
    auto v = AuthenticationHeaderValue::Parse("  Bearer   token123  ");
    EXPECT_EQ(v.getSchemeProperty(), "Bearer");
    EXPECT_EQ(v.getParameterProperty(), "token123");
}

TEST(AuthenticationHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    AuthenticationHeaderValue result("x");
    EXPECT_FALSE(AuthenticationHeaderValue::TryParse("bad/scheme value", result));
}

TEST(AuthenticationHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(AuthenticationHeaderValue::Parse("bad/scheme value"), System::FormatException);
}

TEST(AuthenticationHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    AuthenticationHeaderValue a("NTLM");
    AuthenticationHeaderValue b("ntlm");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}
