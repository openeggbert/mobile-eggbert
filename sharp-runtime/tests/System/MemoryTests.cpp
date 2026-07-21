// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Memory.hpp"
#include "System/ArraySegment.hpp"
#include "System/ReadOnlyMemory.hpp"

using System::Memory;
using System::ArraySegment;
using System::ReadOnlyMemory;
using SharpRuntime::intcs;

// ---------------------------------------------------------------------------
// Default constructor / Empty
// ---------------------------------------------------------------------------

TEST(MemoryTests, DefaultCtor_LengthIsZero) {
    Memory<int> m;
    EXPECT_EQ(m.getLengthProperty(), 0);
}

TEST(MemoryTests, DefaultCtor_IsEmpty) {
    Memory<int> m;
    EXPECT_TRUE(m.getIsEmptyProperty());
}

TEST(MemoryTests, GetEmpty_IsEmpty) {
    auto m = Memory<int>::getEmpty();
    EXPECT_TRUE(m.getIsEmptyProperty());
}

TEST(MemoryTests, GetEmpty_LengthIsZero) {
    auto m = Memory<int>::getEmpty();
    EXPECT_EQ(m.getLengthProperty(), 0);
}

// ---------------------------------------------------------------------------
// 1-arg constructor (full vector)
// ---------------------------------------------------------------------------

TEST(MemoryTests, VectorCtor_LengthMatchesVector) {
    std::vector<int> v{10, 20, 30};
    Memory<int> m(v);
    EXPECT_EQ(m.getLengthProperty(), 3);
}

TEST(MemoryTests, VectorCtor_IsNotEmpty) {
    std::vector<int> v{1, 2};
    Memory<int> m(v);
    EXPECT_FALSE(m.getIsEmptyProperty());
}

TEST(MemoryTests, VectorCtor_EmptyVector_IsEmpty) {
    std::vector<int> v;
    Memory<int> m(v);
    EXPECT_TRUE(m.getIsEmptyProperty());
}

// ---------------------------------------------------------------------------
// 3-arg constructor (start, length)
// ---------------------------------------------------------------------------

TEST(MemoryTests, SubrangeCtor_LengthCorrect) {
    std::vector<int> v{10, 20, 30, 40, 50};
    Memory<int> m(v, 1, 3);
    EXPECT_EQ(m.getLengthProperty(), 3);
}

TEST(MemoryTests, SubrangeCtor_OutOfRange_Throws) {
    std::vector<int> v{1, 2, 3};
    EXPECT_THROW((Memory<int>(v, 0, 10)), System::ArgumentOutOfRangeException);
}

TEST(MemoryTests, SubrangeCtor_NegativeStart_Throws) {
    std::vector<int> v{1, 2, 3};
    EXPECT_THROW((Memory<int>(v, -1, 2)), System::ArgumentOutOfRangeException);
}

TEST(MemoryTests, SubrangeCtor_NegativeLength_Throws) {
    std::vector<int> v{1, 2, 3};
    EXPECT_THROW((Memory<int>(v, 0, -1)), System::ArgumentOutOfRangeException);
}

TEST(MemoryTests, SubrangeCtor_ZeroLength_IsEmpty) {
    std::vector<int> v{1, 2, 3};
    Memory<int> m(v, 1, 0);
    EXPECT_TRUE(m.getIsEmptyProperty());
}

// ---------------------------------------------------------------------------
// getSpanProperty
// ---------------------------------------------------------------------------

TEST(MemoryTests, Span_FullVector_ReadsCorrect) {
    std::vector<int> v{10, 20, 30};
    Memory<int> m(v);
    auto span = m.getSpanProperty();
    EXPECT_EQ(span[0], 10);
    EXPECT_EQ(span[1], 20);
    EXPECT_EQ(span[2], 30);
}

TEST(MemoryTests, Span_Subrange_ReadsCorrect) {
    std::vector<int> v{10, 20, 30, 40};
    Memory<int> m(v, 1, 2);
    auto span = m.getSpanProperty();
    EXPECT_EQ(span[0], 20);
    EXPECT_EQ(span[1], 30);
}

TEST(MemoryTests, Span_Default_LengthIsZero) {
    Memory<int> m;
    EXPECT_EQ(m.getSpanProperty().getLengthProperty(), 0);
}

// ---------------------------------------------------------------------------
// Slice (1-arg)
// ---------------------------------------------------------------------------

TEST(MemoryTests, Slice1_FromMiddle_CorrectLength) {
    std::vector<int> v{10, 20, 30, 40, 50};
    Memory<int> m(v);
    Memory<int> s = m.Slice(2);
    EXPECT_EQ(s.getLengthProperty(), 3);
}

TEST(MemoryTests, Slice1_FromZero_SameLength) {
    std::vector<int> v{1, 2, 3};
    Memory<int> m(v);
    EXPECT_EQ(m.Slice(0).getLengthProperty(), 3);
}

TEST(MemoryTests, Slice1_FromEnd_IsEmpty) {
    std::vector<int> v{1, 2, 3};
    Memory<int> m(v);
    EXPECT_TRUE(m.Slice(3).getIsEmptyProperty());
}

TEST(MemoryTests, Slice1_OutOfRange_Throws) {
    std::vector<int> v{1, 2};
    Memory<int> m(v);
    EXPECT_THROW(m.Slice(5), System::ArgumentOutOfRangeException);
}

TEST(MemoryTests, Slice1_Negative_Throws) {
    std::vector<int> v{1, 2, 3};
    Memory<int> m(v);
    EXPECT_THROW(m.Slice(-1), System::ArgumentOutOfRangeException);
}

TEST(MemoryTests, Slice1_CorrectElements) {
    std::vector<int> v{10, 20, 30, 40};
    Memory<int> m(v);
    auto s = m.Slice(1);
    EXPECT_EQ(s.getSpanProperty()[0], 20);
    EXPECT_EQ(s.getSpanProperty()[1], 30);
    EXPECT_EQ(s.getSpanProperty()[2], 40);
}

// ---------------------------------------------------------------------------
// Slice (2-arg)
// ---------------------------------------------------------------------------

TEST(MemoryTests, Slice2_CorrectLength) {
    std::vector<int> v{10, 20, 30, 40, 50};
    Memory<int> m(v);
    EXPECT_EQ(m.Slice(1, 3).getLengthProperty(), 3);
}

TEST(MemoryTests, Slice2_CorrectElements) {
    std::vector<int> v{10, 20, 30, 40};
    Memory<int> m(v);
    auto s = m.Slice(1, 2);
    EXPECT_EQ(s.getSpanProperty()[0], 20);
    EXPECT_EQ(s.getSpanProperty()[1], 30);
}

TEST(MemoryTests, Slice2_OutOfRange_Throws) {
    std::vector<int> v{1, 2, 3};
    Memory<int> m(v);
    EXPECT_THROW(m.Slice(0, 10), System::ArgumentOutOfRangeException);
}

// Same overflow-bypasses-the-check bug as Span<T>::Slice (ticket 265/1487): start+length used
// to be computed directly in intcs (int32) arithmetic, which overflows for large start/length
// and silently bypasses the check instead of throwing.
TEST(MemoryTests, Slice2_StartPlusLengthOverflow_ThrowsInsteadOfBypassingCheck) {
    std::vector<int> v{1, 2, 3};
    Memory<int> m(v);
    EXPECT_THROW(m.Slice(2147483647, 10), System::ArgumentOutOfRangeException);
}

TEST(MemoryTests, Constructor_StartPlusCountOverflow_ThrowsInsteadOfBypassingCheck) {
    std::vector<int> v{1, 2, 3};
    EXPECT_THROW(Memory<int>(v, 2147483647, 10), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// CopyTo / TryCopyTo
// ---------------------------------------------------------------------------

TEST(MemoryTests, CopyTo_CopiesElements) {
    std::vector<int> src{10, 20, 30};
    std::vector<int> dst{0, 0, 0};
    Memory<int> s(src);
    Memory<int> d(dst);
    s.CopyTo(d);
    EXPECT_EQ(dst[0], 10);
    EXPECT_EQ(dst[1], 20);
    EXPECT_EQ(dst[2], 30);
}

TEST(MemoryTests, CopyTo_DestTooShort_Throws) {
    std::vector<int> src{1, 2, 3};
    std::vector<int> dst{0, 0};
    Memory<int> s(src);
    Memory<int> d(dst);
    EXPECT_THROW(s.CopyTo(d), System::ArgumentException);
}

TEST(MemoryTests, TryCopyTo_Success_ReturnsTrue) {
    std::vector<int> src{1, 2};
    std::vector<int> dst{0, 0, 0};
    Memory<int> s(src);
    Memory<int> d(dst);
    EXPECT_TRUE(s.TryCopyTo(d));
    EXPECT_EQ(dst[0], 1);
    EXPECT_EQ(dst[1], 2);
}

TEST(MemoryTests, TryCopyTo_DestTooShort_ReturnsFalse) {
    std::vector<int> src{1, 2, 3};
    std::vector<int> dst{0};
    Memory<int> s(src);
    Memory<int> d(dst);
    EXPECT_FALSE(s.TryCopyTo(d));
}

// ---------------------------------------------------------------------------
// ToArray
// ---------------------------------------------------------------------------

TEST(MemoryTests, ToArray_FullVector_CorrectElements) {
    std::vector<int> v{10, 20, 30};
    Memory<int> m(v);
    auto arr = m.ToArray();
    EXPECT_EQ(arr, (std::vector<int>{10, 20, 30}));
}

TEST(MemoryTests, ToArray_Subrange_CorrectElements) {
    std::vector<int> v{10, 20, 30, 40};
    Memory<int> m(v, 1, 2);
    auto arr = m.ToArray();
    EXPECT_EQ(arr, (std::vector<int>{20, 30}));
}

TEST(MemoryTests, ToArray_Empty_ReturnsEmptyVector) {
    Memory<int> m;
    EXPECT_TRUE(m.ToArray().empty());
}

TEST(MemoryTests, ToArray_IsIndependentCopy) {
    std::vector<int> v{1, 2, 3};
    Memory<int> m(v);
    auto arr = m.ToArray();
    v[0] = 99;
    EXPECT_EQ(arr[0], 1);
}

// ---------------------------------------------------------------------------
// Equals / operator== / operator!=
// ---------------------------------------------------------------------------

TEST(MemoryTests, Equals_SameRegion_True) {
    std::vector<int> v{1, 2, 3};
    Memory<int> a(v, 0, 2);
    Memory<int> b(v, 0, 2);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(MemoryTests, Equals_DiffOffset_False) {
    std::vector<int> v{1, 2, 3};
    Memory<int> a(v, 0, 2);
    Memory<int> b(v, 1, 2);
    EXPECT_FALSE(a.Equals(b));
    EXPECT_TRUE(a != b);
}

TEST(MemoryTests, Equals_DiffLength_False) {
    std::vector<int> v{1, 2, 3};
    Memory<int> a(v, 0, 2);
    Memory<int> b(v, 0, 3);
    EXPECT_FALSE(a.Equals(b));
}

TEST(MemoryTests, Equals_DiffVector_False) {
    std::vector<int> v1{1, 2, 3};
    std::vector<int> v2{1, 2, 3};
    Memory<int> a(v1);
    Memory<int> b(v2);
    EXPECT_FALSE(a.Equals(b));
}

// ---------------------------------------------------------------------------
// GetHashCode
// ---------------------------------------------------------------------------

TEST(MemoryTests, GetHashCode_SameRegion_SameHash) {
    std::vector<int> v{1, 2, 3};
    Memory<int> a(v, 0, 2);
    Memory<int> b(v, 0, 2);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(MemoryTests, GetHashCode_NonNegative) {
    std::vector<int> v{1, 2, 3};
    Memory<int> m(v);
    EXPECT_GE(m.GetHashCode(), 0);
}

// ---------------------------------------------------------------------------
// ToString
// ---------------------------------------------------------------------------

TEST(MemoryTests, ToString_ContainsLength) {
    std::vector<int> v{1, 2, 3};
    Memory<int> m(v);
    std::string s = m.ToString();
    EXPECT_NE(s.find("3"), std::string::npos);
}

TEST(MemoryTests, ToString_EmptyMemory_ContainsZero) {
    Memory<int> m;
    std::string s = m.ToString();
    EXPECT_NE(s.find("0"), std::string::npos);
}

TEST(MemoryTests, ConvertFromArraySegment_Length) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    ArraySegment<int> seg(v, 1, 3);
    Memory<int> m(seg);
    EXPECT_EQ(m.getLengthProperty(), 3);
}

TEST(MemoryTests, ConvertFromArraySegment_Values) {
    std::vector<int> v = {10, 20, 30, 40};
    ArraySegment<int> seg(v, 1, 2);
    Memory<int> m(seg);
    auto arr = m.ToArray();
    EXPECT_EQ(arr[0], 20);
    EXPECT_EQ(arr[1], 30);
}

TEST(MemoryTests, ConvertToReadOnlyMemory_Length) {
    std::vector<int> v = {1, 2, 3};
    Memory<int> m(v);
    ReadOnlyMemory<int> rom = m;
    EXPECT_EQ(rom.getLengthProperty(), 3);
}

TEST(MemoryTests, ConvertToReadOnlyMemory_Empty) {
    Memory<int> m;
    ReadOnlyMemory<int> rom = m;
    EXPECT_TRUE(rom.getIsEmptyProperty());
}

// ---------------------------------------------------------------------------
// Pin
// ---------------------------------------------------------------------------

TEST(MemoryTests, Pin_PointerMatchesData) {
    std::vector<int> v = {10, 20, 30};
    Memory<int> m(v);
    auto handle = m.Pin();
    EXPECT_EQ(handle.getPointerProperty(), static_cast<void*>(v.data()));
}

TEST(MemoryTests, Pin_Subrange_PointerOffsetCorrectly) {
    std::vector<int> v = {10, 20, 30, 40};
    Memory<int> m(v, 1, 2);
    auto handle = m.Pin();
    EXPECT_EQ(handle.getPointerProperty(), static_cast<void*>(v.data() + 1));
}

TEST(MemoryTests, Pin_DefaultMemory_NullPointer) {
    Memory<int> m;
    auto handle = m.Pin();
    EXPECT_EQ(handle.getPointerProperty(), nullptr);
}
