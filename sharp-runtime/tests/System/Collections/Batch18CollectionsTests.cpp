// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 18: BitArray (LeftShift/RightShift/PopCount/int-ctor),
//   Comparer::DefaultInvariant, ICollection::getSyncRootProperty.
#include <gtest/gtest.h>
#include <vector>
#include "System/Collections/BitArray.hpp"
#include "System/Collections/Comparer.hpp"
#include "System/Collections/ArrayList.hpp"

using namespace System::Collections;

// -----------------------------------------------------------------------
// BitArray — LeftShift
// -----------------------------------------------------------------------

TEST(BitArrayBatch18, LeftShift_BasicShiftByOne) {
    // bits: 1 0 1 0  -> shift left by 1 -> 0 1 0 1... wait no:
    // LeftShift shifts toward higher index; low positions become false.
    // {true,false,true,false} left-shift 1 → {false,true,false,true} ... no:
    // bit[i] = (i>=count) ? old[i-count] : false
    // bit[0] = (0>=1)? ... = false
    // bit[1] = old[0] = true
    // bit[2] = old[1] = false
    // bit[3] = old[2] = true
    BitArray ba(4, false);
    ba.Set(0, true);
    ba.Set(2, true);
    ba.LeftShift(1);
    EXPECT_FALSE(ba.Get(0));
    EXPECT_TRUE(ba.Get(1));
    EXPECT_FALSE(ba.Get(2));
    EXPECT_TRUE(ba.Get(3));
}

TEST(BitArrayBatch18, LeftShift_ByZero_Unchanged) {
    BitArray ba(4, false);
    ba.Set(0, true);
    ba.Set(3, true);
    ba.LeftShift(0);
    EXPECT_TRUE(ba.Get(0));
    EXPECT_TRUE(ba.Get(3));
}

TEST(BitArrayBatch18, LeftShift_ByLength_AllFalse) {
    BitArray ba(4, true);
    ba.LeftShift(4);
    EXPECT_FALSE(ba.HasAnySet());
}

TEST(BitArrayBatch18, LeftShift_ReturnsThis) {
    BitArray ba(4, false);
    BitArray& ref = ba.LeftShift(1);
    EXPECT_EQ(&ref, &ba);
}

// -----------------------------------------------------------------------
// BitArray — RightShift
// -----------------------------------------------------------------------

TEST(BitArrayBatch18, RightShift_BasicShiftByOne) {
    // {false,true,false,true} right-shift 1:
    // bit[i] = (i+1<4) ? old[i+1] : false
    // bit[0]=old[1]=true, bit[1]=old[2]=false, bit[2]=old[3]=true, bit[3]=false
    BitArray ba(4, false);
    ba.Set(1, true);
    ba.Set(3, true);
    ba.RightShift(1);
    EXPECT_TRUE(ba.Get(0));
    EXPECT_FALSE(ba.Get(1));
    EXPECT_TRUE(ba.Get(2));
    EXPECT_FALSE(ba.Get(3));
}

TEST(BitArrayBatch18, RightShift_ByZero_Unchanged) {
    BitArray ba(4, false);
    ba.Set(2, true);
    ba.RightShift(0);
    EXPECT_TRUE(ba.Get(2));
}

TEST(BitArrayBatch18, RightShift_ByLength_AllFalse) {
    BitArray ba(4, true);
    ba.RightShift(4);
    EXPECT_FALSE(ba.HasAnySet());
}

TEST(BitArrayBatch18, RightShift_ReturnsThis) {
    BitArray ba(4, false);
    BitArray& ref = ba.RightShift(1);
    EXPECT_EQ(&ref, &ba);
}

// -----------------------------------------------------------------------
// BitArray — PopCount
// -----------------------------------------------------------------------

TEST(BitArrayBatch18, PopCount_AllFalse) {
    BitArray ba(8, false);
    EXPECT_EQ(ba.PopCount(), 0);
}

TEST(BitArrayBatch18, PopCount_AllTrue) {
    BitArray ba(8, true);
    EXPECT_EQ(ba.PopCount(), 8);
}

TEST(BitArrayBatch18, PopCount_Mixed) {
    BitArray ba(8, false);
    ba.Set(0, true);
    ba.Set(3, true);
    ba.Set(7, true);
    EXPECT_EQ(ba.PopCount(), 3);
}

TEST(BitArrayBatch18, PopCount_Empty) {
    BitArray ba(0);
    EXPECT_EQ(ba.PopCount(), 0);
}

// -----------------------------------------------------------------------
// BitArray — int[] constructor
// -----------------------------------------------------------------------

TEST(BitArrayBatch18, IntCtor_UnpacksBitsLSBFirst) {
    // int value 0b00000001 = 1 → bit[0]=true, bit[1..31]=false
    std::vector<SharpRuntime::intcs> vals = {1};
    BitArray ba(vals);
    EXPECT_EQ(ba.getLengthProperty(), 32);
    EXPECT_TRUE(ba.Get(0));
    EXPECT_FALSE(ba.Get(1));
    EXPECT_EQ(ba.PopCount(), 1);
}

TEST(BitArrayBatch18, IntCtor_TwoInts) {
    std::vector<SharpRuntime::intcs> vals = {0, 0};
    BitArray ba(vals);
    EXPECT_EQ(ba.getLengthProperty(), 64);
    EXPECT_FALSE(ba.HasAnySet());
}

// -----------------------------------------------------------------------
// Comparer — DefaultInvariant
// -----------------------------------------------------------------------

TEST(ComparerBatch18, DefaultInvariant_IsSingleton) {
    const Comparer& a = Comparer::DefaultInvariant();
    const Comparer& b = Comparer::DefaultInvariant();
    EXPECT_EQ(&a, &b);
}

TEST(ComparerBatch18, DefaultInvariant_Compare_SamePtr_Zero) {
    int x = 42;
    const Comparer& c = Comparer::DefaultInvariant();
    EXPECT_EQ(c.Compare(&x, &x), 0);
}

TEST(ComparerBatch18, DefaultInvariant_Compare_NullNull_Zero) {
    EXPECT_EQ(Comparer::DefaultInvariant().Compare(nullptr, nullptr), 0);
}

// -----------------------------------------------------------------------
// ICollection — getSyncRootProperty
// -----------------------------------------------------------------------

TEST(ICollectionBatch18, ArrayList_SyncRoot_ReturnsNonNull) {
    ArrayList al;
    const void* root = al.getSyncRootProperty();
    EXPECT_NE(root, nullptr);
}

TEST(ICollectionBatch18, ArrayList_SyncRoot_IsStable) {
    ArrayList al;
    EXPECT_EQ(al.getSyncRootProperty(), al.getSyncRootProperty());
}
