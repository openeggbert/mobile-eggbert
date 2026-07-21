// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <vector>
#include "System/Array.hpp"

using System::Array;
using SharpRuntime::intcs;

// ---------------------------------------------------------------------------
// Sort — default comparer
// ---------------------------------------------------------------------------

TEST(ArrayTests, Sort_Ascending_Integer) {
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6};
    Array::Sort(v);
    EXPECT_TRUE(std::is_sorted(v.begin(), v.end()));
}

TEST(ArrayTests, Sort_EmptyVector_NoOp) {
    std::vector<int> v;
    Array::Sort(v);
    EXPECT_TRUE(v.empty());
}

TEST(ArrayTests, Sort_SingleElement_NoOp) {
    std::vector<int> v = {42};
    Array::Sort(v);
    EXPECT_EQ(v[0], 42);
}

TEST(ArrayTests, Sort_AlreadySorted_StaysOrdered) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    Array::Sort(v);
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3, 4, 5}));
}

// ---------------------------------------------------------------------------
// Sort — custom comparison
// ---------------------------------------------------------------------------

TEST(ArrayTests, Sort_Descending_CustomComparison) {
    std::vector<int> v = {3, 1, 4, 1, 5};
    Array::Sort<int>(v, [](const int& a, const int& b) { return b - a; });
    EXPECT_TRUE(std::is_sorted(v.begin(), v.end(), std::greater<int>()));
}

TEST(ArrayTests, Sort_ByStringLength) {
    std::vector<std::string> v = {"banana", "fig", "apple", "kiwi"};
    Array::Sort<std::string>(v, [](const std::string& a, const std::string& b) {
        return static_cast<int>(a.size()) - static_cast<int>(b.size());
    });
    EXPECT_LE(v[0].size(), v[1].size());
    EXPECT_LE(v[1].size(), v[2].size());
    EXPECT_LE(v[2].size(), v[3].size());
}

// ---------------------------------------------------------------------------
// Copy — vector overload
// ---------------------------------------------------------------------------

TEST(ArrayTests, Copy_VectorSubrange) {
    std::vector<int> src = {10, 20, 30, 40, 50};
    std::vector<int> dst(5, 0);
    Array::Copy(src, 1, dst, 2, 3);
    EXPECT_EQ(dst[2], 20);
    EXPECT_EQ(dst[3], 30);
    EXPECT_EQ(dst[4], 40);
    EXPECT_EQ(dst[0], 0);  // untouched
    EXPECT_EQ(dst[1], 0);  // untouched
}

TEST(ArrayTests, Copy_VectorFullRange) {
    std::vector<int> src = {1, 2, 3};
    std::vector<int> dst(3, 0);
    Array::Copy(src, 0, dst, 0, 3);
    EXPECT_EQ(dst, src);
}

TEST(ArrayTests, Copy_VectorZeroLength_NoChange) {
    std::vector<int> src = {9, 9, 9};
    std::vector<int> dst = {1, 2, 3};
    Array::Copy(src, 0, dst, 0, 0);
    EXPECT_EQ(dst, (std::vector<int>{1, 2, 3}));
}

// ---------------------------------------------------------------------------
// Copy — raw pointer overload
// ---------------------------------------------------------------------------

TEST(ArrayTests, Copy_RawPointer_CopiesBytes) {
    int src[4] = {10, 20, 30, 40};
    int dst[4] = {0, 0, 0, 0};
    Array::Copy(src, 1, dst, 0, 3);
    EXPECT_EQ(dst[0], 20);
    EXPECT_EQ(dst[1], 30);
    EXPECT_EQ(dst[2], 40);
}

// ---------------------------------------------------------------------------
// Resize
// ---------------------------------------------------------------------------

TEST(ArrayTests, Resize_GrowVector_NewElementsAreZero) {
    std::vector<int> v = {1, 2, 3};
    Array::Resize(v, 5);
    EXPECT_EQ(static_cast<intcs>(v.size()), 5);
    EXPECT_EQ(v[3], 0);
    EXPECT_EQ(v[4], 0);
}

TEST(ArrayTests, Resize_ShrinkVector_TruncatesElements) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    Array::Resize(v, 2);
    EXPECT_EQ(static_cast<intcs>(v.size()), 2);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
}

TEST(ArrayTests, Resize_SameSize_NoChange) {
    std::vector<int> v = {7, 8};
    Array::Resize(v, 2);
    EXPECT_EQ(v, (std::vector<int>{7, 8}));
}

// ---------------------------------------------------------------------------
// IndexOf
// ---------------------------------------------------------------------------

TEST(ArrayTests, IndexOf_PresentElement_ReturnsCorrectIndex) {
    std::vector<int> v = {10, 20, 30, 40};
    EXPECT_EQ(Array::IndexOf(v, 30), 2);
}

TEST(ArrayTests, IndexOf_FirstElement_ReturnsZero) {
    std::vector<int> v = {5, 10, 15};
    EXPECT_EQ(Array::IndexOf(v, 5), 0);
}

TEST(ArrayTests, IndexOf_LastElement_ReturnsLastIndex) {
    std::vector<int> v = {5, 10, 15};
    EXPECT_EQ(Array::IndexOf(v, 15), 2);
}

TEST(ArrayTests, IndexOf_AbsentElement_ReturnsMinusOne) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_EQ(Array::IndexOf(v, 99), -1);
}

TEST(ArrayTests, IndexOf_EmptyVector_ReturnsMinusOne) {
    std::vector<int> v;
    EXPECT_EQ(Array::IndexOf(v, 0), -1);
}

TEST(ArrayTests, IndexOf_DuplicateValues_ReturnsFirst) {
    std::vector<int> v = {7, 7, 7};
    EXPECT_EQ(Array::IndexOf(v, 7), 0);
}

// ---------------------------------------------------------------------------
// Reverse
// ---------------------------------------------------------------------------

TEST(ArrayTests, Reverse_OddLength) {
    std::vector<int> v = {1, 2, 3};
    Array::Reverse(v);
    EXPECT_EQ(v, (std::vector<int>{3, 2, 1}));
}

TEST(ArrayTests, Reverse_EvenLength) {
    std::vector<int> v = {1, 2, 3, 4};
    Array::Reverse(v);
    EXPECT_EQ(v, (std::vector<int>{4, 3, 2, 1}));
}

TEST(ArrayTests, Reverse_SingleElement_Unchanged) {
    std::vector<int> v = {42};
    Array::Reverse(v);
    EXPECT_EQ(v[0], 42);
}

TEST(ArrayTests, Reverse_EmptyVector_NoOp) {
    std::vector<int> v;
    Array::Reverse(v);
    EXPECT_TRUE(v.empty());
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------

TEST(ArrayTests, Clear_MiddleRange_ZeroesTargetElements) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    Array::Clear(v, 1, 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 0);
    EXPECT_EQ(v[2], 0);
    EXPECT_EQ(v[3], 0);
    EXPECT_EQ(v[4], 5);
}

TEST(ArrayTests, Clear_EntireVector) {
    std::vector<int> v = {9, 9, 9};
    Array::Clear(v, 0, 3);
    EXPECT_EQ(v, (std::vector<int>{0, 0, 0}));
}

TEST(ArrayTests, Clear_ZeroLength_NoChange) {
    std::vector<int> v = {5, 6, 7};
    Array::Clear(v, 0, 0);
    EXPECT_EQ(v, (std::vector<int>{5, 6, 7}));
}

// --- BinarySearch ---

TEST(ArrayTests, BinarySearch_Found_ReturnsIndex) {
    std::vector<int> v = {1, 3, 5, 7, 9};
    EXPECT_EQ(Array::BinarySearch(v, 5), 2);
}

TEST(ArrayTests, BinarySearch_FirstElement) {
    std::vector<int> v = {1, 3, 5, 7, 9};
    EXPECT_EQ(Array::BinarySearch(v, 1), 0);
}

TEST(ArrayTests, BinarySearch_LastElement) {
    std::vector<int> v = {1, 3, 5, 7, 9};
    EXPECT_EQ(Array::BinarySearch(v, 9), 4);
}

TEST(ArrayTests, BinarySearch_NotFound_NegativeComplement) {
    std::vector<int> v = {1, 3, 5, 7, 9};
    int r = Array::BinarySearch(v, 4);
    EXPECT_LT(r, 0); // not found → negative
}

TEST(ArrayTests, BinarySearch_EmptyVector_ReturnsNegative) {
    std::vector<int> v;
    EXPECT_LT(Array::BinarySearch(v, 1), 0);
}

// --- Fill ---

TEST(ArrayTests, Fill_EntireVector) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    Array::Fill(v, 0);
    EXPECT_EQ(v, (std::vector<int>{0, 0, 0, 0, 0}));
}

TEST(ArrayTests, Fill_Range) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    Array::Fill(v, 9, 1, 3);
    EXPECT_EQ(v, (std::vector<int>{1, 9, 9, 9, 5}));
}

TEST(ArrayTests, Fill_StringVector) {
    std::vector<std::string> v(3);
    Array::Fill(v, std::string("x"));
    EXPECT_EQ(v[0], "x");
    EXPECT_EQ(v[2], "x");
}

// ---------------------------------------------------------------------------
// Empty<T>
// ---------------------------------------------------------------------------

TEST(ArrayTests, Empty_Int_IsEmpty) {
    auto v = Array::Empty<int>();
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0u);
}
TEST(ArrayTests, Empty_String_IsEmpty) {
    auto v = Array::Empty<std::string>();
    EXPECT_TRUE(v.empty());
}
TEST(ArrayTests, Empty_ReturnsSeparateInstances) {
    auto a = Array::Empty<int>();
    auto b = Array::Empty<int>();
    a.push_back(1);
    EXPECT_TRUE(b.empty());
}

// ---------------------------------------------------------------------------
// IndexOf with startIndex
// ---------------------------------------------------------------------------

TEST(ArrayTests, IndexOf_StartIndex_Found) {
    std::vector<int> v = {1, 2, 3, 2, 4};
    EXPECT_EQ(Array::IndexOf(v, 2, 2), 3);
}
TEST(ArrayTests, IndexOf_StartIndex_NotFound) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_EQ(Array::IndexOf(v, 1, 1), -1);
}
TEST(ArrayTests, IndexOf_StartIndex_Zero_SameAsNoStart) {
    std::vector<int> v = {5, 6, 7};
    EXPECT_EQ(Array::IndexOf(v, 6, 0), 1);
}

// ---------------------------------------------------------------------------
// ConvertAll
// ---------------------------------------------------------------------------
TEST(ArrayTests, ConvertAll_IntToDouble) {
    std::vector<int> v = {1, 2, 3};
    auto r = Array::ConvertAll<int, double>(v, [](const int& x) { return x * 2.0; });
    EXPECT_EQ(r.size(), 3u);
    EXPECT_NEAR(r[0], 2.0, 1e-9);
    EXPECT_NEAR(r[2], 6.0, 1e-9);
}
TEST(ArrayTests, ConvertAll_Empty) {
    std::vector<int> v;
    auto r = Array::ConvertAll<int, std::string>(v, [](const int& x) { return std::to_string(x); });
    EXPECT_TRUE(r.empty());
}

// ---------------------------------------------------------------------------
// Exists
// ---------------------------------------------------------------------------
TEST(ArrayTests, Exists_Found)    { EXPECT_TRUE(Array::Exists<int>({1,2,3}, [](const int& x){return x==2;})); }
TEST(ArrayTests, Exists_NotFound) { EXPECT_FALSE(Array::Exists<int>({1,2,3}, [](const int& x){return x==9;})); }
TEST(ArrayTests, Exists_Empty)    { EXPECT_FALSE(Array::Exists<int>({}, [](const int&){return true;})); }

// ---------------------------------------------------------------------------
// Find / FindLast
// ---------------------------------------------------------------------------
TEST(ArrayTests, Find_Found)      { EXPECT_EQ(Array::Find<int>({1,2,3,4}, [](const int& x){return x>2;}), 3); }
TEST(ArrayTests, Find_NotFound)   { EXPECT_EQ(Array::Find<int>({1,2}, [](const int& x){return x>9;}), 0); }
TEST(ArrayTests, FindLast_Found)  { EXPECT_EQ(Array::FindLast<int>({1,3,5,2}, [](const int& x){return x%2!=0;}), 5); }
TEST(ArrayTests, FindLast_NotFound){ EXPECT_EQ(Array::FindLast<int>({2,4}, [](const int& x){return x%2!=0;}), 0); }

// ---------------------------------------------------------------------------
// FindAll
// ---------------------------------------------------------------------------
TEST(ArrayTests, FindAll_Multiple) {
    std::vector<int> v = {1,2,3,4,5};
    auto r = Array::FindAll<int>(v, [](const int& x){return x%2==0;});
    EXPECT_EQ(r.size(), 2u);
    EXPECT_EQ(r[0], 2);
    EXPECT_EQ(r[1], 4);
}
TEST(ArrayTests, FindAll_None) {
    auto r = Array::FindAll<int>({1,3,5}, [](const int& x){return x%2==0;});
    EXPECT_TRUE(r.empty());
}

// ---------------------------------------------------------------------------
// FindIndex / FindLastIndex
// ---------------------------------------------------------------------------
TEST(ArrayTests, FindIndex_Found)     { EXPECT_EQ(Array::FindIndex<int>({10,20,30}, [](const int& x){return x>15;}), 1); }
TEST(ArrayTests, FindIndex_NotFound)  { EXPECT_EQ(Array::FindIndex<int>({1,2,3}, [](const int& x){return x>9;}), -1); }
TEST(ArrayTests, FindLastIndex_Found) { EXPECT_EQ(Array::FindLastIndex<int>({1,2,3,2}, [](const int& x){return x==2;}), 3); }

// ---------------------------------------------------------------------------
// ForEach
// ---------------------------------------------------------------------------
TEST(ArrayTests, ForEach_SumsElements) {
    std::vector<int> v = {1,2,3,4};
    int sum = 0;
    Array::ForEach<int>(v, [&](const int& x){ sum += x; });
    EXPECT_EQ(sum, 10);
}

// ---------------------------------------------------------------------------
// TrueForAll
// ---------------------------------------------------------------------------
TEST(ArrayTests, TrueForAll_AllPositive)  { EXPECT_TRUE(Array::TrueForAll<int>({1,2,3}, [](const int& x){return x>0;})); }
TEST(ArrayTests, TrueForAll_OneNegative)  { EXPECT_FALSE(Array::TrueForAll<int>({1,-1,3}, [](const int& x){return x>0;})); }
TEST(ArrayTests, TrueForAll_Empty)        { EXPECT_TRUE(Array::TrueForAll<int>({}, [](const int&){return false;})); }

// ---------------------------------------------------------------------------
// LastIndexOf
// ---------------------------------------------------------------------------
TEST(ArrayTests, LastIndexOf_Found)    { EXPECT_EQ(Array::LastIndexOf<int>({1,2,1,3}, 1), 2); }
TEST(ArrayTests, LastIndexOf_NotFound) { EXPECT_EQ(Array::LastIndexOf<int>({1,2,3}, 9), -1); }

// ---------------------------------------------------------------------------
// MaxLengthProperty
// ---------------------------------------------------------------------------
TEST(ArrayTests, MaxLength_IsPositive) {
    EXPECT_GT(Array::MaxLengthProperty(), 0);
}

// ---------------------------------------------------------------------------
// Bounds validation (ticket 269): every index/count-taking method in this file previously did
// zero validation before raw std::vector iterator/index arithmetic -- a negative index in
// particular, cast to size_t, becomes a huge positive offset: a genuine out-of-bounds access,
// not just a wrong-answer bug. Matches real .NET Array's own validation exactly (unsigned
// comparison + subtraction, so large index/length can't integer-overflow past the check either
// -- same fix already applied this session to Span<T>::Slice and its many siblings).
// ---------------------------------------------------------------------------

TEST(ArrayTests, Sort_RangeOutOfBounds_Throws) {
    std::vector<int> v = {3, 1, 2};
    EXPECT_THROW(Array::Sort(v, 1, 5), System::ArgumentException);
    EXPECT_THROW(Array::Sort(v, -1, 1), System::ArgumentOutOfRangeException);
}

TEST(ArrayTests, Copy_ThreeArg_LengthExceedsSource_Throws) {
    std::vector<int> src = {1, 2};
    std::vector<int> dst(5, 0);
    EXPECT_THROW(Array::Copy(src, dst, 5), System::ArgumentException);
}

TEST(ArrayTests, Copy_FiveArg_RangeOutOfBounds_Throws) {
    std::vector<int> src = {1, 2, 3};
    std::vector<int> dst(3, 0);
    EXPECT_THROW(Array::Copy(src, 1, dst, 0, 5), System::ArgumentException);
    EXPECT_THROW(Array::Copy(src, -1, dst, 0, 1), System::ArgumentOutOfRangeException);
}

TEST(ArrayTests, Resize_Negative_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(Array::Resize(v, -1), System::ArgumentOutOfRangeException);
}

TEST(ArrayTests, IndexOf_NegativeStartIndex_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(Array::IndexOf(v, 1, -1), System::ArgumentOutOfRangeException);
}

TEST(ArrayTests, IndexOf_StartIndexPastEnd_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(Array::IndexOf(v, 1, 4), System::ArgumentOutOfRangeException);
}

TEST(ArrayTests, IndexOf_StartIndexEqualsSize_Legal) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_EQ(Array::IndexOf(v, 1, 3), -1); // legal: an immediately-exhausted search
}

TEST(ArrayTests, IndexOf_FourArg_CountPastEnd_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(Array::IndexOf(v, 1, 0, 10), System::ArgumentOutOfRangeException);
}

TEST(ArrayTests, Reverse_RangeOutOfBounds_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(Array::Reverse(v, 0, 10), System::ArgumentException);
}

TEST(ArrayTests, Clear_RangeOutOfBounds_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(Array::Clear(v, 0, 10), System::ArgumentException);
}

TEST(ArrayTests, BinarySearch_RangeOutOfBounds_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(Array::BinarySearch(v, 0, 10, 2), System::ArgumentException);
}

TEST(ArrayTests, Fill_RangeOutOfBounds_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(Array::Fill(v, 9, 0, 10), System::ArgumentException);
}

TEST(ArrayTests, FindIndex_NegativeStartIndex_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(Array::FindIndex<int>(v, -1, [](const int&) { return true; }), System::ArgumentOutOfRangeException);
}

TEST(ArrayTests, FindIndex_ThreeArg_CountPastEnd_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(Array::FindIndex<int>(v, 0, 10, [](const int&) { return true; }), System::ArgumentOutOfRangeException);
}

TEST(ArrayTests, FindLastIndex_StartIndexOutOfRange_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(Array::FindLastIndex<int>(v, 5, [](const int&) { return true; }), System::ArgumentOutOfRangeException);
}

TEST(ArrayTests, FindLastIndex_OnEmptyArray_ReturnsNegativeOneWithoutThrowing) {
    std::vector<int> v;
    // Matches real .NET's documented empty-array special case: startIndex of -1 or 0 is
    // accepted "for compatibility reasons" rather than throwing.
    EXPECT_EQ(Array::FindLastIndex<int>(v, -1, [](const int&) { return true; }), -1);
}

TEST(ArrayTests, LastIndexOf_StartIndexOutOfRange_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(Array::LastIndexOf(v, 1, 5), System::ArgumentOutOfRangeException);
}

TEST(ArrayTests, LastIndexOf_OnEmptyArray_ReturnsNegativeOneWithoutThrowing) {
    std::vector<int> v;
    EXPECT_EQ(Array::LastIndexOf(v, 1, -1), -1);
}

TEST(ArrayTests, LastIndexOf_FourArg_CountPastStart_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(Array::LastIndexOf(v, 1, 1, 5), System::ArgumentOutOfRangeException);
}
