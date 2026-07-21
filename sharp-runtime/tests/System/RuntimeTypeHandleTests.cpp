// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/RuntimeTypeHandle.hpp"

using System::RuntimeTypeHandle;

TEST(RuntimeTypeHandleTest, DefaultCtor_ZeroValue) {
    RuntimeTypeHandle h;
    EXPECT_EQ(h.getValueProperty(), 0);
}

TEST(RuntimeTypeHandleTest, ExplicitCtor) {
    RuntimeTypeHandle h(42);
    EXPECT_EQ(h.getValueProperty(), 42);
}

TEST(RuntimeTypeHandleTest, FromIntPtr_ToIntPtr_RoundTrip) {
    auto h = RuntimeTypeHandle::FromIntPtr(123);
    EXPECT_EQ(RuntimeTypeHandle::ToIntPtr(h), 123);
}

TEST(RuntimeTypeHandleTest, Equals_SameValue_True) {
    auto a = RuntimeTypeHandle::FromIntPtr(7);
    auto b = RuntimeTypeHandle::FromIntPtr(7);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_TRUE(a == b);
}

TEST(RuntimeTypeHandleTest, Equals_DifferentValue_False) {
    auto a = RuntimeTypeHandle::FromIntPtr(1);
    auto b = RuntimeTypeHandle::FromIntPtr(2);
    EXPECT_FALSE(a.Equals(b));
    EXPECT_TRUE(a != b);
}

TEST(RuntimeTypeHandleTest, GetHashCode_MatchesValue) {
    auto h = RuntimeTypeHandle::FromIntPtr(55);
    EXPECT_EQ(h.GetHashCode(), 55);
}

TEST(RuntimeTypeHandleTest, GetModuleHandle_NoThrow) {
    RuntimeTypeHandle h;
    EXPECT_NO_THROW(h.GetModuleHandle());
}

TEST(RuntimeTypeHandleTest, NegativeValue) {
    auto h = RuntimeTypeHandle::FromIntPtr(-1);
    EXPECT_EQ(h.getValueProperty(), -1);
}
