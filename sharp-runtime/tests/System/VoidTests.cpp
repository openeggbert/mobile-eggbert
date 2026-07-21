// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Void.hpp"

using System::Void;

TEST(VoidTest, DefaultConstruct) {
    Void v;
    (void)v;
}

TEST(VoidTest, ToStringEmpty) {
    Void v;
    EXPECT_EQ(v.ToString(), "");
}

TEST(VoidTest, EqualityAlwaysTrue) {
    Void a, b;
    EXPECT_TRUE(a == b);
}

TEST(VoidTest, InequalityAlwaysFalse) {
    Void a, b;
    EXPECT_FALSE(a != b);
}

TEST(VoidTest, UsableAsTemplateArg) {
    std::vector<Void> vec(3);
    EXPECT_EQ(vec.size(), 3u);
}
