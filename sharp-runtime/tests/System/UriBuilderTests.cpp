// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UriBuilder.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/UriFormatException.hpp"

using System::UriBuilder;
using System::ArgumentOutOfRangeException;
using System::UriFormatException;

TEST(UriBuilderTest, DefaultCtor) {
    UriBuilder b;
    EXPECT_EQ(b.getSchemeProperty(), "http");
    EXPECT_EQ(b.getPathProperty(), "/");
}

TEST(UriBuilderTest, DefaultCtor_HostIsLocalhost) {
    // .NET's UriBuilder() default host is "localhost", not empty.
    UriBuilder b;
    EXPECT_EQ(b.getHostProperty(), "localhost");
    EXPECT_EQ(b.ToString(), "http://localhost/");
}

TEST(UriBuilderTest, CtorFromString) {
    UriBuilder b("http://example.com/path");
    EXPECT_EQ(b.getSchemeProperty(), "http");
    EXPECT_EQ(b.getHostProperty(), "example.com");
}

TEST(UriBuilderTest, SetScheme) {
    UriBuilder b;
    b.setSchemeProperty("https");
    EXPECT_EQ(b.getSchemeProperty(), "https");
}

TEST(UriBuilderTest, SetHost) {
    UriBuilder b;
    b.setHostProperty("example.org");
    EXPECT_EQ(b.getHostProperty(), "example.org");
}

TEST(UriBuilderTest, SetPath) {
    UriBuilder b;
    b.setPathProperty("/api/v1");
    EXPECT_EQ(b.getPathProperty(), "/api/v1");
}

TEST(UriBuilderTest, SetQuery) {
    UriBuilder b;
    b.setQueryProperty("?key=value");
    EXPECT_FALSE(b.getQueryProperty().empty());
}

TEST(UriBuilderTest, ToUri) {
    UriBuilder b;
    b.setSchemeProperty("https");
    b.setHostProperty("example.com");
    b.setPathProperty("/test");
    auto uri = b.getUriProperty();
    EXPECT_NE(uri.getAbsoluteUriProperty().find("example.com"), std::string::npos);
}

TEST(UriBuilderTest, ToStringNotEmpty) {
    UriBuilder b;
    b.setHostProperty("localhost");
    EXPECT_FALSE(b.ToString().empty());
}

TEST(UriBuilderTest, SchemeHostCtor) {
    UriBuilder b("https", "example.com");
    EXPECT_EQ(b.getSchemeProperty(), "https");
    EXPECT_EQ(b.getHostProperty(), "example.com");
}

TEST(UriBuilderTest, SchemeHostPortCtor) {
    UriBuilder b("https", "example.com", 8443);
    EXPECT_EQ(b.getPortProperty(), 8443);
}

TEST(UriBuilderTest, SchemeHostPortPathCtor) {
    UriBuilder b("https", "example.com", 8443, "api/v1");
    EXPECT_EQ(b.getPathProperty(), "api/v1");
}

TEST(UriBuilderTest, SchemeHostPortPathQueryExtraCtor) {
    UriBuilder b("https", "example.com", -1, "/x", "?a=1");
    EXPECT_EQ(b.getQueryProperty(), "?a=1");
    EXPECT_TRUE(b.getFragmentProperty().empty());
}

TEST(UriBuilderTest, SchemeHostPortPathFragmentExtraCtor) {
    UriBuilder b("https", "example.com", -1, "/x", "#frag");
    EXPECT_EQ(b.getFragmentProperty(), "#frag");
    EXPECT_TRUE(b.getQueryProperty().empty());
}

TEST(UriBuilderTest, ExtraValue_InvalidPrefix_Throws) {
    EXPECT_THROW(UriBuilder("https", "example.com", -1, "/x", "bad"), System::ArgumentException);
}

TEST(UriBuilderTest, Port_OutOfRange_Throws) {
    UriBuilder b;
    EXPECT_THROW(b.setPortProperty(-2), ArgumentOutOfRangeException);
    EXPECT_THROW(b.setPortProperty(70000), ArgumentOutOfRangeException);
}

TEST(UriBuilderTest, Port_MinusOne_Allowed) {
    UriBuilder b;
    EXPECT_NO_THROW(b.setPortProperty(-1));
}

TEST(UriBuilderTest, SetPath_Empty_NormalizesToSlash) {
    UriBuilder b;
    b.setPathProperty("");
    EXPECT_EQ(b.getPathProperty(), "/");
}

TEST(UriBuilderTest, ToString_PasswordWithoutUsername_Throws) {
    UriBuilder b;
    b.setPasswordProperty("secret");
    EXPECT_THROW(b.ToString(), UriFormatException);
}

TEST(UriBuilderTest, Equals_SameComponents) {
    UriBuilder a("https", "example.com", -1, "/x");
    UriBuilder b("https", "example.com", -1, "/x");
    EXPECT_TRUE(a.Equals(b));
}

TEST(UriBuilderTest, GetHashCode_Consistent) {
    UriBuilder a("https", "example.com", -1, "/x");
    UriBuilder b("https", "example.com", -1, "/x");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

// ---------------------------------------------------------------------------
// Query/Fragment setter normalization (regression: real .NET's UriBuilder.Query/
// Fragment setters prepend '?'/'#' when missing, so the getter always returns the
// prefixed form -- this port previously stored the raw value verbatim.
// ---------------------------------------------------------------------------

TEST(UriBuilderTest, SetQuery_WithoutLeadingQuestionMark_Normalizes) {
    UriBuilder b;
    b.setQueryProperty("foo=bar");
    EXPECT_EQ(b.getQueryProperty(), "?foo=bar");
}

TEST(UriBuilderTest, SetQuery_WithLeadingQuestionMark_Unchanged) {
    UriBuilder b;
    b.setQueryProperty("?foo=bar");
    EXPECT_EQ(b.getQueryProperty(), "?foo=bar");
}

TEST(UriBuilderTest, SetQuery_Empty_StaysEmpty) {
    UriBuilder b;
    b.setQueryProperty("");
    EXPECT_TRUE(b.getQueryProperty().empty());
}

TEST(UriBuilderTest, SetFragment_WithoutLeadingHash_Normalizes) {
    UriBuilder b;
    b.setFragmentProperty("section1");
    EXPECT_EQ(b.getFragmentProperty(), "#section1");
}

TEST(UriBuilderTest, SetFragment_WithLeadingHash_Unchanged) {
    UriBuilder b;
    b.setFragmentProperty("#section1");
    EXPECT_EQ(b.getFragmentProperty(), "#section1");
}

TEST(UriBuilderTest, SetFragment_Empty_StaysEmpty) {
    UriBuilder b;
    b.setFragmentProperty("");
    EXPECT_TRUE(b.getFragmentProperty().empty());
}
