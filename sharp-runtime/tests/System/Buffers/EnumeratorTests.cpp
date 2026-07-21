// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Buffers/ReadOnlySequence.hpp"

using System::Buffers::ReadOnlySequence;

TEST(ReadOnlySequenceEnumeratorTest, MoveNextTrueOnce) {
    std::vector<int> data = {1, 2, 3};
    ReadOnlySequence<int> seq(data.data(), static_cast<int>(data.size()));
    auto e = seq.GetEnumerator();
    EXPECT_TRUE(e.MoveNext());
    EXPECT_FALSE(e.MoveNext());
}

TEST(ReadOnlySequenceEnumeratorTest, CurrentHasCorrectLength) {
    std::vector<int> data = {10, 20, 30};
    ReadOnlySequence<int> seq(data.data(), static_cast<int>(data.size()));
    auto e = seq.GetEnumerator();
    e.MoveNext();
    EXPECT_EQ(e.getCurrentProperty().getLengthProperty(), 3);
}

TEST(ReadOnlySequenceEnumeratorTest, EmptySequenceNoMoveNext) {
    ReadOnlySequence<int> seq;
    auto e = seq.GetEnumerator();
    e.MoveNext();
    auto mem = e.getCurrentProperty();
    EXPECT_EQ(mem.getLengthProperty(), 0);
}

TEST(ReadOnlySequenceEnumeratorTest, GetEnumeratorNotNull) {
    std::vector<char> data = {'a', 'b'};
    ReadOnlySequence<char> seq(data.data(), 2);
    auto e = seq.GetEnumerator();
    EXPECT_TRUE(e.MoveNext());
}
