// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/CLSCompliantAttribute.hpp"

using System::CLSCompliantAttribute;

TEST(CLSCompliantAttributeTests, Ctor_True_IsCompliantTrue) {
    CLSCompliantAttribute attr(true);
    EXPECT_TRUE(attr.getIsCompliantProperty());
}

TEST(CLSCompliantAttributeTests, Ctor_False_IsCompliantFalse) {
    CLSCompliantAttribute attr(false);
    EXPECT_FALSE(attr.getIsCompliantProperty());
}
