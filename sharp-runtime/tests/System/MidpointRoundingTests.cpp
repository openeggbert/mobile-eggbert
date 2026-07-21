// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/MidpointRounding.hpp"

using System::MidpointRounding;

TEST(MidpointRoundingTest, Values) {
    EXPECT_EQ(static_cast<int>(MidpointRounding::ToEven),             0);
    EXPECT_EQ(static_cast<int>(MidpointRounding::AwayFromZero),       1);
    EXPECT_EQ(static_cast<int>(MidpointRounding::ToZero),             2);
    EXPECT_EQ(static_cast<int>(MidpointRounding::ToNegativeInfinity), 3);
    EXPECT_EQ(static_cast<int>(MidpointRounding::ToPositiveInfinity), 4);
}

TEST(MidpointRoundingTest, Distinct) {
    EXPECT_NE(MidpointRounding::ToEven, MidpointRounding::AwayFromZero);
    EXPECT_NE(MidpointRounding::ToZero, MidpointRounding::ToNegativeInfinity);
    EXPECT_NE(MidpointRounding::ToNegativeInfinity, MidpointRounding::ToPositiveInfinity);
}
