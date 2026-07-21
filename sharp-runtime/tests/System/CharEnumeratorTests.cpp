// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/CharEnumerator.hpp"
#include "System/InvalidOperationException.hpp"

using System::CharEnumerator;

// ---------------------------------------------------------------------------
// MoveNext / Current
// ---------------------------------------------------------------------------

TEST(CharEnumeratorTests, MoveNext_EmptyString_ReturnsFalse) {
    CharEnumerator e("");
    EXPECT_FALSE(e.MoveNext());
}

TEST(CharEnumeratorTests, MoveNext_SingleChar_ReturnsTrueThenFalse) {
    CharEnumerator e("A");
    EXPECT_TRUE(e.MoveNext());
    EXPECT_FALSE(e.MoveNext());
}

TEST(CharEnumeratorTests, Current_AfterFirstMoveNext_IsFirstChar) {
    CharEnumerator e("Hello");
    e.MoveNext();
    EXPECT_EQ(e.getCurrentProperty(), 'H');
}

TEST(CharEnumeratorTests, Current_BeforeMoveNext_Throws) {
    CharEnumerator e("Hi");
    EXPECT_THROW(e.getCurrentProperty(), System::InvalidOperationException);
}

TEST(CharEnumeratorTests, Current_AfterEnd_Throws) {
    CharEnumerator e("X");
    e.MoveNext();
    e.MoveNext();
    EXPECT_THROW(e.getCurrentProperty(), System::InvalidOperationException);
}

TEST(CharEnumeratorTests, Iterates_AllChars) {
    CharEnumerator e("abc");
    std::string result;
    while (e.MoveNext())
        result += e.getCurrentProperty();
    EXPECT_EQ(result, "abc");
}

TEST(CharEnumeratorTests, Iterates_ThreeChars_CountCorrect) {
    CharEnumerator e("xyz");
    int count = 0;
    while (e.MoveNext()) ++count;
    EXPECT_EQ(count, 3);
}

// ---------------------------------------------------------------------------
// Reset
// ---------------------------------------------------------------------------

TEST(CharEnumeratorTests, Reset_RestartsIteration) {
    CharEnumerator e("ab");
    e.MoveNext();
    e.MoveNext();
    e.Reset();
    e.MoveNext();
    EXPECT_EQ(e.getCurrentProperty(), 'a');
}

TEST(CharEnumeratorTests, Reset_AfterReset_MoveNextWorks) {
    CharEnumerator e("hi");
    e.MoveNext();
    e.Reset();
    EXPECT_TRUE(e.MoveNext());
    EXPECT_EQ(e.getCurrentProperty(), 'h');
}

// ---------------------------------------------------------------------------
// Clone
// ---------------------------------------------------------------------------

TEST(CharEnumeratorTests, Clone_IsSeparateCursor) {
    CharEnumerator e("ab");
    e.MoveNext();
    CharEnumerator clone = e.Clone();
    e.MoveNext();
    EXPECT_EQ(clone.getCurrentProperty(), 'a');
}

// ---------------------------------------------------------------------------
// Dispose
// ---------------------------------------------------------------------------

TEST(CharEnumeratorTests, Dispose_ThenMoveNextReturnsFalse) {
    CharEnumerator e("hello");
    e.Dispose();
    EXPECT_FALSE(e.MoveNext());
}
