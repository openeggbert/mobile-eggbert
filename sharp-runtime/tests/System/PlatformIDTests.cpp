// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/PlatformID.hpp"

using System::PlatformID;

TEST(PlatformIDTest, Win32S_Value) {
    EXPECT_EQ(static_cast<int>(PlatformID::Win32S), 0);
}

TEST(PlatformIDTest, Win32Windows_Value) {
    EXPECT_EQ(static_cast<int>(PlatformID::Win32Windows), 1);
}

TEST(PlatformIDTest, Win32NT_Value) {
    EXPECT_EQ(static_cast<int>(PlatformID::Win32NT), 2);
}

TEST(PlatformIDTest, WinCE_Value) {
    EXPECT_EQ(static_cast<int>(PlatformID::WinCE), 3);
}

TEST(PlatformIDTest, Unix_Value) {
    EXPECT_EQ(static_cast<int>(PlatformID::Unix), 4);
}

TEST(PlatformIDTest, Xbox_Value) {
    EXPECT_EQ(static_cast<int>(PlatformID::Xbox), 5);
}

TEST(PlatformIDTest, MacOSX_Value) {
    EXPECT_EQ(static_cast<int>(PlatformID::MacOSX), 6);
}

TEST(PlatformIDTest, Other_Value) {
    EXPECT_EQ(static_cast<int>(PlatformID::Other), 7);
}

TEST(PlatformIDTest, Equality) {
    PlatformID a = PlatformID::Unix;
    PlatformID b = PlatformID::Unix;
    EXPECT_EQ(a, b);
}

TEST(PlatformIDTest, Inequality) {
    EXPECT_NE(PlatformID::Unix, PlatformID::Win32NT);
}
