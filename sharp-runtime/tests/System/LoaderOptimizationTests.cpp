// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/LoaderOptimization.hpp"
#include "System/LoaderOptimizationAttribute.hpp"
#include "System/Attribute.hpp"

using System::LoaderOptimization;
using System::LoaderOptimizationAttribute;

// ---------------------------------------------------------------------------
// LoaderOptimization enum values
// ---------------------------------------------------------------------------

TEST(LoaderOptimizationTests, NotSpecified_Value_IsZero) {
    EXPECT_EQ(static_cast<int>(LoaderOptimization::NotSpecified), 0);
}

TEST(LoaderOptimizationTests, SingleDomain_Value_IsOne) {
    EXPECT_EQ(static_cast<int>(LoaderOptimization::SingleDomain), 1);
}

TEST(LoaderOptimizationTests, MultiDomain_Value_IsTwo) {
    EXPECT_EQ(static_cast<int>(LoaderOptimization::MultiDomain), 2);
}

TEST(LoaderOptimizationTests, MultiDomainHost_Value_IsThree) {
    EXPECT_EQ(static_cast<int>(LoaderOptimization::MultiDomainHost), 3);
}

TEST(LoaderOptimizationTests, DomainMask_Value_IsThree) {
    EXPECT_EQ(static_cast<int>(LoaderOptimization::DomainMask), 3);
}

TEST(LoaderOptimizationTests, DomainMask_EqualsMultiDomainHost) {
    EXPECT_EQ(LoaderOptimization::DomainMask, LoaderOptimization::MultiDomainHost);
}

TEST(LoaderOptimizationTests, DisallowBindings_Value_IsFour) {
    EXPECT_EQ(static_cast<int>(LoaderOptimization::DisallowBindings), 4);
}

// ---------------------------------------------------------------------------
// LoaderOptimizationAttribute — byte constructor
// ---------------------------------------------------------------------------

TEST(LoaderOptimizationAttributeTests, ByteCtor_Zero_IsNotSpecified) {
    LoaderOptimizationAttribute attr(uint8_t(0));
    EXPECT_EQ(attr.getValueProperty(), LoaderOptimization::NotSpecified);
}

TEST(LoaderOptimizationAttributeTests, ByteCtor_One_IsSingleDomain) {
    LoaderOptimizationAttribute attr(uint8_t(1));
    EXPECT_EQ(attr.getValueProperty(), LoaderOptimization::SingleDomain);
}

TEST(LoaderOptimizationAttributeTests, ByteCtor_Two_IsMultiDomain) {
    LoaderOptimizationAttribute attr(uint8_t(2));
    EXPECT_EQ(attr.getValueProperty(), LoaderOptimization::MultiDomain);
}

TEST(LoaderOptimizationAttributeTests, ByteCtor_Three_IsMultiDomainHost) {
    LoaderOptimizationAttribute attr(uint8_t(3));
    EXPECT_EQ(attr.getValueProperty(), LoaderOptimization::MultiDomainHost);
}

TEST(LoaderOptimizationAttributeTests, ByteCtor_Four_IsDisallowBindings) {
    LoaderOptimizationAttribute attr(uint8_t(4));
    EXPECT_EQ(attr.getValueProperty(), LoaderOptimization::DisallowBindings);
}

// ---------------------------------------------------------------------------
// LoaderOptimizationAttribute — LoaderOptimization constructor
// ---------------------------------------------------------------------------

TEST(LoaderOptimizationAttributeTests, EnumCtor_NotSpecified) {
    LoaderOptimizationAttribute attr(LoaderOptimization::NotSpecified);
    EXPECT_EQ(attr.getValueProperty(), LoaderOptimization::NotSpecified);
}

TEST(LoaderOptimizationAttributeTests, EnumCtor_MultiDomain) {
    LoaderOptimizationAttribute attr(LoaderOptimization::MultiDomain);
    EXPECT_EQ(attr.getValueProperty(), LoaderOptimization::MultiDomain);
}

TEST(LoaderOptimizationAttributeTests, EnumCtor_DisallowBindings) {
    LoaderOptimizationAttribute attr(LoaderOptimization::DisallowBindings);
    EXPECT_EQ(attr.getValueProperty(), LoaderOptimization::DisallowBindings);
}

// ---------------------------------------------------------------------------
// Inheritance
// ---------------------------------------------------------------------------

TEST(LoaderOptimizationAttributeTests, IsA_Attribute_New) {
    LoaderOptimizationAttribute attr(LoaderOptimization::SingleDomain);
    EXPECT_NE(dynamic_cast<System::Attribute*>(&attr), nullptr);
}
