// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UriFormat.hpp"

using System::UriFormat;

TEST(UriFormatTest, UriEscapedValue) {
    EXPECT_EQ(static_cast<int>(UriFormat::UriEscaped), 1);
}

TEST(UriFormatTest, UnescapedValue) {
    EXPECT_EQ(static_cast<int>(UriFormat::Unescaped), 2);
}

TEST(UriFormatTest, SafeUnescapedValue) {
    EXPECT_EQ(static_cast<int>(UriFormat::SafeUnescaped), 3);
}

TEST(UriFormatTest, DistinctValues) {
    EXPECT_NE(UriFormat::UriEscaped, UriFormat::Unescaped);
    EXPECT_NE(UriFormat::Unescaped, UriFormat::SafeUnescaped);
}
