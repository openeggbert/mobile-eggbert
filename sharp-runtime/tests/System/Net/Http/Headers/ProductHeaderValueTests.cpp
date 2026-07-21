// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/ProductHeaderValue.hpp"
#include "System/ArgumentException.hpp"
#include "System/FormatException.hpp"

using System::Net::Http::Headers::ProductHeaderValue;

TEST(ProductHeaderValueTests, NameOnly_ToString) {
    ProductHeaderValue v("MyApp");
    EXPECT_EQ(v.getNameProperty(), "MyApp");
    EXPECT_TRUE(v.getVersionProperty().empty());
    EXPECT_EQ(v.ToString(), "MyApp");
}

TEST(ProductHeaderValueTests, NameAndVersion_ToString) {
    ProductHeaderValue v("MyApp", "1.0");
    EXPECT_EQ(v.ToString(), "MyApp/1.0");
}

TEST(ProductHeaderValueTests, NonNumericVersion_Allowed) {
    ProductHeaderValue v("MyApp", "x11");
    EXPECT_EQ(v.getVersionProperty(), "x11");
}

TEST(ProductHeaderValueTests, Constructor_EmptyName_Throws) {
    EXPECT_THROW(ProductHeaderValue(""), System::ArgumentException);
}

TEST(ProductHeaderValueTests, Constructor_InvalidNameChars_Throws) {
    EXPECT_THROW(ProductHeaderValue("bad name"), System::FormatException);
}

TEST(ProductHeaderValueTests, Constructor_InvalidVersionChars_Throws) {
    EXPECT_THROW(ProductHeaderValue("app", "1.0 beta"), System::FormatException);
}

TEST(ProductHeaderValueTests, Equals_CaseInsensitive) {
    ProductHeaderValue a("MyApp", "1.0");
    ProductHeaderValue b("myapp", "1.0");
    EXPECT_TRUE(a == b);
}

TEST(ProductHeaderValueTests, Equals_DifferentVersion_False) {
    ProductHeaderValue a("MyApp", "1.0");
    ProductHeaderValue b("MyApp", "2.0");
    EXPECT_FALSE(a == b);
}

TEST(ProductHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    ProductHeaderValue a("MyApp", "1.0");
    ProductHeaderValue b("myapp", "1.0");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ProductHeaderValueTests, Parse_NameOnly) {
    auto v = ProductHeaderValue::Parse("curl");
    EXPECT_EQ(v.getNameProperty(), "curl");
}

TEST(ProductHeaderValueTests, Parse_NameAndVersion) {
    auto v = ProductHeaderValue::Parse("curl/7.68.0");
    EXPECT_EQ(v.getNameProperty(), "curl");
    EXPECT_EQ(v.getVersionProperty(), "7.68.0");
}

TEST(ProductHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    ProductHeaderValue result("x");
    EXPECT_FALSE(ProductHeaderValue::TryParse("bad name", result));
}

TEST(ProductHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(ProductHeaderValue::Parse("bad name"), System::FormatException);
}
