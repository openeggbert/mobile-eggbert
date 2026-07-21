// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ApplicationIdentity.hpp"

using System::ApplicationIdentity;

TEST(ApplicationIdentityTests, FullName_StoredCorrectly) {
    ApplicationIdentity id("MyApp, Version=1.0.0.0, Culture=neutral");
    EXPECT_EQ(id.getFullNameProperty(), "MyApp, Version=1.0.0.0, Culture=neutral");
}

TEST(ApplicationIdentityTests, ToString_ReturnsFullName) {
    ApplicationIdentity id("MyApp, Version=1.0.0.0");
    EXPECT_EQ(id.ToString(), "MyApp, Version=1.0.0.0");
}

TEST(ApplicationIdentityTests, CodeBase_EmptyWhenNoHash) {
    ApplicationIdentity id("MyApp, Version=1.0.0.0");
    EXPECT_TRUE(id.getCodeBaseProperty().empty());
}

TEST(ApplicationIdentityTests, CodeBase_ParsedAfterHash) {
    ApplicationIdentity id("MyApp, Version=1.0.0.0#http://example.com/app");
    EXPECT_EQ(id.getCodeBaseProperty(), "http://example.com/app");
}

TEST(ApplicationIdentityTests, FullName_PreservesHashPart) {
    std::string full = "MyApp#http://example.com";
    ApplicationIdentity id(full);
    EXPECT_EQ(id.getFullNameProperty(), full);
}

TEST(ApplicationIdentityTests, CodeBase_EmptyStringInput) {
    ApplicationIdentity id("");
    EXPECT_TRUE(id.getFullNameProperty().empty());
    EXPECT_TRUE(id.getCodeBaseProperty().empty());
}
