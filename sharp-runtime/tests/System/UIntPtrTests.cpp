// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <limits>
#include "System/UIntPtr.hpp"

using System::UIntPtr;

TEST(UIntPtrTest, DefaultCtorIsZero) {
    UIntPtr p;
    EXPECT_EQ(p.value, 0UL);
}

TEST(UIntPtrTest, ConstructFromValue) {
    UIntPtr p(42);
    EXPECT_EQ(p.value, 42UL);
}

TEST(UIntPtrTest, ConstructFromPointer) {
    int x = 0;
    UIntPtr p(&x);
    EXPECT_EQ(p.value, reinterpret_cast<uintptr_t>(&x));
}

TEST(UIntPtrTest, ZeroStatic) {
    EXPECT_EQ(UIntPtr::Zero.value, 0UL);
}

TEST(UIntPtrTest, IsZeroTrue) {
    UIntPtr p;
    EXPECT_TRUE(p.IsZero());
}

TEST(UIntPtrTest, IsZeroFalse) {
    UIntPtr p(1);
    EXPECT_FALSE(p.IsZero());
}

TEST(UIntPtrTest, EqualityTrue) {
    UIntPtr a(10), b(10);
    EXPECT_TRUE(a == b);
}

TEST(UIntPtrTest, EqualityFalse) {
    UIntPtr a(10), b(20);
    EXPECT_TRUE(a != b);
}

TEST(UIntPtrTest, CastToUintptr) {
    UIntPtr p(99);
    EXPECT_EQ(static_cast<uintptr_t>(p), 99UL);
}

TEST(UIntPtrTest, CastToVoidPtr) {
    UIntPtr p(uintptr_t(0));
    void* vp = static_cast<void*>(p);
    EXPECT_EQ(vp, nullptr);
}

TEST(UIntPtrTest, Size_MatchesPointerSize) {
    EXPECT_EQ(UIntPtr::Size, static_cast<SharpRuntime::intcs>(sizeof(uintptr_t)));
}

TEST(UIntPtrTest, MaxValue_IsAllOnes) {
    EXPECT_EQ(UIntPtr::MaxValue.value, std::numeric_limits<uintptr_t>::max());
}

TEST(UIntPtrTest, CompareTo) {
    UIntPtr a(5), b(10);
    EXPECT_LT(a.CompareTo(b), 0);
    EXPECT_GT(b.CompareTo(a), 0);
    EXPECT_EQ(a.CompareTo(a), 0);
}

TEST(UIntPtrTest, Equals) {
    UIntPtr a(10), b(10), c(20);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_FALSE(a.Equals(c));
}

TEST(UIntPtrTest, GetHashCode_SameValue_SameHash) {
    UIntPtr a(42), b(42);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(UIntPtrTest, ToString) {
    UIntPtr p(12345);
    EXPECT_EQ(p.ToString(), "12345");
}

TEST(UIntPtrTest, ToPointer) {
    int x = 0;
    UIntPtr p(&x);
    EXPECT_EQ(p.ToPointer(), static_cast<void*>(&x));
}

TEST(UIntPtrTest, Sign) {
    EXPECT_EQ(UIntPtr::Sign(UIntPtr(uintptr_t(0))), 0);
    EXPECT_EQ(UIntPtr::Sign(UIntPtr(uintptr_t(1))), 1);
}
