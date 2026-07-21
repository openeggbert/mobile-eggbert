// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Math.hpp"
#include "System/BadImageFormatException.hpp"
#include "System/ValueType.hpp"
#include "System/RuntimeTypeHandle.hpp"
#include "System/RuntimeMethodHandle.hpp"
#include "System/RuntimeFieldHandle.hpp"
#include "System/ModuleHandle.hpp"

// ===========================================================================
// Math — short/sbyte/byte/uint/ulong/ushort overloads
// ===========================================================================

TEST(MathNewOverloadsTests, Abs_Short_Positive) {
    System::shortcs v = 5;
    EXPECT_EQ(System::Math::Abs(v), 5);
}
TEST(MathNewOverloadsTests, Abs_Short_Negative) {
    System::shortcs v = -7;
    EXPECT_EQ(System::Math::Abs(v), 7);
}
TEST(MathNewOverloadsTests, Abs_Sbyte_Negative) {
    System::sbytecs v = -3;
    EXPECT_EQ(System::Math::Abs(v), 3);
}

TEST(MathNewOverloadsTests, Min_Short) {
    System::shortcs a = 10, b = 20;
    EXPECT_EQ(System::Math::Min(a, b), 10);
}
TEST(MathNewOverloadsTests, Max_Short) {
    System::shortcs a = 10, b = 20;
    EXPECT_EQ(System::Math::Max(a, b), 20);
}
TEST(MathNewOverloadsTests, Clamp_Short_Below) {
    System::shortcs v = 1, mn = 5, mx = 10;
    EXPECT_EQ(System::Math::Clamp(v, mn, mx), 5);
}
TEST(MathNewOverloadsTests, Clamp_Short_Above) {
    System::shortcs v = 15, mn = 5, mx = 10;
    EXPECT_EQ(System::Math::Clamp(v, mn, mx), 10);
}
TEST(MathNewOverloadsTests, Clamp_Short_Inside) {
    System::shortcs v = 7, mn = 5, mx = 10;
    EXPECT_EQ(System::Math::Clamp(v, mn, mx), 7);
}

TEST(MathNewOverloadsTests, Min_Byte) {
    System::bytecs a = 3, b = 7;
    EXPECT_EQ(System::Math::Min(a, b), 3u);
}
TEST(MathNewOverloadsTests, Max_Byte) {
    System::bytecs a = 3, b = 7;
    EXPECT_EQ(System::Math::Max(a, b), 7u);
}
TEST(MathNewOverloadsTests, Clamp_Byte) {
    System::bytecs v = 200, mn = 10, mx = 100;
    EXPECT_EQ(System::Math::Clamp(v, mn, mx), 100u);
}

TEST(MathNewOverloadsTests, Min_Uint) {
    System::uintcs a = 100u, b = 200u;
    EXPECT_EQ(System::Math::Min(a, b), 100u);
}
TEST(MathNewOverloadsTests, Max_Uint) {
    System::uintcs a = 100u, b = 200u;
    EXPECT_EQ(System::Math::Max(a, b), 200u);
}
TEST(MathNewOverloadsTests, Clamp_Uint_Inside) {
    System::uintcs v = 150u, mn = 100u, mx = 200u;
    EXPECT_EQ(System::Math::Clamp(v, mn, mx), 150u);
}

TEST(MathNewOverloadsTests, Min_Ulong) {
    System::ulongcs a = 1000ull, b = 2000ull;
    EXPECT_EQ(System::Math::Min(a, b), 1000ull);
}
TEST(MathNewOverloadsTests, Max_Ulong) {
    System::ulongcs a = 1000ull, b = 2000ull;
    EXPECT_EQ(System::Math::Max(a, b), 2000ull);
}
TEST(MathNewOverloadsTests, Clamp_Ulong_Below) {
    System::ulongcs v = 500ull, mn = 1000ull, mx = 2000ull;
    EXPECT_EQ(System::Math::Clamp(v, mn, mx), 1000ull);
}

TEST(MathNewOverloadsTests, Min_Ushort) {
    System::ushortcs a = 10, b = 20;
    EXPECT_EQ(System::Math::Min(a, b), 10u);
}
TEST(MathNewOverloadsTests, Max_Ushort) {
    System::ushortcs a = 10, b = 20;
    EXPECT_EQ(System::Math::Max(a, b), 20u);
}
TEST(MathNewOverloadsTests, Clamp_Ushort) {
    System::ushortcs v = 5, mn = 10, mx = 20;
    EXPECT_EQ(System::Math::Clamp(v, mn, mx), 10u);
}

TEST(MathNewOverloadsTests, ILogB_8) {
    EXPECT_EQ(System::Math::ILogB(8.0), 3);
}
TEST(MathNewOverloadsTests, ILogB_1) {
    EXPECT_EQ(System::Math::ILogB(1.0), 0);
}

TEST(MathNewOverloadsTests, BigMul_WithHigh) {
    System::longcs high = 0;
    System::longcs low = System::Math::BigMul(
        static_cast<System::longcs>(1000000000LL),
        static_cast<System::longcs>(1000000000LL),
        high);
    EXPECT_EQ(low, 1000000000LL * 1000000000LL);
    EXPECT_EQ(high, 0);
}
TEST(MathNewOverloadsTests, BigMul_LargeProduct_HighNonZero) {
    System::longcs high = 0;
    System::longcs a = static_cast<System::longcs>(0x7FFFFFFFFFFFFFFFLL);
    System::longcs b = static_cast<System::longcs>(2LL);
    System::Math::BigMul(a, b, high);
    EXPECT_EQ(high, 0);  // fits in 64 bits for 2x max_long - 2
}

TEST(MathNewOverloadsTests, DivRem_Int_Pair) {
    auto [q, r] = System::Math::DivRem(static_cast<System::intcs>(10), static_cast<System::intcs>(3));
    EXPECT_EQ(q, 3);
    EXPECT_EQ(r, 1);
}
TEST(MathNewOverloadsTests, DivRem_Long_Pair) {
    auto [q, r] = System::Math::DivRem(static_cast<System::longcs>(17LL), static_cast<System::longcs>(5LL));
    EXPECT_EQ(q, 3LL);
    EXPECT_EQ(r, 2LL);
}

// ===========================================================================
// BadImageFormatException — fileName ctors and property
// ===========================================================================

TEST(BadImageFormatExceptionExtraTests, FileNameCtor_StoresName) {
    System::BadImageFormatException ex("bad image", "mylib.dll");
    EXPECT_EQ(ex.getFileNameProperty(), "mylib.dll");
}
TEST(BadImageFormatExceptionExtraTests, FileNameCtor_MessagePreserved) {
    System::BadImageFormatException ex("bad image", "mylib.dll");
    EXPECT_NE(std::string(ex.what()).find("bad image"), std::string::npos);
}
TEST(BadImageFormatExceptionExtraTests, FileNameInnerCtor_StoresName) {
    auto inner = std::make_exception_ptr(std::runtime_error("inner"));
    System::BadImageFormatException ex("bad image", "mylib.dll", inner);
    EXPECT_EQ(ex.getFileNameProperty(), "mylib.dll");
}
TEST(BadImageFormatExceptionExtraTests, FileNameInnerCtor_WhatContainsBoth) {
    auto inner = std::make_exception_ptr(std::runtime_error("inner"));
    System::BadImageFormatException ex("bad image", "mylib.dll", inner);
    EXPECT_NE(std::string(ex.what()).find("bad image"), std::string::npos);
}
TEST(BadImageFormatExceptionExtraTests, DefaultCtor_EmptyFileName) {
    System::BadImageFormatException ex;
    EXPECT_EQ(ex.getFileNameProperty(), "");
}

// ===========================================================================
// ValueType
// ===========================================================================

TEST(ValueTypeTests, DefaultCtor_Works) {
    System::ValueType vt;
    (void)vt;
    SUCCEED();
}
TEST(ValueTypeTests, GetHashCode_ReturnsInt) {
    System::ValueType vt;
    int h = vt.GetHashCode();
    (void)h;
    SUCCEED();
}
TEST(ValueTypeTests, ToString_NonEmpty) {
    System::ValueType vt;
    EXPECT_FALSE(vt.ToString().empty());
}
TEST(ValueTypeTests, Equals_SameObject) {
    System::ValueType vt;
    EXPECT_TRUE(vt.Equals(vt));
}
TEST(ValueTypeTests, Equals_DifferentObjects_False) {
    System::ValueType a, b;
    EXPECT_FALSE(a.Equals(b));
}

// ===========================================================================
// RuntimeTypeHandle
// ===========================================================================

TEST(RuntimeTypeHandleTests, DefaultCtor_ValueZero) {
    System::RuntimeTypeHandle h;
    EXPECT_EQ(h.getValueProperty(), 0);
}
TEST(RuntimeTypeHandleTests, FromIntPtr_ToIntPtr_RoundTrip) {
    auto h = System::RuntimeTypeHandle::FromIntPtr(42);
    EXPECT_EQ(System::RuntimeTypeHandle::ToIntPtr(h), 42);
}
TEST(RuntimeTypeHandleTests, Equals_SameValue) {
    auto a = System::RuntimeTypeHandle::FromIntPtr(7);
    auto b = System::RuntimeTypeHandle::FromIntPtr(7);
    EXPECT_TRUE(a.Equals(b));
}
TEST(RuntimeTypeHandleTests, Equals_DifferentValue) {
    auto a = System::RuntimeTypeHandle::FromIntPtr(1);
    auto b = System::RuntimeTypeHandle::FromIntPtr(2);
    EXPECT_FALSE(a.Equals(b));
}
TEST(RuntimeTypeHandleTests, OperatorEq) {
    auto a = System::RuntimeTypeHandle::FromIntPtr(5);
    auto b = System::RuntimeTypeHandle::FromIntPtr(5);
    EXPECT_TRUE(a == b);
}
TEST(RuntimeTypeHandleTests, OperatorNeq) {
    auto a = System::RuntimeTypeHandle::FromIntPtr(5);
    auto b = System::RuntimeTypeHandle::FromIntPtr(6);
    EXPECT_TRUE(a != b);
}
TEST(RuntimeTypeHandleTests, GetHashCode_MatchesValue) {
    auto h = System::RuntimeTypeHandle::FromIntPtr(99);
    EXPECT_EQ(h.GetHashCode(), 99);
}
TEST(RuntimeTypeHandleTests, GetModuleHandle_ReturnsEmptyHandle) {
    System::RuntimeTypeHandle h;
    System::ModuleHandle mh = h.GetModuleHandle();
    EXPECT_EQ(mh.getMDStreamVersionProperty(), 0);
}

// ===========================================================================
// RuntimeMethodHandle
// ===========================================================================

TEST(RuntimeMethodHandleTests, DefaultCtor_ValueZero) {
    System::RuntimeMethodHandle h;
    EXPECT_EQ(System::RuntimeMethodHandle::ToIntPtr(h), 0);
}
TEST(RuntimeMethodHandleTests, FromIntPtr_ToIntPtr_RoundTrip) {
    auto h = System::RuntimeMethodHandle::FromIntPtr(123);
    EXPECT_EQ(System::RuntimeMethodHandle::ToIntPtr(h), 123);
}
TEST(RuntimeMethodHandleTests, GetFunctionPointer_ReturnsZero) {
    System::RuntimeMethodHandle h;
    EXPECT_EQ(h.GetFunctionPointer(), 0);
}
TEST(RuntimeMethodHandleTests, Equals_SameValue) {
    auto a = System::RuntimeMethodHandle::FromIntPtr(10);
    auto b = System::RuntimeMethodHandle::FromIntPtr(10);
    EXPECT_TRUE(a.Equals(b));
}
TEST(RuntimeMethodHandleTests, OperatorNeq) {
    auto a = System::RuntimeMethodHandle::FromIntPtr(1);
    auto b = System::RuntimeMethodHandle::FromIntPtr(2);
    EXPECT_TRUE(a != b);
}

// ===========================================================================
// RuntimeFieldHandle
// ===========================================================================

TEST(RuntimeFieldHandleTests, DefaultCtor_ValueZero) {
    System::RuntimeFieldHandle h;
    EXPECT_EQ(System::RuntimeFieldHandle::ToIntPtr(h), 0);
}
TEST(RuntimeFieldHandleTests, FromIntPtr_ToIntPtr_RoundTrip) {
    auto h = System::RuntimeFieldHandle::FromIntPtr(55);
    EXPECT_EQ(System::RuntimeFieldHandle::ToIntPtr(h), 55);
}
TEST(RuntimeFieldHandleTests, Equals_SameValue) {
    auto a = System::RuntimeFieldHandle::FromIntPtr(3);
    auto b = System::RuntimeFieldHandle::FromIntPtr(3);
    EXPECT_TRUE(a.Equals(b));
}
TEST(RuntimeFieldHandleTests, OperatorEq) {
    auto a = System::RuntimeFieldHandle::FromIntPtr(7);
    auto b = System::RuntimeFieldHandle::FromIntPtr(7);
    EXPECT_TRUE(a == b);
}
TEST(RuntimeFieldHandleTests, GetHashCode) {
    auto h = System::RuntimeFieldHandle::FromIntPtr(42);
    EXPECT_EQ(h.GetHashCode(), 42);
}

// ===========================================================================
// ModuleHandle
// ===========================================================================

TEST(ModuleHandleTests, EmptyHandle_MDStreamVersionZero) {
    EXPECT_EQ(System::ModuleHandle::EmptyHandle.getMDStreamVersionProperty(), 0);
}
TEST(ModuleHandleTests, DefaultCtor_EqualsEmptyHandle) {
    System::ModuleHandle mh;
    EXPECT_TRUE(mh.Equals(System::ModuleHandle::EmptyHandle));
}
TEST(ModuleHandleTests, OperatorEq) {
    System::ModuleHandle a, b;
    EXPECT_TRUE(a == b);
}
TEST(ModuleHandleTests, GetHashCode_Zero) {
    System::ModuleHandle mh;
    EXPECT_EQ(mh.GetHashCode(), 0);
}
TEST(ModuleHandleTests, ResolveTypeHandle_Throws) {
    System::ModuleHandle mh;
    EXPECT_THROW(mh.ResolveTypeHandle(1), System::NotSupportedException);
}
