// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <limits>
#include <vector>
#include "System/MemoryExtensions.hpp"

using System::Span;
using System::ReadOnlySpan;
using System::MemoryExtensions;

// ---------------------------------------------------------------------------
// AsSpan
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, AsSpan_Vector_FullLength) {
    std::vector<int> v = {1, 2, 3};
    auto s = MemoryExtensions::AsSpan(v);
    EXPECT_EQ(s.getLengthProperty(), 3);
    EXPECT_EQ(s[0], 1);
    EXPECT_EQ(s[2], 3);
}

TEST(MemoryExtensionsTests, AsSpan_Vector_WithStart) {
    std::vector<int> v = {10, 20, 30, 40};
    auto s = MemoryExtensions::AsSpan(v, 2);
    EXPECT_EQ(s.getLengthProperty(), 2);
    EXPECT_EQ(s[0], 30);
}

TEST(MemoryExtensionsTests, AsSpan_Vector_StartAndLength) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto s = MemoryExtensions::AsSpan(v, 1, 3);
    EXPECT_EQ(s.getLengthProperty(), 3);
    EXPECT_EQ(s[0], 2);
    EXPECT_EQ(s[2], 4);
}

// Regression tests for a code-audit finding (ticket 238): AsSpan(vector&, start) and
// AsSpan(vector&, start, length) had no validation at all, unlike their AsSpan(string, ...)
// siblings just below (AsSpan_String_StartOutOfRange_Throws etc.), which already correctly
// throw. An out-of-bounds start/length here fell straight into raw pointer arithmetic
// (v.data() + start) with no bounds check at all -- undefined behavior, not even a defined
// std:: exception to catch. Verified against real .NET's MemoryExtensions.AsSpan<T>(T[], int)
// ((uint)start > (uint)array.Length -> ArgumentOutOfRangeException) and AsSpan<T>(T[], int,
// int) (delegates to the Span<T> constructor, which validates the same way).
TEST(MemoryExtensionsTests, AsSpan_Vector_StartOutOfRange_Throws) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_THROW(MemoryExtensions::AsSpan(v, 99), System::ArgumentOutOfRangeException);
    EXPECT_THROW(MemoryExtensions::AsSpan(v, -1), System::ArgumentOutOfRangeException);
}

TEST(MemoryExtensionsTests, AsSpan_Vector_StartAndLengthOutOfRange_Throws) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    EXPECT_THROW(MemoryExtensions::AsSpan(v, 0, 99), System::ArgumentOutOfRangeException);
    EXPECT_THROW(MemoryExtensions::AsSpan(v, 1, -1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(MemoryExtensions::AsSpan(v, 4, 2), System::ArgumentOutOfRangeException);
}

// Same overflow-bypasses-the-check bug as Span<T>::Slice (ticket 265/1487): start+length used
// to be computed directly in intcs (int32) arithmetic, which overflows for large start/length
// and silently bypasses the check instead of throwing.
TEST(MemoryExtensionsTests, AsSpan_Vector_StartPlusLengthOverflow_ThrowsInsteadOfBypassingCheck) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    EXPECT_THROW(MemoryExtensions::AsSpan(v, 2147483647, 10), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// Contains
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, Contains_Found) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_TRUE(MemoryExtensions::Contains(ReadOnlySpan<int>(v), 2));
}

TEST(MemoryExtensionsTests, Contains_NotFound) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_FALSE(MemoryExtensions::Contains(ReadOnlySpan<int>(v), 99));
}

// ---------------------------------------------------------------------------
// IndexOf
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, IndexOf_Found) {
    std::vector<int> v = {10, 20, 30};
    EXPECT_EQ(MemoryExtensions::IndexOf(ReadOnlySpan<int>(v), 20), 1);
}

TEST(MemoryExtensionsTests, IndexOf_NotFound) {
    std::vector<int> v = {10, 20, 30};
    EXPECT_EQ(MemoryExtensions::IndexOf(ReadOnlySpan<int>(v), 99), -1);
}

TEST(MemoryExtensionsTests, IndexOf_FirstOccurrence) {
    std::vector<int> v = {5, 5, 5};
    EXPECT_EQ(MemoryExtensions::IndexOf(ReadOnlySpan<int>(v), 5), 0);
}

// ---------------------------------------------------------------------------
// LastIndexOf
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, LastIndexOf_Found) {
    std::vector<int> v = {1, 2, 1};
    EXPECT_EQ(MemoryExtensions::LastIndexOf(ReadOnlySpan<int>(v), 1), 2);
}

TEST(MemoryExtensionsTests, LastIndexOf_NotFound) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_EQ(MemoryExtensions::LastIndexOf(ReadOnlySpan<int>(v), 9), -1);
}

// ---------------------------------------------------------------------------
// SequenceEqual
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, SequenceEqual_Equal) {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {1, 2, 3};
    EXPECT_TRUE(MemoryExtensions::SequenceEqual(ReadOnlySpan<int>(a), ReadOnlySpan<int>(b)));
}

TEST(MemoryExtensionsTests, SequenceEqual_DifferentValues) {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {1, 2, 4};
    EXPECT_FALSE(MemoryExtensions::SequenceEqual(ReadOnlySpan<int>(a), ReadOnlySpan<int>(b)));
}

TEST(MemoryExtensionsTests, SequenceEqual_DifferentLength) {
    std::vector<int> a = {1, 2};
    std::vector<int> b = {1, 2, 3};
    EXPECT_FALSE(MemoryExtensions::SequenceEqual(ReadOnlySpan<int>(a), ReadOnlySpan<int>(b)));
}

// ---------------------------------------------------------------------------
// Fill
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, Fill_AllElements) {
    std::vector<int> v(4, 0);
    MemoryExtensions::Fill(Span<int>(v), 7);
    for (int x : v) EXPECT_EQ(x, 7);
}

// ---------------------------------------------------------------------------
// Reverse
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, Reverse_Elements) {
    std::vector<int> v = {1, 2, 3, 4};
    MemoryExtensions::Reverse(Span<int>(v));
    EXPECT_EQ(v[0], 4);
    EXPECT_EQ(v[3], 1);
}

// ---------------------------------------------------------------------------
// CopyTo
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, CopyTo_CopiesElements) {
    std::vector<int> src = {1, 2, 3};
    std::vector<int> dst(3, 0);
    MemoryExtensions::CopyTo(ReadOnlySpan<int>(src), Span<int>(dst));
    EXPECT_EQ(dst[0], 1);
    EXPECT_EQ(dst[2], 3);
}

// ---------------------------------------------------------------------------
// Sort
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, Sort_Ascending) {
    std::vector<int> v = {3, 1, 4, 1, 5};
    MemoryExtensions::Sort(Span<int>(v));
    EXPECT_TRUE(std::is_sorted(v.begin(), v.end()));
}

TEST(MemoryExtensionsTests, Sort_WithComparer) {
    std::vector<int> v = {1, 3, 2};
    MemoryExtensions::Sort(Span<int>(v), std::greater<int>{});
    EXPECT_EQ(v[0], 3);
    EXPECT_EQ(v[2], 1);
}

// ---------------------------------------------------------------------------
// StartsWith / EndsWith
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, StartsWith_True) {
    std::vector<int> v = {1, 2, 3, 4};
    std::vector<int> prefix = {1, 2};
    EXPECT_TRUE(MemoryExtensions::StartsWith(ReadOnlySpan<int>(v), ReadOnlySpan<int>(prefix)));
}

TEST(MemoryExtensionsTests, StartsWith_False) {
    std::vector<int> v = {1, 2, 3};
    std::vector<int> prefix = {2, 3};
    EXPECT_FALSE(MemoryExtensions::StartsWith(ReadOnlySpan<int>(v), ReadOnlySpan<int>(prefix)));
}

TEST(MemoryExtensionsTests, EndsWith_True) {
    std::vector<int> v = {1, 2, 3, 4};
    std::vector<int> suffix = {3, 4};
    EXPECT_TRUE(MemoryExtensions::EndsWith(ReadOnlySpan<int>(v), ReadOnlySpan<int>(suffix)));
}

TEST(MemoryExtensionsTests, EndsWith_False) {
    std::vector<int> v = {1, 2, 3};
    std::vector<int> suffix = {1, 2};
    EXPECT_FALSE(MemoryExtensions::EndsWith(ReadOnlySpan<int>(v), ReadOnlySpan<int>(suffix)));
}

TEST(MemoryExtensionsTests, StartsWith_LongerValue_False) {
    std::vector<int> v = {1};
    std::vector<int> prefix = {1, 2, 3};
    EXPECT_FALSE(MemoryExtensions::StartsWith(ReadOnlySpan<int>(v), ReadOnlySpan<int>(prefix)));
}

// ---------------------------------------------------------------------------
// AsSpan from string
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, AsSpan_String_Full) {
    std::string s = "hello";
    auto span = MemoryExtensions::AsSpan(s);
    EXPECT_EQ(span.getLengthProperty(), 5);
    EXPECT_EQ(span[0], 'h');
    EXPECT_EQ(span[4], 'o');
}

TEST(MemoryExtensionsTests, AsSpan_String_WithStart) {
    std::string s = "hello";
    auto span = MemoryExtensions::AsSpan(s, 2);
    EXPECT_EQ(span.getLengthProperty(), 3);
    EXPECT_EQ(span[0], 'l');
}

TEST(MemoryExtensionsTests, AsSpan_String_StartAndLength) {
    std::string s = "hello world";
    auto span = MemoryExtensions::AsSpan(s, 6, 5);
    EXPECT_EQ(span.getLengthProperty(), 5);
    EXPECT_EQ(span[0], 'w');
    EXPECT_EQ(span[4], 'd');
}

TEST(MemoryExtensionsTests, AsSpan_String_StartOutOfRange_Throws) {
    std::string s = "hi";
    EXPECT_THROW(MemoryExtensions::AsSpan(s, 99), System::ArgumentOutOfRangeException);
}

TEST(MemoryExtensionsTests, AsSpan_String_LengthOutOfRange_Throws) {
    std::string s = "hi";
    EXPECT_THROW(MemoryExtensions::AsSpan(s, 0, 99), System::ArgumentOutOfRangeException);
}

// Same overflow-bypasses-the-check bug as Span<T>::Slice (ticket 265/1487).
TEST(MemoryExtensionsTests, AsSpan_String_StartPlusLengthOverflow_ThrowsInsteadOfBypassingCheck) {
    std::string s = "hi";
    EXPECT_THROW(MemoryExtensions::AsSpan(s, 2147483647, 10), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// ContainsAny
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, ContainsAny_TwoValues_Found) {
    std::vector<int> v = {1, 3, 5};
    EXPECT_TRUE(MemoryExtensions::ContainsAny(ReadOnlySpan<int>(v), 3, 99));
}

TEST(MemoryExtensionsTests, ContainsAny_TwoValues_NotFound) {
    std::vector<int> v = {1, 3, 5};
    EXPECT_FALSE(MemoryExtensions::ContainsAny(ReadOnlySpan<int>(v), 2, 4));
}

TEST(MemoryExtensionsTests, ContainsAny_ThreeValues_Found) {
    std::vector<int> v = {10, 20, 30};
    EXPECT_TRUE(MemoryExtensions::ContainsAny(ReadOnlySpan<int>(v), 0, 0, 20));
}

TEST(MemoryExtensionsTests, ContainsAny_Span_Found) {
    std::vector<int> v = {1, 2, 3};
    std::vector<int> vals = {9, 2};
    EXPECT_TRUE(MemoryExtensions::ContainsAny(
        ReadOnlySpan<int>(v), ReadOnlySpan<int>(vals)));
}

// ---------------------------------------------------------------------------
// IndexOf — subsequence
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, IndexOf_Subsequence_Found) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    std::vector<int> needle = {3, 4};
    EXPECT_EQ(MemoryExtensions::IndexOf(ReadOnlySpan<int>(v), ReadOnlySpan<int>(needle)), 2);
}

TEST(MemoryExtensionsTests, IndexOf_Subsequence_NotFound) {
    std::vector<int> v = {1, 2, 3};
    std::vector<int> needle = {4, 5};
    EXPECT_EQ(MemoryExtensions::IndexOf(ReadOnlySpan<int>(v), ReadOnlySpan<int>(needle)), -1);
}

TEST(MemoryExtensionsTests, IndexOf_EmptyNeedle_ReturnsZero) {
    std::vector<int> v = {1, 2, 3};
    std::vector<int> needle;
    EXPECT_EQ(MemoryExtensions::IndexOf(ReadOnlySpan<int>(v), ReadOnlySpan<int>(needle)), 0);
}

// ---------------------------------------------------------------------------
// LastIndexOf — subsequence
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, LastIndexOf_Subsequence_Found) {
    std::vector<int> v = {1, 2, 3, 1, 2};
    std::vector<int> needle = {1, 2};
    EXPECT_EQ(MemoryExtensions::LastIndexOf(ReadOnlySpan<int>(v), ReadOnlySpan<int>(needle)), 3);
}

TEST(MemoryExtensionsTests, LastIndexOf_Subsequence_NotFound) {
    std::vector<int> v = {1, 2, 3};
    std::vector<int> needle = {9, 9};
    EXPECT_EQ(MemoryExtensions::LastIndexOf(ReadOnlySpan<int>(v), ReadOnlySpan<int>(needle)), -1);
}

// ---------------------------------------------------------------------------
// SequenceCompareTo
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, SequenceCompareTo_Equal) {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {1, 2, 3};
    EXPECT_EQ(MemoryExtensions::SequenceCompareTo(ReadOnlySpan<int>(a), ReadOnlySpan<int>(b)), 0);
}

TEST(MemoryExtensionsTests, SequenceCompareTo_LessThan) {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {1, 2, 4};
    EXPECT_LT(MemoryExtensions::SequenceCompareTo(ReadOnlySpan<int>(a), ReadOnlySpan<int>(b)), 0);
}

TEST(MemoryExtensionsTests, SequenceCompareTo_GreaterThan) {
    std::vector<int> a = {1, 2, 4};
    std::vector<int> b = {1, 2, 3};
    EXPECT_GT(MemoryExtensions::SequenceCompareTo(ReadOnlySpan<int>(a), ReadOnlySpan<int>(b)), 0);
}

TEST(MemoryExtensionsTests, SequenceCompareTo_ShorterIsLess) {
    std::vector<int> a = {1, 2};
    std::vector<int> b = {1, 2, 3};
    EXPECT_LT(MemoryExtensions::SequenceCompareTo(ReadOnlySpan<int>(a), ReadOnlySpan<int>(b)), 0);
}

// ---------------------------------------------------------------------------
// Count
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, Count_MultipleOccurrences) {
    std::vector<int> v = {1, 2, 1, 3, 1};
    EXPECT_EQ(MemoryExtensions::Count(ReadOnlySpan<int>(v), 1), 3);
}

TEST(MemoryExtensionsTests, Count_NoOccurrences) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_EQ(MemoryExtensions::Count(ReadOnlySpan<int>(v), 9), 0);
}

// ---------------------------------------------------------------------------
// BinarySearch
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, BinarySearch_Found) {
    std::vector<int> v = {1, 3, 5, 7, 9};
    EXPECT_EQ(MemoryExtensions::BinarySearch(ReadOnlySpan<int>(v), 5), 2);
}

TEST(MemoryExtensionsTests, BinarySearch_NotFound_ReturnsComplement) {
    std::vector<int> v = {1, 3, 5, 7, 9};
    int idx = MemoryExtensions::BinarySearch(ReadOnlySpan<int>(v), 4);
    EXPECT_LT(idx, 0);
    EXPECT_EQ(~idx, 2);
}

// ---------------------------------------------------------------------------
// Overlaps
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, Overlaps_SameMemory_True) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    ReadOnlySpan<int> a(v.data(), 3);
    ReadOnlySpan<int> b(v.data() + 2, 3);
    EXPECT_TRUE(MemoryExtensions::Overlaps(a, b));
}

TEST(MemoryExtensionsTests, Overlaps_Disjoint_False) {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {4, 5, 6};
    EXPECT_FALSE(MemoryExtensions::Overlaps(ReadOnlySpan<int>(a), ReadOnlySpan<int>(b)));
}

TEST(MemoryExtensionsTests, Overlaps_EmptySpan_False) {
    std::vector<int> v = {1, 2, 3};
    ReadOnlySpan<int> empty(v.data(), 0);
    EXPECT_FALSE(MemoryExtensions::Overlaps(ReadOnlySpan<int>(v), empty));
}

// ---------------------------------------------------------------------------
// Trim / TrimStart / TrimEnd — whitespace
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, Trim_Whitespace) {
    std::string s = "  hello  ";
    auto span = MemoryExtensions::Trim(MemoryExtensions::AsSpan(s));
    EXPECT_EQ(span.getLengthProperty(), 5);
    EXPECT_EQ(span[0], 'h');
}

TEST(MemoryExtensionsTests, TrimStart_Whitespace) {
    std::string s = "  hi";
    auto span = MemoryExtensions::TrimStart(MemoryExtensions::AsSpan(s));
    EXPECT_EQ(span.getLengthProperty(), 2);
    EXPECT_EQ(span[0], 'h');
}

TEST(MemoryExtensionsTests, TrimEnd_Whitespace) {
    std::string s = "hi  ";
    auto span = MemoryExtensions::TrimEnd(MemoryExtensions::AsSpan(s));
    EXPECT_EQ(span.getLengthProperty(), 2);
    EXPECT_EQ(span[1], 'i');
}

TEST(MemoryExtensionsTests, Trim_NothingToTrim) {
    std::string s = "hello";
    auto span = MemoryExtensions::Trim(MemoryExtensions::AsSpan(s));
    EXPECT_EQ(span.getLengthProperty(), 5);
}

// ---------------------------------------------------------------------------
// Trim / TrimStart / TrimEnd — single element
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, Trim_SingleElement) {
    std::vector<int> v = {0, 0, 1, 2, 0, 0};
    auto span = MemoryExtensions::Trim(ReadOnlySpan<int>(v), 0);
    EXPECT_EQ(span.getLengthProperty(), 2);
    EXPECT_EQ(span[0], 1);
    EXPECT_EQ(span[1], 2);
}

TEST(MemoryExtensionsTests, TrimStart_SingleElement) {
    std::vector<int> v = {9, 9, 1, 2, 9};
    auto span = MemoryExtensions::TrimStart(ReadOnlySpan<int>(v), 9);
    EXPECT_EQ(span.getLengthProperty(), 3);
    EXPECT_EQ(span[0], 1);
}

TEST(MemoryExtensionsTests, TrimEnd_SingleElement) {
    std::vector<int> v = {1, 2, 9, 9};
    auto span = MemoryExtensions::TrimEnd(ReadOnlySpan<int>(v), 9);
    EXPECT_EQ(span.getLengthProperty(), 2);
    EXPECT_EQ(span[1], 2);
}

// ---------------------------------------------------------------------------
// IndexOfAny
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, IndexOfAny_TwoValues_FirstMatch) {
    std::vector<int> v = {1, 5, 3, 7};
    EXPECT_EQ(MemoryExtensions::IndexOfAny(ReadOnlySpan<int>(v), 5, 7), 1);
}

TEST(MemoryExtensionsTests, IndexOfAny_TwoValues_NotFound) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_EQ(MemoryExtensions::IndexOfAny(ReadOnlySpan<int>(v), 8, 9), -1);
}

TEST(MemoryExtensionsTests, IndexOfAny_ThreeValues_Found) {
    std::vector<int> v = {10, 20, 30, 40};
    EXPECT_EQ(MemoryExtensions::IndexOfAny(ReadOnlySpan<int>(v), 0, 30, 99), 2);
}

TEST(MemoryExtensionsTests, IndexOfAny_Span_Found) {
    std::vector<int> v = {1, 2, 3, 4};
    std::vector<int> vals = {7, 3};
    EXPECT_EQ(MemoryExtensions::IndexOfAny(ReadOnlySpan<int>(v), ReadOnlySpan<int>(vals)), 2);
}

TEST(MemoryExtensionsTests, IndexOfAny_Span_NotFound) {
    std::vector<int> v = {1, 2, 3};
    std::vector<int> vals = {8, 9};
    EXPECT_EQ(MemoryExtensions::IndexOfAny(ReadOnlySpan<int>(v), ReadOnlySpan<int>(vals)), -1);
}

// ---------------------------------------------------------------------------
// LastIndexOfAny
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, LastIndexOfAny_TwoValues_LastMatch) {
    std::vector<int> v = {5, 2, 7, 5};
    EXPECT_EQ(MemoryExtensions::LastIndexOfAny(ReadOnlySpan<int>(v), 5, 7), 3);
}

TEST(MemoryExtensionsTests, LastIndexOfAny_TwoValues_NotFound) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_EQ(MemoryExtensions::LastIndexOfAny(ReadOnlySpan<int>(v), 8, 9), -1);
}

// Regression test for a code-audit finding (ticket 238): LastIndexOfAny was missing the
// 3-value overload that its sibling IndexOfAny already has (IndexOfAny_ThreeValues_Found
// above), and that real .NET's MemoryExtensions.LastIndexOfAny<T>(ReadOnlySpan<T>, T, T, T)
// provides.
TEST(MemoryExtensionsTests, LastIndexOfAny_ThreeValues_Found) {
    std::vector<int> v = {10, 20, 30, 40, 20};
    EXPECT_EQ(MemoryExtensions::LastIndexOfAny(ReadOnlySpan<int>(v), 0, 20, 99), 4);
}

TEST(MemoryExtensionsTests, LastIndexOfAny_ThreeValues_NotFound) {
    std::vector<int> v = {1, 2, 3};
    EXPECT_EQ(MemoryExtensions::LastIndexOfAny(ReadOnlySpan<int>(v), 7, 8, 9), -1);
}

TEST(MemoryExtensionsTests, LastIndexOfAny_Span_Found) {
    std::vector<int> v = {1, 2, 3, 2, 1};
    std::vector<int> vals = {2, 3};
    EXPECT_EQ(MemoryExtensions::LastIndexOfAny(ReadOnlySpan<int>(v), ReadOnlySpan<int>(vals)), 3);
}

// ---------------------------------------------------------------------------
// Replace
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, Replace_SingleOccurrence) {
    std::vector<int> v = {1, 2, 3};
    MemoryExtensions::Replace(Span<int>(v), 2, 99);
    EXPECT_EQ(v[1], 99);
}

TEST(MemoryExtensionsTests, Replace_MultipleOccurrences) {
    std::vector<int> v = {5, 1, 5, 2, 5};
    MemoryExtensions::Replace(Span<int>(v), 5, 0);
    EXPECT_EQ(v[0], 0);
    EXPECT_EQ(v[2], 0);
    EXPECT_EQ(v[4], 0);
    EXPECT_EQ(v[1], 1);
}

TEST(MemoryExtensionsTests, Replace_NoMatch_Unchanged) {
    std::vector<int> v = {1, 2, 3};
    MemoryExtensions::Replace(Span<int>(v), 9, 0);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

// ---------------------------------------------------------------------------
// CommonPrefixLength
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, CommonPrefixLength_Matching) {
    std::vector<int> a = {1, 2, 3, 4};
    std::vector<int> b = {1, 2, 9, 9};
    EXPECT_EQ(MemoryExtensions::CommonPrefixLength(ReadOnlySpan<int>(a), ReadOnlySpan<int>(b)), 2);
}

TEST(MemoryExtensionsTests, CommonPrefixLength_NoCommon) {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {9, 8, 7};
    EXPECT_EQ(MemoryExtensions::CommonPrefixLength(ReadOnlySpan<int>(a), ReadOnlySpan<int>(b)), 0);
}

TEST(MemoryExtensionsTests, CommonPrefixLength_FullMatch) {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {1, 2, 3};
    EXPECT_EQ(MemoryExtensions::CommonPrefixLength(ReadOnlySpan<int>(a), ReadOnlySpan<int>(b)), 3);
}

// ---------------------------------------------------------------------------
// StartsWith / EndsWith — single element
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, StartsWith_SingleElement_True) {
    std::vector<int> v = {5, 1, 2};
    EXPECT_TRUE(MemoryExtensions::StartsWith(ReadOnlySpan<int>(v), 5));
}

TEST(MemoryExtensionsTests, StartsWith_SingleElement_False) {
    std::vector<int> v = {5, 1, 2};
    EXPECT_FALSE(MemoryExtensions::StartsWith(ReadOnlySpan<int>(v), 1));
}

TEST(MemoryExtensionsTests, EndsWith_SingleElement_True) {
    std::vector<int> v = {1, 2, 7};
    EXPECT_TRUE(MemoryExtensions::EndsWith(ReadOnlySpan<int>(v), 7));
}

TEST(MemoryExtensionsTests, EndsWith_SingleElement_False) {
    std::vector<int> v = {1, 2, 7};
    EXPECT_FALSE(MemoryExtensions::EndsWith(ReadOnlySpan<int>(v), 2));
}

// ---------------------------------------------------------------------------
// Trim / TrimStart / TrimEnd — span of trim elements
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, Trim_SpanElements_BothEnds) {
    std::vector<int> v = {0, 9, 1, 2, 9, 0};
    std::vector<int> trimVals = {0, 9};
    auto span = MemoryExtensions::Trim(ReadOnlySpan<int>(v), ReadOnlySpan<int>(trimVals));
    EXPECT_EQ(span.getLengthProperty(), 2);
    EXPECT_EQ(span[0], 1);
    EXPECT_EQ(span[1], 2);
}

TEST(MemoryExtensionsTests, TrimStart_SpanElements_Leading) {
    std::vector<int> v = {0, 9, 1, 2};
    std::vector<int> trimVals = {0, 9};
    auto span = MemoryExtensions::TrimStart(ReadOnlySpan<int>(v), ReadOnlySpan<int>(trimVals));
    EXPECT_EQ(span.getLengthProperty(), 2);
    EXPECT_EQ(span[0], 1);
}

TEST(MemoryExtensionsTests, TrimEnd_SpanElements_Trailing) {
    std::vector<int> v = {1, 2, 9, 0};
    std::vector<int> trimVals = {0, 9};
    auto span = MemoryExtensions::TrimEnd(ReadOnlySpan<int>(v), ReadOnlySpan<int>(trimVals));
    EXPECT_EQ(span.getLengthProperty(), 2);
    EXPECT_EQ(span[1], 2);
}

// ---------------------------------------------------------------------------
// NaN equality: .NET's Contains/IndexOf/etc. constrain T to IEquatable<T> and
// dispatch through .Equals, not ==. float.Equals(NaN) treats NaN as equal to
// itself (unlike operator==, which is always false for NaN). Verified against
// Single.cs: "obj == m_value || (IsNaN(obj) && IsNaN(m_value))".
// ---------------------------------------------------------------------------

TEST(MemoryExtensionsTests, Contains_FindsNaN_MatchingDotNetEquatable) {
    float nan = std::numeric_limits<float>::quiet_NaN();
    std::vector<float> v = {1.0f, nan, 3.0f};
    EXPECT_TRUE(MemoryExtensions::Contains(ReadOnlySpan<float>(v), nan));
}

TEST(MemoryExtensionsTests, IndexOf_FindsNaN_MatchingDotNetEquatable) {
    float nan = std::numeric_limits<float>::quiet_NaN();
    std::vector<float> v = {1.0f, nan, 3.0f};
    EXPECT_EQ(MemoryExtensions::IndexOf(ReadOnlySpan<float>(v), nan), 1);
}

TEST(MemoryExtensionsTests, LastIndexOf_FindsNaN_MatchingDotNetEquatable) {
    double nan = std::numeric_limits<double>::quiet_NaN();
    std::vector<double> v = {nan, 2.0, nan};
    EXPECT_EQ(MemoryExtensions::LastIndexOf(ReadOnlySpan<double>(v), nan), 2);
}

TEST(MemoryExtensionsTests, Count_CountsNaN_MatchingDotNetEquatable) {
    float nan = std::numeric_limits<float>::quiet_NaN();
    std::vector<float> v = {nan, 1.0f, nan};
    EXPECT_EQ(MemoryExtensions::Count(ReadOnlySpan<float>(v), nan), 2);
}

TEST(MemoryExtensionsTests, SequenceEqual_TreatsNaNAsEqual_MatchingDotNetEquatable) {
    float nan = std::numeric_limits<float>::quiet_NaN();
    std::vector<float> a = {1.0f, nan, 3.0f};
    std::vector<float> b = {1.0f, nan, 3.0f};
    EXPECT_TRUE(MemoryExtensions::SequenceEqual(ReadOnlySpan<float>(a), ReadOnlySpan<float>(b)));
}

TEST(MemoryExtensionsTests, Replace_ReplacesNaN_MatchingDotNetEquatable) {
    float nan = std::numeric_limits<float>::quiet_NaN();
    std::vector<float> v = {1.0f, nan, 3.0f};
    Span<float> span(v);
    MemoryExtensions::Replace(span, nan, 0.0f);
    EXPECT_FLOAT_EQ(v[1], 0.0f);
}

TEST(MemoryExtensionsTests, IndexOfAny_FindsNaN_MatchingDotNetEquatable) {
    float nan = std::numeric_limits<float>::quiet_NaN();
    std::vector<float> v = {1.0f, 2.0f, nan};
    EXPECT_EQ(MemoryExtensions::IndexOfAny(ReadOnlySpan<float>(v), nan, 99.0f), 2);
}

TEST(MemoryExtensionsTests, ContainsAny_FindsNaN_MatchingDotNetEquatable) {
    float nan = std::numeric_limits<float>::quiet_NaN();
    std::vector<float> v = {1.0f, 2.0f, nan};
    EXPECT_TRUE(MemoryExtensions::ContainsAny(ReadOnlySpan<float>(v), nan, 99.0f));
}

TEST(MemoryExtensionsTests, StartsWith_SingleElement_NaN_MatchingDotNetEquatable) {
    float nan = std::numeric_limits<float>::quiet_NaN();
    std::vector<float> v = {nan, 2.0f};
    EXPECT_TRUE(MemoryExtensions::StartsWith(ReadOnlySpan<float>(v), nan));
}

TEST(MemoryExtensionsTests, IntSpan_NaNPathNotTaken_StillOrdinaryEquality) {
    // Non-floating-point T must still use plain equality (no NaN branch).
    std::vector<int> v = {1, 2, 3};
    EXPECT_TRUE(MemoryExtensions::Contains(ReadOnlySpan<int>(v), 2));
    EXPECT_FALSE(MemoryExtensions::Contains(ReadOnlySpan<int>(v), 4));
}
