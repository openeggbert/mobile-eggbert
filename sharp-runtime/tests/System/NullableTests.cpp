// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Nullable.hpp"

using System::Nullable;
using System::NullableHelper;

TEST(NullableTest, DefaultCtor_HasValueFalse) {
    Nullable<int> n;
    EXPECT_FALSE(n.getHasValueProperty());
}

TEST(NullableTest, ValueCtor_HasValueTrue) {
    Nullable<int> n(42);
    EXPECT_TRUE(n.getHasValueProperty());
    EXPECT_EQ(n.getValueProperty(), 42);
}

TEST(NullableTest, GetValue_ThrowsWhenEmpty) {
    Nullable<int> n;
    EXPECT_THROW(n.getValueProperty(), System::InvalidOperationException);
}

TEST(NullableTest, GetValueOrDefault_Empty) {
    Nullable<int> n;
    EXPECT_EQ(n.GetValueOrDefault(), 0);
}

TEST(NullableTest, GetValueOrDefault_WithDefault) {
    Nullable<int> n;
    EXPECT_EQ(n.GetValueOrDefault(99), 99);
}

TEST(NullableTest, GetValueOrDefault_HasValue) {
    Nullable<int> n(7);
    EXPECT_EQ(n.GetValueOrDefault(99), 7);
}

TEST(NullableTest, Equals_BothEmpty) {
    Nullable<int> a, b;
    EXPECT_TRUE(a.Equals(b));
    EXPECT_TRUE(a == b);
}

TEST(NullableTest, Equals_SameValue) {
    Nullable<int> a(5), b(5);
    EXPECT_TRUE(a.Equals(b));
}

TEST(NullableTest, Equals_DifferentValues) {
    Nullable<int> a(5), b(6);
    EXPECT_FALSE(a.Equals(b));
    EXPECT_TRUE(a != b);
}

TEST(NullableTest, Equals_OneEmpty) {
    Nullable<int> a(5), b;
    EXPECT_FALSE(a.Equals(b));
}

TEST(NullableTest, GetHashCode_Empty_IsZero) {
    Nullable<int> n;
    EXPECT_EQ(n.GetHashCode(), 0u);
}

TEST(NullableTest, GetHashCode_WithValue) {
    Nullable<int> n(42);
    EXPECT_NE(n.GetHashCode(), 0u);
}

TEST(NullableTest, ToString_Empty) {
    Nullable<int> n;
    EXPECT_EQ(n.ToString(), "");
}

TEST(NullableTest, ToString_WithValue) {
    Nullable<int> n(123);
    EXPECT_EQ(n.ToString(), "123");
}

TEST(NullableTest, ToString_PrefersMemberToStringOverStreamOutput) {
    struct Widget {
        [[nodiscard]] std::string ToString() const { return "a-widget"; }
    };
    Nullable<Widget> n(Widget{});
    EXPECT_EQ(n.ToString(), "a-widget");
}

TEST(NullableTest, ExplicitConversion_ToUnderlyingType) {
    Nullable<int> n(42);
    EXPECT_EQ(static_cast<int>(n), 42);
}

TEST(NullableTest, ExplicitConversion_NoValue_Throws) {
    Nullable<int> n;
    EXPECT_THROW((void)static_cast<int>(n), System::InvalidOperationException);
}

TEST(NullableTest, BoolOperator) {
    Nullable<int> empty;
    Nullable<int> full(1);
    EXPECT_FALSE(static_cast<bool>(empty));
    EXPECT_TRUE(static_cast<bool>(full));
}

TEST(NullableTest, NulloptComparison) {
    Nullable<int> n;
    EXPECT_TRUE(n == std::nullopt);
    Nullable<int> m(1);
    EXPECT_TRUE(m != std::nullopt);
}

TEST(NullableHelperTest, Compare_BothNull) {
    Nullable<int> a, b;
    EXPECT_EQ(NullableHelper::Compare(a, b), 0);
}

TEST(NullableHelperTest, Compare_NullLessThanValue) {
    Nullable<int> a, b(5);
    EXPECT_LT(NullableHelper::Compare(a, b), 0);
    EXPECT_GT(NullableHelper::Compare(b, a), 0);
}

TEST(NullableHelperTest, Compare_Values) {
    Nullable<int> a(3), b(7);
    EXPECT_LT(NullableHelper::Compare(a, b), 0);
}

TEST(NullableHelperTest, Equals_BothNull) {
    Nullable<int> a, b;
    EXPECT_TRUE(NullableHelper::Equals(a, b));
}

TEST(NullableHelperTest, Equals_SameValue) {
    Nullable<int> a(5), b(5);
    EXPECT_TRUE(NullableHelper::Equals(a, b));
}
