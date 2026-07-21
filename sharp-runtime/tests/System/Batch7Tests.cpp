// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include "System/MathF.hpp"
#include "System/BinaryData.hpp"

using System::MathF;
using System::BinaryData;

// ===========================================================================
// MathF — methods not covered by NumericsRemainingTests
// ===========================================================================

TEST(MathFTests, Exp_Zero_ReturnsOne) {
    EXPECT_FLOAT_EQ(MathF::Exp(0.0f), 1.0f);
}

TEST(MathFTests, Exp_One_ReturnsE) {
    EXPECT_NEAR(MathF::Exp(1.0f), MathF::E, 1e-6f);
}

TEST(MathFTests, Log_E_ReturnsOne) {
    EXPECT_NEAR(MathF::Log(MathF::E), 1.0f, 1e-6f);
}

TEST(MathFTests, Log_BaseOverload) {
    EXPECT_NEAR(MathF::Log(8.0f, 2.0f), 3.0f, 1e-5f);
}

TEST(MathFTests, Cbrt_Eight_ReturnsTwo) {
    EXPECT_NEAR(MathF::Cbrt(8.0f), 2.0f, 1e-6f);
}

TEST(MathFTests, Tan_Zero_ReturnsZero) {
    EXPECT_NEAR(MathF::Tan(0.0f), 0.0f, 1e-7f);
}

TEST(MathFTests, Tan_PiOver4_ReturnsOne) {
    EXPECT_NEAR(MathF::Tan(MathF::PI / 4.0f), 1.0f, 1e-6f);
}

TEST(MathFTests, Asin_Zero_ReturnsZero) {
    EXPECT_NEAR(MathF::Asin(0.0f), 0.0f, 1e-7f);
}

TEST(MathFTests, Acos_One_ReturnsZero) {
    EXPECT_NEAR(MathF::Acos(1.0f), 0.0f, 1e-7f);
}

TEST(MathFTests, Atan_Zero_ReturnsZero) {
    EXPECT_NEAR(MathF::Atan(0.0f), 0.0f, 1e-7f);
}

TEST(MathFTests, Sinh_Zero_ReturnsZero) {
    EXPECT_NEAR(MathF::Sinh(0.0f), 0.0f, 1e-7f);
}

TEST(MathFTests, Cosh_Zero_ReturnsOne) {
    EXPECT_NEAR(MathF::Cosh(0.0f), 1.0f, 1e-6f);
}

TEST(MathFTests, Tanh_Zero_ReturnsZero) {
    EXPECT_NEAR(MathF::Tanh(0.0f), 0.0f, 1e-7f);
}

TEST(MathFTests, IEEERemainder_Basic) {
    // IEEE remainder of 5/3 is -1 (rounds to nearest even)
    EXPECT_NEAR(MathF::IEEERemainder(5.0f, 3.0f), -1.0f, 1e-6f);
}

// ===========================================================================
// BinaryData
// ===========================================================================

TEST(BinaryDataTests, Empty_LengthIsZero) {
    EXPECT_EQ(BinaryData::Empty().getLengthProperty(), 0);
    EXPECT_TRUE(BinaryData::Empty().getIsEmptyProperty());
}

TEST(BinaryDataTests, VectorConstructor_Length) {
    std::vector<uint8_t> v = {1, 2, 3};
    BinaryData bd(v);
    EXPECT_EQ(bd.getLengthProperty(), 3);
    EXPECT_FALSE(bd.getIsEmptyProperty());
}

TEST(BinaryDataTests, VectorConstructor_WithMediaType) {
    std::vector<uint8_t> v = {0xFF};
    BinaryData bd(v, "application/octet-stream");
    EXPECT_EQ(bd.getMediaTypeProperty(), "application/octet-stream");
}

TEST(BinaryDataTests, StringConstructor_EncodesDatAsUtf8) {
    BinaryData bd(std::string("hello"));
    EXPECT_EQ(bd.getLengthProperty(), 5);
}

TEST(BinaryDataTests, StringConstructor_WithMediaType) {
    BinaryData bd(std::string("hi"), "text/plain");
    EXPECT_EQ(bd.getMediaTypeProperty(), "text/plain");
}

TEST(BinaryDataTests, ReadOnlyMemoryConstructor_Length) {
    uint8_t data[] = {10, 20, 30, 40};
    auto mem = System::ReadOnlyMemory<uint8_t>(data, 4);
    BinaryData bd(mem);
    EXPECT_EQ(bd.getLengthProperty(), 4);
}

TEST(BinaryDataTests, MediaTypeProperty_DefaultIsEmpty) {
    BinaryData bd(std::string("x"));
    EXPECT_TRUE(bd.getMediaTypeProperty().empty());
}

TEST(BinaryDataTests, FromString_Length) {
    auto bd = BinaryData::FromString("abc");
    EXPECT_EQ(bd.getLengthProperty(), 3);
}

TEST(BinaryDataTests, FromString_WithMediaType) {
    auto bd = BinaryData::FromString("abc", "text/plain");
    EXPECT_EQ(bd.getMediaTypeProperty(), "text/plain");
    EXPECT_EQ(bd.getLengthProperty(), 3);
}

TEST(BinaryDataTests, FromBytes_Vector) {
    std::vector<uint8_t> v = {5, 6, 7};
    auto bd = BinaryData::FromBytes(v);
    EXPECT_EQ(bd.getLengthProperty(), 3);
}

TEST(BinaryDataTests, FromBytes_ReadOnlyMemory) {
    uint8_t data[] = {1, 2};
    auto mem = System::ReadOnlyMemory<uint8_t>(data, 2);
    auto bd = BinaryData::FromBytes(mem);
    EXPECT_EQ(bd.getLengthProperty(), 2);
}

TEST(BinaryDataTests, FromBytes_WithMediaType) {
    uint8_t data[] = {0xAB};
    auto mem = System::ReadOnlyMemory<uint8_t>(data, 1);
    auto bd = BinaryData::FromBytes(mem, "application/octet-stream");
    EXPECT_EQ(bd.getMediaTypeProperty(), "application/octet-stream");
}

TEST(BinaryDataTests, ToString_DecodesUtf8) {
    auto bd = BinaryData::FromString("hello world");
    EXPECT_EQ(bd.ToString(), "hello world");
}

TEST(BinaryDataTests, ToArray_ReturnsBytes) {
    std::vector<uint8_t> v = {1, 2, 3};
    BinaryData bd(v);
    auto arr = bd.ToArray();
    EXPECT_EQ(arr.size(), 3u);
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[2], 3);
}

TEST(BinaryDataTests, ToMemory_CorrectLength) {
    std::vector<uint8_t> v = {10, 20};
    BinaryData bd(v);
    auto mem = bd.ToMemory();
    EXPECT_EQ(mem.getLengthProperty(), 2);
    EXPECT_EQ(mem[0], 10);
}

TEST(BinaryDataTests, WithMediaType_ChangesMediaType) {
    auto bd = BinaryData::FromString("data");
    auto bd2 = bd.WithMediaType("text/csv");
    EXPECT_EQ(bd.getMediaTypeProperty(), "");
    EXPECT_EQ(bd2.getMediaTypeProperty(), "text/csv");
    EXPECT_EQ(bd2.getLengthProperty(), 4);
}

TEST(BinaryDataTests, IndexOperator_ReturnsCorrectByte) {
    std::vector<uint8_t> v = {0xDE, 0xAD, 0xBE, 0xEF};
    BinaryData bd(v);
    EXPECT_EQ(bd[0], 0xDE);
    EXPECT_EQ(bd[3], 0xEF);
}

TEST(BinaryDataTests, RoundTrip_StringToBytes) {
    std::string original = "sharp-runtime";
    auto bd = BinaryData::FromString(original);
    EXPECT_EQ(bd.ToString(), original);
}

TEST(BinaryDataTests, EmptyVector_IsEmpty) {
    std::vector<uint8_t> v;
    BinaryData bd(v);
    EXPECT_TRUE(bd.getIsEmptyProperty());
    EXPECT_EQ(bd.getLengthProperty(), 0);
}
