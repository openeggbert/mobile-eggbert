// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/StringSplitOptions.hpp"

using System::StringSplitOptions;

TEST(StringSplitOptionsTest, None_Value) {
    EXPECT_EQ(static_cast<int>(StringSplitOptions::None), 0);
}

TEST(StringSplitOptionsTest, RemoveEmptyEntries_Value) {
    EXPECT_EQ(static_cast<int>(StringSplitOptions::RemoveEmptyEntries), 1);
}

TEST(StringSplitOptionsTest, TrimEntries_Value) {
    EXPECT_EQ(static_cast<int>(StringSplitOptions::TrimEntries), 2);
}

TEST(StringSplitOptionsTest, BitwiseOr_Combined) {
    auto combined = StringSplitOptions::RemoveEmptyEntries | StringSplitOptions::TrimEntries;
    EXPECT_EQ(static_cast<int>(combined), 3);
}

TEST(StringSplitOptionsTest, BitwiseAnd_Mask) {
    auto combined = StringSplitOptions::RemoveEmptyEntries | StringSplitOptions::TrimEntries;
    auto masked = combined & StringSplitOptions::RemoveEmptyEntries;
    EXPECT_EQ(masked, StringSplitOptions::RemoveEmptyEntries);
}

TEST(StringSplitOptionsTest, BitwiseAnd_NoMatch) {
    auto masked = StringSplitOptions::TrimEntries & StringSplitOptions::RemoveEmptyEntries;
    EXPECT_EQ(masked, StringSplitOptions::None);
}
