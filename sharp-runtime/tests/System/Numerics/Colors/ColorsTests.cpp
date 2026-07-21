// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/Numerics/Colors/Colors.hpp"

using namespace System::Numerics::Colors;

TEST(ArgbTests, ConstructAndAccessors) {
    Argb<uint8_t> color(1, 2, 3, 4);
    EXPECT_EQ(color.A, 1);
    EXPECT_EQ(color.R, 2);
    EXPECT_EQ(color.G, 3);
    EXPECT_EQ(color.B, 4);
}

TEST(ArgbTests, FromVector) {
    Argb<uint8_t> color(std::vector<uint8_t>{10, 20, 30, 40});
    EXPECT_EQ(color.A, 10);
    EXPECT_EQ(color.R, 20);
    EXPECT_EQ(color.G, 30);
    EXPECT_EQ(color.B, 40);
}

TEST(ArgbTests, FromVector_TooShort_Throws) {
    EXPECT_THROW((Argb<uint8_t>(std::vector<uint8_t>{1, 2, 3})), System::ArgumentException);
}

TEST(ArgbTests, CopyTo_TooShort_Throws) {
    Argb<uint8_t> color(1, 2, 3, 4);
    std::vector<uint8_t> dest(3);
    EXPECT_THROW(color.CopyTo(dest), System::ArgumentException);
}

TEST(ArgbTests, CopyTo_WritesInOrder) {
    Argb<uint8_t> color(1, 2, 3, 4);
    std::vector<uint8_t> dest(4);
    color.CopyTo(dest);
    EXPECT_EQ(dest, (std::vector<uint8_t>{1, 2, 3, 4}));
}

TEST(ArgbTests, Equality) {
    Argb<uint8_t> a(1, 2, 3, 4);
    Argb<uint8_t> b(1, 2, 3, 4);
    Argb<uint8_t> c(1, 2, 3, 5);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(ArgbTests, GetHashCode_MatchesForEqualColors) {
    Argb<uint8_t> a(1, 2, 3, 4);
    Argb<uint8_t> b(1, 2, 3, 4);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ArgbTests, ToString_Format) {
    Argb<uint8_t> color(1, 2, 3, 4);
    EXPECT_EQ(color.ToString(), "<1, 2, 3, 4>");
}

TEST(ArgbTests, RoundTripsThroughRgba) {
    Argb<uint8_t> a(255, 10, 20, 30);
    auto rgba = a.ToRgba();
    EXPECT_EQ(rgba.R, 10);
    EXPECT_EQ(rgba.G, 20);
    EXPECT_EQ(rgba.B, 30);
    EXPECT_EQ(rgba.A, 255);
    EXPECT_EQ(rgba.ToArgb(), a);
}

TEST(ArgbTests, CreateBigEndian_ProducesAARRGGBBOrder) {
    // 0xAARRGGBB = 0x01020304 -> A=0x01, R=0x02, G=0x03, B=0x04
    auto color = Argb<uint8_t>::CreateBigEndian(0x01020304);
    EXPECT_EQ(color.A, 0x01);
    EXPECT_EQ(color.R, 0x02);
    EXPECT_EQ(color.G, 0x03);
    EXPECT_EQ(color.B, 0x04);
}

TEST(ArgbTests, ToUInt32BigEndian_RoundTrips) {
    Argb<uint8_t> color(0x01, 0x02, 0x03, 0x04);
    uint32_t bits = Argb<uint8_t>::ToUInt32BigEndian(color);
    EXPECT_EQ(bits, 0x01020304u);
    auto back = Argb<uint8_t>::CreateBigEndian(bits);
    EXPECT_EQ(back, color);
}

TEST(ArgbTests, LittleEndian_RoundTrips) {
    Argb<uint8_t> color(0x01, 0x02, 0x03, 0x04);
    uint32_t bits = Argb<uint8_t>::ToUInt32LittleEndian(color);
    auto back = Argb<uint8_t>::CreateLittleEndian(bits);
    EXPECT_EQ(back, color);
}

// --- Rgba ----------------------------------------------------------------------------------

TEST(RgbaTests, ConstructAndAccessors) {
    Rgba<uint8_t> color(1, 2, 3, 4);
    EXPECT_EQ(color.R, 1);
    EXPECT_EQ(color.G, 2);
    EXPECT_EQ(color.B, 3);
    EXPECT_EQ(color.A, 4);
}

TEST(RgbaTests, FromVector_TooShort_Throws) {
    EXPECT_THROW((Rgba<uint8_t>(std::vector<uint8_t>{1, 2, 3})), System::ArgumentException);
}

TEST(RgbaTests, CopyTo_TooShort_Throws) {
    Rgba<uint8_t> color(1, 2, 3, 4);
    std::vector<uint8_t> dest(3);
    EXPECT_THROW(color.CopyTo(dest), System::ArgumentException);
}

TEST(RgbaTests, Equality) {
    Rgba<uint8_t> a(1, 2, 3, 4);
    Rgba<uint8_t> b(1, 2, 3, 4);
    Rgba<uint8_t> c(1, 2, 3, 5);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(RgbaTests, GetHashCode_MatchesForEqualColors) {
    Rgba<uint8_t> a(1, 2, 3, 4);
    Rgba<uint8_t> b(1, 2, 3, 4);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(RgbaTests, ToString_Format) {
    Rgba<uint8_t> color(1, 2, 3, 4);
    EXPECT_EQ(color.ToString(), "<1, 2, 3, 4>");
}

TEST(RgbaTests, RoundTripsThroughArgb) {
    Rgba<uint8_t> r(10, 20, 30, 255);
    auto argb = r.ToArgb();
    EXPECT_EQ(argb.ToRgba(), r);
}

TEST(RgbaTests, CreateBigEndian_ProducesRRGGBBAAOrder) {
    // 0xRRGGBBAA = 0x01020304 -> R=0x01, G=0x02, B=0x03, A=0x04
    auto color = Rgba<uint8_t>::CreateBigEndian(0x01020304);
    EXPECT_EQ(color.R, 0x01);
    EXPECT_EQ(color.G, 0x02);
    EXPECT_EQ(color.B, 0x03);
    EXPECT_EQ(color.A, 0x04);
}

TEST(RgbaTests, ToUInt32BigEndian_RoundTrips) {
    Rgba<uint8_t> color(0x01, 0x02, 0x03, 0x04);
    uint32_t bits = Rgba<uint8_t>::ToUInt32BigEndian(color);
    EXPECT_EQ(bits, 0x01020304u);
    auto back = Rgba<uint8_t>::CreateBigEndian(bits);
    EXPECT_EQ(back, color);
}

TEST(RgbaTests, LittleEndian_RoundTrips) {
    Rgba<uint8_t> color(0x01, 0x02, 0x03, 0x04);
    uint32_t bits = Rgba<uint8_t>::ToUInt32LittleEndian(color);
    auto back = Rgba<uint8_t>::CreateLittleEndian(bits);
    EXPECT_EQ(back, color);
}
