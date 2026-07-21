// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArraySegment.hpp"

using System::ArraySegment;
using SharpRuntime::intcs;

// ---------------------------------------------------------------------------
// getEmpty
// ---------------------------------------------------------------------------

TEST(ArraySegmentTests, GetEmpty_CountIsZero) {
    auto seg = ArraySegment<int>::getEmpty();
    EXPECT_EQ(seg.getCountProperty(), 0);
}

TEST(ArraySegmentTests, GetEmpty_OffsetIsZero) {
    auto seg = ArraySegment<int>::getEmpty();
    EXPECT_EQ(seg.getOffsetProperty(), 0);
}

TEST(ArraySegmentTests, GetEmpty_ArrayNotNull) {
    auto seg = ArraySegment<int>::getEmpty();
    EXPECT_NE(seg.getArrayProperty(), nullptr);
}

TEST(ArraySegmentTests, GetEmpty_CalledTwice_SamePtr) {
    auto a = ArraySegment<int>::getEmpty();
    auto b = ArraySegment<int>::getEmpty();
    EXPECT_EQ(a.getArrayProperty(), b.getArrayProperty());
}

// ---------------------------------------------------------------------------
// Default constructor
// ---------------------------------------------------------------------------

TEST(ArraySegmentTests, DefaultCtor_ArrayIsNull) {
    ArraySegment<int> seg;
    EXPECT_EQ(seg.getArrayProperty(), nullptr);
}

TEST(ArraySegmentTests, DefaultCtor_OffsetAndCountAreZero) {
    ArraySegment<int> seg;
    EXPECT_EQ(seg.getOffsetProperty(), 0);
    EXPECT_EQ(seg.getCountProperty(), 0);
}

// ---------------------------------------------------------------------------
// Slice (1-arg)
// ---------------------------------------------------------------------------

TEST(ArraySegmentTests, Slice1Arg_FromMiddle_CorrectCountAndOffset) {
    std::vector<int> v{10, 20, 30, 40, 50};
    ArraySegment<int> seg(v);
    ArraySegment<int> s = seg.Slice(2);
    EXPECT_EQ(s.getCountProperty(), 3);
    EXPECT_EQ(s.getOffsetProperty(), 2);
}

TEST(ArraySegmentTests, Slice1Arg_FromZero_SameAsOriginal) {
    std::vector<int> v{1, 2, 3};
    ArraySegment<int> seg(v);
    ArraySegment<int> s = seg.Slice(0);
    EXPECT_EQ(s.getCountProperty(), seg.getCountProperty());
}

TEST(ArraySegmentTests, Slice1Arg_FromEnd_EmptySegment) {
    std::vector<int> v{1, 2, 3};
    ArraySegment<int> seg(v);
    ArraySegment<int> s = seg.Slice(3);
    EXPECT_EQ(s.getCountProperty(), 0);
}

TEST(ArraySegmentTests, Slice1Arg_OutOfRange_Throws) {
    std::vector<int> v{1, 2};
    ArraySegment<int> seg(v);
    EXPECT_THROW(seg.Slice(5), System::ArgumentOutOfRangeException);
}

TEST(ArraySegmentTests, Slice1Arg_Negative_Throws) {
    std::vector<int> v{1, 2, 3};
    ArraySegment<int> seg(v);
    EXPECT_THROW(seg.Slice(-1), System::ArgumentOutOfRangeException);
}

// Regression tests (ticket 1487): offset+count (constructor) / index+count (Slice(int,int))
// used to be computed directly in intcs (int32) arithmetic, which overflows for large inputs
// and silently bypasses the bounds check instead of throwing -- same bug class as
// Span<T>::Slice (ticket 265), confirmed via a standalone UBSan repro before fixing.
TEST(ArraySegmentTests, Constructor_OffsetPlusCountOverflow_ThrowsInsteadOfBypassingCheck) {
    std::vector<int> v{1, 2, 3};
    EXPECT_THROW((ArraySegment<int>(v, 2147483647, 10)), System::ArgumentOutOfRangeException);
}

TEST(ArraySegmentTests, Slice2Arg_IndexPlusCountOverflow_ThrowsInsteadOfBypassingCheck) {
    std::vector<int> v{1, 2, 3, 4, 5};
    ArraySegment<int> seg(v);
    EXPECT_THROW(seg.Slice(2147483647, 10), System::ArgumentOutOfRangeException);
}

TEST(ArraySegmentTests, Slice2Arg_ValidRange_Works) {
    std::vector<int> v{10, 20, 30, 40, 50};
    ArraySegment<int> seg(v);
    ArraySegment<int> s = seg.Slice(1, 3);
    EXPECT_EQ(s.getCountProperty(), 3);
    EXPECT_EQ(s[0], 20);
    EXPECT_EQ(s[2], 40);
}

TEST(ArraySegmentTests, Slice1Arg_CorrectElements) {
    std::vector<int> v{10, 20, 30, 40};
    ArraySegment<int> seg(v);
    ArraySegment<int> s = seg.Slice(1);
    EXPECT_EQ(s[0], 20);
    EXPECT_EQ(s[1], 30);
    EXPECT_EQ(s[2], 40);
}

// ---------------------------------------------------------------------------
// CopyTo(vector)
// ---------------------------------------------------------------------------

TEST(ArraySegmentTests, CopyTo_Vector_CopiesAllElements) {
    std::vector<int> v{10, 20, 30};
    ArraySegment<int> seg(v);
    std::vector<int> dest;
    seg.CopyTo(dest);
    EXPECT_EQ(dest, (std::vector<int>{10, 20, 30}));
}

TEST(ArraySegmentTests, CopyTo_Vector_PartialSegment) {
    std::vector<int> v{1, 2, 3, 4, 5};
    ArraySegment<int> seg(v, 1, 3);
    std::vector<int> dest;
    seg.CopyTo(dest);
    EXPECT_EQ(dest, (std::vector<int>{2, 3, 4}));
}

// ---------------------------------------------------------------------------
// CopyTo(vector, destinationIndex)
// ---------------------------------------------------------------------------

TEST(ArraySegmentTests, CopyTo_VectorWithOffset_WritesAtCorrectIndex) {
    std::vector<int> v{10, 20, 30};
    ArraySegment<int> seg(v);
    std::vector<int> dest(5, 0);
    seg.CopyTo(dest, 2);
    EXPECT_EQ(dest[2], 10);
    EXPECT_EQ(dest[3], 20);
    EXPECT_EQ(dest[4], 30);
    EXPECT_EQ(dest[0], 0);
    EXPECT_EQ(dest[1], 0);
}

TEST(ArraySegmentTests, CopyTo_VectorWithOffset_ExpandsDest) {
    std::vector<int> v{7, 8};
    ArraySegment<int> seg(v);
    std::vector<int> dest;
    seg.CopyTo(dest, 3);
    EXPECT_EQ(static_cast<intcs>(dest.size()), 5);
    EXPECT_EQ(dest[3], 7);
    EXPECT_EQ(dest[4], 8);
}

TEST(ArraySegmentTests, CopyTo_VectorNegativeDestinationIndex_Throws) {
    // destinationIndex + count_ (e.g. -1 + 5 = 4) previously bypassed the resize
    // check against a large-enough destination, then destination.begin() + (-1)
    // computed an out-of-bounds iterator -- a genuine heap-buffer-overflow write.
    std::vector<int> v{1, 2, 3, 4, 5};
    ArraySegment<int> seg(v);
    std::vector<int> dest(10, 0);
    EXPECT_THROW(seg.CopyTo(dest, -1), System::ArgumentOutOfRangeException);
}


// ---------------------------------------------------------------------------
// CopyTo(ArraySegment)
// ---------------------------------------------------------------------------

TEST(ArraySegmentTests, CopyTo_ArraySegment_CopiesElements) {
    std::vector<int> src{1, 2, 3};
    std::vector<int> dst{0, 0, 0, 0};
    ArraySegment<int> srcSeg(src);
    ArraySegment<int> dstSeg(dst, 1, 3);
    srcSeg.CopyTo(dstSeg);
    EXPECT_EQ(dst[1], 1);
    EXPECT_EQ(dst[2], 2);
    EXPECT_EQ(dst[3], 3);
    EXPECT_EQ(dst[0], 0);
}

TEST(ArraySegmentTests, CopyTo_ArraySegment_TooSmall_Throws) {
    std::vector<int> src{1, 2, 3};
    std::vector<int> dst{0, 0};
    ArraySegment<int> srcSeg(src);
    ArraySegment<int> dstSeg(dst);
    EXPECT_THROW(srcSeg.CopyTo(dstSeg), System::ArgumentException);
}

// ---------------------------------------------------------------------------
// ToArray
// ---------------------------------------------------------------------------

TEST(ArraySegmentTests, ToArray_ReturnsCorrectElements) {
    std::vector<int> v{10, 20, 30, 40, 50};
    ArraySegment<int> seg(v, 1, 3);
    auto arr = seg.ToArray();
    EXPECT_EQ(arr, (std::vector<int>{20, 30, 40}));
}

TEST(ArraySegmentTests, ToArray_EmptySegment_EmptyVector) {
    std::vector<int> v{1, 2};
    ArraySegment<int> seg(v, 0, 0);
    EXPECT_TRUE(seg.ToArray().empty());
}

TEST(ArraySegmentTests, ToArray_IsIndependentCopy) {
    std::vector<int> v{1, 2, 3};
    ArraySegment<int> seg(v);
    auto arr = seg.ToArray();
    v[0] = 99;
    EXPECT_EQ(arr[0], 1);
}

// ---------------------------------------------------------------------------
// Contains
// ---------------------------------------------------------------------------

TEST(ArraySegmentTests, Contains_PresentElement_True) {
    std::vector<int> v{10, 20, 30};
    ArraySegment<int> seg(v);
    EXPECT_TRUE(seg.Contains(20));
}

TEST(ArraySegmentTests, Contains_AbsentElement_False) {
    std::vector<int> v{10, 20, 30};
    ArraySegment<int> seg(v);
    EXPECT_FALSE(seg.Contains(99));
}

TEST(ArraySegmentTests, Contains_OnlyWithinSegmentBounds) {
    std::vector<int> v{10, 20, 30, 40};
    ArraySegment<int> seg(v, 1, 2);
    EXPECT_FALSE(seg.Contains(10));
    EXPECT_FALSE(seg.Contains(40));
    EXPECT_TRUE(seg.Contains(20));
    EXPECT_TRUE(seg.Contains(30));
}

// ---------------------------------------------------------------------------
// IndexOf
// ---------------------------------------------------------------------------

TEST(ArraySegmentTests, IndexOf_Found_ReturnsSegmentLocalIndex) {
    std::vector<int> v{10, 20, 30, 40};
    ArraySegment<int> seg(v, 1, 3);
    EXPECT_EQ(seg.IndexOf(20), 0);
    EXPECT_EQ(seg.IndexOf(30), 1);
    EXPECT_EQ(seg.IndexOf(40), 2);
}

TEST(ArraySegmentTests, IndexOf_NotFound_ReturnsMinusOne) {
    std::vector<int> v{10, 20, 30};
    ArraySegment<int> seg(v);
    EXPECT_EQ(seg.IndexOf(99), -1);
}

TEST(ArraySegmentTests, IndexOf_OutsideBounds_NotFound) {
    std::vector<int> v{10, 20, 30, 40};
    ArraySegment<int> seg(v, 1, 2);
    EXPECT_EQ(seg.IndexOf(10), -1);
    EXPECT_EQ(seg.IndexOf(40), -1);
}

// ---------------------------------------------------------------------------
// Equals / operator== / operator!=
// ---------------------------------------------------------------------------

TEST(ArraySegmentTests, Equals_SameSegment_True) {
    std::vector<int> v{1, 2, 3};
    ArraySegment<int> a(v, 0, 2);
    ArraySegment<int> b(v, 0, 2);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(ArraySegmentTests, Equals_DifferentOffset_False) {
    std::vector<int> v{1, 2, 3};
    ArraySegment<int> a(v, 0, 2);
    ArraySegment<int> b(v, 1, 2);
    EXPECT_FALSE(a.Equals(b));
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST(ArraySegmentTests, Equals_DifferentCount_False) {
    std::vector<int> v{1, 2, 3};
    ArraySegment<int> a(v, 0, 2);
    ArraySegment<int> b(v, 0, 3);
    EXPECT_FALSE(a.Equals(b));
}

TEST(ArraySegmentTests, Equals_DifferentArray_False) {
    std::vector<int> v1{1, 2, 3};
    std::vector<int> v2{1, 2, 3};
    ArraySegment<int> a(v1);
    ArraySegment<int> b(v2);
    EXPECT_FALSE(a.Equals(b));
}

// ---------------------------------------------------------------------------
// GetHashCode
// ---------------------------------------------------------------------------

TEST(ArraySegmentTests, GetHashCode_SameSegment_SameHash) {
    std::vector<int> v{1, 2, 3};
    ArraySegment<int> a(v, 0, 2);
    ArraySegment<int> b(v, 0, 2);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ArraySegmentTests, GetHashCode_NonNegative) {
    std::vector<int> v{1, 2, 3};
    ArraySegment<int> seg(v);
    EXPECT_GE(seg.GetHashCode(), 0);
}

TEST(ArraySegmentTests, GetHashCode_DifferentOffset_DifferentHash) {
    std::vector<int> v{1, 2, 3};
    ArraySegment<int> a(v, 0, 2);
    ArraySegment<int> b(v, 1, 2);
    EXPECT_NE(a.GetHashCode(), b.GetHashCode());
}

// ---------------------------------------------------------------------------
// Range-for iteration
// ---------------------------------------------------------------------------

TEST(ArraySegmentTests, RangeFor_IteratesSegmentElements) {
    std::vector<int> v{10, 20, 30, 40, 50};
    ArraySegment<int> seg(v, 1, 3);
    std::vector<int> collected;
    for (int x : seg) collected.push_back(x);
    EXPECT_EQ(collected, (std::vector<int>{20, 30, 40}));
}
