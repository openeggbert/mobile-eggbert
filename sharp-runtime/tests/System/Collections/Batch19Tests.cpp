// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 19:
//   ReadOnlyObservableCollection: CollectionChanged forwarding, Empty, Count, Contains
//   ReadOnlySet: Empty, Count, IsEmpty, Contains, set operations
//   BitVector32: Equals, GetHashCode, ToString(value), Section equality/hash/ToString
#include <gtest/gtest.h>
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Collections/ObjectModel/ReadOnlyObservableCollection.hpp"
#include "System/Collections/ObjectModel/ReadOnlySet.hpp"
#include "System/Collections/Specialized/BitVector32.hpp"
#include <limits>
#include <memory>
#include <string>
#include <unordered_set>
#include <type_traits>

using System::Collections::ObjectModel::ObservableCollection;
using System::Collections::ObjectModel::ReadOnlyObservableCollection;
using System::Collections::ObjectModel::ReadOnlySet;
using System::Collections::Specialized::BitVector32;

// ===========================================================================
// ReadOnlyObservableCollection
// ===========================================================================

TEST(ReadOnlyObservableCollectionBatch19Test, CountAndIndexer) {
    auto src = std::make_shared<ObservableCollection<int>>();
    src->Add(10); src->Add(20); src->Add(30);
    ReadOnlyObservableCollection<int> roc(src);
    EXPECT_EQ(roc.getCountProperty(), 3);
    EXPECT_EQ(roc[1], 20);
}

TEST(ReadOnlyObservableCollectionBatch19Test, IsEmpty_True) {
    auto empty = std::make_shared<ObservableCollection<int>>();
    ReadOnlyObservableCollection<int> roc(empty);
    EXPECT_TRUE(roc.getIsEmptyProperty());
}

TEST(ReadOnlyObservableCollectionBatch19Test, IsEmpty_False) {
    auto src = std::make_shared<ObservableCollection<int>>();
    src->Add(1);
    ReadOnlyObservableCollection<int> roc(src);
    EXPECT_FALSE(roc.getIsEmptyProperty());
}

TEST(ReadOnlyObservableCollectionBatch19Test, ContainsAndIndexOf) {
    auto src = std::make_shared<ObservableCollection<std::string>>();
    src->Add("alpha"); src->Add("beta");
    ReadOnlyObservableCollection<std::string> roc(src);
    EXPECT_TRUE(roc.Contains("beta"));
    EXPECT_FALSE(roc.Contains("gamma"));
    EXPECT_EQ(roc.IndexOf("alpha"), 0);
    EXPECT_EQ(roc.IndexOf("missing"), -1);
}

TEST(ReadOnlyObservableCollectionBatch19Test, Empty_IsEmpty) {
    auto& e = ReadOnlyObservableCollection<int>::Empty();
    EXPECT_EQ(e.getCountProperty(), 0);
    EXPECT_TRUE(e.getIsEmptyProperty());
}

TEST(ReadOnlyObservableCollectionBatch19Test, STLIteration) {
    auto src = std::make_shared<ObservableCollection<int>>();
    src->Add(1); src->Add(2); src->Add(3);
    ReadOnlyObservableCollection<int> roc(src);
    int sum = 0;
    for (const auto& v : roc) sum += v;
    EXPECT_EQ(sum, 6);
}

TEST(ReadOnlyObservableCollectionBatch19Test, LiveView_ReflectsSourceMutation) {
    // The wrapper is a live view, not a snapshot: mutating the shared source afterwards
    // must be visible through the read-only wrapper.
    auto src = std::make_shared<ObservableCollection<int>>();
    src->Add(1);
    ReadOnlyObservableCollection<int> roc(src);
    EXPECT_EQ(roc.getCountProperty(), 1);
    src->Add(2);
    EXPECT_EQ(roc.getCountProperty(), 2);
    EXPECT_EQ(roc[1], 2);
}

TEST(ReadOnlyObservableCollectionBatch19Test, ForwardsCollectionChangedFromSource) {
    auto src = std::make_shared<ObservableCollection<int>>();
    ReadOnlyObservableCollection<int> roc(src);
    bool fired = false;
    roc.CollectionChanged.push_back([&](void*, const auto&) { fired = true; });
    src->Add(42);
    EXPECT_TRUE(fired);
}

// The constructor registers a forwarding callback with the shared source that captures `this`.
// Copying or moving the wrapper would leave that callback pointing at the wrong (or, once the
// original is destroyed, dangling) object while the shared source keeps invoking it -- confirmed
// via an ASan repro (stack-use-after-scope) before this class was made non-copyable/non-movable.
// This static_assert is the regression guard: it fails to compile if copy/move is ever
// re-enabled without an accompanying re-registration fix.
static_assert(!std::is_copy_constructible_v<ReadOnlyObservableCollection<int>>);
static_assert(!std::is_copy_assignable_v<ReadOnlyObservableCollection<int>>);
static_assert(!std::is_move_constructible_v<ReadOnlyObservableCollection<int>>);
static_assert(!std::is_move_assignable_v<ReadOnlyObservableCollection<int>>);

// ===========================================================================
// ReadOnlySet
// ===========================================================================

TEST(ReadOnlySetBatch19Test, Ctor_NullSet_ThrowsArgumentNullException) {
    EXPECT_THROW(ReadOnlySet<int>(nullptr), System::ArgumentNullException);
}

TEST(ReadOnlySetBatch19Test, CountAndContains) {
    auto s = std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{1, 2, 3});
    ReadOnlySet<int> rs(s);
    EXPECT_EQ(rs.getCountProperty(), 3);
    EXPECT_TRUE(rs.Contains(2));
    EXPECT_FALSE(rs.Contains(99));
}

TEST(ReadOnlySetBatch19Test, IsEmpty) {
    auto empty = std::make_shared<std::unordered_set<int>>();
    ReadOnlySet<int> rs(empty);
    EXPECT_TRUE(rs.getIsEmptyProperty());
    auto s = std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{1});
    ReadOnlySet<int> rs2(s);
    EXPECT_FALSE(rs2.getIsEmptyProperty());
}

TEST(ReadOnlySetBatch19Test, EmptyStatic) {
    auto e = ReadOnlySet<int>::Empty();
    EXPECT_EQ(e.getCountProperty(), 0);
    EXPECT_TRUE(e.getIsEmptyProperty());
}

TEST(ReadOnlySetBatch19Test, IsSubsetOf) {
    auto a = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{1, 2}));
    auto b = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{1, 2, 3}));
    EXPECT_TRUE(a.IsSubsetOf(b));
    EXPECT_FALSE(b.IsSubsetOf(a));
}

TEST(ReadOnlySetBatch19Test, IsSupersetOf) {
    auto sup = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{1, 2, 3}));
    auto sub = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{2, 3}));
    EXPECT_TRUE(sup.IsSupersetOf(sub));
    EXPECT_FALSE(sub.IsSupersetOf(sup));
}

TEST(ReadOnlySetBatch19Test, IsProperSubsetOf) {
    auto sub = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{1, 2}));
    auto sup = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{1, 2, 3}));
    auto eq  = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{1, 2}));
    EXPECT_TRUE(sub.IsProperSubsetOf(sup));
    EXPECT_FALSE(sub.IsProperSubsetOf(eq));
}

TEST(ReadOnlySetBatch19Test, IsProperSupersetOf) {
    auto sup = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{1, 2, 3}));
    auto sub = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{2, 3}));
    auto eq  = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{1, 2, 3}));
    EXPECT_TRUE(sup.IsProperSupersetOf(sub));
    EXPECT_FALSE(sup.IsProperSupersetOf(eq));
}

TEST(ReadOnlySetBatch19Test, Overlaps) {
    auto a = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{1, 2}));
    auto b = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{2, 3}));
    auto c = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{4, 5}));
    EXPECT_TRUE(a.Overlaps(b));
    EXPECT_FALSE(a.Overlaps(c));
}

TEST(ReadOnlySetBatch19Test, SetEquals) {
    auto a = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{1, 2, 3}));
    auto b = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{3, 1, 2}));
    auto c = ReadOnlySet<int>(std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{1, 2}));
    EXPECT_TRUE(a.SetEquals(b));
    EXPECT_FALSE(a.SetEquals(c));
}

TEST(ReadOnlySetBatch19Test, STLIteration) {
    auto s = std::make_shared<std::unordered_set<int>>(std::initializer_list<int>{10, 20, 30});
    ReadOnlySet<int> rs(s);
    int sum = 0;
    for (const auto& v : rs) sum += v;
    EXPECT_EQ(sum, 60);
}

// ===========================================================================
// BitVector32
// ===========================================================================

TEST(BitVector32Batch19Test, Equals_SameData) {
    BitVector32 a(42), b(42);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_TRUE(a == b);
}

TEST(BitVector32Batch19Test, Equals_DiffData) {
    BitVector32 a(1), b(2);
    EXPECT_FALSE(a.Equals(b));
    EXPECT_TRUE(a != b);
}

TEST(BitVector32Batch19Test, GetHashCode_MatchesData) {
    BitVector32 bv(0xDEAD);
    EXPECT_EQ(bv.GetHashCode(), static_cast<int>(0xDEAD));
}

TEST(BitVector32Batch19Test, ToStringStatic) {
    BitVector32 bv(0);
    std::string s = BitVector32::ToString(bv);
    EXPECT_EQ(s, bv.ToString());
    EXPECT_EQ(s.substr(0, 10), "BitVector3");
}

TEST(BitVector32Batch19Test, Section_Equals) {
    // Section's constructor is private (matches real .NET's `internal` ctor) -- build via
    // CreateSection chaining instead: base has mask=0x0F/offset=0, so a Section chained after
    // it lands at mask=0x0F/offset=4 (0x0F has 4 set bits), and chained twice at offset=8.
    auto base = BitVector32::CreateSection(15);
    auto s1 = BitVector32::CreateSection(15, base);
    auto s2 = BitVector32::CreateSection(15, base);
    auto s3 = BitVector32::CreateSection(15, s1);
    EXPECT_TRUE(s1.Equals(s2));
    EXPECT_TRUE(s1 == s2);
    EXPECT_FALSE(s1.Equals(s3));
    EXPECT_TRUE(s1 != s3);
}

TEST(BitVector32Batch19Test, Section_GetHashCode_Stable) {
    auto base = BitVector32::CreateSection(15);
    auto s = BitVector32::CreateSection(15, base);
    EXPECT_EQ(s.GetHashCode(), s.GetHashCode());
    auto s2 = BitVector32::CreateSection(15, base);
    EXPECT_EQ(s.GetHashCode(), s2.GetHashCode());
}

TEST(BitVector32Batch19Test, Section_ToString) {
    // mask=3 (0b11, 2 set bits) chained once lands at offset=2, reproducing Section(3, 2).
    auto base = BitVector32::CreateSection(3);
    auto s = BitVector32::CreateSection(3, base);
    std::string t = s.ToString();
    EXPECT_NE(t.find("3"), std::string::npos);
    EXPECT_NE(t.find("2"), std::string::npos);
    EXPECT_EQ(BitVector32::Section::ToString(s), t);
}

TEST(BitVector32Batch19Test, Section_ToString_MatchesDotNetHexFormat) {
    // Real .NET: $"Section{{0x{Mask:x}, 0x{Offset:x}}}" -- lowercase hex, no leading zeros, no
    // field-name labels. An earlier version of this port used decimal "mask=N, offset=N" labels,
    // a real format mismatch (wrong number base, not just paraphrased text).
    auto base = BitVector32::CreateSection(15);       // mask=15 (0xf), offset=0
    auto s = BitVector32::CreateSection(15, base);     // mask=15 (0xf), offset=4
    EXPECT_EQ(s.ToString(), "Section{0xf, 0x4}");
    EXPECT_EQ(BitVector32::Section::ToString(s), "Section{0xf, 0x4}");

    auto zero = BitVector32::CreateSection(1);         // mask=1 (0x1), offset=0
    EXPECT_EQ(zero.ToString(), "Section{0x1, 0x0}");
}

TEST(BitVector32Batch19Test, CreateMaskSequence) {
    int m1 = BitVector32::CreateMask();
    int m2 = BitVector32::CreateMask(m1);
    int m3 = BitVector32::CreateMask(m2);
    EXPECT_EQ(m1, 1);
    EXPECT_EQ(m2, 2);
    EXPECT_EQ(m3, 4);
}

TEST(BitVector32Batch19Test, BitFlags) {
    BitVector32 bv;
    int m1 = BitVector32::CreateMask();
    int m2 = BitVector32::CreateMask(m1);
    bv.set(m1, true);
    EXPECT_TRUE(bv[m1]);
    EXPECT_FALSE(bv[m2]);
    bv.set(m1, false);
    EXPECT_FALSE(bv[m1]);
}

TEST(BitVector32Batch19Test, SectionGetSet) {
    BitVector32 bv;
    auto s1 = BitVector32::CreateSection(7);
    auto s2 = BitVector32::CreateSection(15, s1);
    bv.set(s1, 5);
    bv.set(s2, 12);
    EXPECT_EQ(bv[s1], 5);
    EXPECT_EQ(bv[s2], 12);
}

TEST(BitVector32Batch19Test, CreateMask_LastMask_ThrowsInvalidOperationException) {
    EXPECT_THROW(BitVector32::CreateMask(std::numeric_limits<SharpRuntime::intcs>::min()),
                 System::InvalidOperationException);
}

TEST(BitVector32Batch19Test, CreateMask_Series_ReachesIntMinThenThrows) {
    SharpRuntime::intcs mask = 0;
    for (int i = 0; i < 32; ++i) mask = BitVector32::CreateMask(mask);
    EXPECT_EQ(mask, std::numeric_limits<SharpRuntime::intcs>::min());
    EXPECT_THROW(BitVector32::CreateMask(mask), System::InvalidOperationException);
}

TEST(BitVector32Batch19Test, CreateSection_NonPositiveMaxValue_ThrowsArgumentOutOfRangeException) {
    EXPECT_THROW(BitVector32::CreateSection(0), System::ArgumentOutOfRangeException);
    EXPECT_THROW(BitVector32::CreateSection(-1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(BitVector32::CreateSection(std::numeric_limits<SharpRuntime::shortcs>::min()),
                 System::ArgumentOutOfRangeException);

    auto valid = BitVector32::CreateSection(1);
    EXPECT_THROW(BitVector32::CreateSection(0, valid), System::ArgumentOutOfRangeException);
}

TEST(BitVector32Batch19Test, CreateSection_Full_ThrowsInvalidOperationException) {
    auto initial = BitVector32::CreateSection(std::numeric_limits<SharpRuntime::shortcs>::max(),
                                               BitVector32::CreateSection(std::numeric_limits<SharpRuntime::shortcs>::max()));
    auto overflow = BitVector32::CreateSection(7, initial);
    EXPECT_THROW(BitVector32::CreateSection(std::numeric_limits<SharpRuntime::shortcs>::max(), overflow),
                 System::InvalidOperationException);
}
