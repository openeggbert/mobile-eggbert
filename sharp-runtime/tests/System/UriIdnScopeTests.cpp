// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UriIdnScope.hpp"

using System::UriIdnScope;

TEST(UriIdnScopeTest, NoneIsZero) {
    EXPECT_EQ(static_cast<int>(UriIdnScope::None), 0);
}

TEST(UriIdnScopeTest, AllExceptIntranetIsOne) {
    EXPECT_EQ(static_cast<int>(UriIdnScope::AllExceptIntranet), 1);
}

TEST(UriIdnScopeTest, AllIsTwo) {
    EXPECT_EQ(static_cast<int>(UriIdnScope::All), 2);
}

TEST(UriIdnScopeTest, DistinctValues) {
    EXPECT_NE(UriIdnScope::None, UriIdnScope::All);
    EXPECT_NE(UriIdnScope::AllExceptIntranet, UriIdnScope::All);
}
