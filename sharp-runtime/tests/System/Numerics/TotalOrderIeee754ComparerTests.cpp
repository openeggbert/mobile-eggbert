// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <limits>

#include "System/Half.hpp"
#include "System/Numerics/DivisionRounding.hpp"
#include "System/Numerics/TotalOrderIeee754Comparer.hpp"

using System::Half;
using System::Numerics::DivisionRounding;
using System::Numerics::TotalOrderIeee754Comparer;

TEST(TotalOrderIeee754ComparerTests, Float_NegativeLessThanPositive) {
    TotalOrderIeee754Comparer<float> cmp;
    EXPECT_LT(cmp.Compare(-1.0f, 1.0f), 0);
    EXPECT_GT(cmp.Compare(1.0f, -1.0f), 0);
    EXPECT_EQ(cmp.Compare(1.0f, 1.0f), 0);
}

TEST(TotalOrderIeee754ComparerTests, Float_NegativeZeroLessThanPositiveZero) {
    TotalOrderIeee754Comparer<float> cmp;
    EXPECT_LT(cmp.Compare(-0.0f, 0.0f), 0);
    EXPECT_GT(cmp.Compare(0.0f, -0.0f), 0);
}

TEST(TotalOrderIeee754ComparerTests, Float_PositiveInfinityLessThanNaN) {
    TotalOrderIeee754Comparer<float> cmp;
    float posInf = std::numeric_limits<float>::infinity();
    float nan = std::numeric_limits<float>::quiet_NaN();
    EXPECT_LT(cmp.Compare(posInf, nan), 0);
}

TEST(TotalOrderIeee754ComparerTests, Float_NegativeInfinityIsSmallest) {
    TotalOrderIeee754Comparer<float> cmp;
    float negInf = -std::numeric_limits<float>::infinity();
    EXPECT_LT(cmp.Compare(negInf, -1.0f), 0);
    EXPECT_LT(cmp.Compare(negInf, 0.0f), 0);
}

TEST(TotalOrderIeee754ComparerTests, Double_OrdersLikeFloat) {
    TotalOrderIeee754Comparer<double> cmp;
    EXPECT_LT(cmp.Compare(-2.0, -1.0), 0);
    EXPECT_LT(cmp.Compare(-0.0, 0.0), 0);
    EXPECT_EQ(cmp.Compare(3.5, 3.5), 0);
}

TEST(TotalOrderIeee754ComparerTests, Half_OrdersNegativeBeforePositive) {
    TotalOrderIeee754Comparer<Half> cmp;
    Half negOne = Half::FromSingle(-1.0f);
    Half posOne = Half::FromSingle(1.0f);
    EXPECT_LT(cmp.Compare(negOne, posOne), 0);
    EXPECT_EQ(cmp.Compare(posOne, posOne), 0);
}

TEST(DivisionRoundingTests, HasFiveValuesMatchingDotNet) {
    EXPECT_EQ(static_cast<int>(DivisionRounding::Truncate), 0);
    EXPECT_EQ(static_cast<int>(DivisionRounding::Floor), 1);
    EXPECT_EQ(static_cast<int>(DivisionRounding::Ceiling), 2);
    EXPECT_EQ(static_cast<int>(DivisionRounding::AwayFromZero), 3);
    EXPECT_EQ(static_cast<int>(DivisionRounding::Euclidean), 4);
}
