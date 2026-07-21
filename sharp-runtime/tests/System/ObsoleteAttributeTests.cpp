// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ObsoleteAttribute.hpp"

using System::ObsoleteAttribute;

TEST(ObsoleteAttributeTest, DefaultCtor) {
    ObsoleteAttribute a;
    EXPECT_TRUE(a.getMessageProperty().empty());
    EXPECT_FALSE(a.getIsErrorProperty());
}

TEST(ObsoleteAttributeTest, MessageCtor) {
    ObsoleteAttribute a("use NewMethod instead");
    EXPECT_EQ(a.getMessageProperty(), "use NewMethod instead");
    EXPECT_FALSE(a.getIsErrorProperty());
}

TEST(ObsoleteAttributeTest, MessageAndErrorCtor) {
    ObsoleteAttribute a("deprecated", true);
    EXPECT_EQ(a.getMessageProperty(), "deprecated");
    EXPECT_TRUE(a.getIsErrorProperty());
}

TEST(ObsoleteAttributeTest, DiagnosticId) {
    ObsoleteAttribute a("msg");
    EXPECT_TRUE(a.getDiagnosticIdProperty().empty());
    a.setDiagnosticIdProperty("SYSLIB0001");
    EXPECT_EQ(a.getDiagnosticIdProperty(), "SYSLIB0001");
}

TEST(ObsoleteAttributeTest, UrlFormat) {
    ObsoleteAttribute a("msg");
    EXPECT_TRUE(a.getUrlFormatProperty().empty());
    a.setUrlFormatProperty("https://example.com/{0}");
    EXPECT_EQ(a.getUrlFormatProperty(), "https://example.com/{0}");
}

TEST(ObsoleteAttributeTest, IsAttribute) {
    ObsoleteAttribute a("msg");
    EXPECT_NO_THROW({ System::Attribute& ref = a; (void)ref; });
}
