// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Base64FormattingOptions.hpp"

using System::Base64FormattingOptions;

TEST(Base64FormattingOptionsTests, None_IsZero) {
    EXPECT_EQ(static_cast<int>(Base64FormattingOptions::None), 0);
}

TEST(Base64FormattingOptionsTests, InsertLineBreaks_IsOne) {
    EXPECT_EQ(static_cast<int>(Base64FormattingOptions::InsertLineBreaks), 1);
}

TEST(Base64FormattingOptionsTests, ValuesAreDistinct) {
    EXPECT_NE(Base64FormattingOptions::None, Base64FormattingOptions::InsertLineBreaks);
}
