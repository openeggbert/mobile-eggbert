// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Generic/NonRandomizedStringEqualityComparer.hpp"
#include "System/Collections/Generic/NullableComparer.hpp"
#include "System/Collections/Generic/NullableEqualityComparer.hpp"
#include "System/Collections/Generic/ObjectComparer.hpp"
#include "System/Collections/Generic/ObjectEqualityComparer.hpp"
#include <optional>
#include <string>

using namespace System::Collections::Generic;

// ---- NonRandomizedStringEqualityComparer ----
TEST(NonRandomizedStringEqualityComparerTest, EqualStrings) {
    const auto& cmp = NonRandomizedStringEqualityComparer::getOrdinalInstance();
    EXPECT_TRUE(cmp.Equals("hello", "hello"));
}

TEST(NonRandomizedStringEqualityComparerTest, UnequalStrings) {
    const auto& cmp = NonRandomizedStringEqualityComparer::getOrdinalInstance();
    EXPECT_FALSE(cmp.Equals("hello", "world"));
}

TEST(NonRandomizedStringEqualityComparerTest, HashCodeIsStable) {
    const auto& cmp = NonRandomizedStringEqualityComparer::getOrdinalInstance();
    EXPECT_EQ(cmp.GetHashCode("test"), cmp.GetHashCode("test"));
}

TEST(NonRandomizedStringEqualityComparerTest, EqualStringsHaveSameHash) {
    const auto& cmp = NonRandomizedStringEqualityComparer::getOrdinalInstance();
    EXPECT_EQ(cmp.GetHashCode("abc"), cmp.GetHashCode("abc"));
}

TEST(NonRandomizedStringEqualityComparerTest, DifferentStringsLikelyDifferentHash) {
    const auto& cmp = NonRandomizedStringEqualityComparer::getOrdinalInstance();
    EXPECT_NE(cmp.GetHashCode("hello"), cmp.GetHashCode("world"));
}

TEST(NonRandomizedStringEqualityComparerTest, IgnoreCaseEquals) {
    const auto& cmp = NonRandomizedStringEqualityComparer::getOrdinalIgnoreCaseInstance();
    EXPECT_TRUE(cmp.Equals("Hello", "hello"));
    EXPECT_FALSE(cmp.Equals("Hello", "world"));
}

TEST(NonRandomizedStringEqualityComparerTest, IgnoreCaseHashSameForDifferentCase) {
    const auto& cmp = NonRandomizedStringEqualityComparer::getOrdinalIgnoreCaseInstance();
    EXPECT_EQ(cmp.GetHashCode("ABC"), cmp.GetHashCode("abc"));
}

// ---- NullableComparer ----
TEST(NullableComparerTest, BothNull) {
    NullableComparer<int> cmp;
    EXPECT_EQ(cmp.Compare(std::nullopt, std::nullopt), 0);
}

TEST(NullableComparerTest, NullLessThanValue) {
    NullableComparer<int> cmp;
    EXPECT_LT(cmp.Compare(std::nullopt, 5), 0);
}

TEST(NullableComparerTest, ValueGreaterThanNull) {
    NullableComparer<int> cmp;
    EXPECT_GT(cmp.Compare(5, std::nullopt), 0);
}

TEST(NullableComparerTest, EqualValues) {
    NullableComparer<int> cmp;
    EXPECT_EQ(cmp.Compare(3, 3), 0);
}

TEST(NullableComparerTest, OrderedValues) {
    NullableComparer<int> cmp;
    EXPECT_LT(cmp.Compare(1, 2), 0);
    EXPECT_GT(cmp.Compare(2, 1), 0);
}

// ---- NullableEqualityComparer ----
TEST(NullableEqualityComparerTest, BothNullAreEqual) {
    NullableEqualityComparer<int> cmp;
    EXPECT_TRUE(cmp.Equals(std::nullopt, std::nullopt));
}

TEST(NullableEqualityComparerTest, NullAndValueNotEqual) {
    NullableEqualityComparer<int> cmp;
    EXPECT_FALSE(cmp.Equals(std::nullopt, 5));
    EXPECT_FALSE(cmp.Equals(5, std::nullopt));
}

TEST(NullableEqualityComparerTest, EqualValues) {
    NullableEqualityComparer<int> cmp;
    EXPECT_TRUE(cmp.Equals(7, 7));
}

TEST(NullableEqualityComparerTest, UnequalValues) {
    NullableEqualityComparer<int> cmp;
    EXPECT_FALSE(cmp.Equals(3, 4));
}

TEST(NullableEqualityComparerTest, HashOfNullIsZero) {
    NullableEqualityComparer<int> cmp;
    EXPECT_EQ(cmp.GetHashCode(std::nullopt), 0u);
}

TEST(NullableEqualityComparerTest, HashOfValueIsConsistent) {
    NullableEqualityComparer<int> cmp;
    EXPECT_EQ(cmp.GetHashCode(42), cmp.GetHashCode(42));
}

// ---- ObjectComparer ----
TEST(ObjectComparerTest, LessThan) {
    ObjectComparer<int> cmp;
    EXPECT_LT(cmp.Compare(1, 2), 0);
}

TEST(ObjectComparerTest, Equal) {
    ObjectComparer<int> cmp;
    EXPECT_EQ(cmp.Compare(5, 5), 0);
}

TEST(ObjectComparerTest, GreaterThan) {
    ObjectComparer<int> cmp;
    EXPECT_GT(cmp.Compare(3, 1), 0);
}

TEST(ObjectComparerTest, StringComparison) {
    ObjectComparer<std::string> cmp;
    EXPECT_LT(cmp.Compare("apple", "banana"), 0);
    EXPECT_EQ(cmp.Compare("cat", "cat"), 0);
}

// ---- ObjectEqualityComparer ----
TEST(ObjectEqualityComparerTest, EqualInts) {
    ObjectEqualityComparer<int> cmp;
    EXPECT_TRUE(cmp.Equals(42, 42));
}

TEST(ObjectEqualityComparerTest, UnequalInts) {
    ObjectEqualityComparer<int> cmp;
    EXPECT_FALSE(cmp.Equals(1, 2));
}

TEST(ObjectEqualityComparerTest, HashConsistent) {
    ObjectEqualityComparer<int> cmp;
    EXPECT_EQ(cmp.GetHashCode(99), cmp.GetHashCode(99));
}

TEST(ObjectEqualityComparerTest, EqualObjectsSameHash) {
    ObjectEqualityComparer<std::string> cmp;
    EXPECT_EQ(cmp.GetHashCode("hello"), cmp.GetHashCode("hello"));
}

TEST(ObjectEqualityComparerTest, StringEquality) {
    ObjectEqualityComparer<std::string> cmp;
    EXPECT_TRUE(cmp.Equals("foo", "foo"));
    EXPECT_FALSE(cmp.Equals("foo", "bar"));
}
