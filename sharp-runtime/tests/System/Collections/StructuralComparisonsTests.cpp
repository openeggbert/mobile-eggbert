// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/StructuralComparisons.hpp"

using namespace System::Collections;

namespace {

class IntComparable : public IStructuralComparable {
    int val_;
public:
    explicit IntComparable(int v) : val_(v) {}
    int CompareTo(const void* other, const IComparer&) const override {
        int o = *static_cast<const int*>(other);
        return (val_ < o) ? -1 : (val_ > o) ? 1 : 0;
    }
};

class IntEquatable : public IStructuralEquatable {
    int val_;
public:
    explicit IntEquatable(int v) : val_(v) {}
    int getVal() const { return val_; }
    bool Equals(const void* other, const IEqualityComparer&) const override {
        return val_ == static_cast<const IntEquatable*>(other)->val_;
    }
    int GetHashCode(const IEqualityComparer&) const override {
        return val_;
    }
};

} // namespace

TEST(StructuralComparisonsTest, StructuralComparerSingleton) {
    const IComparer& a = StructuralComparisons::getStructuralComparerProperty();
    const IComparer& b = StructuralComparisons::getStructuralComparerProperty();
    EXPECT_EQ(&a, &b);
}

TEST(StructuralComparisonsTest, StructuralEqualityComparerSingleton) {
    const IEqualityComparer& a = StructuralComparisons::getStructuralEqualityComparerProperty();
    const IEqualityComparer& b = StructuralComparisons::getStructuralEqualityComparerProperty();
    EXPECT_EQ(&a, &b);
}

TEST(StructuralComparisonsTest, ComparerNullNullEqual) {
    EXPECT_EQ(StructuralComparisons::getStructuralComparerProperty().Compare(nullptr, nullptr), 0);
}

TEST(StructuralComparisonsTest, ComparerNullLessThanNonNull) {
    IntComparable x(1);
    EXPECT_LT(StructuralComparisons::getStructuralComparerProperty().Compare(nullptr, &x), 0);
}

TEST(StructuralComparisonsTest, EqualityComparerNullNullEqual) {
    EXPECT_TRUE(StructuralComparisons::getStructuralEqualityComparerProperty().Equals(nullptr, nullptr));
}

TEST(StructuralComparisonsTest, EqualityComparerDelegatesEquals) {
    IntEquatable a(5), b(5), c(6);
    const auto& cmp = StructuralComparisons::getStructuralEqualityComparerProperty();
    EXPECT_TRUE(cmp.Equals(&a, &b));
    EXPECT_FALSE(cmp.Equals(&a, &c));
}

TEST(StructuralComparisonsTest, EqualityComparerDelegatesHashCode) {
    IntEquatable a(42);
    const auto& cmp = StructuralComparisons::getStructuralEqualityComparerProperty();
    EXPECT_EQ(cmp.GetHashCode(&a), 42);
}
