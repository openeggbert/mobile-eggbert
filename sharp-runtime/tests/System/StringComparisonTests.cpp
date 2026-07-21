// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/StringComparison.hpp"

using System::StringComparison;

TEST(StringComparisonTest, CurrentCulture_Value) {
    EXPECT_EQ(static_cast<int>(StringComparison::CurrentCulture), 0);
}

TEST(StringComparisonTest, CurrentCultureIgnoreCase_Value) {
    EXPECT_EQ(static_cast<int>(StringComparison::CurrentCultureIgnoreCase), 1);
}

TEST(StringComparisonTest, InvariantCulture_Value) {
    EXPECT_EQ(static_cast<int>(StringComparison::InvariantCulture), 2);
}

TEST(StringComparisonTest, InvariantCultureIgnoreCase_Value) {
    EXPECT_EQ(static_cast<int>(StringComparison::InvariantCultureIgnoreCase), 3);
}

TEST(StringComparisonTest, Ordinal_Value) {
    EXPECT_EQ(static_cast<int>(StringComparison::Ordinal), 4);
}

TEST(StringComparisonTest, OrdinalIgnoreCase_Value) {
    EXPECT_EQ(static_cast<int>(StringComparison::OrdinalIgnoreCase), 5);
}

TEST(StringComparisonTest, Equality) {
    StringComparison a = StringComparison::Ordinal;
    StringComparison b = StringComparison::Ordinal;
    EXPECT_EQ(a, b);
}

TEST(StringComparisonTest, Inequality) {
    EXPECT_NE(StringComparison::Ordinal, StringComparison::OrdinalIgnoreCase);
}
