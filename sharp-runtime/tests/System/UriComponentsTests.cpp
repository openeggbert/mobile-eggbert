// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UriComponents.hpp"

using System::UriComponents;

TEST(UriComponentsTest, SchemeValue) {
    EXPECT_EQ(static_cast<unsigned int>(UriComponents::Scheme), 0x1u);
}

TEST(UriComponentsTest, HostValue) {
    EXPECT_EQ(static_cast<unsigned int>(UriComponents::Host), 0x4u);
}

TEST(UriComponentsTest, PathValue) {
    EXPECT_EQ(static_cast<unsigned int>(UriComponents::Path), 0x10u);
}

TEST(UriComponentsTest, QueryValue) {
    EXPECT_EQ(static_cast<unsigned int>(UriComponents::Query), 0x20u);
}

TEST(UriComponentsTest, FragmentValue) {
    EXPECT_EQ(static_cast<unsigned int>(UriComponents::Fragment), 0x40u);
}

TEST(UriComponentsTest, BitwiseOrCombines) {
    UriComponents combo = UriComponents::Scheme | UriComponents::Host;
    EXPECT_EQ(static_cast<unsigned int>(combo), 0x5u);
}

TEST(UriComponentsTest, BitwiseAndMasks) {
    UriComponents combo = UriComponents::Scheme | UriComponents::Host;
    UriComponents masked = combo & UriComponents::Host;
    EXPECT_EQ(static_cast<unsigned int>(masked), 0x4u);
}

TEST(UriComponentsTest, HostAndPortCombo) {
    auto v = static_cast<unsigned int>(UriComponents::HostAndPort);
    EXPECT_TRUE((v & static_cast<unsigned int>(UriComponents::Host)) != 0u);
    EXPECT_TRUE((v & static_cast<unsigned int>(UriComponents::StrongPort)) != 0u);
}

TEST(UriComponentsTest, PathAndQueryCombo) {
    auto pq = static_cast<unsigned int>(UriComponents::PathAndQuery);
    EXPECT_TRUE((pq & static_cast<unsigned int>(UriComponents::Path)) != 0u);
    EXPECT_TRUE((pq & static_cast<unsigned int>(UriComponents::Query)) != 0u);
}

TEST(UriComponentsTest, AbsoluteUri_UsesPlainPortNotStrongPort) {
    // .NET: AbsoluteUri = Scheme | UserInfo | Host | Port | Path | Query | Fragment (plain Port, not StrongPort)
    auto v = static_cast<unsigned int>(UriComponents::AbsoluteUri);
    EXPECT_TRUE((v & static_cast<unsigned int>(UriComponents::Port)) != 0u);
    EXPECT_FALSE((v & static_cast<unsigned int>(UriComponents::StrongPort)) != 0u);
}

TEST(UriComponentsTest, SchemeAndServerCombo) {
    auto v = static_cast<unsigned int>(UriComponents::SchemeAndServer);
    EXPECT_EQ(v, static_cast<unsigned int>(UriComponents::Scheme) |
                 static_cast<unsigned int>(UriComponents::Host) |
                 static_cast<unsigned int>(UriComponents::Port));
}

TEST(UriComponentsTest, HttpRequestUrlCombo) {
    auto v = static_cast<unsigned int>(UriComponents::HttpRequestUrl);
    EXPECT_EQ(v, static_cast<unsigned int>(UriComponents::Scheme) |
                 static_cast<unsigned int>(UriComponents::Host) |
                 static_cast<unsigned int>(UriComponents::Port) |
                 static_cast<unsigned int>(UriComponents::Path) |
                 static_cast<unsigned int>(UriComponents::Query));
}
