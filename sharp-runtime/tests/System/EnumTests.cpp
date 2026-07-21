// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Enum.hpp"

using System::Enum;

enum class Color { Red = 0, Green = 1, Blue = 2 };

enum class Perm { None = 0, Read = 1, Write = 2, Execute = 4, All = 7 };

// ---------------------------------------------------------------------------
// ToString
// ---------------------------------------------------------------------------

TEST(EnumTests, ToString_Red_IsZero) {
    EXPECT_EQ(Enum::ToString(Color::Red), "0");
}
TEST(EnumTests, ToString_Green_IsOne) {
    EXPECT_EQ(Enum::ToString(Color::Green), "1");
}
TEST(EnumTests, ToString_Blue_IsTwo) {
    EXPECT_EQ(Enum::ToString(Color::Blue), "2");
}

// ---------------------------------------------------------------------------
// ToUnderlying
// ---------------------------------------------------------------------------

TEST(EnumTests, ToUnderlying_Red_IsZero) {
    EXPECT_EQ(Enum::ToUnderlying(Color::Red), 0);
}
TEST(EnumTests, ToUnderlying_Green_IsOne) {
    EXPECT_EQ(Enum::ToUnderlying(Color::Green), 1);
}
TEST(EnumTests, ToUnderlying_Blue_IsTwo) {
    EXPECT_EQ(Enum::ToUnderlying(Color::Blue), 2);
}

// ---------------------------------------------------------------------------
// ToInt32
// ---------------------------------------------------------------------------

TEST(EnumTests, ToInt32_Red_IsZero) {
    EXPECT_EQ(Enum::ToInt32(Color::Red), 0);
}
TEST(EnumTests, ToInt32_Blue_IsTwo) {
    EXPECT_EQ(Enum::ToInt32(Color::Blue), 2);
}

// ---------------------------------------------------------------------------
// ToObject
// ---------------------------------------------------------------------------

TEST(EnumTests, ToObject_Zero_IsRed) {
    EXPECT_EQ(Enum::ToObject<Color>(0), Color::Red);
}
TEST(EnumTests, ToObject_One_IsGreen) {
    EXPECT_EQ(Enum::ToObject<Color>(1), Color::Green);
}
TEST(EnumTests, ToObject_Two_IsBlue) {
    EXPECT_EQ(Enum::ToObject<Color>(2), Color::Blue);
}

// ---------------------------------------------------------------------------
// HasFlag
// ---------------------------------------------------------------------------

TEST(EnumTests, HasFlag_All_HasRead) {
    EXPECT_TRUE(Enum::HasFlag(Perm::All, Perm::Read));
}
TEST(EnumTests, HasFlag_All_HasWrite) {
    EXPECT_TRUE(Enum::HasFlag(Perm::All, Perm::Write));
}
TEST(EnumTests, HasFlag_All_HasExecute) {
    EXPECT_TRUE(Enum::HasFlag(Perm::All, Perm::Execute));
}
TEST(EnumTests, HasFlag_Read_NotHasWrite) {
    EXPECT_FALSE(Enum::HasFlag(Perm::Read, Perm::Write));
}
TEST(EnumTests, HasFlag_None_NotHasRead) {
    EXPECT_FALSE(Enum::HasFlag(Perm::None, Perm::Read));
}
TEST(EnumTests, HasFlag_ReadWrite_HasRead) {
    Perm rw = static_cast<Perm>(static_cast<int>(Perm::Read) | static_cast<int>(Perm::Write));
    EXPECT_TRUE(Enum::HasFlag(rw, Perm::Read));
    EXPECT_TRUE(Enum::HasFlag(rw, Perm::Write));
    EXPECT_FALSE(Enum::HasFlag(rw, Perm::Execute));
}
TEST(EnumTests, HasFlag_Self_IsTrue) {
    EXPECT_TRUE(Enum::HasFlag(Color::Green, Color::Green));
}

// Regression/coverage test for a gap found while auditing reflection stubs (ticket 71):
// a zero flag (Perm::None) is always considered "present" -- (value & 0) == 0 is
// unconditionally true regardless of value, matching real .NET's documented Enum.HasFlag
// contract that a zero flag argument always returns true. This edge case (flag == 0, as
// opposed to value == 0, which the pre-existing HasFlag_None_NotHasRead test already covers)
// had no test coverage.
TEST(EnumTests, HasFlag_ZeroFlag_AlwaysTrue) {
    EXPECT_TRUE(Enum::HasFlag(Perm::Read, Perm::None));
    EXPECT_TRUE(Enum::HasFlag(Perm::All, Perm::None));
    EXPECT_TRUE(Enum::HasFlag(Perm::None, Perm::None));
}
