// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ApplicationId.hpp"
#include "System/Version.hpp"

using System::ApplicationId;
using System::Version;

static ApplicationId makeId() {
    return ApplicationId("token123", "MyApp", Version(1, 2, 3, 4), "amd64", "neutral");
}

TEST(ApplicationIdTests, GetName_CorrectValue) {
    EXPECT_EQ(makeId().getNameProperty(), "MyApp");
}
TEST(ApplicationIdTests, GetVersion_CorrectValue) {
    EXPECT_EQ(makeId().getVersionProperty().ToString(), "1.2.3.4");
}
TEST(ApplicationIdTests, GetProcessorArchitecture) {
    EXPECT_EQ(makeId().getProcessorArchitectureProperty(), "amd64");
}
TEST(ApplicationIdTests, GetCulture) {
    EXPECT_EQ(makeId().getCultureProperty(), "neutral");
}
TEST(ApplicationIdTests, GetPublicKeyToken) {
    EXPECT_EQ(makeId().getPublicKeyTokenProperty(), "token123");
}
TEST(ApplicationIdTests, ToString_ContainsName_New) {
    EXPECT_NE(makeId().ToString().find("MyApp"), std::string::npos);
}
TEST(ApplicationIdTests, ToString_ContainsVersion) {
    EXPECT_NE(makeId().ToString().find("1.2.3.4"), std::string::npos);
}
TEST(ApplicationIdTests, ToString_ContainsCulture) {
    EXPECT_NE(makeId().ToString().find("neutral"), std::string::npos);
}
TEST(ApplicationIdTests, ToString_ContainsArchitecture) {
    EXPECT_NE(makeId().ToString().find("amd64"), std::string::npos);
}
TEST(ApplicationIdTests, Copy_IsEqual) {
    auto orig = makeId();
    auto copy = orig.Copy();
    EXPECT_TRUE(copy.Equals(orig));
}
TEST(ApplicationIdTests, Copy_IsIndependent) {
    auto orig = makeId();
    auto copy = orig.Copy();
    EXPECT_EQ(copy.getNameProperty(), orig.getNameProperty());
}
TEST(ApplicationIdTests, Equals_SameFields_True) {
    EXPECT_TRUE(makeId().Equals(makeId()));
}
TEST(ApplicationIdTests, Equals_DiffName_False) {
    auto a = makeId();
    ApplicationId b("token123", "Other", Version(1,2,3,4), "amd64", "neutral");
    EXPECT_FALSE(a.Equals(b));
}
TEST(ApplicationIdTests, GetHashCode_Consistent) {
    EXPECT_EQ(makeId().GetHashCode(), makeId().GetHashCode());
}
TEST(ApplicationIdTests, OperatorEq_SameFields_True) {
    EXPECT_TRUE(makeId() == makeId());
}
TEST(ApplicationIdTests, OperatorNe_DiffName_True) {
    ApplicationId b("token123", "Other", Version(1,2,3,4), "amd64", "neutral");
    EXPECT_TRUE(makeId() != b);
}
