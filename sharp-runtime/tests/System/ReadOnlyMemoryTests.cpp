// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ReadOnlyMemory.hpp"
#include "System/Memory.hpp"
#include "System/ArraySegment.hpp"

using System::ReadOnlyMemory;
using System::Memory;
using System::ArraySegment;

TEST(ReadOnlyMemoryTest, DefaultCtor_IsEmpty) {
    ReadOnlyMemory<int> m;
    EXPECT_TRUE(m.getIsEmptyProperty());
    EXPECT_EQ(m.getLengthProperty(), 0);
}

TEST(ReadOnlyMemoryTest, VectorCtor) {
    std::vector<int> v = {1, 2, 3};
    ReadOnlyMemory<int> m(v);
    EXPECT_EQ(m.getLengthProperty(), 3);
    EXPECT_FALSE(m.getIsEmptyProperty());
}

TEST(ReadOnlyMemoryTest, PtrLengthCtor) {
    int data[] = {10, 20, 30};
    ReadOnlyMemory<int> m(data, 3);
    EXPECT_EQ(m.getLengthProperty(), 3);
    EXPECT_EQ(m[0], 10);
    EXPECT_EQ(m[2], 30);
}

TEST(ReadOnlyMemoryTest, IndexAccess) {
    std::vector<int> v = {5, 6, 7};
    ReadOnlyMemory<int> m(v);
    EXPECT_EQ(m[0], 5);
    EXPECT_EQ(m[1], 6);
    EXPECT_EQ(m[2], 7);
}

TEST(ReadOnlyMemoryTest, IndexOutOfRange_Throws) {
    std::vector<int> v = {1, 2};
    ReadOnlyMemory<int> m(v);
    EXPECT_THROW(m[2], System::ArgumentOutOfRangeException);
    EXPECT_THROW(m[-1], System::ArgumentOutOfRangeException);
}

TEST(ReadOnlyMemoryTest, SliceStartLength) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    ReadOnlyMemory<int> m(v);
    auto s = m.Slice(1, 3);
    EXPECT_EQ(s.getLengthProperty(), 3);
    EXPECT_EQ(s[0], 2);
    EXPECT_EQ(s[2], 4);
}

TEST(ReadOnlyMemoryTest, SliceStart) {
    std::vector<int> v = {1, 2, 3, 4};
    ReadOnlyMemory<int> m(v);
    auto s = m.Slice(2);
    EXPECT_EQ(s.getLengthProperty(), 2);
    EXPECT_EQ(s[0], 3);
}

TEST(ReadOnlyMemoryTest, SliceOutOfRange_Throws) {
    std::vector<int> v = {1, 2, 3};
    ReadOnlyMemory<int> m(v);
    EXPECT_THROW(m.Slice(1, 5), System::ArgumentOutOfRangeException);
}

// Same overflow-bypasses-the-check bug as Span<T>::Slice (ticket 265/1487): start+length used
// to be computed directly in intcs (int32) arithmetic, which overflows for large start/length
// and silently bypasses the check instead of throwing.
TEST(ReadOnlyMemoryTest, Slice_StartPlusLengthOverflow_ThrowsInsteadOfBypassingCheck) {
    std::vector<int> v = {1, 2, 3};
    ReadOnlyMemory<int> m(v);
    EXPECT_THROW(m.Slice(2147483647, 10), System::ArgumentOutOfRangeException);
}

TEST(ReadOnlyMemoryTest, ToArray) {
    std::vector<int> v = {10, 20, 30};
    ReadOnlyMemory<int> m(v);
    auto arr = m.ToArray();
    EXPECT_EQ(arr.size(), 3u);
    EXPECT_EQ(arr[1], 20);
}

TEST(ReadOnlyMemoryTest, GetEmptyProperty) {
    auto e = ReadOnlyMemory<int>::getEmptyProperty();
    EXPECT_TRUE(e.getIsEmptyProperty());
    EXPECT_EQ(e.getLengthProperty(), 0);
}

TEST(ReadOnlyMemoryTest, GetSpanProperty) {
    std::vector<int> v = {1, 2, 3};
    ReadOnlyMemory<int> m(v);
    auto sp = m.getSpanProperty();
    EXPECT_EQ(sp.getLengthProperty(), 3);
    EXPECT_EQ(sp[0], 1);
}

TEST(ReadOnlyMemoryTest, Equals_SameRegion) {
    std::vector<int> v = {1, 2, 3};
    ReadOnlyMemory<int> a(v);
    ReadOnlyMemory<int> b(v);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_TRUE(a == b);
}

TEST(ReadOnlyMemoryTest, Equals_DifferentRegion) {
    std::vector<int> v1 = {1, 2, 3};
    std::vector<int> v2 = {1, 2, 3};
    ReadOnlyMemory<int> a(v1);
    ReadOnlyMemory<int> b(v2);
    EXPECT_FALSE(a.Equals(b));
    EXPECT_TRUE(a != b);
}

TEST(ReadOnlyMemoryTest, ToString_ContainsLength) {
    std::vector<int> v = {1, 2, 3};
    ReadOnlyMemory<int> m(v);
    EXPECT_NE(m.ToString().find("3"), std::string::npos);
}

TEST(ReadOnlyMemoryTest, GetHashCode_SameRegion_SameHash) {
    std::vector<int> v = {1, 2, 3};
    ReadOnlyMemory<int> a(v);
    ReadOnlyMemory<int> b(v);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(ReadOnlyMemoryTest, GetHashCode_DifferentRegion_LikelyDifferent) {
    std::vector<int> v1 = {1, 2, 3};
    std::vector<int> v2 = {1, 2, 3};
    ReadOnlyMemory<int> a(v1);
    ReadOnlyMemory<int> b(v2);
    EXPECT_NE(a.GetHashCode(), b.GetHashCode());
}

TEST(ReadOnlyMemoryTest, CopyTo_CopiesElements) {
    std::vector<int> src = {1, 2, 3};
    std::vector<int> dst(3, 0);
    ReadOnlyMemory<int> source(src);
    Memory<int> destination(dst);
    source.CopyTo(destination);
    EXPECT_EQ(dst[0], 1);
    EXPECT_EQ(dst[1], 2);
    EXPECT_EQ(dst[2], 3);
}

TEST(ReadOnlyMemoryTest, CopyTo_DestTooShort_Throws) {
    std::vector<int> src = {1, 2, 3};
    std::vector<int> dst(1, 0);
    ReadOnlyMemory<int> source(src);
    Memory<int> destination(dst);
    EXPECT_THROW(source.CopyTo(destination), System::ArgumentException);
}

TEST(ReadOnlyMemoryTest, TryCopyTo_Success_ReturnsTrue) {
    std::vector<int> src = {1, 2, 3};
    std::vector<int> dst(3, 0);
    ReadOnlyMemory<int> source(src);
    Memory<int> destination(dst);
    EXPECT_TRUE(source.TryCopyTo(destination));
    EXPECT_EQ(dst[1], 2);
}

TEST(ReadOnlyMemoryTest, TryCopyTo_DestTooShort_ReturnsFalse) {
    std::vector<int> src = {1, 2, 3};
    std::vector<int> dst(1, 0);
    ReadOnlyMemory<int> source(src);
    Memory<int> destination(dst);
    EXPECT_FALSE(source.TryCopyTo(destination));
}

TEST(ReadOnlyMemoryTest, Pin_PointerMatchesData) {
    std::vector<int> v = {10, 20, 30};
    ReadOnlyMemory<int> m(v);
    auto handle = m.Pin();
    EXPECT_EQ(handle.getPointerProperty(), static_cast<const void*>(v.data()));
}

TEST(ReadOnlyMemoryTest, ConvertFromArraySegment_Length) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    ArraySegment<int> seg(v, 1, 3);
    ReadOnlyMemory<int> m(seg);
    EXPECT_EQ(m.getLengthProperty(), 3);
    EXPECT_EQ(m[0], 2);
}
