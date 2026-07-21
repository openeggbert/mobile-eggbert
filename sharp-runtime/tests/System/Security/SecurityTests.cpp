// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Security/SecurityAttributes.hpp"
#include "System/Security/SecurityException.hpp"
#include "System/Security/VerificationException.hpp"

using System::Security::SecurityRuleSet;
using System::Security::PartialTrustVisibilityLevel;
using System::Security::AllowPartiallyTrustedCallersAttribute;
using System::Security::SecurityCriticalAttribute;
using System::Security::SecuritySafeCriticalAttribute;
using System::Security::SecurityTransparentAttribute;
using System::Security::DynamicSecurityMethodAttribute;
using System::Security::SuppressUnmanagedCodeSecurityAttribute;
using System::Security::UnverifiableCodeAttribute;
using System::Security::SecurityRulesAttribute;
using System::Security::SecurityException;

// ===========================================================================
// SecurityRuleSet enum
// ===========================================================================

TEST(SecurityRuleSetTests, None_IsZero) {
    EXPECT_EQ(static_cast<int>(SecurityRuleSet::None), 0);
}

TEST(SecurityRuleSetTests, Level1_IsOne) {
    EXPECT_EQ(static_cast<int>(SecurityRuleSet::Level1), 1);
}

TEST(SecurityRuleSetTests, Level2_IsTwo) {
    EXPECT_EQ(static_cast<int>(SecurityRuleSet::Level2), 2);
}

// ===========================================================================
// PartialTrustVisibilityLevel enum
// ===========================================================================

TEST(PartialTrustVisibilityLevelTests, VisibleToAllHosts_IsZero) {
    EXPECT_EQ(static_cast<int>(PartialTrustVisibilityLevel::VisibleToAllHosts), 0);
}

TEST(PartialTrustVisibilityLevelTests, NotVisibleByDefault_IsOne) {
    EXPECT_EQ(static_cast<int>(PartialTrustVisibilityLevel::NotVisibleByDefault), 1);
}

// ===========================================================================
// AllowPartiallyTrustedCallersAttribute
// ===========================================================================

TEST(AllowPartiallyTrustedCallersAttributeTests, DefaultConstructor_Instantiates) {
    AllowPartiallyTrustedCallersAttribute attr;
    (void)attr;
    SUCCEED();
}

TEST(AllowPartiallyTrustedCallersAttributeTests, DefaultVisibilityLevel_IsVisibleToAllHosts) {
    AllowPartiallyTrustedCallersAttribute attr;
    EXPECT_EQ(attr.getPartialTrustVisibilityLevelProperty(),
              PartialTrustVisibilityLevel::VisibleToAllHosts);
}

TEST(AllowPartiallyTrustedCallersAttributeTests, SetVisibilityLevel) {
    AllowPartiallyTrustedCallersAttribute attr;
    attr.setPartialTrustVisibilityLevelProperty(PartialTrustVisibilityLevel::NotVisibleByDefault);
    EXPECT_EQ(attr.getPartialTrustVisibilityLevelProperty(),
              PartialTrustVisibilityLevel::NotVisibleByDefault);
}

// ===========================================================================
// SecurityRulesAttribute
// ===========================================================================

TEST(SecurityRulesAttributeTests, Constructor_StoresRuleSet) {
    SecurityRulesAttribute attr(SecurityRuleSet::Level2);
    EXPECT_EQ(attr.getRuleSetProperty(), SecurityRuleSet::Level2);
}

TEST(SecurityRulesAttributeTests, Constructor_Level1) {
    SecurityRulesAttribute attr(SecurityRuleSet::Level1);
    EXPECT_EQ(attr.getRuleSetProperty(), SecurityRuleSet::Level1);
}

TEST(SecurityRulesAttributeTests, DefaultSkipVerification_False) {
    SecurityRulesAttribute attr(SecurityRuleSet::Level2);
    EXPECT_FALSE(attr.skipVerificationInFullTrust_);
}

// ===========================================================================
// Marker security attributes — just instantiation
// ===========================================================================

TEST(SecurityMarkerAttributeTests, SecurityCritical_Instantiates) {
    SecurityCriticalAttribute attr;
    (void)attr;
    SUCCEED();
}

TEST(SecurityMarkerAttributeTests, SecuritySafeCritical_Instantiates) {
    SecuritySafeCriticalAttribute attr;
    (void)attr;
    SUCCEED();
}

TEST(SecurityMarkerAttributeTests, SecurityTransparent_Instantiates) {
    SecurityTransparentAttribute attr;
    (void)attr;
    SUCCEED();
}

TEST(SecurityMarkerAttributeTests, DynamicSecurityMethod_Instantiates) {
    DynamicSecurityMethodAttribute attr;
    (void)attr;
    SUCCEED();
}

TEST(SecurityMarkerAttributeTests, SuppressUnmanagedCodeSecurity_Instantiates) {
    SuppressUnmanagedCodeSecurityAttribute attr;
    (void)attr;
    SUCCEED();
}

TEST(SecurityMarkerAttributeTests, UnverifiableCode_Instantiates) {
    UnverifiableCodeAttribute attr;
    (void)attr;
    SUCCEED();
}

// ===========================================================================
// SecurityException
// ===========================================================================

TEST(SecurityExceptionTests, DefaultMessage) {
    SecurityException ex;
    EXPECT_STREQ(ex.what(), "Security error.");
}

TEST(SecurityExceptionTests, CustomMessage) {
    SecurityException ex("Access denied to resource");
    EXPECT_STREQ(ex.what(), "Access denied to resource");
}

TEST(SecurityExceptionTests, MessageWithPermissionType) {
    SecurityException ex("Not allowed", "FileIOPermission");
    std::string msg = ex.what();
    EXPECT_NE(msg.find("Not allowed"), std::string::npos);
    EXPECT_NE(msg.find("FileIOPermission"), std::string::npos);
}

TEST(SecurityExceptionTests, IsThrowable) {
    EXPECT_THROW(throw SecurityException(), SecurityException);
}

TEST(SecurityExceptionTests, CaughtAsSystemException) {
    EXPECT_THROW(
        { try { throw SecurityException(); }
          catch (const System::SystemException&) { throw; } },
        System::SystemException);
}

TEST(SecurityExceptionTests, HResult_IsCorSecurity) {
    SecurityException ex;
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x8013150A));
}

TEST(SecurityExceptionTests, MessageAndInnerException) {
    try {
        try {
            throw std::runtime_error("root cause");
        } catch (...) {
            throw SecurityException("wrapped", std::current_exception());
        }
    } catch (const SecurityException& ex) {
        EXPECT_STREQ(ex.what(), "wrapped");
    }
}

