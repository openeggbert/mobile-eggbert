// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Comparer.hpp"

using System::Collections::Comparer;

TEST(ComparerTest, DefaultSingleton) {
    const Comparer& a = Comparer::Default();
    const Comparer& b = Comparer::Default();
    EXPECT_EQ(&a, &b);
}

TEST(ComparerTest, DefaultInvariantSingleton) {
    const Comparer& a = Comparer::DefaultInvariant();
    const Comparer& b = Comparer::DefaultInvariant();
    EXPECT_EQ(&a, &b);
}

TEST(ComparerTest, EqualPointersReturnZero) {
    int x = 0;
    EXPECT_EQ(Comparer::Default().Compare(&x, &x), 0);
}

TEST(ComparerTest, NullLessThanNonNull) {
    int x = 0;
    EXPECT_EQ(Comparer::Default().Compare(nullptr, &x), -1);
}

TEST(ComparerTest, NonNullGreaterThanNull) {
    int x = 0;
    EXPECT_EQ(Comparer::Default().Compare(&x, nullptr), 1);
}

TEST(ComparerTest, NullEqualsNull) {
    EXPECT_EQ(Comparer::Default().Compare(nullptr, nullptr), 0);
}

TEST(ComparerTest, ImplementsIComparer) {
    const System::Collections::IComparer& ic = Comparer::Default();
    int x = 0;
    EXPECT_EQ(ic.Compare(nullptr, &x), -1);
}
