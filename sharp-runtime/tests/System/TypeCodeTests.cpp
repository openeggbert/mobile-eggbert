// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/TypeCode.hpp"

using System::TypeCode;

TEST(TypeCodeTest, EmptyIsZero) {
    EXPECT_EQ(static_cast<int>(TypeCode::Empty), 0);
}

TEST(TypeCodeTest, ObjectIsOne) {
    EXPECT_EQ(static_cast<int>(TypeCode::Object), 1);
}

TEST(TypeCodeTest, BooleanIsThree) {
    EXPECT_EQ(static_cast<int>(TypeCode::Boolean), 3);
}

TEST(TypeCodeTest, Int32IsNine) {
    EXPECT_EQ(static_cast<int>(TypeCode::Int32), 9);
}

TEST(TypeCodeTest, Int64IsEleven) {
    EXPECT_EQ(static_cast<int>(TypeCode::Int64), 11);
}

TEST(TypeCodeTest, SingleIsThirteen) {
    EXPECT_EQ(static_cast<int>(TypeCode::Single), 13);
}

TEST(TypeCodeTest, DoubleIsFourteen) {
    EXPECT_EQ(static_cast<int>(TypeCode::Double), 14);
}

TEST(TypeCodeTest, DateTimeIsSixteen) {
    EXPECT_EQ(static_cast<int>(TypeCode::DateTime), 16);
}

TEST(TypeCodeTest, StringIsEighteen) {
    EXPECT_EQ(static_cast<int>(TypeCode::String), 18);
}

TEST(TypeCodeTest, DistinctValues) {
    EXPECT_NE(TypeCode::Int32, TypeCode::Int64);
    EXPECT_NE(TypeCode::Single, TypeCode::Double);
    EXPECT_NE(TypeCode::Boolean, TypeCode::Char);
}
