// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Mime/ContentType.hpp"
#include "System/ArgumentException.hpp"
#include "System/FormatException.hpp"

using System::Net::Mime::ContentType;

TEST(ContentTypeTests, DefaultConstructor_IsOctetStream) {
    ContentType ct;
    EXPECT_EQ(ct.getMediaTypeProperty(), "application/octet-stream");
}

TEST(ContentTypeTests, Constructor_ParsesMediaType) {
    ContentType ct("text/plain");
    EXPECT_EQ(ct.getMediaTypeProperty(), "text/plain");
}

TEST(ContentTypeTests, Constructor_EmptyString_Throws) {
    EXPECT_THROW(ContentType(""), System::ArgumentException);
}

TEST(ContentTypeTests, Constructor_Invalid_Throws) {
    EXPECT_THROW(ContentType("noSlash"), System::FormatException);
}

TEST(ContentTypeTests, Constructor_ParsesParameters) {
    ContentType ct("text/plain; charset=utf-8; boundary=xyz");
    EXPECT_EQ(ct.getCharSetProperty(), "utf-8");
    EXPECT_EQ(ct.getBoundaryProperty(), "xyz");
}

TEST(ContentTypeTests, Constructor_QuotedParameterValue) {
    ContentType ct("multipart/form-data; boundary=\"abc def\"");
    EXPECT_EQ(ct.getBoundaryProperty(), "abc def");
}

TEST(ContentTypeTests, CharSet_RoundTrips) {
    ContentType ct("text/plain");
    ct.setCharSetProperty("utf-8");
    EXPECT_EQ(ct.getCharSetProperty(), "utf-8");
    ct.setCharSetProperty("");
    EXPECT_EQ(ct.getCharSetProperty(), "");
}

TEST(ContentTypeTests, Boundary_RoundTrips) {
    ContentType ct("multipart/mixed");
    ct.setBoundaryProperty("myBoundary");
    EXPECT_EQ(ct.getBoundaryProperty(), "myBoundary");
}

TEST(ContentTypeTests, Name_RoundTrips) {
    ContentType ct("application/octet-stream");
    ct.setNameProperty("file.txt");
    EXPECT_EQ(ct.getNameProperty(), "file.txt");
}

TEST(ContentTypeTests, MediaType_Setter_RoundTrips) {
    ContentType ct("text/plain");
    ct.setMediaTypeProperty("application/json");
    EXPECT_EQ(ct.getMediaTypeProperty(), "application/json");
}

TEST(ContentTypeTests, MediaType_Setter_Invalid_Throws) {
    ContentType ct("text/plain");
    EXPECT_THROW(ct.setMediaTypeProperty("noSlash"), System::FormatException);
}

TEST(ContentTypeTests, MediaType_Setter_TrailingContent_Throws) {
    ContentType ct("text/plain");
    EXPECT_THROW(ct.setMediaTypeProperty("text/plain; charset=utf-8"), System::FormatException);
}

TEST(ContentTypeTests, ToString_RoundTrips) {
    ContentType ct("text/plain; charset=utf-8");
    EXPECT_EQ(ct.ToString(), "text/plain; charset=utf-8");
}

TEST(ContentTypeTests, ToString_QuotesValueWithSpecialChars) {
    ContentType ct("multipart/mixed");
    ct.setBoundaryProperty("a b");
    EXPECT_EQ(ct.ToString(), "multipart/mixed; boundary=\"a b\"");
}

TEST(ContentTypeTests, Equals_CaseInsensitive) {
    ContentType a("text/plain; charset=utf-8");
    ContentType b("TEXT/PLAIN; CHARSET=utf-8");
    EXPECT_TRUE(a.Equals(b));
    EXPECT_TRUE(a == b);
}

TEST(ContentTypeTests, Equals_Different_ReturnsFalse) {
    ContentType a("text/plain");
    ContentType b("text/html");
    EXPECT_FALSE(a.Equals(b));
}

TEST(ContentTypeTests, GetHashCode_ConsistentWithEquals) {
    ContentType a("text/plain");
    ContentType b("TEXT/PLAIN");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ContentTypeTests, Parameters_MutableAccess) {
    ContentType ct("text/plain");
    // operator[] is read-only (matches .NET's getter, which never inserts on a miss); use
    // set() for dict[key]=value-style writes.
    ct.getParametersProperty().set("custom", "value");
    EXPECT_EQ(ct.getParametersProperty().GetValue("custom"), "value");
}
