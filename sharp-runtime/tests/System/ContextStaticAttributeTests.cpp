// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <memory>
#include "System/ContextStaticAttribute.hpp"
#include "System/Attribute.hpp"

using System::ContextStaticAttribute;
using System::Attribute;

TEST(ContextStaticAttributeTests, DefaultCtor_NoThrow) {
    EXPECT_NO_THROW(ContextStaticAttribute attr);
}

TEST(ContextStaticAttributeTests, IsA_Attribute) {
    ContextStaticAttribute attr;
    EXPECT_NE(dynamic_cast<Attribute*>(&attr), nullptr);
}

TEST(ContextStaticAttributeTests, HeapAllocatable) {
    auto attr = std::make_unique<ContextStaticAttribute>();
    EXPECT_NE(attr, nullptr);
}

TEST(ContextStaticAttributeTests, PolyDelete_ViaAttribute) {
    std::unique_ptr<Attribute> p = std::make_unique<ContextStaticAttribute>();
    EXPECT_NE(p, nullptr);
}

TEST(ContextStaticAttributeTests, GetHashCode_Consistent) {
    ContextStaticAttribute attr;
    EXPECT_EQ(attr.GetHashCode(), attr.GetHashCode());
}

TEST(ContextStaticAttributeTests, Equals_SameObject_True) {
    ContextStaticAttribute attr;
    EXPECT_TRUE(attr.Equals(attr));
}

TEST(ContextStaticAttributeTests, Equals_DifferentObjects_False) {
    ContextStaticAttribute a, b;
    EXPECT_FALSE(a.Equals(b));
}
