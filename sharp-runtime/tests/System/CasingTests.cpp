// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Casing.hpp"

using System::Casing;

TEST(CasingTests, Upper_IsZero) {
    EXPECT_EQ(static_cast<int>(Casing::Upper), 0);
}

TEST(CasingTests, Lower_Is0x2020) {
    EXPECT_EQ(static_cast<unsigned int>(Casing::Lower), 0x2020U);
}

TEST(CasingTests, ValuesAreDistinct) {
    EXPECT_NE(Casing::Upper, Casing::Lower);
}
