// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UriHostNameType.hpp"

using System::UriHostNameType;

TEST(UriHostNameTypeTest, UnknownIsZero) {
    EXPECT_EQ(static_cast<int>(UriHostNameType::Unknown), 0);
}

TEST(UriHostNameTypeTest, BasicIsOne) {
    EXPECT_EQ(static_cast<int>(UriHostNameType::Basic), 1);
}

TEST(UriHostNameTypeTest, DnsIsTwo) {
    EXPECT_EQ(static_cast<int>(UriHostNameType::Dns), 2);
}

TEST(UriHostNameTypeTest, IPv4IsThree) {
    EXPECT_EQ(static_cast<int>(UriHostNameType::IPv4), 3);
}

TEST(UriHostNameTypeTest, IPv6IsFour) {
    EXPECT_EQ(static_cast<int>(UriHostNameType::IPv6), 4);
}

TEST(UriHostNameTypeTest, DistinctValues) {
    EXPECT_NE(UriHostNameType::Dns, UriHostNameType::IPv4);
    EXPECT_NE(UriHostNameType::IPv4, UriHostNameType::IPv6);
}
