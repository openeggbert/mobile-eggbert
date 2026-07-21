// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ValueType.hpp"

using System::ValueType;

namespace {

// Uses base Equals (identity)
class SimpleValueType final : public ValueType {
public:
    SimpleValueType() = default;
};

// Overrides Equals with field equality
class ConcreteValueType final : public ValueType {
    int val_;
public:
    explicit ConcreteValueType(int v) : val_(v) {}
    bool Equals(const ValueType& other) const override {
        const auto* o = dynamic_cast<const ConcreteValueType*>(&other);
        return o && o->val_ == val_;
    }
    int GetHashCode() const override { return val_; }
    std::string ToString() const override { return std::to_string(val_); }
};
}

TEST(ValueTypeTest, DefaultEqualsIdentity) {
    SimpleValueType a, b;
    EXPECT_TRUE(a.Equals(a));
    EXPECT_FALSE(a.Equals(b));
}

TEST(ValueTypeTest, OverriddenEquals) {
    ConcreteValueType a(42), b(42), c(99);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_FALSE(a.Equals(c));
}

TEST(ValueTypeTest, GetHashCode) {
    ConcreteValueType a(5), b(5);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ValueTypeTest, ToString) {
    ConcreteValueType a(7);
    EXPECT_EQ(a.ToString(), "7");
}

TEST(ValueTypeTest, BaseToString) {
    ValueType* p = new ConcreteValueType(3);
    EXPECT_EQ(p->ToString(), "3");
    delete p;
}
