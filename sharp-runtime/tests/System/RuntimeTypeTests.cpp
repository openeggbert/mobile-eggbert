// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/RuntimeType.hpp"

using System::RuntimeType;

TEST(RuntimeTypeTest, None_Value) {
    EXPECT_EQ(static_cast<int>(RuntimeType::None), 0);
}

TEST(RuntimeTypeTest, Primitive_Value) {
    EXPECT_EQ(static_cast<int>(RuntimeType::Primitive), 1);
}

TEST(RuntimeTypeTest, ValueType_Value) {
    EXPECT_EQ(static_cast<int>(RuntimeType::ValueType), 2);
}

TEST(RuntimeTypeTest, ReferenceType_Value) {
    EXPECT_EQ(static_cast<int>(RuntimeType::ReferenceType), 3);
}

TEST(RuntimeTypeTest, Array_Value) {
    EXPECT_EQ(static_cast<int>(RuntimeType::Array), 4);
}

TEST(RuntimeTypeTest, GenericParameter_Value) {
    EXPECT_EQ(static_cast<int>(RuntimeType::GenericParameter), 5);
}

TEST(RuntimeTypeTest, Equality) {
    RuntimeType a = RuntimeType::Primitive;
    RuntimeType b = RuntimeType::Primitive;
    EXPECT_EQ(a, b);
}

TEST(RuntimeTypeTest, Inequality) {
    EXPECT_NE(RuntimeType::Primitive, RuntimeType::Array);
}
