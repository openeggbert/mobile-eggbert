// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/Http/Headers/ProductInfoHeaderValue.hpp"
#include "System/FormatException.hpp"

using System::Net::Http::Headers::ProductInfoHeaderValue;
using System::Net::Http::Headers::ProductHeaderValue;

TEST(ProductInfoHeaderValueTests, ProductConstructor_NameVersion) {
    ProductInfoHeaderValue v("MyApp", "1.0");
    ASSERT_TRUE(v.getProductProperty().has_value());
    EXPECT_EQ(v.getProductProperty()->getNameProperty(), "MyApp");
    EXPECT_TRUE(v.getCommentProperty().empty());
    EXPECT_EQ(v.ToString(), "MyApp/1.0");
}

TEST(ProductInfoHeaderValueTests, ProductHeaderValueConstructor) {
    ProductHeaderValue p("curl", "7.68.0");
    ProductInfoHeaderValue v(p);
    EXPECT_EQ(v.ToString(), "curl/7.68.0");
}

TEST(ProductInfoHeaderValueTests, CommentConstructor) {
    ProductInfoHeaderValue v("(compatible)");
    EXPECT_FALSE(v.getProductProperty().has_value());
    EXPECT_EQ(v.getCommentProperty(), "(compatible)");
    EXPECT_EQ(v.ToString(), "(compatible)");
}

TEST(ProductInfoHeaderValueTests, CommentConstructor_Nested) {
    ProductInfoHeaderValue v("(outer (inner) text)");
    EXPECT_EQ(v.getCommentProperty(), "(outer (inner) text)");
}

TEST(ProductInfoHeaderValueTests, CommentConstructor_Unbalanced_Throws) {
    EXPECT_THROW(ProductInfoHeaderValue("(unbalanced"), System::FormatException);
}

TEST(ProductInfoHeaderValueTests, CommentConstructor_NoParens_Throws) {
    EXPECT_THROW(ProductInfoHeaderValue("plain text"), System::FormatException);
}

TEST(ProductInfoHeaderValueTests, Parse_Product) {
    auto v = ProductInfoHeaderValue::Parse("Mozilla/5.0");
    ASSERT_TRUE(v.getProductProperty().has_value());
    EXPECT_EQ(v.getProductProperty()->getNameProperty(), "Mozilla");
    EXPECT_EQ(v.getProductProperty()->getVersionProperty(), "5.0");
}

TEST(ProductInfoHeaderValueTests, Parse_Comment) {
    auto v = ProductInfoHeaderValue::Parse("(compatible; MSIE 6.0)");
    EXPECT_EQ(v.getCommentProperty(), "(compatible; MSIE 6.0)");
}

TEST(ProductInfoHeaderValueTests, Parse_Invalid_Throws) {
    EXPECT_THROW(ProductInfoHeaderValue::Parse("bad name"), System::FormatException);
}

TEST(ProductInfoHeaderValueTests, TryParse_Invalid_ReturnsFalse) {
    ProductInfoHeaderValue result("(placeholder)");
    EXPECT_FALSE(ProductInfoHeaderValue::TryParse("bad name", result));
}

TEST(ProductInfoHeaderValueTests, Equals_SameProduct) {
    ProductInfoHeaderValue a("MyApp", "1.0");
    ProductInfoHeaderValue b("MyApp", "1.0");
    EXPECT_TRUE(a == b);
}

TEST(ProductInfoHeaderValueTests, Equals_ProductVsComment_False) {
    ProductInfoHeaderValue a("MyApp", "1.0");
    ProductInfoHeaderValue b("(comment)");
    EXPECT_FALSE(a == b);
}

TEST(ProductInfoHeaderValueTests, Equals_CommentsCaseSensitive) {
    ProductInfoHeaderValue a("(Comment)");
    ProductInfoHeaderValue b("(comment)");
    EXPECT_FALSE(a == b);
}

TEST(ProductInfoHeaderValueTests, GetHashCode_ConsistentWithEquals) {
    ProductInfoHeaderValue a("MyApp", "1.0");
    ProductInfoHeaderValue b("MyApp", "1.0");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}
