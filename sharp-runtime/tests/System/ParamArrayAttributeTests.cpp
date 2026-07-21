// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ParamArrayAttribute.hpp"

using System::ParamArrayAttribute;

TEST(ParamArrayAttributeTest, DefaultCtor) {
    ParamArrayAttribute a;
    (void)a;
}

TEST(ParamArrayAttributeTest, IsAttribute) {
    ParamArrayAttribute a;
    EXPECT_NO_THROW({ System::Attribute& ref = a; (void)ref; });
}

TEST(ParamArrayAttributeTest, TwoInstancesAreIndependent) {
    ParamArrayAttribute a;
    ParamArrayAttribute b;
    EXPECT_NE(&a, &b);
}
