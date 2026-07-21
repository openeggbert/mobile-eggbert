// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Generic/EqualityComparer.hpp"
#include <string>

using System::Collections::Generic::EqualityComparer;

TEST(EqualityComparerTest, DefaultIntEqualTrue) {
    EXPECT_TRUE(EqualityComparer<int>::Default().Equals(5, 5));
}

TEST(EqualityComparerTest, DefaultIntEqualFalse) {
    EXPECT_FALSE(EqualityComparer<int>::Default().Equals(3, 7));
}

TEST(EqualityComparerTest, DefaultStringSingleton) {
    const auto& a = EqualityComparer<std::string>::Default();
    const auto& b = EqualityComparer<std::string>::Default();
    EXPECT_EQ(&a, &b);
}

TEST(EqualityComparerTest, DefaultStringEquals) {
    const auto& c = EqualityComparer<std::string>::Default();
    EXPECT_TRUE(c.Equals("hello", "hello"));
    EXPECT_FALSE(c.Equals("hello", "world"));
}

TEST(EqualityComparerTest, DefaultHashCodeConsistent) {
    const auto& c = EqualityComparer<int>::Default();
    EXPECT_EQ(c.GetHashCode(42), c.GetHashCode(42));
}

TEST(EqualityComparerTest, DefaultHashCodeDifferent) {
    const auto& c = EqualityComparer<int>::Default();
    EXPECT_NE(c.GetHashCode(1), c.GetHashCode(2));
}

TEST(EqualityComparerTest, CreateCustomEquals) {
    auto c = EqualityComparer<int>::Create(
        [](const int& x, const int& y){ return (x % 2) == (y % 2); }
    );
    EXPECT_TRUE(c->Equals(2, 4));
    EXPECT_FALSE(c->Equals(2, 3));
}

TEST(EqualityComparerTest, CreateCustomHashCode) {
    auto c = EqualityComparer<int>::Create(
        [](const int& x, const int& y){ return x == y; },
        [](const int& v){ return static_cast<std::size_t>(v % 10); }
    );
    EXPECT_EQ(c->GetHashCode(13), static_cast<std::size_t>(3));
}

TEST(EqualityComparerTest, CreateNoHashCodeThrows) {
    auto c = EqualityComparer<int>::Create(
        [](const int& x, const int& y){ return x == y; }
    );
    EXPECT_THROW(c->GetHashCode(1), System::NotSupportedException);
}
