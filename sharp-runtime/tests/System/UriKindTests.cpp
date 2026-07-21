// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UriKind.hpp"

using System::UriKind;

TEST(UriKindTest, RelativeOrAbsoluteIsZero) {
    EXPECT_EQ(static_cast<int>(UriKind::RelativeOrAbsolute), 0);
}

TEST(UriKindTest, AbsoluteIsOne) {
    EXPECT_EQ(static_cast<int>(UriKind::Absolute), 1);
}

TEST(UriKindTest, RelativeIsTwo) {
    EXPECT_EQ(static_cast<int>(UriKind::Relative), 2);
}

TEST(UriKindTest, DistinctValues) {
    EXPECT_NE(UriKind::Absolute, UriKind::Relative);
    EXPECT_NE(UriKind::Absolute, UriKind::RelativeOrAbsolute);
}
