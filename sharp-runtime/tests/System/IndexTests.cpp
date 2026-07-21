// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Index.hpp"

using System::Index;

TEST(IndexTests2, DefaultCtor_IsFromStart_ValueZero) {
    Index idx;
    EXPECT_FALSE(idx.getIsFromEndProperty());
    EXPECT_EQ(idx.getValueProperty(), 0);
}

TEST(IndexTests2, FromStart_CorrectOffset) {
    Index idx = Index::FromStart(2);
    EXPECT_EQ(idx.GetOffset(5), 2);
    EXPECT_FALSE(idx.getIsFromEndProperty());
}

TEST(IndexTests2, FromEnd_CorrectOffset) {
    Index idx = Index::FromEnd(1);
    EXPECT_TRUE(idx.getIsFromEndProperty());
    EXPECT_EQ(idx.GetOffset(5), 4);
}

TEST(IndexTests2, Start_IsZeroFromStart) {
    Index idx = Index::Start();
    EXPECT_EQ(idx.GetOffset(10), 0);
}

TEST(IndexTests2, End_IsZeroFromEnd) {
    Index idx = Index::End();
    EXPECT_EQ(idx.GetOffset(5), 5);
}

TEST(IndexTests2, NegativeValue_Throws) {
    EXPECT_THROW(Index(-1), System::ArgumentOutOfRangeException);
}

TEST(IndexTests2, GetOffset_OutOfRange_DoesNotThrow_MatchesDotNet) {
    // .NET's Index.GetOffset does not validate the result against the collection
    // length, by design (see Index.cs remarks) - validation is the caller's job
    // (e.g. Range.GetOffsetAndLength).
    Index idx = Index::FromStart(10);
    EXPECT_EQ(idx.GetOffset(5), 10);
}

TEST(IndexTests2, ImplicitConversion_FromInt_IsFromStart) {
    Index idx = 3;
    EXPECT_FALSE(idx.getIsFromEndProperty());
    EXPECT_EQ(idx.getValueProperty(), 3);
}

TEST(IndexTests2, Equals_SameValueAndDirection_True) {
    EXPECT_TRUE(Index::FromStart(2).Equals(Index::FromStart(2)));
    EXPECT_TRUE(Index::FromStart(2) == Index::FromStart(2));
}

TEST(IndexTests2, Equals_DifferentDirection_False) {
    EXPECT_FALSE(Index::FromStart(2).Equals(Index::FromEnd(2)));
    EXPECT_TRUE(Index::FromStart(2) != Index::FromEnd(2));
}

TEST(IndexTests2, GetHashCode_MatchesForEqualIndexes) {
    EXPECT_EQ(Index::FromStart(4).GetHashCode(), Index::FromStart(4).GetHashCode());
    EXPECT_EQ(Index::FromEnd(1).GetHashCode(), Index::FromEnd(1).GetHashCode());
}

TEST(IndexTests2, ToString_FromStart) {
    EXPECT_EQ(Index::FromStart(3).ToString(), "3");
}

TEST(IndexTests2, ToString_FromEnd) {
    EXPECT_EQ(Index::FromEnd(1).ToString(), "^1");
}
