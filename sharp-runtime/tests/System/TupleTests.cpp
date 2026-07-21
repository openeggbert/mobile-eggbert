// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <string>
#include "System/Tuple.hpp"
#include "System/TupleExtensions.hpp"

using System::Tuple2;
using System::Tuple3;
using System::Tuple4;

// ---------------------------------------------------------------------------
// Tuple2
// ---------------------------------------------------------------------------

TEST(TupleTests, Tuple2_ConstructAndAccessItems) {
    Tuple2<int, std::string> t(42, "hello");
    EXPECT_EQ(t.Item1, 42);
    EXPECT_EQ(t.Item2, "hello");
}

TEST(TupleTests, Tuple2_IntInt_Items) {
    Tuple2<int, int> t(10, 20);
    EXPECT_EQ(t.Item1, 10);
    EXPECT_EQ(t.Item2, 20);
}

TEST(TupleTests, Tuple2_EqualityTrue) {
    Tuple2<int, int> a(1, 2);
    Tuple2<int, int> b(1, 2);
    EXPECT_TRUE(a == b);
}

TEST(TupleTests, Tuple2_EqualityFalse_DifferentItem1) {
    Tuple2<int, int> a(1, 2);
    Tuple2<int, int> b(9, 2);
    EXPECT_FALSE(a == b);
}

TEST(TupleTests, Tuple2_EqualityFalse_DifferentItem2) {
    Tuple2<int, int> a(1, 2);
    Tuple2<int, int> b(1, 9);
    EXPECT_FALSE(a == b);
}

TEST(TupleTests, Tuple2_InequalityTrue) {
    Tuple2<int, int> a(1, 2);
    Tuple2<int, int> b(3, 4);
    EXPECT_TRUE(a != b);
}

TEST(TupleTests, Tuple2_InequalityFalse_SameValues) {
    Tuple2<int, int> a(5, 6);
    Tuple2<int, int> b(5, 6);
    EXPECT_FALSE(a != b);
}

TEST(TupleTests, Tuple2_StringString) {
    Tuple2<std::string, std::string> t("key", "value");
    EXPECT_EQ(t.Item1, "key");
    EXPECT_EQ(t.Item2, "value");
}

TEST(TupleTests, Tuple2_DoubleDouble_Equality) {
    Tuple2<double, double> a(1.5, 2.5);
    Tuple2<double, double> b(1.5, 2.5);
    EXPECT_TRUE(a == b);
}

TEST(TupleTests, Tuple2_BoolInt) {
    Tuple2<bool, int> t(true, 99);
    EXPECT_TRUE(t.Item1);
    EXPECT_EQ(t.Item2, 99);
}

// ---------------------------------------------------------------------------
// Tuple3
// ---------------------------------------------------------------------------

TEST(TupleTests, Tuple3_ConstructAndAccessItems) {
    Tuple3<int, std::string, double> t(1, "abc", 3.14);
    EXPECT_EQ(t.Item1, 1);
    EXPECT_EQ(t.Item2, "abc");
    EXPECT_DOUBLE_EQ(t.Item3, 3.14);
}

TEST(TupleTests, Tuple3_IntIntInt_Items) {
    Tuple3<int, int, int> t(10, 20, 30);
    EXPECT_EQ(t.Item1, 10);
    EXPECT_EQ(t.Item2, 20);
    EXPECT_EQ(t.Item3, 30);
}

TEST(TupleTests, Tuple3_EqualityTrue) {
    Tuple3<int, int, int> a(1, 2, 3);
    Tuple3<int, int, int> b(1, 2, 3);
    EXPECT_TRUE(a == b);
}

TEST(TupleTests, Tuple3_EqualityFalse_DifferentItem3) {
    Tuple3<int, int, int> a(1, 2, 3);
    Tuple3<int, int, int> b(1, 2, 9);
    EXPECT_FALSE(a == b);
}

TEST(TupleTests, Tuple3_InequalityTrue) {
    Tuple3<int, int, int> a(1, 2, 3);
    Tuple3<int, int, int> b(4, 5, 6);
    EXPECT_TRUE(a != b);
}

TEST(TupleTests, Tuple3_InequalityFalse_SameValues) {
    Tuple3<int, int, int> a(7, 8, 9);
    Tuple3<int, int, int> b(7, 8, 9);
    EXPECT_FALSE(a != b);
}

TEST(TupleTests, Tuple3_StringIntBool) {
    Tuple3<std::string, int, bool> t("x", 42, false);
    EXPECT_EQ(t.Item1, "x");
    EXPECT_EQ(t.Item2, 42);
    EXPECT_FALSE(t.Item3);
}

// ---------------------------------------------------------------------------
// Tuple4
// ---------------------------------------------------------------------------

TEST(TupleTests, Tuple4_ConstructAndAccessItems) {
    Tuple4<int, int, int, int> t(1, 2, 3, 4);
    EXPECT_EQ(t.Item1, 1);
    EXPECT_EQ(t.Item2, 2);
    EXPECT_EQ(t.Item3, 3);
    EXPECT_EQ(t.Item4, 4);
}

TEST(TupleTests, Tuple4_MixedTypes) {
    Tuple4<std::string, int, double, bool> t("hello", 7, 2.5, true);
    EXPECT_EQ(t.Item1, "hello");
    EXPECT_EQ(t.Item2, 7);
    EXPECT_DOUBLE_EQ(t.Item3, 2.5);
    EXPECT_TRUE(t.Item4);
}

TEST(TupleTests, Tuple4_AllZero) {
    Tuple4<int, int, int, int> t(0, 0, 0, 0);
    EXPECT_EQ(t.Item1, 0);
    EXPECT_EQ(t.Item2, 0);
    EXPECT_EQ(t.Item3, 0);
    EXPECT_EQ(t.Item4, 0);
}

// ===========================================================================
// TupleExtensions — Deconstruct
// ===========================================================================

TEST(TupleExtensionsTests, Deconstruct_Tuple2) {
    System::Tuple2<int, std::string> t(42, "hello");
    int a; std::string b;
    t.Deconstruct(a, b);
    EXPECT_EQ(a, 42);
    EXPECT_EQ(b, "hello");
}

TEST(TupleExtensionsTests, Deconstruct_Tuple3) {
    System::Tuple3<int, double, std::string> t(1, 2.5, "x");
    int a; double b; std::string c;
    t.Deconstruct(a, b, c);
    EXPECT_EQ(a, 1);
    EXPECT_DOUBLE_EQ(b, 2.5);
    EXPECT_EQ(c, "x");
}

TEST(TupleExtensionsTests, Deconstruct_Tuple4) {
    System::Tuple4<int, int, int, int> t(1, 2, 3, 4);
    int a, b, c, d;
    t.Deconstruct(a, b, c, d);
    EXPECT_EQ(a, 1); EXPECT_EQ(b, 2); EXPECT_EQ(c, 3); EXPECT_EQ(d, 4);
}

// ===========================================================================
// TupleExtensions — ToValueTuple / ToTuple
// ===========================================================================

TEST(TupleExtensionsTests, ToValueTuple_Tuple2) {
    System::Tuple2<int, std::string> t(7, "hi");
    auto vt = System::TupleExtensions::ToValueTuple(t);
    EXPECT_EQ(std::get<0>(vt), 7);
    EXPECT_EQ(std::get<1>(vt), "hi");
}

TEST(TupleExtensionsTests, ToValueTuple_Tuple3) {
    System::Tuple3<int, int, int> t(1, 2, 3);
    auto vt = System::TupleExtensions::ToValueTuple(t);
    EXPECT_EQ(std::get<2>(vt), 3);
}

TEST(TupleExtensionsTests, ToTuple_StdTuple2) {
    auto st = std::make_tuple(10, std::string("world"));
    auto t = System::TupleExtensions::ToTuple(st);
    EXPECT_EQ(t.Item1, 10);
    EXPECT_EQ(t.Item2, "world");
}

TEST(TupleExtensionsTests, ToTuple_StdTuple3) {
    auto st = std::make_tuple(1, 2, 3);
    auto t = System::TupleExtensions::ToTuple(st);
    EXPECT_EQ(t.Item3, 3);
}

TEST(TupleExtensionsTests, ToTuple_StdTuple4) {
    auto st = std::make_tuple(1, 2, 3, 4);
    auto t = System::TupleExtensions::ToTuple(st);
    EXPECT_EQ(t.Item4, 4);
}

TEST(TupleExtensionsTests, RoundTrip_Tuple2_ToValueTuple_ToTuple) {
    System::Tuple2<int, int> original(5, 6);
    auto vt = System::TupleExtensions::ToValueTuple(original);
    auto back = System::TupleExtensions::ToTuple(vt);
    EXPECT_EQ(back, original);
}

// ---------------------------------------------------------------------------
// CompareTo / operator< (element-by-element, first non-zero wins)
// ---------------------------------------------------------------------------

TEST(TupleCompareTests, Tuple1_CompareTo) {
    System::Tuple1<int> a(1), b(2);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_GT(b.CompareTo(a), 0);
    EXPECT_EQ(a.CompareTo(a), 0);
    EXPECT_TRUE(a < b);
}

TEST(TupleCompareTests, Tuple2_FirstItemDecides) {
    System::Tuple2<int, int> a(1, 100), b(2, 0);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_TRUE(a < b);
}

TEST(TupleCompareTests, Tuple2_FallsThroughToSecondItem) {
    System::Tuple2<int, int> a(5, 1), b(5, 2);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

TEST(TupleCompareTests, Tuple3_LexicographicOrdering) {
    System::Tuple3<int, int, int> a(1, 2, 3), b(1, 2, 4);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_EQ(a.CompareTo(a), 0);
}
