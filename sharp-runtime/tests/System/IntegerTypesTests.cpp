// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
// Tests for: Int16, Int32, Int128, IntPtr
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include "System/Int16.hpp"
#include "System/Int32.hpp"
#include "System/Int128.hpp"
#include "System/IntPtr.hpp"

// ---------------------------------------------------------------------------
// Int16
// ---------------------------------------------------------------------------
TEST(Int16Tests, MaxValue_Is32767) {
    EXPECT_EQ(System::Int16::MaxValue, 32767);
}

TEST(Int16Tests, MinValue_IsMinus32768) {
    EXPECT_EQ(System::Int16::MinValue, -32768);
}

TEST(Int16Tests, Parse_ValidString) {
    EXPECT_EQ(System::Int16::Parse("100"), 100);
}

TEST(Int16Tests, TryParse_Valid_ReturnsTrue) {
    SharpRuntime::shortcs r = 0;
    EXPECT_TRUE(System::Int16::TryParse("42", r));
    EXPECT_EQ(r, 42);
}

TEST(Int16Tests, TryParse_Invalid_ReturnsFalse) {
    SharpRuntime::shortcs r = 0;
    EXPECT_FALSE(System::Int16::TryParse("abc", r));
}

TEST(Int16Tests, ToString_Default) {
    EXPECT_EQ(System::Int16::ToString(255), "255");
}

TEST(Int16Tests, ToString_HexFormat) {
    EXPECT_EQ(System::Int16::ToString(255, "X"), "FF");
}

TEST(Int16Tests, ToString_MalformedWidth_ThrowsFormatException) {
    EXPECT_THROW(System::Int16::ToString(5, "Xz"), System::FormatException);
    EXPECT_THROW(System::Int16::ToString(5, "X99999999999999999999"), System::FormatException);
}

// ---------------------------------------------------------------------------
// Int32
// ---------------------------------------------------------------------------
TEST(Int32Tests2, MaxValue_IsCorrect) {
    EXPECT_EQ(System::Int32::MaxValue, 2147483647);
}

TEST(Int32Tests2, MinValue_IsCorrect) {
    EXPECT_EQ(System::Int32::MinValue, -2147483648LL);
}

TEST(Int32Tests2, Parse_ValidString) {
    EXPECT_EQ(System::Int32::Parse("12345"), 12345);
}

TEST(Int32Tests2, TryParse_Valid) {
    SharpRuntime::intcs r = 0;
    EXPECT_TRUE(System::Int32::TryParse("99", r));
    EXPECT_EQ(r, 99);
}

TEST(Int32Tests2, TryParse_Invalid_ReturnsFalse) {
    SharpRuntime::intcs r = 0;
    EXPECT_FALSE(System::Int32::TryParse("xyz", r));
}

// ---------------------------------------------------------------------------
// Int128 (basic smoke test — may be stub)
// ---------------------------------------------------------------------------
TEST(Int128Tests2, DefaultCtor_DoesNotThrow) {
    EXPECT_NO_THROW(System::Int128 v);
}

TEST(Int128Tests2, Abs_PositiveValue_ReturnsSameValue) {
    System::Int128 v(static_cast<__int128>(5));
    EXPECT_TRUE(System::Int128::Abs(v) == v);
}

TEST(Int128Tests2, Abs_NegativeValue_ReturnsPositive) {
    System::Int128 v(static_cast<__int128>(-5));
    EXPECT_TRUE(System::Int128::Abs(v) == System::Int128(static_cast<__int128>(5)));
}

// ---------------------------------------------------------------------------
// IntPtr
// ---------------------------------------------------------------------------
TEST(IntPtrTests2, Zero_IsZero) {
    System::IntPtr p = System::IntPtr::Zero;
    EXPECT_TRUE(p.IsZero());
}

TEST(IntPtrTests2, Ctor_FromInt) {
    System::IntPtr p(42);
    EXPECT_FALSE(p.IsZero());
    EXPECT_EQ(static_cast<intptr_t>(p), 42);
}
