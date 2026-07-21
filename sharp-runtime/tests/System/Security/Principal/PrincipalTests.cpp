// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentNullException.hpp"
#include "System/Security/Principal/GenericIdentity.hpp"
#include "System/Security/Principal/GenericPrincipal.hpp"
#include "System/Security/Principal/TokenImpersonationLevel.hpp"

using namespace System::Security::Principal;

TEST(TokenImpersonationLevelTests, Values) {
    EXPECT_EQ(static_cast<int>(TokenImpersonationLevel::None), 0);
    EXPECT_EQ(static_cast<int>(TokenImpersonationLevel::Delegation), 4);
}

TEST(GenericIdentityTests, NameOnly_IsAuthenticated) {
    GenericIdentity identity("alice");
    ASSERT_TRUE(identity.getNameProperty().has_value());
    EXPECT_EQ(*identity.getNameProperty(), "alice");
    EXPECT_TRUE(identity.getIsAuthenticatedProperty());
    EXPECT_EQ(*identity.getAuthenticationTypeProperty(), "");
}

TEST(GenericIdentityTests, EmptyName_IsNotAuthenticated) {
    GenericIdentity identity("");
    EXPECT_FALSE(identity.getIsAuthenticatedProperty());
}

TEST(GenericIdentityTests, WithAuthenticationType) {
    GenericIdentity identity("bob", "Basic");
    EXPECT_EQ(*identity.getAuthenticationTypeProperty(), "Basic");
}

TEST(GenericPrincipalTests, IsInRole_CaseInsensitiveMatch) {
    auto identity = std::make_shared<GenericIdentity>("alice");
    GenericPrincipal principal(identity, {"Admin", "User"});
    EXPECT_TRUE(principal.IsInRole("admin"));
    EXPECT_TRUE(principal.IsInRole("USER"));
    EXPECT_FALSE(principal.IsInRole("Guest"));
}

TEST(GenericPrincipalTests, Identity_ReturnsSameIdentity) {
    auto identity = std::make_shared<GenericIdentity>("alice");
    GenericPrincipal principal(identity, {});
    EXPECT_EQ(principal.getIdentityProperty(), identity);
}

// .NET's GenericPrincipal constructor throws ArgumentNullException for a null identity
// (GenericPrincipal.cs: `ArgumentNullException.ThrowIfNull(identity)`); without this check
// a null identity_ would silently propagate and crash a later, unrelated caller of
// getIdentityProperty() instead of failing fast at construction.
TEST(GenericPrincipalTests, NullIdentity_Throws) {
    std::shared_ptr<GenericIdentity> nullIdentity;
    EXPECT_THROW(GenericPrincipal(nullIdentity, {}), System::ArgumentNullException);
}
