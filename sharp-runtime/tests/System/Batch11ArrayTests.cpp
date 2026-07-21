// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <vector>
#include "System/Array.hpp"

using System::Array;
using SharpRuntime::intcs;

// ===========================================================================
// Sort — sub-range (index, length)
// ===========================================================================

TEST(ArraySortRangeTests, Sort_SubRange_OnlyRangeSorted) {
    std::vector<int> v = {5, 3, 1, 4, 2};
    Array::Sort(v, 1, 3);   // sort indices 1-3 → {5, 1, 3, 4, 2}
    EXPECT_EQ(v[0], 5);
    EXPECT_EQ(v[1], 1);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 4);
    EXPECT_EQ(v[4], 2);
}

TEST(ArraySortRangeTests, Sort_SubRange_WholeArray_SameAsFullSort) {
    std::vector<int> v = {4, 2, 1, 3};
    Array::Sort(v, 0, static_cast<intcs>(v.size()));
    EXPECT_TRUE(std::is_sorted(v.begin(), v.end()));
}

TEST(ArraySortRangeTests, Sort_SubRange_WithComparison_Descending) {
    std::vector<int> v = {1, 5, 2, 4, 3};
    Array::Sort<int>(v, 1, 3, [](const int& a, const int& b){ return b - a; });
    // indices 1-3 sorted descending: 5,2,4 → 5,4,2
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 5);
    EXPECT_EQ(v[2], 4);
    EXPECT_EQ(v[3], 2);
    EXPECT_EQ(v[4], 3);
}

// ===========================================================================
// Copy — from-start overload
// ===========================================================================

TEST(ArrayCopyTests, Copy_FromStart_CopiesLength) {
    std::vector<int> src = {10, 20, 30, 40, 50};
    std::vector<int> dst(5, 0);
    Array::Copy(src, dst, 3);
    EXPECT_EQ(dst[0], 10);
    EXPECT_EQ(dst[1], 20);
    EXPECT_EQ(dst[2], 30);
    EXPECT_EQ(dst[3], 0);  // untouched
    EXPECT_EQ(dst[4], 0);
}

TEST(ArrayCopyTests, Copy_FromStart_ZeroLength_NoChange) {
    std::vector<int> src = {1, 2, 3};
    std::vector<int> dst = {9, 9, 9};
    Array::Copy(src, dst, 0);
    EXPECT_EQ(dst[0], 9);
}

// ===========================================================================
// ConstrainedCopy
// ===========================================================================

TEST(ArrayConstrainedCopyTests, ConstrainedCopy_CopiesElements) {
    std::vector<int> src = {7, 8, 9};
    std::vector<int> dst(3, 0);
    Array::ConstrainedCopy(src, 0, dst, 0, 3);
    EXPECT_EQ(dst, src);
}

TEST(ArrayConstrainedCopyTests, ConstrainedCopy_SubRange) {
    std::vector<int> src = {1, 2, 3, 4};
    std::vector<int> dst(4, 0);
    Array::ConstrainedCopy(src, 1, dst, 2, 2);
    EXPECT_EQ(dst[2], 2);
    EXPECT_EQ(dst[3], 3);
    EXPECT_EQ(dst[0], 0);
}

// ===========================================================================
// Reverse — sub-range (index, length)
// ===========================================================================

TEST(ArrayReverseRangeTests, Reverse_SubRange_OnlyRangeReversed) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    Array::Reverse(v, 1, 3);  // reverse indices 1-3: 2,3,4 → 4,3,2
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 4);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 2);
    EXPECT_EQ(v[4], 5);
}

TEST(ArrayReverseRangeTests, Reverse_SubRange_SingleElement_Unchanged) {
    std::vector<int> v = {10, 20, 30};
    Array::Reverse(v, 1, 1);
    EXPECT_EQ(v[1], 20);
}

// ===========================================================================
// Clear — entire array
// ===========================================================================

TEST(ArrayClearAllTests, Clear_All_ZeroesEntireVector) {
    std::vector<int> v = {1, 2, 3, 4};
    Array::Clear(v);
    EXPECT_EQ(v, (std::vector<int>{0, 0, 0, 0}));
}

TEST(ArrayClearAllTests, Clear_All_EmptyVector_NoOp) {
    std::vector<int> v;
    EXPECT_NO_THROW(Array::Clear(v));
    EXPECT_TRUE(v.empty());
}

TEST(ArrayClearAllTests, Clear_All_Strings_EmptiesStrings) {
    std::vector<std::string> v = {"a", "b", "c"};
    Array::Clear(v);
    for (const auto& s : v) EXPECT_EQ(s, "");
}

// ===========================================================================
// BinarySearch — range overload
// ===========================================================================

TEST(ArrayBinarySearchRangeTests, BinarySearch_Range_Found) {
    std::vector<int> v = {1, 3, 5, 7, 9};
    EXPECT_EQ(Array::BinarySearch(v, 1, 3, 5), 2);
}

TEST(ArrayBinarySearchRangeTests, BinarySearch_Range_NotInRange_Negative) {
    std::vector<int> v = {1, 3, 5, 7, 9};
    EXPECT_LT(Array::BinarySearch(v, 0, 2, 9), 0);
}

TEST(ArrayBinarySearchRangeTests, BinarySearch_Range_SingleElement_Found) {
    std::vector<int> v = {1, 5, 10};
    EXPECT_EQ(Array::BinarySearch(v, 1, 1, 5), 1);
}

// ===========================================================================
// BinarySearch — with comparison
// ===========================================================================

TEST(ArrayBinarySearchCompTests, BinarySearch_WithComparison_Found) {
    std::vector<int> v = {1, 3, 5, 7, 9};
    intcs idx = Array::BinarySearch<int>(v, 7,
        [](const int& a, const int& b){ return a < b ? -1 : a > b ? 1 : 0; });
    EXPECT_EQ(idx, 3);
}

TEST(ArrayBinarySearchCompTests, BinarySearch_WithComparison_NotFound_Negative) {
    std::vector<int> v = {2, 4, 6, 8};
    intcs idx = Array::BinarySearch<int>(v, 5,
        [](const int& a, const int& b){ return a - b; });
    EXPECT_LT(idx, 0);
}

TEST(ArrayBinarySearchCompTests, BinarySearch_RangeWithComparison_Found) {
    std::vector<int> v = {1, 3, 5, 7, 9};
    intcs idx = Array::BinarySearch<int>(v, 1, 4, 7,
        [](const int& a, const int& b){ return a - b; });
    EXPECT_EQ(idx, 3);
}

// ===========================================================================
// IndexOf — with count
// ===========================================================================

TEST(ArrayIndexOfCountTests, IndexOf_WithCount_Found) {
    std::vector<int> v = {1, 2, 3, 2, 5};
    EXPECT_EQ(Array::IndexOf(v, 2, 0, 3), 1);
}

TEST(ArrayIndexOfCountTests, IndexOf_WithCount_NotInRange) {
    std::vector<int> v = {1, 2, 3, 2, 5};
    EXPECT_EQ(Array::IndexOf(v, 2, 2, 1), -1);  // only index 2 searched, value is 3
}

TEST(ArrayIndexOfCountTests, IndexOf_WithCount_ZeroCount_NotFound) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_EQ(Array::IndexOf(v, 1, 0, 0), -1);
}

// ===========================================================================
// LastIndexOf — startIndex overload
// ===========================================================================

TEST(ArrayLastIndexOfTests, LastIndexOf_StartIndex_Found) {
    std::vector<int> v = {1, 2, 3, 2, 1};
    EXPECT_EQ(Array::LastIndexOf(v, 2, 3), 3);
}

TEST(ArrayLastIndexOfTests, LastIndexOf_StartIndex_NotFound_BeforeValue) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    EXPECT_EQ(Array::LastIndexOf(v, 5, 3), -1);  // 5 is at index 4, not found before index 3
}

TEST(ArrayLastIndexOfTests, LastIndexOf_StartIndex_AtExactIndex) {
    std::vector<int> v = {10, 20, 30};
    EXPECT_EQ(Array::LastIndexOf(v, 20, 1), 1);
}

// ===========================================================================
// LastIndexOf — startIndex + count overload
// ===========================================================================

TEST(ArrayLastIndexOfCountTests, LastIndexOf_Count_Found) {
    std::vector<int> v = {1, 2, 3, 2, 1};
    // search backward from index 3, count 3 → indices 1,2,3
    EXPECT_EQ(Array::LastIndexOf(v, 2, 3, 3), 3);
}

TEST(ArrayLastIndexOfCountTests, LastIndexOf_Count_NotInRange) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    // search backward from index 4, count 2 → indices 3,4  (values 4,5)
    EXPECT_EQ(Array::LastIndexOf(v, 2, 4, 2), -1);
}

TEST(ArrayLastIndexOfCountTests, LastIndexOf_Count_One_Found) {
    std::vector<int> v = {7, 8, 9};
    EXPECT_EQ(Array::LastIndexOf(v, 9, 2, 1), 2);
}

// ===========================================================================
// FindIndex — startIndex overload
// ===========================================================================

TEST(ArrayFindIndexStartTests, FindIndex_StartIndex_Found) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    EXPECT_EQ(Array::FindIndex<int>(v, 2, [](const int& x){ return x > 3; }), 3);
}

TEST(ArrayFindIndexStartTests, FindIndex_StartIndex_NotFound) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_EQ(Array::FindIndex<int>(v, 2, [](const int& x){ return x < 2; }), -1);
}

TEST(ArrayFindIndexStartTests, FindIndex_StartAtEnd_NotFound) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_EQ(Array::FindIndex<int>(v, static_cast<intcs>(v.size()), [](const int&){ return true; }), -1);
}

// ===========================================================================
// FindIndex — startIndex + count overload
// ===========================================================================

TEST(ArrayFindIndexCountTests, FindIndex_Count_Found) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    EXPECT_EQ(Array::FindIndex<int>(v, 1, 3, [](const int& x){ return x == 3; }), 2);
}

TEST(ArrayFindIndexCountTests, FindIndex_Count_NotInRange) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    EXPECT_EQ(Array::FindIndex<int>(v, 0, 2, [](const int& x){ return x > 3; }), -1);
}

// ===========================================================================
// FindLastIndex — startIndex overload
// ===========================================================================

TEST(ArrayFindLastIndexStartTests, FindLastIndex_StartIndex_Found) {
    std::vector<int> v = {1, 2, 3, 2, 1};
    EXPECT_EQ(Array::FindLastIndex<int>(v, 3, [](const int& x){ return x == 2; }), 3);
}

TEST(ArrayFindLastIndexStartTests, FindLastIndex_StartIndex_NotFound) {
    std::vector<int> v = {1, 2, 3, 4};
    EXPECT_EQ(Array::FindLastIndex<int>(v, 1, [](const int& x){ return x > 3; }), -1);
}

// ===========================================================================
// FindLastIndex — startIndex + count overload
// ===========================================================================

TEST(ArrayFindLastIndexCountTests, FindLastIndex_Count_Found) {
    std::vector<int> v = {5, 3, 5, 2, 1};
    // search backward from index 3, count 3 → indices 1,2,3
    EXPECT_EQ(Array::FindLastIndex<int>(v, 3, 3, [](const int& x){ return x == 5; }), 2);
}

TEST(ArrayFindLastIndexCountTests, FindLastIndex_Count_NotInRange) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    EXPECT_EQ(Array::FindLastIndex<int>(v, 1, 2, [](const int& x){ return x > 3; }), -1);
}

// ===========================================================================
// AsReadOnly
// ===========================================================================

TEST(ArrayAsReadOnlyTests, AsReadOnly_ReturnsSameReference) {
    std::vector<int> v = {1, 2, 3};
    const std::vector<int>& ro = Array::AsReadOnly(v);
    EXPECT_EQ(ro[0], 1);
    EXPECT_EQ(ro[2], 3);
}

TEST(ArrayAsReadOnlyTests, AsReadOnly_ReflectsMutations) {
    std::vector<int> v = {10, 20};
    const std::vector<int>& ro = Array::AsReadOnly(v);
    v[0] = 99;
    EXPECT_EQ(ro[0], 99);
}
