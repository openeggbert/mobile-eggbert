// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for System-level attribute and annotation types:
// Attribute, AttributeTargets, AttributeUsageAttribute, CLSCompliantAttribute,
// ObsoleteAttribute, FlagsAttribute, and other marker attributes.
#include <gtest/gtest.h>
#include <string>
#include "System/Attribute.hpp"
#include "System/AttributeTargets.hpp"
#include "System/AttributeUsageAttribute.hpp"
#include "System/CLSCompliantAttribute.hpp"
#include "System/ObsoleteAttribute.hpp"
#include "System/FlagsAttribute.hpp"
#include "System/ParamArrayAttribute.hpp"
#include "System/NonSerializedAttribute.hpp"
#include "System/SerializableAttribute.hpp"
#include "System/ThreadStaticAttribute.hpp"
#include "System/LoaderOptimization.hpp"
#include "System/LoaderOptimizationAttribute.hpp"

using System::Attribute;
using System::AttributeTargets;
using System::AttributeUsageAttribute;
using System::CLSCompliantAttribute;
using System::ObsoleteAttribute;

// ===========================================================================
// Attribute (base)
// ===========================================================================

TEST(AttributeTests, DefaultCtor_IsDefaultAttributeFalse) {
    Attribute a;
    EXPECT_FALSE(a.getIsDefaultAttributeProperty());
}

TEST(AttributeTests, Match_SameInstance_True) {
    Attribute a;
    EXPECT_TRUE(a.Match(a));
}

TEST(AttributeTests, Match_DifferentInstance_False) {
    Attribute a, b;
    EXPECT_FALSE(a.Match(b));
}

TEST(AttributeTests, Equals_SameInstance_True) {
    Attribute a;
    EXPECT_TRUE(a.Equals(a));
}

TEST(AttributeTests, Equals_DifferentInstance_False) {
    Attribute a, b;
    EXPECT_FALSE(a.Equals(b));
}

TEST(AttributeTests, GetHashCode_SameInstance_Consistent) {
    Attribute a;
    EXPECT_EQ(a.GetHashCode(), a.GetHashCode());
}

TEST(AttributeTests, GetHashCode_DifferentInstances_TypicallyDifferent) {
    Attribute a, b;
    // Identity-based hash: different addresses → different hashes (not guaranteed
    // by the contract, but true in practice for stack objects).
    // We just verify both calls succeed without throwing.
    (void)a.GetHashCode();
    (void)b.GetHashCode();
}

TEST(AttributeTests, TypeId_ReturnsAttributeType) {
    Attribute a;
    EXPECT_EQ(a.getTypeIdProperty(), typeid(Attribute));
}

TEST(AttributeTests, Match_DelegatesTo_Equals) {
    // Match should return the same result as Equals for the base class.
    Attribute a, b;
    EXPECT_EQ(a.Match(a), a.Equals(a));
    EXPECT_EQ(a.Match(b), a.Equals(b));
}

// ===========================================================================
// AttributeTargets
// ===========================================================================

TEST(AttributeTargetsTests, Assembly_IsOne) {
    EXPECT_EQ(static_cast<int>(AttributeTargets::Assembly), 0x0001);
}

TEST(AttributeTargetsTests, Class_IsFour) {
    EXPECT_EQ(static_cast<int>(AttributeTargets::Class), 0x0004);
}

TEST(AttributeTargetsTests, Method_Is0x40) {
    EXPECT_EQ(static_cast<int>(AttributeTargets::Method), 0x0040);
}

TEST(AttributeTargetsTests, All_ContainsClass) {
    EXPECT_NE(static_cast<int>(AttributeTargets::All & AttributeTargets::Class), 0);
}

TEST(AttributeTargetsTests, OrOperator_CombinesFlags) {
    auto combined = AttributeTargets::Class | AttributeTargets::Method;
    EXPECT_NE(static_cast<int>(combined & AttributeTargets::Class), 0);
    EXPECT_NE(static_cast<int>(combined & AttributeTargets::Method), 0);
}

TEST(AttributeTargetsTests, AndOperator_Filters) {
    auto combined = AttributeTargets::Class | AttributeTargets::Method;
    EXPECT_EQ(static_cast<int>(combined & AttributeTargets::Property), 0);
}

// ===========================================================================
// AttributeUsageAttribute
// ===========================================================================

TEST(AttributeUsageAttributeTests, Constructor_StoresValidOn) {
    AttributeUsageAttribute attr(AttributeTargets::Class);
    EXPECT_EQ(attr.getValidOnProperty(), AttributeTargets::Class);
}

TEST(AttributeUsageAttributeTests, AllowMultiple_DefaultFalse) {
    AttributeUsageAttribute attr(AttributeTargets::All);
    EXPECT_FALSE(attr.getAllowMultipleProperty());
}

TEST(AttributeUsageAttributeTests, Inherited_DefaultTrue) {
    AttributeUsageAttribute attr(AttributeTargets::All);
    EXPECT_TRUE(attr.getInheritedProperty());
}

TEST(AttributeUsageAttributeTests, SetAllowMultiple_True) {
    AttributeUsageAttribute attr(AttributeTargets::All);
    attr.setAllowMultipleProperty(true);
    EXPECT_TRUE(attr.getAllowMultipleProperty());
}

TEST(AttributeUsageAttributeTests, SetInherited_False) {
    AttributeUsageAttribute attr(AttributeTargets::All);
    attr.setInheritedProperty(false);
    EXPECT_FALSE(attr.getInheritedProperty());
}

TEST(AttributeUsageAttributeTests, ThreeArgCtor_StoresAll) {
    AttributeUsageAttribute attr(AttributeTargets::Method, true, false);
    EXPECT_EQ(attr.getValidOnProperty(), AttributeTargets::Method);
    EXPECT_TRUE(attr.getAllowMultipleProperty());
    EXPECT_FALSE(attr.getInheritedProperty());
}

TEST(AttributeUsageAttributeTests, Default_TargetsAll) {
    EXPECT_EQ(AttributeUsageAttribute::Default.getValidOnProperty(), AttributeTargets::All);
}

TEST(AttributeUsageAttributeTests, Default_AllowMultipleFalse) {
    EXPECT_FALSE(AttributeUsageAttribute::Default.getAllowMultipleProperty());
}

TEST(AttributeUsageAttributeTests, Default_InheritedTrue) {
    EXPECT_TRUE(AttributeUsageAttribute::Default.getInheritedProperty());
}

// ===========================================================================
// CLSCompliantAttribute
// ===========================================================================

TEST(CLSCompliantAttributeTests, IsCompliant_True) {
    CLSCompliantAttribute attr(true);
    EXPECT_TRUE(attr.getIsCompliantProperty());
}

TEST(CLSCompliantAttributeTests, IsCompliant_False) {
    CLSCompliantAttribute attr(false);
    EXPECT_FALSE(attr.getIsCompliantProperty());
}

// ===========================================================================
// ObsoleteAttribute
// ===========================================================================

TEST(ObsoleteAttributeTests, DefaultCtor_MessageEmpty) {
    ObsoleteAttribute attr;
    EXPECT_TRUE(attr.getMessageProperty().empty());
}

TEST(ObsoleteAttributeTests, MessageCtor_StoresMessage) {
    ObsoleteAttribute attr("Use Foo instead");
    EXPECT_EQ(attr.getMessageProperty(), "Use Foo instead");
}

TEST(ObsoleteAttributeTests, MessageIsError_IsError_True) {
    ObsoleteAttribute attr("Removed", true);
    EXPECT_TRUE(attr.getIsErrorProperty());
}

TEST(ObsoleteAttributeTests, MessageIsError_IsError_False) {
    ObsoleteAttribute attr("Deprecated", false);
    EXPECT_FALSE(attr.getIsErrorProperty());
}

TEST(ObsoleteAttributeTests, DefaultCtor_IsError_False) {
    ObsoleteAttribute attr;
    EXPECT_FALSE(attr.getIsErrorProperty());
}

TEST(ObsoleteAttributeTests, DiagnosticId_DefaultEmpty) {
    ObsoleteAttribute attr("msg");
    EXPECT_TRUE(attr.getDiagnosticIdProperty().empty());
}

TEST(ObsoleteAttributeTests, SetDiagnosticId_Stored) {
    ObsoleteAttribute attr("msg");
    attr.setDiagnosticIdProperty("SHARP001");
    EXPECT_EQ(attr.getDiagnosticIdProperty(), "SHARP001");
}

TEST(ObsoleteAttributeTests, SetUrlFormat_Stored) {
    ObsoleteAttribute attr("msg");
    attr.setUrlFormatProperty("https://example.com/{0}");
    EXPECT_EQ(attr.getUrlFormatProperty(), "https://example.com/{0}");
}

// ===========================================================================
// Marker-only attributes (instantiation only)
// ===========================================================================

TEST(MarkerAttributeTests, FlagsAttribute_DefaultCtor) {
    EXPECT_NO_THROW(System::FlagsAttribute{});
}

TEST(MarkerAttributeTests, ParamArrayAttribute_DefaultCtor) {
    EXPECT_NO_THROW(System::ParamArrayAttribute{});
}

TEST(MarkerAttributeTests, NonSerializedAttribute_DefaultCtor) {
    EXPECT_NO_THROW(System::NonSerializedAttribute{});
}

TEST(MarkerAttributeTests, SerializableAttribute_DefaultCtor) {
    EXPECT_NO_THROW(System::SerializableAttribute{});
}

TEST(MarkerAttributeTests, ThreadStaticAttribute_DefaultCtor) {
    EXPECT_NO_THROW(System::ThreadStaticAttribute{});
}

// ===========================================================================
// LoaderOptimization / LoaderOptimizationAttribute
// ===========================================================================

TEST(LoaderOptimizationTests, NotSpecified_IsZero) {
    EXPECT_EQ(static_cast<int>(System::LoaderOptimization::NotSpecified), 0);
}
TEST(LoaderOptimizationTests, SingleDomain_IsOne) {
    EXPECT_EQ(static_cast<int>(System::LoaderOptimization::SingleDomain), 1);
}
TEST(LoaderOptimizationTests, MultiDomain_IsTwo) {
    EXPECT_EQ(static_cast<int>(System::LoaderOptimization::MultiDomain), 2);
}
TEST(LoaderOptimizationTests, MultiDomainHost_IsThree) {
    EXPECT_EQ(static_cast<int>(System::LoaderOptimization::MultiDomainHost), 3);
}
TEST(LoaderOptimizationAttributeTests, Ctor_StoresValue) {
    System::LoaderOptimizationAttribute attr(System::LoaderOptimization::SingleDomain);
    EXPECT_EQ(attr.getValueProperty(), System::LoaderOptimization::SingleDomain);
}
TEST(LoaderOptimizationAttributeTests, Ctor_MultiDomain) {
    System::LoaderOptimizationAttribute attr(System::LoaderOptimization::MultiDomain);
    EXPECT_EQ(attr.getValueProperty(), System::LoaderOptimization::MultiDomain);
}
TEST(LoaderOptimizationAttributeTests, IsA_Attribute) {
    System::LoaderOptimizationAttribute attr(System::LoaderOptimization::NotSpecified);
    EXPECT_NE(dynamic_cast<System::Attribute*>(&attr), nullptr);
}
