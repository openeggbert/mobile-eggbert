// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <string>
#include "System/Tuple.hpp"
#include "System/TupleExtensions.hpp"

// ===========================================================================
// Tuple1
// ===========================================================================

TEST(TupleTests, Tuple1_ConstructAndAccess) {
    System::Tuple1<int> t(42);
    EXPECT_EQ(t.Item1, 42);
}

TEST(TupleTests, Tuple1_EqualityTrue) {
    System::Tuple1<int> a(5), b(5);
    EXPECT_TRUE(a == b);
}

TEST(TupleTests, Tuple1_EqualityFalse) {
    System::Tuple1<int> a(5), b(6);
    EXPECT_FALSE(a == b);
}

TEST(TupleTests, Tuple1_Inequality) {
    System::Tuple1<int> a(1), b(2);
    EXPECT_TRUE(a != b);
}

TEST(TupleTests, Tuple1_Deconstruct) {
    System::Tuple1<std::string> t("hello");
    std::string s;
    t.Deconstruct(s);
    EXPECT_EQ(s, "hello");
}

TEST(TupleTests, Tuple1_ToString) {
    System::Tuple1<int> t(7);
    EXPECT_EQ(t.ToString(), "(7)");
}

TEST(TupleTests, Tuple1_GetHashCode_Consistent) {
    System::Tuple1<int> a(99), b(99);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(TupleTests, Tuple1_GetHashCode_Differs) {
    System::Tuple1<int> a(1), b(2);
    EXPECT_NE(a.GetHashCode(), b.GetHashCode());
}

// ===========================================================================
// Tuple2 — ToString / GetHashCode (new)
// ===========================================================================

TEST(TupleTests, Tuple2_ToString) {
    System::Tuple2<int, int> t(3, 4);
    EXPECT_EQ(t.ToString(), "(3, 4)");
}

TEST(TupleTests, Tuple2_ToString_Mixed) {
    System::Tuple2<std::string, int> t("hi", 2);
    EXPECT_EQ(t.ToString(), "(hi, 2)");
}

TEST(TupleTests, Tuple2_GetHashCode_SameValues_Equal) {
    System::Tuple2<int, int> a(1, 2), b(1, 2);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(TupleTests, Tuple2_GetHashCode_DiffValues_Differ) {
    System::Tuple2<int, int> a(1, 2), b(2, 1);
    EXPECT_NE(a.GetHashCode(), b.GetHashCode());
}

// ===========================================================================
// Tuple3 — ToString / GetHashCode (new)
// ===========================================================================

TEST(TupleTests, Tuple3_ToString) {
    System::Tuple3<int, int, int> t(1, 2, 3);
    EXPECT_EQ(t.ToString(), "(1, 2, 3)");
}

TEST(TupleTests, Tuple3_GetHashCode_Consistent) {
    System::Tuple3<int, int, int> a(1, 2, 3), b(1, 2, 3);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

// ===========================================================================
// Tuple4 — equality / inequality / ToString / GetHashCode (new)
// ===========================================================================

TEST(TupleTests, Tuple4_EqualityTrue) {
    System::Tuple4<int,int,int,int> a(1,2,3,4), b(1,2,3,4);
    EXPECT_TRUE(a == b);
}

TEST(TupleTests, Tuple4_EqualityFalse_DifferentItem4) {
    System::Tuple4<int,int,int,int> a(1,2,3,4), b(1,2,3,9);
    EXPECT_FALSE(a == b);
}

TEST(TupleTests, Tuple4_InequalityTrue) {
    System::Tuple4<int,int,int,int> a(1,2,3,4), b(5,6,7,8);
    EXPECT_TRUE(a != b);
}

TEST(TupleTests, Tuple4_InequalityFalse_SameValues) {
    System::Tuple4<int,int,int,int> a(1,2,3,4), b(1,2,3,4);
    EXPECT_FALSE(a != b);
}

TEST(TupleTests, Tuple4_ToString) {
    System::Tuple4<int,int,int,int> t(1,2,3,4);
    EXPECT_EQ(t.ToString(), "(1, 2, 3, 4)");
}

TEST(TupleTests, Tuple4_GetHashCode_Consistent) {
    System::Tuple4<int,int,int,int> a(1,2,3,4), b(1,2,3,4);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

// ===========================================================================
// Tuple5
// ===========================================================================

TEST(TupleTests, Tuple5_ConstructAndAccess) {
    System::Tuple5<int,int,int,int,int> t(1,2,3,4,5);
    EXPECT_EQ(t.Item1, 1);
    EXPECT_EQ(t.Item5, 5);
}

TEST(TupleTests, Tuple5_EqualityTrue) {
    System::Tuple5<int,int,int,int,int> a(1,2,3,4,5), b(1,2,3,4,5);
    EXPECT_TRUE(a == b);
}

TEST(TupleTests, Tuple5_EqualityFalse) {
    System::Tuple5<int,int,int,int,int> a(1,2,3,4,5), b(1,2,3,4,9);
    EXPECT_FALSE(a == b);
}

TEST(TupleTests, Tuple5_Deconstruct) {
    System::Tuple5<int,int,int,int,int> t(1,2,3,4,5);
    int a,b,c,d,e;
    t.Deconstruct(a,b,c,d,e);
    EXPECT_EQ(a,1); EXPECT_EQ(e,5);
}

TEST(TupleTests, Tuple5_ToString) {
    System::Tuple5<int,int,int,int,int> t(1,2,3,4,5);
    EXPECT_EQ(t.ToString(), "(1, 2, 3, 4, 5)");
}

TEST(TupleTests, Tuple5_GetHashCode_Consistent) {
    System::Tuple5<int,int,int,int,int> a(1,2,3,4,5), b(1,2,3,4,5);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

// ===========================================================================
// Tuple6
// ===========================================================================

TEST(TupleTests, Tuple6_ConstructAndAccess) {
    System::Tuple6<int,int,int,int,int,int> t(1,2,3,4,5,6);
    EXPECT_EQ(t.Item1, 1);
    EXPECT_EQ(t.Item6, 6);
}

TEST(TupleTests, Tuple6_EqualityTrue) {
    System::Tuple6<int,int,int,int,int,int> a(1,2,3,4,5,6), b(1,2,3,4,5,6);
    EXPECT_TRUE(a == b);
}

TEST(TupleTests, Tuple6_EqualityFalse) {
    System::Tuple6<int,int,int,int,int,int> a(1,2,3,4,5,6), b(1,2,3,4,5,9);
    EXPECT_FALSE(a == b);
}

TEST(TupleTests, Tuple6_ToString) {
    System::Tuple6<int,int,int,int,int,int> t(1,2,3,4,5,6);
    EXPECT_EQ(t.ToString(), "(1, 2, 3, 4, 5, 6)");
}

TEST(TupleTests, Tuple6_GetHashCode_Consistent) {
    System::Tuple6<int,int,int,int,int,int> a(1,2,3,4,5,6), b(1,2,3,4,5,6);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

// ===========================================================================
// Tuple7
// ===========================================================================

TEST(TupleTests, Tuple7_ConstructAndAccess) {
    System::Tuple7<int,int,int,int,int,int,int> t(1,2,3,4,5,6,7);
    EXPECT_EQ(t.Item1, 1);
    EXPECT_EQ(t.Item7, 7);
}

TEST(TupleTests, Tuple7_EqualityTrue) {
    System::Tuple7<int,int,int,int,int,int,int> a(1,2,3,4,5,6,7), b(1,2,3,4,5,6,7);
    EXPECT_TRUE(a == b);
}

TEST(TupleTests, Tuple7_EqualityFalse) {
    System::Tuple7<int,int,int,int,int,int,int> a(1,2,3,4,5,6,7), b(1,2,3,4,5,6,9);
    EXPECT_FALSE(a == b);
}

TEST(TupleTests, Tuple7_Deconstruct) {
    System::Tuple7<int,int,int,int,int,int,int> t(1,2,3,4,5,6,7);
    int a,b,c,d,e,f,g;
    t.Deconstruct(a,b,c,d,e,f,g);
    EXPECT_EQ(a,1); EXPECT_EQ(g,7);
}

TEST(TupleTests, Tuple7_ToString) {
    System::Tuple7<int,int,int,int,int,int,int> t(1,2,3,4,5,6,7);
    EXPECT_EQ(t.ToString(), "(1, 2, 3, 4, 5, 6, 7)");
}

TEST(TupleTests, Tuple7_GetHashCode_Consistent) {
    System::Tuple7<int,int,int,int,int,int,int> a(1,2,3,4,5,6,7), b(1,2,3,4,5,6,7);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

// ===========================================================================
// Tuple8 -- regression coverage for a code-audit finding (ticket 246): real .NET's
// Tuple.Create supports arities 1-8 (Tuple.Create(T1..T8) wraps item8 in a Tuple1<T8> Rest
// field, verified against Tuple.cs), but this port only implemented Tuple1-Tuple7 and
// Create() up to 7 args -- unlike its sibling ValueTuple.hpp, which already supports 8.
// Tuple8/Tuple::Create(8 args) were added to close that gap.
// ===========================================================================

TEST(TupleTests, Tuple8_ConstructAndAccess) {
    System::Tuple8<int,int,int,int,int,int,int,System::Tuple1<int>> t(
        1,2,3,4,5,6,7, System::Tuple1<int>(8));
    EXPECT_EQ(t.Item1, 1);
    EXPECT_EQ(t.Item7, 7);
    EXPECT_EQ(t.Rest.Item1, 8);
}

TEST(TupleTests, Tuple8_EqualityTrue) {
    auto a = System::Tuple::Create(1,2,3,4,5,6,7,8);
    auto b = System::Tuple::Create(1,2,3,4,5,6,7,8);
    EXPECT_TRUE(a == b);
}

TEST(TupleTests, Tuple8_EqualityFalse_DifferentRest) {
    auto a = System::Tuple::Create(1,2,3,4,5,6,7,8);
    auto b = System::Tuple::Create(1,2,3,4,5,6,7,9);
    EXPECT_FALSE(a == b);
}

TEST(TupleTests, Tuple8_ToString) {
    auto t = System::Tuple::Create(1,2,3,4,5,6,7,8);
    EXPECT_EQ(t.ToString(), "(1, 2, 3, 4, 5, 6, 7, 8)");
}

TEST(TupleTests, Tuple8_CompareTo) {
    auto a = System::Tuple::Create(1,2,3,4,5,6,7,1);
    auto b = System::Tuple::Create(1,2,3,4,5,6,7,2);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_GT(b.CompareTo(a), 0);
    EXPECT_EQ(a.CompareTo(a), 0);
}

TEST(TupleTests, Tuple8_GetHashCode_Consistent) {
    auto a = System::Tuple::Create(1,2,3,4,5,6,7,8);
    auto b = System::Tuple::Create(1,2,3,4,5,6,7,8);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

// ===========================================================================
// Tuple::Create factory
// ===========================================================================

TEST(TupleTests, TupleCreate1) {
    auto t = System::Tuple::Create(42);
    EXPECT_EQ(t.Item1, 42);
}

TEST(TupleTests, TupleCreate2) {
    auto t = System::Tuple::Create(1, std::string("hi"));
    EXPECT_EQ(t.Item1, 1);
    EXPECT_EQ(t.Item2, "hi");
}

TEST(TupleTests, TupleCreate3) {
    auto t = System::Tuple::Create(1, 2, 3);
    EXPECT_EQ(t.Item3, 3);
}

TEST(TupleTests, TupleCreate4) {
    auto t = System::Tuple::Create(1, 2, 3, 4);
    EXPECT_EQ(t.Item4, 4);
}

TEST(TupleTests, TupleCreate5) {
    auto t = System::Tuple::Create(1, 2, 3, 4, 5);
    EXPECT_EQ(t.Item5, 5);
}

TEST(TupleTests, TupleCreate6) {
    auto t = System::Tuple::Create(1, 2, 3, 4, 5, 6);
    EXPECT_EQ(t.Item6, 6);
}

TEST(TupleTests, TupleCreate7) {
    auto t = System::Tuple::Create(1, 2, 3, 4, 5, 6, 7);
    EXPECT_EQ(t.Item7, 7);
}

TEST(TupleTests, TupleCreate8) {
    auto t = System::Tuple::Create(1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(t.Item7, 7);
    EXPECT_EQ(t.Rest.Item1, 8);
}

TEST(TupleTests, TupleCreate2_TypeDeduction) {
    auto t = System::Tuple::Create(3.14, true);
    EXPECT_DOUBLE_EQ(t.Item1, 3.14);
    EXPECT_TRUE(t.Item2);
}

// ===========================================================================
// TupleExtensions — new types
// ===========================================================================

TEST(TupleExtensionsTests, ToValueTuple_Tuple1) {
    System::Tuple1<int> t(99);
    auto vt = System::TupleExtensions::ToValueTuple(t);
    EXPECT_EQ(std::get<0>(vt), 99);
}

TEST(TupleExtensionsTests, ToTuple_StdTuple1) {
    auto st = std::make_tuple(42);
    auto t = System::TupleExtensions::ToTuple(st);
    EXPECT_EQ(t.Item1, 42);
}

TEST(TupleExtensionsTests, ToValueTuple_Tuple5) {
    System::Tuple5<int,int,int,int,int> t(1,2,3,4,5);
    auto vt = System::TupleExtensions::ToValueTuple(t);
    EXPECT_EQ(std::get<4>(vt), 5);
}

TEST(TupleExtensionsTests, ToTuple_StdTuple5) {
    auto st = std::make_tuple(1,2,3,4,5);
    auto t = System::TupleExtensions::ToTuple(st);
    EXPECT_EQ(t.Item5, 5);
}

TEST(TupleExtensionsTests, ToValueTuple_Tuple6) {
    System::Tuple6<int,int,int,int,int,int> t(1,2,3,4,5,6);
    auto vt = System::TupleExtensions::ToValueTuple(t);
    EXPECT_EQ(std::get<5>(vt), 6);
}

TEST(TupleExtensionsTests, ToTuple_StdTuple6) {
    auto st = std::make_tuple(1,2,3,4,5,6);
    auto t = System::TupleExtensions::ToTuple(st);
    EXPECT_EQ(t.Item6, 6);
}

TEST(TupleExtensionsTests, ToValueTuple_Tuple7) {
    System::Tuple7<int,int,int,int,int,int,int> t(1,2,3,4,5,6,7);
    auto vt = System::TupleExtensions::ToValueTuple(t);
    EXPECT_EQ(std::get<6>(vt), 7);
}

TEST(TupleExtensionsTests, ToTuple_StdTuple7) {
    auto st = std::make_tuple(1,2,3,4,5,6,7);
    auto t = System::TupleExtensions::ToTuple(st);
    EXPECT_EQ(t.Item7, 7);
}

TEST(TupleExtensionsTests, RoundTrip_Tuple1) {
    System::Tuple1<int> original(77);
    auto vt = System::TupleExtensions::ToValueTuple(original);
    auto back = System::TupleExtensions::ToTuple(vt);
    EXPECT_EQ(back, original);
}
