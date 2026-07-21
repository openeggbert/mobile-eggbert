// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UriPartial.hpp"

using System::UriPartial;

TEST(UriPartialTest, SchemeIsZero) {
    EXPECT_EQ(static_cast<int>(UriPartial::Scheme), 0);
}

TEST(UriPartialTest, AuthorityIsOne) {
    EXPECT_EQ(static_cast<int>(UriPartial::Authority), 1);
}

TEST(UriPartialTest, PathIsTwo) {
    EXPECT_EQ(static_cast<int>(UriPartial::Path), 2);
}

TEST(UriPartialTest, QueryIsThree) {
    EXPECT_EQ(static_cast<int>(UriPartial::Query), 3);
}

TEST(UriPartialTest, DistinctValues) {
    EXPECT_NE(UriPartial::Scheme, UriPartial::Path);
    EXPECT_NE(UriPartial::Authority, UriPartial::Query);
}
