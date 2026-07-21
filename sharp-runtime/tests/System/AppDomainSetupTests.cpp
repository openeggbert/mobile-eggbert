// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/AppDomainSetup.hpp"
#include "System/AppContext.hpp"

using System::AppDomainSetup;

TEST(AppDomainSetupTests, DefaultCtor_DoesNotThrow) {
    EXPECT_NO_THROW(AppDomainSetup());
}

TEST(AppDomainSetupTests, ApplicationBase_NonEmpty) {
    AppDomainSetup setup;
    EXPECT_FALSE(setup.getApplicationBaseProperty().empty());
}

TEST(AppDomainSetupTests, ApplicationBase_MatchesAppContext) {
    AppDomainSetup setup;
    EXPECT_EQ(setup.getApplicationBaseProperty(),
              System::AppContext::getBaseDirectoryProperty());
}

TEST(AppDomainSetupTests, TargetFrameworkName_IsEmpty) {
    AppDomainSetup setup;
    EXPECT_TRUE(setup.getTargetFrameworkNameProperty().empty());
}

TEST(AppDomainSetupTests, TwoInstances_SameApplicationBase) {
    AppDomainSetup a, b;
    EXPECT_EQ(a.getApplicationBaseProperty(), b.getApplicationBaseProperty());
}
