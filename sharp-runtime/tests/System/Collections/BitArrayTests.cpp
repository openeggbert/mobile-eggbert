// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/BitArray.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using System::Collections::BitArray;

TEST(BitArrayTest, ConstructWithLength) {
    BitArray ba(8);
    EXPECT_EQ(ba.getLengthProperty(), 8);
    EXPECT_FALSE(ba.Get(0));
}

TEST(BitArrayTest, ConstructWithDefault) {
    BitArray ba(4, true);
    EXPECT_TRUE(ba.Get(0));
    EXPECT_TRUE(ba.HasAllSet());
}

TEST(BitArrayTest, ConstructWithNegativeLength_Throws) {
    // Without this check, a negative length cast to size_t wraps to a huge value passed
    // directly to std::vector<bool>'s constructor -- confirmed via a standalone ASan repro
    // that this was a genuine heap-buffer-overflow (std::vector<bool>'s internal
    // bit-to-word-count calculation overflows for a size_t(-1)-scale request, silently
    // under-allocating while claiming a huge size()), not merely the wrong exception type.
    EXPECT_THROW(BitArray(-1), System::ArgumentOutOfRangeException);
}

TEST(BitArrayTest, SetAndGet) {
    BitArray ba(8);
    ba.Set(3, true);
    EXPECT_TRUE(ba.Get(3));
    EXPECT_FALSE(ba.Get(2));
}

TEST(BitArrayTest, SetAll) {
    BitArray ba(5);
    ba.SetAll(true);
    EXPECT_TRUE(ba.HasAllSet());
    ba.SetAll(false);
    EXPECT_FALSE(ba.HasAnySet());
}

TEST(BitArrayTest, And) {
    BitArray a(4, true), b(4);
    b.Set(1, true);
    a.And(b);
    EXPECT_FALSE(a.Get(0));
    EXPECT_TRUE(a.Get(1));
}

TEST(BitArrayTest, Or) {
    BitArray a(4), b(4);
    b.Set(2, true);
    a.Or(b);
    EXPECT_TRUE(a.Get(2));
}

TEST(BitArrayTest, Xor) {
    BitArray a(4, true), b(4, true);
    a.Xor(b);
    EXPECT_FALSE(a.HasAnySet());
}

TEST(BitArrayTest, Not) {
    BitArray ba(4);
    ba.Not();
    EXPECT_TRUE(ba.HasAllSet());
}

TEST(BitArrayTest, PopCount) {
    BitArray ba(8);
    ba.Set(0, true);
    ba.Set(3, true);
    ba.Set(7, true);
    EXPECT_EQ(ba.PopCount(), 3);
}

TEST(BitArrayTest, Clone) {
    BitArray a(4);
    a.Set(1, true);
    BitArray b = a.Clone();
    b.Set(2, true);
    EXPECT_FALSE(a.Get(2));
    EXPECT_TRUE(b.Get(2));
}

TEST(BitArrayTest, LeftShift) {
    BitArray ba(4);
    ba.Set(0, true);
    ba.LeftShift(1);
    EXPECT_FALSE(ba.Get(0));
    EXPECT_TRUE(ba.Get(1));
}

TEST(BitArrayTest, RightShift) {
    BitArray ba(4);
    ba.Set(3, true);
    ba.RightShift(1);
    EXPECT_TRUE(ba.Get(2));
    EXPECT_FALSE(ba.Get(3));
}

TEST(BitArrayTest, LeftShift_NegativeCount_Throws) {
    BitArray ba(4);
    EXPECT_THROW(ba.LeftShift(-1), System::ArgumentOutOfRangeException);
}

TEST(BitArrayTest, RightShift_NegativeCount_Throws) {
    BitArray ba(4);
    EXPECT_THROW(ba.RightShift(-1), System::ArgumentOutOfRangeException);
}

TEST(BitArrayTest, SetLength_Grows_NewBitsAreFalse) {
    BitArray ba(4, true);
    ba.setLengthProperty(8);
    EXPECT_EQ(ba.getLengthProperty(), 8);
    EXPECT_TRUE(ba.Get(3));
    EXPECT_FALSE(ba.Get(4));
    EXPECT_FALSE(ba.Get(7));
}

TEST(BitArrayTest, SetLength_Shrinks) {
    BitArray ba(8, true);
    ba.setLengthProperty(3);
    EXPECT_EQ(ba.getLengthProperty(), 3);
}

TEST(BitArrayTest, SetLength_Negative_Throws) {
    BitArray ba(4);
    EXPECT_THROW(ba.setLengthProperty(-1), System::ArgumentOutOfRangeException);
}

TEST(BitArrayTest, And_DifferentLengths_Throws) {
    BitArray a(4), b(5);
    EXPECT_THROW(a.And(b), System::ArgumentException);
}

TEST(BitArrayTest, Or_DifferentLengths_Throws) {
    BitArray a(4), b(5);
    EXPECT_THROW(a.Or(b), System::ArgumentException);
}

TEST(BitArrayTest, Xor_DifferentLengths_Throws) {
    BitArray a(4), b(5);
    EXPECT_THROW(a.Xor(b), System::ArgumentException);
}

// Get/Set/operator[] used std::vector<bool>::at(), which throws raw std::out_of_range
// instead of the System::ArgumentOutOfRangeException real .NET throws (BitArray.cs's
// `(uint)index >= (uint)_bitLength` check) -- code catching the .NET-matching exception
// type wouldn't catch it.
TEST(BitArrayTest, Get_OutOfRange_ThrowsArgumentOutOfRange) {
    BitArray a(4);
    EXPECT_THROW(a.Get(4), System::ArgumentOutOfRangeException);
    EXPECT_THROW(a.Get(-1), System::ArgumentOutOfRangeException);
}

TEST(BitArrayTest, Set_OutOfRange_ThrowsArgumentOutOfRange) {
    BitArray a(4);
    EXPECT_THROW(a.Set(4, true), System::ArgumentOutOfRangeException);
}

TEST(BitArrayTest, OperatorBracket_OutOfRange_ThrowsArgumentOutOfRange) {
    const BitArray a(4);
    EXPECT_THROW(a[4], System::ArgumentOutOfRangeException);
}
