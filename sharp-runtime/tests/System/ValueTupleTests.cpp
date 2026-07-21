// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ValueTuple.hpp"

using System::ValueTuple1;
using System::ValueTuple2;
using System::ValueTuple3;
using System::ValueTuple4;
using System::ValueTuple5;
using System::ValueTuple6;
using System::ValueTuple7;

TEST(ValueTuple1Test, BasicConstruct) {
    ValueTuple1<int> t(42);
    EXPECT_EQ(t.Item1, 42);
}

TEST(ValueTuple1Test, Equality) {
    ValueTuple1<int> a(1), b(1), c(2);
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a != c);
}

TEST(ValueTuple1Test, CompareTo) {
    ValueTuple1<int> a(1), b(2);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_EQ(a.CompareTo(a), 0);
    EXPECT_GT(b.CompareTo(a), 0);
}

TEST(ValueTuple2Test, BasicConstruct) {
    ValueTuple2<int, std::string> t(1, "hello");
    EXPECT_EQ(t.Item1, 1);
    EXPECT_EQ(t.Item2, "hello");
}

TEST(ValueTuple2Test, Equality) {
    ValueTuple2<int, int> a(1, 2), b(1, 2), c(1, 3);
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a != c);
}

TEST(ValueTuple2Test, GetHashCode) {
    ValueTuple2<int, int> a(1, 2), b(1, 2);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ValueTuple3Test, BasicConstruct) {
    ValueTuple3<int, int, int> t(1, 2, 3);
    EXPECT_EQ(t.Item1, 1);
    EXPECT_EQ(t.Item2, 2);
    EXPECT_EQ(t.Item3, 3);
}

TEST(ValueTuple4Test, BasicConstruct) {
    ValueTuple4<int, int, int, int> t(10, 20, 30, 40);
    EXPECT_EQ(t.Item4, 40);
}

TEST(ValueTuple2Test, ToString) {
    ValueTuple2<int, int> t(1, 2);
    std::string s = t.ToString();
    EXPECT_NE(s.find("1"), std::string::npos);
}

// Regression tests for a code-audit finding (ticket 239): GetHashCode/CompareTo/ToString
// had test coverage for arities 1, 2 and 8 (in ValueTupleTests.cpp / SystemTypesRemainingTests.cpp)
// but nothing directly exercised arities 3-7 for these three methods.

TEST(ValueTuple3Test, GetHashCode_SameForEqualValues) {
    ValueTuple3<int, int, int> a(1, 2, 3), b(1, 2, 3);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ValueTuple3Test, CompareTo_Lexicographic) {
    ValueTuple3<int, int, int> a(1, 2, 3), b(1, 2, 4);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_GT(b.CompareTo(a), 0);
    EXPECT_EQ(a.CompareTo(a), 0);
}

TEST(ValueTuple4Test, GetHashCode_SameForEqualValues) {
    ValueTuple4<int, int, int, int> a(1, 2, 3, 4), b(1, 2, 3, 4);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ValueTuple4Test, CompareTo_Lexicographic) {
    ValueTuple4<int, int, int, int> a(1, 2, 3, 4), b(1, 2, 3, 5);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_EQ(a.CompareTo(a), 0);
}

TEST(ValueTuple4Test, ToString) {
    ValueTuple4<int, int, int, int> t(1, 2, 3, 4);
    EXPECT_EQ(t.ToString(), "(1, 2, 3, 4)");
}

TEST(ValueTuple5Test, BasicConstruct) {
    ValueTuple5<int, int, int, int, int> t(1, 2, 3, 4, 5);
    EXPECT_EQ(t.Item5, 5);
}

TEST(ValueTuple5Test, GetHashCode_SameForEqualValues) {
    ValueTuple5<int, int, int, int, int> a(1, 2, 3, 4, 5), b(1, 2, 3, 4, 5);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ValueTuple5Test, CompareTo_Lexicographic) {
    ValueTuple5<int, int, int, int, int> a(1, 2, 3, 4, 5), b(1, 2, 3, 4, 6);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_EQ(a.CompareTo(a), 0);
}

TEST(ValueTuple5Test, ToString) {
    ValueTuple5<int, int, int, int, int> t(1, 2, 3, 4, 5);
    EXPECT_EQ(t.ToString(), "(1, 2, 3, 4, 5)");
}

TEST(ValueTuple6Test, BasicConstruct) {
    ValueTuple6<int, int, int, int, int, int> t(1, 2, 3, 4, 5, 6);
    EXPECT_EQ(t.Item6, 6);
}

TEST(ValueTuple6Test, GetHashCode_SameForEqualValues) {
    ValueTuple6<int, int, int, int, int, int> a(1, 2, 3, 4, 5, 6), b(1, 2, 3, 4, 5, 6);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ValueTuple6Test, CompareTo_Lexicographic) {
    ValueTuple6<int, int, int, int, int, int> a(1, 2, 3, 4, 5, 6), b(1, 2, 3, 4, 5, 7);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_EQ(a.CompareTo(a), 0);
}

TEST(ValueTuple6Test, ToString) {
    ValueTuple6<int, int, int, int, int, int> t(1, 2, 3, 4, 5, 6);
    EXPECT_EQ(t.ToString(), "(1, 2, 3, 4, 5, 6)");
}

TEST(ValueTuple7Test, GetHashCode_SameForEqualValues) {
    ValueTuple7<int, int, int, int, int, int, int> a(1, 2, 3, 4, 5, 6, 7),
                                                     b(1, 2, 3, 4, 5, 6, 7);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ValueTuple7Test, CompareTo_Lexicographic) {
    ValueTuple7<int, int, int, int, int, int, int> a(1, 2, 3, 4, 5, 6, 7),
                                                     b(1, 2, 3, 4, 5, 6, 8);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_EQ(a.CompareTo(a), 0);
}

TEST(ValueTupleOrderingTest, ValueTuple3_OrderingOperators) {
    ValueTuple3<int, int, int> a(1, 2, 3), b(1, 2, 4);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= a);
    EXPECT_TRUE(a >= a);
}

TEST(ValueTupleOrderingTest, ValueTuple6_OrderingOperators) {
    ValueTuple6<int, int, int, int, int, int> a(1, 2, 3, 4, 5, 6), b(1, 2, 3, 4, 5, 7);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= a);
    EXPECT_TRUE(a >= a);
}
