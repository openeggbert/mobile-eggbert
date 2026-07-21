// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/FlagsAttribute.hpp"
#include "System/Attribute.hpp"

TEST(FlagsAttributeTests, DefaultCtor_DoesNotThrow) {
    EXPECT_NO_THROW(System::FlagsAttribute fa);
}

TEST(FlagsAttributeTests, IsA_Attribute) {
    System::FlagsAttribute fa;
    EXPECT_NE(dynamic_cast<System::Attribute*>(&fa), nullptr);
}
