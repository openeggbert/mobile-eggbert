// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/OverflowException.hpp"
#include "System/UInt16.hpp"

using System::UInt16;

TEST(UInt16Test, MaxValue) {
    EXPECT_EQ(UInt16::MaxValue, 65535u);
}

TEST(UInt16Test, MinValue) {
    EXPECT_EQ(UInt16::MinValue, 0u);
}

TEST(UInt16Test, ParseBasic) {
    EXPECT_EQ(UInt16::Parse("1234"), 1234u);
}

TEST(UInt16Test, ParseZero) {
    EXPECT_EQ(UInt16::Parse("0"), 0u);
}

TEST(UInt16Test, ParseMaxValue) {
    EXPECT_EQ(UInt16::Parse("65535"), 65535u);
}

TEST(UInt16Test, ParseOverflowThrows) {
    EXPECT_THROW(UInt16::Parse("65536"), System::OverflowException);
}

TEST(UInt16Test, Parse_TrailingGarbage_Throws) {
    EXPECT_THROW(UInt16::Parse("5abc"), System::FormatException);
}

TEST(UInt16Test, Parse_NegativeValue_Throws) {
    EXPECT_THROW(UInt16::Parse("-1"), System::OverflowException);
}

TEST(UInt16Test, Parse_NegativeZero_Throws) {
    // Unsigned types reject any leading '-', even "-0" -- std::stoul("-0") returns the
    // literal 0 (not a huge wrapped value), which previously passed the v>MaxValue check
    // and silently returned 0 instead of throwing OverflowException.
    EXPECT_THROW(UInt16::Parse("-0"), System::OverflowException);
}

TEST(UInt16Test, TryParseSuccess) {
    uint16_t result = 0;
    EXPECT_TRUE(UInt16::TryParse("500", result));
    EXPECT_EQ(result, 500u);
}

TEST(UInt16Test, TryParseFailure) {
    uint16_t result = 0;
    EXPECT_FALSE(UInt16::TryParse("abc", result));
}

TEST(UInt16Test, ToStringDecimal) {
    EXPECT_EQ(UInt16::ToString(1000), "1000");
}

TEST(UInt16Test, ToStringHex) {
    EXPECT_EQ(UInt16::ToString(255, "X"), "FF");
}

TEST(UInt16Test, ToString_MalformedWidth_ThrowsFormatException) {
    EXPECT_THROW(UInt16::ToString(5, "Xz"), System::FormatException);
    EXPECT_THROW(UInt16::ToString(5, "X99999999999999999999"), System::FormatException);
}

TEST(UInt16Test, ToStringPadded) {
    EXPECT_EQ(UInt16::ToString(10, "D4"), "0010");
}

TEST(UInt16Test, CompareTo) {
    EXPECT_EQ(UInt16::CompareTo(10u, 20u), -1);
    EXPECT_EQ(UInt16::CompareTo(20u, 20u), 0);
    EXPECT_EQ(UInt16::CompareTo(30u, 20u), 1);
}

TEST(UInt16Test, Equals) {
    EXPECT_TRUE(UInt16::Equals(5u, 5u));
    EXPECT_FALSE(UInt16::Equals(5u, 6u));
}

TEST(UInt16Test, MaxFunction) {
    EXPECT_EQ(UInt16::Max(10u, 20u), 20u);
}

TEST(UInt16Test, MinFunction) {
    EXPECT_EQ(UInt16::Min(10u, 20u), 10u);
}

TEST(UInt16Test, Clamp) {
    EXPECT_EQ(UInt16::Clamp(150u, 100u, 200u), 150u);
    EXPECT_EQ(UInt16::Clamp(50u,  100u, 200u), 100u);
    EXPECT_EQ(UInt16::Clamp(250u, 100u, 200u), 200u);
}

TEST(UInt16Test, IsEvenInteger) {
    EXPECT_TRUE(UInt16::IsEvenInteger(4u));
    EXPECT_FALSE(UInt16::IsEvenInteger(5u));
}

TEST(UInt16Test, IsOddInteger) {
    EXPECT_TRUE(UInt16::IsOddInteger(3u));
    EXPECT_FALSE(UInt16::IsOddInteger(4u));
}

TEST(UInt16Test, IsPow2) {
    EXPECT_TRUE(UInt16::IsPow2(8u));
    EXPECT_FALSE(UInt16::IsPow2(7u));
    EXPECT_FALSE(UInt16::IsPow2(0u));
}

TEST(UInt16Test, PopCount) {
    EXPECT_EQ(UInt16::PopCount(0b1010101u), 4u);
}

TEST(UInt16Test, RotateLeft) {
    EXPECT_EQ(UInt16::RotateLeft(0x0001u, 1), 0x0002u);
}

TEST(UInt16Test, RotateRight) {
    EXPECT_EQ(UInt16::RotateRight(0x0002u, 1), 0x0001u);
}
