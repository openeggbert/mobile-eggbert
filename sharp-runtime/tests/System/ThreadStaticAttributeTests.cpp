// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ThreadStaticAttribute.hpp"

using System::ThreadStaticAttribute;

TEST(ThreadStaticAttributeTest, DefaultCtor) {
    ThreadStaticAttribute a;
    (void)a;
}

TEST(ThreadStaticAttributeTest, IsAttribute) {
    ThreadStaticAttribute a;
    EXPECT_NO_THROW({ System::Attribute& ref = a; (void)ref; });
}

TEST(ThreadStaticAttributeTest, TwoInstancesIndependent) {
    ThreadStaticAttribute a;
    ThreadStaticAttribute b;
    EXPECT_NE(&a, &b);
}
