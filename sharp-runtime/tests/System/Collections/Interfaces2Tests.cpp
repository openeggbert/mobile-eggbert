// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/IEnumerator.hpp"
#include "System/Collections/IEqualityComparer.hpp"
#include "System/Collections/IList.hpp"
#include "System/Collections/IStructuralComparable.hpp"
#include "System/Collections/IComparer.hpp"
#include "System/Collections/ArrayList.hpp"
#include "System/Collections/Comparer.hpp"
#include <any>
#include <vector>

using namespace System::Collections;

// -----------------------------------------------------------------------
// Minimal IEnumerator over a vector<int>
// -----------------------------------------------------------------------

class IntVectorEnumerator : public IEnumerator {
    const std::vector<int>& data_;
    int index_ = -1;
public:
    explicit IntVectorEnumerator(const std::vector<int>& d) : data_(d) {}
    bool MoveNext() override { return ++index_ < static_cast<int>(data_.size()); }
    void Reset()    override { index_ = -1; }
    void* getCurrentProperty() const override {
        return const_cast<int*>(&data_[static_cast<size_t>(index_)]);
    }
};

// -----------------------------------------------------------------------
// Minimal IEqualityComparer for int*
// -----------------------------------------------------------------------

class IntEqComparer : public IEqualityComparer {
public:
    bool Equals(const void* x, const void* y) const override {
        if (!x || !y) return x == y;
        return *static_cast<const int*>(x) == *static_cast<const int*>(y);
    }
    int GetHashCode(const void* obj) const override {
        return *static_cast<const int*>(obj);
    }
};

// -----------------------------------------------------------------------
// Minimal IStructuralComparable
// -----------------------------------------------------------------------

class IntBox : public IStructuralComparable {
    int val_;
public:
    explicit IntBox(int v) : val_(v) {}
    int CompareTo(const void* other, const IComparer& /*comparer*/) const override {
        int o = *static_cast<const int*>(other);
        return (val_ < o) ? -1 : (val_ > o) ? 1 : 0;
    }
};

// -----------------------------------------------------------------------
// IEnumerator tests
// -----------------------------------------------------------------------

TEST(IEnumeratorTest, MoveNextAndCurrent) {
    std::vector<int> data = {10, 20, 30};
    IntVectorEnumerator e(data);
    EXPECT_TRUE(e.MoveNext());
    EXPECT_EQ(*static_cast<int*>(e.getCurrentProperty()), 10);
    EXPECT_TRUE(e.MoveNext());
    EXPECT_EQ(*static_cast<int*>(e.getCurrentProperty()), 20);
    EXPECT_TRUE(e.MoveNext());
    EXPECT_EQ(*static_cast<int*>(e.getCurrentProperty()), 30);
    EXPECT_FALSE(e.MoveNext());
}

TEST(IEnumeratorTest, Reset) {
    std::vector<int> data = {1, 2};
    IntVectorEnumerator e(data);
    e.MoveNext();
    e.Reset();
    EXPECT_TRUE(e.MoveNext());
    EXPECT_EQ(*static_cast<int*>(e.getCurrentProperty()), 1);
}

TEST(IEnumeratorTest, EmptyCollection) {
    std::vector<int> data;
    IntVectorEnumerator e(data);
    EXPECT_FALSE(e.MoveNext());
}

// -----------------------------------------------------------------------
// IEqualityComparer tests
// -----------------------------------------------------------------------

TEST(IEqualityComparerTest, EqualValues) {
    IntEqComparer cmp;
    int a = 5, b = 5;
    EXPECT_TRUE(cmp.Equals(&a, &b));
}

TEST(IEqualityComparerTest, UnequalValues) {
    IntEqComparer cmp;
    int a = 3, b = 7;
    EXPECT_FALSE(cmp.Equals(&a, &b));
}

TEST(IEqualityComparerTest, NullBothEqual) {
    IntEqComparer cmp;
    EXPECT_TRUE(cmp.Equals(nullptr, nullptr));
}

TEST(IEqualityComparerTest, GetHashCodeConsistent) {
    IntEqComparer cmp;
    int a = 42;
    EXPECT_EQ(cmp.GetHashCode(&a), cmp.GetHashCode(&a));
}

// -----------------------------------------------------------------------
// IList tests (via ArrayList)
// -----------------------------------------------------------------------

TEST(IListTest, GetItemAndSetItem) {
    ArrayList al;
    al.Add(std::any(10));
    al.Add(std::any(20));
    IList& list = al;
    std::any* p = static_cast<std::any*>(list.getItem(0));
    EXPECT_EQ(std::any_cast<int>(*p), 10);
    std::any v3(99);
    list.setItem(0, &v3);
    p = static_cast<std::any*>(list.getItem(0));
    EXPECT_EQ(std::any_cast<int>(*p), 99);
}

TEST(IListTest, IsReadOnlyFalse) {
    ArrayList al;
    IList& list = al;
    EXPECT_FALSE(list.getIsReadOnlyProperty());
}

TEST(IListTest, IsFixedSizeFalse) {
    ArrayList al;
    IList& list = al;
    EXPECT_FALSE(list.getIsFixedSizeProperty());
}

// -----------------------------------------------------------------------
// IStructuralComparable tests
// -----------------------------------------------------------------------

TEST(IStructuralComparableTest, LessThan) {
    IntBox box(3);
    int other = 5;
    EXPECT_LT(box.CompareTo(&other, System::Collections::Comparer::Default()), 0);
}

TEST(IStructuralComparableTest, GreaterThan) {
    IntBox box(9);
    int other = 4;
    EXPECT_GT(box.CompareTo(&other, System::Collections::Comparer::Default()), 0);
}

TEST(IStructuralComparableTest, Equal) {
    IntBox box(7);
    int other = 7;
    EXPECT_EQ(box.CompareTo(&other, System::Collections::Comparer::Default()), 0);
}
