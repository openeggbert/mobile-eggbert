// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/IEquatable.hpp"

// ---------------------------------------------------------------------------
// Concrete test implementations
// ---------------------------------------------------------------------------

struct Point : System::IEquatable<Point> {
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
    bool Equals(const Point& other) const override {
        return x == other.x && y == other.y;
    }
};

struct Named : System::IEquatable<Named> {
    std::string name;
    explicit Named(std::string n) : name(std::move(n)) {}
    bool Equals(const Named& other) const override {
        return name == other.name;
    }
};

// ---------------------------------------------------------------------------
// Point — value-based equality
// ---------------------------------------------------------------------------

TEST(IEquatableTests, Point_SameXY_Equals_True) {
    Point a(3, 4), b(3, 4);
    EXPECT_TRUE(a.Equals(b));
}

TEST(IEquatableTests, Point_DiffX_Equals_False) {
    Point a(1, 4), b(2, 4);
    EXPECT_FALSE(a.Equals(b));
}

TEST(IEquatableTests, Point_DiffY_Equals_False) {
    Point a(3, 1), b(3, 2);
    EXPECT_FALSE(a.Equals(b));
}

TEST(IEquatableTests, Point_SelfEquals_True) {
    Point a(5, 6);
    EXPECT_TRUE(a.Equals(a));
}

TEST(IEquatableTests, Point_BothZero_Equals_True) {
    Point a(0, 0), b(0, 0);
    EXPECT_TRUE(a.Equals(b));
}

TEST(IEquatableTests, Point_NegativeCoords_Equal) {
    Point a(-1, -2), b(-1, -2);
    EXPECT_TRUE(a.Equals(b));
}

TEST(IEquatableTests, Point_NegativeCoords_NotEqual) {
    Point a(-1, -2), b(-1, -3);
    EXPECT_FALSE(a.Equals(b));
}

// ---------------------------------------------------------------------------
// Named — string-based equality
// ---------------------------------------------------------------------------

TEST(IEquatableTests, Named_SameName_Equals_True) {
    Named a("hello"), b("hello");
    EXPECT_TRUE(a.Equals(b));
}

TEST(IEquatableTests, Named_DiffName_Equals_False) {
    Named a("foo"), b("bar");
    EXPECT_FALSE(a.Equals(b));
}

TEST(IEquatableTests, Named_Empty_Equal) {
    Named a(""), b("");
    EXPECT_TRUE(a.Equals(b));
}

TEST(IEquatableTests, Named_EmptyVsNonEmpty_NotEqual) {
    Named a(""), b("x");
    EXPECT_FALSE(a.Equals(b));
}

// ---------------------------------------------------------------------------
// Polymorphic dispatch via interface pointer
// ---------------------------------------------------------------------------

TEST(IEquatableTests, PolymorphicDispatch_PointViaInterface) {
    Point a(7, 8), b(7, 8), c(9, 10);
    System::IEquatable<Point>* ia = &a;
    EXPECT_TRUE(ia->Equals(b));
    EXPECT_FALSE(ia->Equals(c));
}

TEST(IEquatableTests, PolymorphicDispatch_NamedViaInterface) {
    Named a("test"), b("test"), c("other");
    System::IEquatable<Named>* ia = &a;
    EXPECT_TRUE(ia->Equals(b));
    EXPECT_FALSE(ia->Equals(c));
}

// ---------------------------------------------------------------------------
// Symmetry
// ---------------------------------------------------------------------------

TEST(IEquatableTests, Symmetry_AEqualsB_ImpliesBEqualsA) {
    Point a(1, 2), b(1, 2);
    EXPECT_EQ(a.Equals(b), b.Equals(a));
}

TEST(IEquatableTests, Symmetry_ANotEqualsB_ImpliesBNotEqualsA) {
    Point a(1, 2), b(3, 4);
    EXPECT_EQ(a.Equals(b), b.Equals(a));
}

// ---------------------------------------------------------------------------
// Transitivity
// ---------------------------------------------------------------------------

TEST(IEquatableTests, Transitivity_AeqB_BeqC_ImpliesAeqC) {
    Named a("x"), b("x"), c("x");
    EXPECT_TRUE(a.Equals(b));
    EXPECT_TRUE(b.Equals(c));
    EXPECT_TRUE(a.Equals(c));
}
