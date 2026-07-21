// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Type.hpp"

using System::Type;

TEST(TypeTest, DefaultCtorIsNull) {
    Type t;
    EXPECT_EQ(t.getTypeInfo(), nullptr);
}

TEST(TypeTest, FromInt) {
    Type t = Type::From<int>();
    EXPECT_NE(t.getTypeInfo(), nullptr);
}

TEST(TypeTest, FromDoubleNameNotEmpty) {
    Type t = Type::From<double>();
    EXPECT_FALSE(t.getNameProperty().empty());
}

TEST(TypeTest, SameTypeEqual) {
    Type a = Type::From<int>();
    Type b = Type::From<int>();
    EXPECT_TRUE(a == b);
}

TEST(TypeTest, DifferentTypesNotEqual) {
    Type a = Type::From<int>();
    Type b = Type::From<double>();
    EXPECT_TRUE(a != b);
}

TEST(TypeTest, ToStringNotEmpty) {
    Type t = Type::From<int>();
    EXPECT_FALSE(t.ToString().empty());
}

TEST(TypeTest, GetHashCodeConsistent) {
    Type a = Type::From<int>();
    Type b = Type::From<int>();
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(TypeTest, IsClassDefault) {
    Type t = Type::From<int>();
    EXPECT_TRUE(t.getIsClassProperty());
}

TEST(TypeTest, IsValueTypeFalse) {
    Type t = Type::From<int>();
    EXPECT_FALSE(t.getIsValueTypeProperty());
}

TEST(TypeTest, IsAbstractFalse) {
    Type t = Type::From<int>();
    EXPECT_FALSE(t.getIsAbstractProperty());
}

TEST(TypeTest, IsInterfaceFalse) {
    Type t = Type::From<int>();
    EXPECT_FALSE(t.getIsInterfaceProperty());
}

TEST(TypeTest, FromTypeInfo) {
    const std::type_info& info = typeid(std::string);
    Type t = Type::FromTypeInfo(info);
    EXPECT_NE(t.getTypeInfo(), nullptr);
    EXPECT_EQ(t.getTypeInfo(), &info);
}

TEST(TypeTest, NullTypeHashCodeZero) {
    Type t;
    EXPECT_EQ(t.GetHashCode(), 0u);
}
