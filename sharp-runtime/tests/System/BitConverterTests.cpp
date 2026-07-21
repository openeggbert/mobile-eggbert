// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <vector>
#include "System/BitConverter.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using System::BitConverter;
using System::bytecs;
using SharpRuntime::ushortcs;
using SharpRuntime::uintcs;
using SharpRuntime::ulongcs;
using SharpRuntime::charcs;

// ---------------------------------------------------------------------------
// IsLittleEndian
// ---------------------------------------------------------------------------

TEST(BitConverterTests, IsLittleEndian_IsTrue) {
    EXPECT_TRUE(BitConverter::IsLittleEndian);
}

// ---------------------------------------------------------------------------
// GetBytes — size checks
// ---------------------------------------------------------------------------

TEST(BitConverterTests, GetBytes_Int16_SizeIsTwo) {
    EXPECT_EQ(BitConverter::GetBytes(static_cast<SharpRuntime::shortcs>(0)).size(), 2u);
}

TEST(BitConverterTests, GetBytes_Int32_SizeIsFour) {
    EXPECT_EQ(BitConverter::GetBytes(static_cast<SharpRuntime::intcs>(0)).size(), 4u);
}

TEST(BitConverterTests, GetBytes_Int64_SizeIsEight) {
    EXPECT_EQ(BitConverter::GetBytes(static_cast<SharpRuntime::longcs>(0)).size(), 8u);
}

TEST(BitConverterTests, GetBytes_Float_SizeIsFour) {
    EXPECT_EQ(BitConverter::GetBytes(0.0f).size(), 4u);
}

TEST(BitConverterTests, GetBytes_Double_SizeIsEight) {
    EXPECT_EQ(BitConverter::GetBytes(0.0).size(), 8u);
}

TEST(BitConverterTests, GetBytes_Bool_SizeIsOne) {
    EXPECT_EQ(BitConverter::GetBytes(true).size(), 1u);
}

// ---------------------------------------------------------------------------
// GetBytes(bool) — known values
// ---------------------------------------------------------------------------

TEST(BitConverterTests, GetBytes_BoolTrue_IsOne) {
    EXPECT_EQ(BitConverter::GetBytes(true)[0], 1u);
}

TEST(BitConverterTests, GetBytes_BoolFalse_IsZero) {
    EXPECT_EQ(BitConverter::GetBytes(false)[0], 0u);
}

// ---------------------------------------------------------------------------
// GetBytes — little-endian known values
// ---------------------------------------------------------------------------

TEST(BitConverterTests, GetBytes_Int32_One_LittleEndian) {
    auto b = BitConverter::GetBytes(static_cast<SharpRuntime::intcs>(1));
    EXPECT_EQ(b[0], 1u);
    EXPECT_EQ(b[1], 0u);
    EXPECT_EQ(b[2], 0u);
    EXPECT_EQ(b[3], 0u);
}

TEST(BitConverterTests, GetBytes_Int16_256_LittleEndian) {
    auto b = BitConverter::GetBytes(static_cast<SharpRuntime::shortcs>(256));
    EXPECT_EQ(b[0], 0u);
    EXPECT_EQ(b[1], 1u);
}

// ---------------------------------------------------------------------------
// Round-trip via raw pointer overloads
// ---------------------------------------------------------------------------

TEST(BitConverterTests, RoundTrip_Int16) {
    SharpRuntime::shortcs val = -1234;
    auto b = BitConverter::GetBytes(val);
    EXPECT_EQ(BitConverter::ToInt16(b.data(), 0), val);
}

TEST(BitConverterTests, RoundTrip_Int32) {
    SharpRuntime::intcs val = 0x12345678;
    auto b = BitConverter::GetBytes(val);
    EXPECT_EQ(BitConverter::ToInt32(b.data(), 0), val);
}

TEST(BitConverterTests, RoundTrip_Int64) {
    SharpRuntime::longcs val = 0x0123456789ABCDEFLL;
    auto b = BitConverter::GetBytes(val);
    EXPECT_EQ(BitConverter::ToInt64(b.data(), 0), val);
}

TEST(BitConverterTests, RoundTrip_Float) {
    float val = 3.14f;
    auto b = BitConverter::GetBytes(val);
    EXPECT_FLOAT_EQ(BitConverter::ToSingle(b.data(), 0), val);
}

TEST(BitConverterTests, RoundTrip_Double) {
    double val = 2.718281828;
    auto b = BitConverter::GetBytes(val);
    EXPECT_DOUBLE_EQ(BitConverter::ToDouble(b.data(), 0), val);
}

// ---------------------------------------------------------------------------
// ToBoolean
// ---------------------------------------------------------------------------

TEST(BitConverterTests, ToBoolean_ZeroByte_ReturnsFalse) {
    bytecs buf[] = {0};
    EXPECT_FALSE(BitConverter::ToBoolean(buf, 0));
}

TEST(BitConverterTests, ToBoolean_NonZeroByte_ReturnsTrue) {
    bytecs buf[] = {42};
    EXPECT_TRUE(BitConverter::ToBoolean(buf, 0));
}

// ---------------------------------------------------------------------------
// Vector overloads
// ---------------------------------------------------------------------------

TEST(BitConverterTests, ToInt32_VectorOverload_MatchesRawPtr) {
    SharpRuntime::intcs val = 999;
    auto arr = BitConverter::GetBytes(val);
    std::vector<bytecs> vec(arr.begin(), arr.end());
    EXPECT_EQ(BitConverter::ToInt32(vec, 0), val);
}

TEST(BitConverterTests, ToInt64_VectorOverload_MatchesRawPtr) {
    SharpRuntime::longcs val = -1LL;
    auto arr = BitConverter::GetBytes(val);
    std::vector<bytecs> vec(arr.begin(), arr.end());
    EXPECT_EQ(BitConverter::ToInt64(vec, 0), val);
}

// ---------------------------------------------------------------------------
// ToString (hex formatting)
// ---------------------------------------------------------------------------

TEST(BitConverterTests, ToString_SingleByte_TwoHexChars) {
    bytecs buf[] = {0xAB};
    EXPECT_EQ(BitConverter::ToString(buf, 0, 1), "AB");
}

TEST(BitConverterTests, ToString_TwoBytes_DashSeparated) {
    bytecs buf[] = {0x01, 0x02};
    EXPECT_EQ(BitConverter::ToString(buf, 0, 2), "01-02");
}

TEST(BitConverterTests, ToString_Vector_AllBytes) {
    std::vector<bytecs> v = {0x0F, 0xFF};
    EXPECT_EQ(BitConverter::ToString(v), "0F-FF");
}

// ---------------------------------------------------------------------------
// Bit reinterpretation — DoubleToInt64Bits / Int64BitsToDouble
// ---------------------------------------------------------------------------

TEST(BitConverterTests, DoubleToInt64Bits_RoundTrip) {
    double val = 3.14159265358979;
    SharpRuntime::longcs bits = BitConverter::DoubleToInt64Bits(val);
    double back = BitConverter::Int64BitsToDouble(bits);
    EXPECT_DOUBLE_EQ(back, val);
}

TEST(BitConverterTests, DoubleToInt64Bits_Zero) {
    EXPECT_EQ(BitConverter::DoubleToInt64Bits(0.0), 0LL);
}

TEST(BitConverterTests, DoubleToInt64Bits_One) {
    // IEEE 754: 1.0 = 0x3FF0000000000000
    EXPECT_EQ(BitConverter::DoubleToInt64Bits(1.0), static_cast<SharpRuntime::longcs>(0x3FF0000000000000LL));
}

TEST(BitConverterTests, Int64BitsToDouble_One) {
    EXPECT_DOUBLE_EQ(BitConverter::Int64BitsToDouble(0x3FF0000000000000LL), 1.0);
}

// ---------------------------------------------------------------------------
// Bit reinterpretation — SingleToInt32Bits / Int32BitsToSingle
// ---------------------------------------------------------------------------

TEST(BitConverterTests, SingleToInt32Bits_RoundTrip) {
    float val = 2.5f;
    SharpRuntime::intcs bits = BitConverter::SingleToInt32Bits(val);
    float back = BitConverter::Int32BitsToSingle(bits);
    EXPECT_FLOAT_EQ(back, val);
}

TEST(BitConverterTests, SingleToInt32Bits_One) {
    // IEEE 754: 1.0f = 0x3F800000
    EXPECT_EQ(BitConverter::SingleToInt32Bits(1.0f), static_cast<SharpRuntime::intcs>(0x3F800000));
}

TEST(BitConverterTests, Int32BitsToSingle_One) {
    EXPECT_FLOAT_EQ(BitConverter::Int32BitsToSingle(0x3F800000), 1.0f);
}

// ---------------------------------------------------------------------------
// GetBytes — unsigned and char variants
// ---------------------------------------------------------------------------

TEST(BitConverterTests, GetBytes_Char_SizeIsTwo) {
    EXPECT_EQ(BitConverter::GetBytes(charcs(u'A')).size(), 2u);
}

TEST(BitConverterTests, GetBytes_UInt16_RoundTrip) {
    ushortcs val = 0xABCDu;
    auto b = BitConverter::GetBytes(val);
    EXPECT_EQ(BitConverter::ToUInt16(b.data(), 0), val);
}

TEST(BitConverterTests, GetBytes_UInt32_RoundTrip) {
    uintcs val = 0xDEADBEEFu;
    auto b = BitConverter::GetBytes(val);
    EXPECT_EQ(BitConverter::ToUInt32(b.data(), 0), val);
}

TEST(BitConverterTests, GetBytes_UInt64_RoundTrip) {
    ulongcs val = 0xCAFEBABEDEADBEEFull;
    auto b = BitConverter::GetBytes(val);
    EXPECT_EQ(BitConverter::ToUInt64(b.data(), 0), val);
}

TEST(BitConverterTests, ToChar_RoundTrip) {
    charcs c = u'é'; // é
    auto b = BitConverter::GetBytes(c);
    EXPECT_EQ(BitConverter::ToChar(b.data(), 0), c);
}

TEST(BitConverterTests, ToUInt32_VectorOverload) {
    uintcs val = 42u;
    auto arr = BitConverter::GetBytes(val);
    std::vector<bytecs> v(arr.begin(), arr.end());
    EXPECT_EQ(BitConverter::ToUInt32(v, 0), val);
}

TEST(BitConverterTests, ToUInt64_VectorOverload) {
    ulongcs val = 0xFFFFFFFFFFFFFFFFull;
    auto arr = BitConverter::GetBytes(val);
    std::vector<bytecs> v(arr.begin(), arr.end());
    EXPECT_EQ(BitConverter::ToUInt64(v, 0), val);
}

// ---------------------------------------------------------------------------
// Unsigned bit reinterpretation
// ---------------------------------------------------------------------------

TEST(BitConverterTests, DoubleToUInt64Bits_RoundTrip) {
    double val = 3.14;
    ulongcs bits = BitConverter::DoubleToUInt64Bits(val);
    EXPECT_DOUBLE_EQ(BitConverter::UInt64BitsToDouble(bits), val);
}

TEST(BitConverterTests, DoubleToUInt64Bits_One) {
    EXPECT_EQ(BitConverter::DoubleToUInt64Bits(1.0), 0x3FF0000000000000ull);
}

TEST(BitConverterTests, SingleToUInt32Bits_RoundTrip) {
    float val = 2.5f;
    uintcs bits = BitConverter::SingleToUInt32Bits(val);
    EXPECT_FLOAT_EQ(BitConverter::UInt32BitsToSingle(bits), val);
}

TEST(BitConverterTests, SingleToUInt32Bits_One) {
    EXPECT_EQ(BitConverter::SingleToUInt32Bits(1.0f), 0x3F800000u);
}

// ---------------------------------------------------------------------------
// ToString(vector, startIndex)
// ---------------------------------------------------------------------------

TEST(BitConverterTests, ToString_VectorStartIndex_SkipsFirst) {
    std::vector<bytecs> v = {0x01, 0x02, 0x03};
    EXPECT_EQ(BitConverter::ToString(v, 1), "02-03");
}

TEST(BitConverterTests, ToString_VectorStartIndex_Zero_MatchesFull) {
    std::vector<bytecs> v = {0xAA, 0xBB};
    EXPECT_EQ(BitConverter::ToString(v, 0), BitConverter::ToString(v));
}

// Regression tests for ticket 352: ToString(vector, startIndex) previously computed
// `length = value.size() - startIndex` with NO validation of startIndex, so a negative
// startIndex produced an inflated length and the underlying raw-pointer overload then read
// value[startIndex + i] -- a genuine, confirmed out-of-bounds read (one element before the
// buffer's start for startIndex == -1), not merely wrong output. Verified against
// BitConverter.cs's ToString(byte[], int, int), including its exact (quirky but real) boundary
// behavior: startIndex == size for a non-empty vector THROWS, it does not return an empty
// string, even though that combination computes length == 0.
TEST(BitConverterTests, ToString_VectorNegativeStartIndex_ThrowsInsteadOfReadingOutOfBounds) {
    std::vector<bytecs> v = {0x01, 0x02, 0x03};
    EXPECT_THROW(BitConverter::ToString(v, -1), System::ArgumentOutOfRangeException);
}

TEST(BitConverterTests, ToString_VectorStartIndexEqualsSize_Throws) {
    std::vector<bytecs> v = {0x01, 0x02, 0x03};
    EXPECT_THROW(BitConverter::ToString(v, 3), System::ArgumentOutOfRangeException);
}

TEST(BitConverterTests, ToString_VectorStartIndexBeyondSize_Throws) {
    std::vector<bytecs> v = {0x01, 0x02, 0x03};
    EXPECT_THROW(BitConverter::ToString(v, 5), System::ArgumentOutOfRangeException);
}

TEST(BitConverterTests, ToString_VectorEmptyStartIndexZero_ReturnsEmptyString) {
    std::vector<bytecs> v;
    EXPECT_EQ(BitConverter::ToString(v, 0), "");
}

TEST(BitConverterTests, ToString_VectorThreeArg_LengthExceedsRemaining_ThrowsArgumentException) {
    std::vector<bytecs> v = {0x01, 0x02, 0x03};
    EXPECT_THROW(BitConverter::ToString(v, 1, 10), System::ArgumentException);
}

TEST(BitConverterTests, ToString_RawPointer_NegativeStartIndex_Throws) {
    bytecs buf[] = {0x01, 0x02};
    EXPECT_THROW(BitConverter::ToString(buf, -1, 1), System::ArgumentOutOfRangeException);
}

TEST(BitConverterTests, ToString_RawPointer_NegativeLength_Throws) {
    bytecs buf[] = {0x01, 0x02};
    EXPECT_THROW(BitConverter::ToString(buf, 0, -1), System::ArgumentOutOfRangeException);
}

// Half bit-reinterpretation
TEST(BitConverterTests, HalfToUInt16Bits_RoundTrip) {
    System::Half h{0x3C00u}; // 1.0 in half-precision
    EXPECT_EQ(BitConverter::HalfToUInt16Bits(h), 0x3C00u);
}
TEST(BitConverterTests, UInt16BitsToHalf_RoundTrip) {
    System::Half h = BitConverter::UInt16BitsToHalf(0x3C00u);
    EXPECT_EQ(h.bits, 0x3C00u);
}
TEST(BitConverterTests, HalfInt16Bits_RoundTrip) {
    System::Half h = BitConverter::Int16BitsToHalf(static_cast<SharpRuntime::shortcs>(0x3C00));
    EXPECT_EQ(BitConverter::HalfToInt16Bits(h), static_cast<SharpRuntime::shortcs>(0x3C00));
}

// ---------------------------------------------------------------------------
// GetBytes(Half) / ToHalf
// ---------------------------------------------------------------------------

TEST(BitConverterTests, GetBytes_Half_SizeIsTwo) {
    EXPECT_EQ(BitConverter::GetBytes(System::Half{0x3C00u}).size(), 2u);
}
TEST(BitConverterTests, RoundTrip_Half) {
    System::Half h{0x4000u}; // 2.0 in half-precision
    auto b = BitConverter::GetBytes(h);
    EXPECT_EQ(BitConverter::ToHalf(b.data(), 0).bits, h.bits);
}
TEST(BitConverterTests, ToHalf_VectorOverload) {
    System::Half h{0x3C00u};
    auto arr = BitConverter::GetBytes(h);
    std::vector<bytecs> v(arr.begin(), arr.end());
    EXPECT_EQ(BitConverter::ToHalf(v, 0).bits, h.bits);
}

// ---------------------------------------------------------------------------
// BFloat16 bit-reinterpretation
// ---------------------------------------------------------------------------

TEST(BitConverterTests, BFloat16ToInt16Bits_RoundTrip) {
    System::Numerics::BFloat16 bf{uint16_t(0x3F80u)}; // 1.0 in BFloat16
    SharpRuntime::shortcs bits = BitConverter::BFloat16ToInt16Bits(bf);
    System::Numerics::BFloat16 back = BitConverter::Int16BitsToBFloat16(bits);
    EXPECT_EQ(back.getBitsProperty(), bf.getBitsProperty());
}
TEST(BitConverterTests, BFloat16ToUInt16Bits_RoundTrip) {
    System::Numerics::BFloat16 bf{uint16_t(0x3F80u)};
    ushortcs bits = BitConverter::BFloat16ToUInt16Bits(bf);
    EXPECT_EQ(bits, 0x3F80u);
    EXPECT_EQ(BitConverter::UInt16BitsToBFloat16(bits).getBitsProperty(), 0x3F80u);
}

// ---------------------------------------------------------------------------
// GetBytes(BFloat16) / ToBFloat16
// ---------------------------------------------------------------------------

TEST(BitConverterTests, GetBytes_BFloat16_SizeIsTwo) {
    System::Numerics::BFloat16 bf{uint16_t(0x3F80u)};
    EXPECT_EQ(BitConverter::GetBytes(bf).size(), 2u);
}
TEST(BitConverterTests, RoundTrip_BFloat16) {
    System::Numerics::BFloat16 bf{uint16_t(0x4000u)};
    auto b = BitConverter::GetBytes(bf);
    EXPECT_EQ(BitConverter::ToBFloat16(b.data(), 0).getBitsProperty(), bf.getBitsProperty());
}
TEST(BitConverterTests, ToBFloat16_VectorOverload) {
    System::Numerics::BFloat16 bf{uint16_t(0x3F80u)};
    auto arr = BitConverter::GetBytes(bf);
    std::vector<bytecs> v(arr.begin(), arr.end());
    EXPECT_EQ(BitConverter::ToBFloat16(v, 0).getBitsProperty(), bf.getBitsProperty());
}

// ---------------------------------------------------------------------------
// GetBytes(Int128) / ToInt128
// ---------------------------------------------------------------------------

TEST(BitConverterTests, GetBytes_Int128_SizeIsSixteen) {
    EXPECT_EQ(BitConverter::GetBytes(System::Int128{}).size(), 16u);
}
TEST(BitConverterTests, RoundTrip_Int128) {
    System::Int128 val(static_cast<__int128>(0x0102030405060708LL));
    auto b = BitConverter::GetBytes(val);
    EXPECT_EQ(b.size(), 16u);
    System::Int128 back = BitConverter::ToInt128(b.data(), 0);
    EXPECT_EQ(back, val);
}
TEST(BitConverterTests, ToInt128_VectorOverload) {
    System::Int128 val(static_cast<__int128>(42));
    auto arr = BitConverter::GetBytes(val);
    std::vector<bytecs> v(arr.begin(), arr.end());
    EXPECT_EQ(BitConverter::ToInt128(v, 0), val);
}

// ---------------------------------------------------------------------------
// GetBytes(UInt128) / ToUInt128
// ---------------------------------------------------------------------------

TEST(BitConverterTests, GetBytes_UInt128_SizeIsSixteen) {
    EXPECT_EQ(BitConverter::GetBytes(System::UInt128{}).size(), 16u);
}
TEST(BitConverterTests, RoundTrip_UInt128) {
    System::UInt128 val(static_cast<unsigned __int128>(0xDEADBEEFCAFEBABEull));
    auto b = BitConverter::GetBytes(val);
    EXPECT_EQ(b.size(), 16u);
    System::UInt128 back = BitConverter::ToUInt128(b.data(), 0);
    EXPECT_EQ(back, val);
}
TEST(BitConverterTests, ToUInt128_VectorOverload) {
    System::UInt128 val(static_cast<unsigned __int128>(99));
    auto arr = BitConverter::GetBytes(val);
    std::vector<bytecs> v(arr.begin(), arr.end());
    EXPECT_EQ(BitConverter::ToUInt128(v, 0), val);
}
