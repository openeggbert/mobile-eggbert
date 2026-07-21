// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Predicate.hpp"
#include <string>

using System::Predicate;

TEST(PredicateTest, IntPredicate_True) {
    Predicate<int> isPositive = [](int x) { return x > 0; };
    EXPECT_TRUE(isPositive(5));
}

TEST(PredicateTest, IntPredicate_False) {
    Predicate<int> isPositive = [](int x) { return x > 0; };
    EXPECT_FALSE(isPositive(-1));
}

TEST(PredicateTest, StringPredicate) {
    Predicate<std::string> isEmpty = [](const std::string& s) { return s.empty(); };
    EXPECT_TRUE(isEmpty(""));
    EXPECT_FALSE(isEmpty("hello"));
}

TEST(PredicateTest, BoolPredicate) {
    Predicate<bool> id = [](bool b) { return b; };
    EXPECT_TRUE(id(true));
    EXPECT_FALSE(id(false));
}

TEST(PredicateTest, NullPredicateThrows) {
    Predicate<int> p;
    EXPECT_THROW(p(0), std::bad_function_call);
}

TEST(PredicateTest, CanBeUsedWithStdFunction) {
    std::function<bool(int)> f = Predicate<int>([](int x) { return x == 42; });
    EXPECT_TRUE(f(42));
    EXPECT_FALSE(f(0));
}
