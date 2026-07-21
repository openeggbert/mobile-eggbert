// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Concurrent/EnumerablePartitionerOptions.hpp"

using System::Collections::Concurrent::EnumerablePartitionerOptions;

TEST(EnumerablePartitionerOptionsTest, NoneIsZero) {
    EXPECT_EQ(static_cast<int>(EnumerablePartitionerOptions::None), 0);
}

TEST(EnumerablePartitionerOptionsTest, NoBufferingIsOne) {
    EXPECT_EQ(static_cast<int>(EnumerablePartitionerOptions::NoBuffering), 1);
}

TEST(EnumerablePartitionerOptionsTest, BitwiseOr) {
    auto combined = EnumerablePartitionerOptions::None | EnumerablePartitionerOptions::NoBuffering;
    EXPECT_EQ(static_cast<int>(combined), 1);
}

TEST(EnumerablePartitionerOptionsTest, BitwiseAnd) {
    auto combined = EnumerablePartitionerOptions::NoBuffering & EnumerablePartitionerOptions::NoBuffering;
    EXPECT_EQ(static_cast<int>(combined), 1);
}

TEST(EnumerablePartitionerOptionsTest, AndWithNone) {
    auto combined = EnumerablePartitionerOptions::NoBuffering & EnumerablePartitionerOptions::None;
    EXPECT_EQ(static_cast<int>(combined), 0);
}
