// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/MDArray.hpp"

TEST(MDArrayTest, MinRank) {
    EXPECT_EQ(System::MDArray::MinRank, 1);
}

TEST(MDArrayTest, MaxRank) {
    EXPECT_EQ(System::MDArray::MaxRank, 32);
}
