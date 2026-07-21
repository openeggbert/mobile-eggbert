// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Range.hpp"

using System::Range;
using System::Index;

TEST(RangeTest, DefaultCtorIsAll) {
    Range r;
    auto ol = r.GetOffsetAndLength(10);
    EXPECT_EQ(ol.Offset, 0);
    EXPECT_EQ(ol.Length, 10);
}

TEST(RangeTest, ExplicitCtorStartEnd) {
    Range r(Index::FromStart(2), Index::FromStart(5));
    auto ol = r.GetOffsetAndLength(10);
    EXPECT_EQ(ol.Offset, 2);
    EXPECT_EQ(ol.Length, 3);
}

TEST(RangeTest, GetAllProperty) {
    Range r = Range::getAllProperty();
    auto ol = r.GetOffsetAndLength(8);
    EXPECT_EQ(ol.Offset, 0);
    EXPECT_EQ(ol.Length, 8);
}

TEST(RangeTest, StartAt) {
    Range r = Range::StartAt(Index::FromStart(3));
    auto ol = r.GetOffsetAndLength(10);
    EXPECT_EQ(ol.Offset, 3);
    EXPECT_EQ(ol.Length, 7);
}

TEST(RangeTest, EndAt) {
    Range r = Range::EndAt(Index::FromStart(4));
    auto ol = r.GetOffsetAndLength(10);
    EXPECT_EQ(ol.Offset, 0);
    EXPECT_EQ(ol.Length, 4);
}

TEST(RangeTest, FromEnd) {
    Range r(Index::FromStart(0), Index::FromEnd(2));
    auto ol = r.GetOffsetAndLength(10);
    EXPECT_EQ(ol.Offset, 0);
    EXPECT_EQ(ol.Length, 8);
}

TEST(RangeTest, GetStartProperty) {
    Range r(Index::FromStart(2), Index::FromStart(5));
    EXPECT_EQ(r.getStartProperty().getValueProperty(), 2);
}

TEST(RangeTest, GetEndProperty) {
    Range r(Index::FromStart(2), Index::FromStart(5));
    EXPECT_EQ(r.getEndProperty().getValueProperty(), 5);
}

TEST(RangeTest, EqualsTrue) {
    Range a(Index::FromStart(1), Index::FromStart(4));
    Range b(Index::FromStart(1), Index::FromStart(4));
    EXPECT_TRUE(a.Equals(b));
    EXPECT_TRUE(a == b);
}

TEST(RangeTest, EqualsFalse) {
    Range a(Index::FromStart(1), Index::FromStart(4));
    Range b(Index::FromStart(2), Index::FromStart(4));
    EXPECT_FALSE(a.Equals(b));
    EXPECT_TRUE(a != b);
}

TEST(RangeTest, GetHashCode_SameRangesSameHash) {
    Range a(Index::FromStart(1), Index::FromStart(3));
    Range b(Index::FromStart(1), Index::FromStart(3));
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(RangeTest, ToString_FromStart) {
    Range r(Index::FromStart(1), Index::FromStart(5));
    EXPECT_EQ(r.ToString(), "1..5");
}

TEST(RangeTest, ToString_FromEnd) {
    Range r(Index::FromStart(0), Index::FromEnd(1));
    EXPECT_EQ(r.ToString(), "0..^1");
}

TEST(RangeTest, GetOffsetAndLength_OutOfOrder_Throws) {
    Range r(Index::FromStart(5), Index::FromStart(2));
    EXPECT_THROW(r.GetOffsetAndLength(10), System::ArgumentOutOfRangeException);
}

TEST(RangeTest, GetOffsetAndLength_EndBeyondLength_Throws) {
    // Index.GetOffset does not itself validate against length (matches .NET), so this
    // must be caught by Range.GetOffsetAndLength's own bounds check.
    Range r(Index::FromStart(2), Index::FromStart(20));
    EXPECT_THROW(r.GetOffsetAndLength(10), System::ArgumentOutOfRangeException);
}

TEST(RangeTest, GetOffsetAndLength_NegativeStart_Throws) {
    Range r(Index::FromEnd(20), Index::FromStart(5));
    EXPECT_THROW(r.GetOffsetAndLength(10), System::ArgumentOutOfRangeException);
}
