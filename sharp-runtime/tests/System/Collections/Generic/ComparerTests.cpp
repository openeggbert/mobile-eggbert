// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Generic/Comparer.hpp"
#include <string>
#include <memory>

using System::Collections::Generic::Comparer;

TEST(GenericComparerTest, DefaultIntLess) {
    EXPECT_LT(Comparer<int>::Default().Compare(1, 2), 0);
}

TEST(GenericComparerTest, DefaultIntEqual) {
    EXPECT_EQ(Comparer<int>::Default().Compare(5, 5), 0);
}

TEST(GenericComparerTest, DefaultIntGreater) {
    EXPECT_GT(Comparer<int>::Default().Compare(10, 3), 0);
}

TEST(GenericComparerTest, DefaultStringSingleton) {
    const auto& a = Comparer<std::string>::Default();
    const auto& b = Comparer<std::string>::Default();
    EXPECT_EQ(&a, &b);
}

TEST(GenericComparerTest, DefaultStringOrdering) {
    const auto& c = Comparer<std::string>::Default();
    EXPECT_LT(c.Compare("apple", "banana"), 0);
    EXPECT_EQ(c.Compare("same", "same"), 0);
    EXPECT_GT(c.Compare("z", "a"), 0);
}

TEST(GenericComparerTest, CreateFromLambdaReversed) {
    std::unique_ptr<Comparer<int>> c(
        Comparer<int>::Create([](const int& x, const int& y){ return y - x; })
    );
    EXPECT_GT(c->Compare(1, 10), 0);
    EXPECT_EQ(c->Compare(5, 5), 0);
    EXPECT_LT(c->Compare(10, 1), 0);
}

TEST(GenericComparerTest, ImplementsIComparer) {
    const System::Collections::Generic::IComparer<int>& ic = Comparer<int>::Default();
    EXPECT_LT(ic.Compare(3, 7), 0);
}
