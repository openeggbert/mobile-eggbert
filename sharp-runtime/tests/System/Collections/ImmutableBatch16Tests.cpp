// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for methods added in Batch 16:
//   ImmutableArray:     Replace, LastIndexOf, InsertRange, RemoveAll
//   ImmutableList:      Replace, InsertRange, RemoveAll, RemoveRange, LastIndexOf, BinarySearch
//   ImmutableDictionary: AddRange, SetItems, RemoveRange, Contains(pair), ContainsValue
//   ImmutableHashSet:   SymmetricExcept, SetEquals, IsProperSubsetOf, IsProperSupersetOf,
//                       IsSupersetOf, Overlaps, Create(vector)
#include <gtest/gtest.h>
#include "System/Collections/Immutable/ImmutableArray.hpp"
#include "System/Collections/Immutable/ImmutableDictionary.hpp"
#include "System/Collections/Immutable/ImmutableHashSet.hpp"
#include "System/Collections/Immutable/ImmutableList.hpp"
#include "System/ArgumentException.hpp"
#include <string>
#include <utility>
#include <vector>

using System::Collections::Immutable::ImmutableArray;
using System::Collections::Immutable::ImmutableDictionary;
using System::Collections::Immutable::ImmutableHashSet;
using System::Collections::Immutable::ImmutableList;

// ===========================================================================
// ImmutableArray — new methods
// ===========================================================================

TEST(ImmArrayBatch16Test, Replace_ReplacesFirstOccurrence) {
    auto a = ImmutableArray<int>::Create({1, 2, 3, 2});
    auto b = a.Replace(2, 99);
    EXPECT_EQ(b[1], 99);
    EXPECT_EQ(b[3], 2); // second occurrence unchanged
    EXPECT_EQ(a[1], 2); // original unchanged
}

TEST(ImmArrayBatch16Test, Replace_NotFound_ReturnsCopy) {
    auto a = ImmutableArray<int>::Create({1, 2, 3});
    auto b = a.Replace(99, 0);
    EXPECT_EQ(b.getLengthProperty(), 3);
    EXPECT_EQ(b[0], 1);
}

TEST(ImmArrayBatch16Test, LastIndexOf_Found) {
    auto a = ImmutableArray<int>::Create({1, 2, 3, 2, 1});
    EXPECT_EQ(a.LastIndexOf(2), 3);
    EXPECT_EQ(a.LastIndexOf(1), 4);
}

TEST(ImmArrayBatch16Test, LastIndexOf_NotFound) {
    auto a = ImmutableArray<int>::Create({1, 2, 3});
    EXPECT_EQ(a.LastIndexOf(99), -1);
}

TEST(ImmArrayBatch16Test, InsertRange_InsertsAtIndex) {
    auto a = ImmutableArray<int>::Create({1, 4});
    auto b = a.InsertRange(1, {2, 3});
    EXPECT_EQ(b.getLengthProperty(), 4);
    EXPECT_EQ(b[0], 1);
    EXPECT_EQ(b[1], 2);
    EXPECT_EQ(b[2], 3);
    EXPECT_EQ(b[3], 4);
}

TEST(ImmArrayBatch16Test, RemoveAll_RemovesMatching) {
    auto a = ImmutableArray<int>::Create({1, 2, 3, 4, 5});
    auto b = a.RemoveAll([](const int& x) { return x % 2 == 0; });
    EXPECT_EQ(b.getLengthProperty(), 3);
    EXPECT_EQ(b[0], 1);
    EXPECT_EQ(b[1], 3);
    EXPECT_EQ(b[2], 5);
}

TEST(ImmArrayBatch16Test, RemoveAll_OriginalUnchanged) {
    auto a = ImmutableArray<int>::Create({1, 2, 3});
    auto b = a.RemoveAll([](const int& x) { return x > 1; });
    EXPECT_EQ(a.getLengthProperty(), 3);
    EXPECT_EQ(b.getLengthProperty(), 1);
}

// ===========================================================================
// ImmutableList — new methods
// ===========================================================================

TEST(ImmListBatch16Test, Replace_ReplacesFirstOccurrence) {
    auto lst = ImmutableList<int>::Create({1, 2, 3, 2});
    auto lst2 = lst.Replace(2, 99);
    EXPECT_EQ(lst2[1], 99);
    EXPECT_EQ(lst2[3], 2);
    EXPECT_EQ(lst[1], 2);
}

TEST(ImmListBatch16Test, InsertRange_InsertsAtIndex) {
    auto lst = ImmutableList<int>::Create({1, 4});
    auto lst2 = lst.InsertRange(1, {2, 3});
    EXPECT_EQ(lst2.getCountProperty(), 4);
    EXPECT_EQ(lst2[1], 2);
    EXPECT_EQ(lst2[2], 3);
}

TEST(ImmListBatch16Test, RemoveAll_RemovesMatching) {
    auto lst = ImmutableList<int>::Create({1, 2, 3, 4});
    auto lst2 = lst.RemoveAll([](const int& x) { return x % 2 == 0; });
    EXPECT_EQ(lst2.getCountProperty(), 2);
    EXPECT_EQ(lst2[0], 1);
    EXPECT_EQ(lst2[1], 3);
}

TEST(ImmListBatch16Test, RemoveRange_RemovesSlice) {
    auto lst = ImmutableList<int>::Create({1, 2, 3, 4, 5});
    auto lst2 = lst.RemoveRange(1, 3);
    EXPECT_EQ(lst2.getCountProperty(), 2);
    EXPECT_EQ(lst2[0], 1);
    EXPECT_EQ(lst2[1], 5);
}

TEST(ImmListBatch16Test, RemoveRange_CountExceedsBounds_ThrowsArgumentOutOfRangeException) {
    // Real .NET's ImmutableList<T>.RemoveRange(int, int) throws ArgumentOutOfRangeException
    // for an out-of-bounds range (via Requires.Range) -- this previously threw the wrong
    // type, System::ArgumentException, for this specific case.
    auto lst = ImmutableList<int>::Create({1, 2, 3});
    EXPECT_THROW(lst.RemoveRange(1, 10), System::ArgumentOutOfRangeException);
}

TEST(ImmListBatch16Test, RemoveRange_NegativeIndex_ThrowsArgumentOutOfRangeException) {
    auto lst = ImmutableList<int>::Create({1, 2, 3});
    EXPECT_THROW(lst.RemoveRange(-1, 1), System::ArgumentOutOfRangeException);
}

TEST(ImmListBatch16Test, RemoveRange_IndexAtSizeZeroCount_Succeeds) {
    auto lst = ImmutableList<int>::Create({1, 2, 3});
    auto lst2 = lst.RemoveRange(3, 0);
    EXPECT_EQ(lst2.getCountProperty(), 3);
}

TEST(ImmListBatch16Test, LastIndexOf_Found) {
    auto lst = ImmutableList<int>::Create({1, 2, 3, 2});
    EXPECT_EQ(lst.LastIndexOf(2), 3);
}

TEST(ImmListBatch16Test, LastIndexOf_NotFound) {
    auto lst = ImmutableList<int>::Create({1, 2, 3});
    EXPECT_EQ(lst.LastIndexOf(99), -1);
}

TEST(ImmListBatch16Test, BinarySearch_Found) {
    auto lst = ImmutableList<int>::Create({1, 3, 5, 7, 9});
    EXPECT_EQ(lst.BinarySearch(5), 2);
    EXPECT_EQ(lst.BinarySearch(1), 0);
    EXPECT_EQ(lst.BinarySearch(9), 4);
}

TEST(ImmListBatch16Test, BinarySearch_NotFound_NegativeResult) {
    auto lst = ImmutableList<int>::Create({1, 3, 5, 7, 9});
    EXPECT_LT(lst.BinarySearch(4), 0); // not found → negative
}

// ===========================================================================
// ImmutableDictionary — new methods
// ===========================================================================

TEST(ImmDictBatch16Test, AddRange_AddsMultiple) {
    auto d = ImmutableDictionary<std::string, int>::Empty();
    std::vector<std::pair<std::string, int>> pairs = {{"a", 1}, {"b", 2}};
    auto d2 = d.AddRange(pairs);
    EXPECT_EQ(d2.getCountProperty(), 2);
    EXPECT_EQ(d2["a"], 1);
    EXPECT_EQ(d2["b"], 2);
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(ImmDictBatch16Test, AddRange_DuplicateKeyThrows) {
    auto d = ImmutableDictionary<std::string, int>::Empty().Add("k", 1);
    std::vector<std::pair<std::string, int>> pairs = {{"k", 2}};
    EXPECT_THROW(d.AddRange(pairs), System::ArgumentException);
}

TEST(ImmDictBatch16Test, SetItems_InsertsAndOverwrites) {
    auto d = ImmutableDictionary<std::string, int>::Empty().Add("a", 1);
    std::vector<std::pair<std::string, int>> items = {{"a", 99}, {"b", 2}};
    auto d2 = d.SetItems(items);
    EXPECT_EQ(d2["a"], 99);
    EXPECT_EQ(d2["b"], 2);
    EXPECT_EQ(d["a"], 1); // original unchanged
}

TEST(ImmDictBatch16Test, RemoveRange_RemovesMultiple) {
    auto d = ImmutableDictionary<std::string, int>::Empty()
                 .Add("a", 1).Add("b", 2).Add("c", 3);
    auto d2 = d.RemoveRange({"a", "c"});
    EXPECT_EQ(d2.getCountProperty(), 1);
    EXPECT_TRUE(d2.ContainsKey("b"));
    EXPECT_FALSE(d2.ContainsKey("a"));
}

TEST(ImmDictBatch16Test, Contains_Pair_TrueAndFalse) {
    auto d = ImmutableDictionary<std::string, int>::Empty().Add("x", 42);
    EXPECT_TRUE(d.Contains({"x", 42}));
    EXPECT_FALSE(d.Contains({"x", 0}));
    EXPECT_FALSE(d.Contains({"y", 42}));
}

TEST(ImmDictBatch16Test, ContainsValue_TrueAndFalse) {
    auto d = ImmutableDictionary<std::string, int>::Empty().Add("k", 77);
    EXPECT_TRUE(d.ContainsValue(77));
    EXPECT_FALSE(d.ContainsValue(0));
}

// ===========================================================================
// ImmutableHashSet — new methods
// ===========================================================================

TEST(ImmHashSetBatch16Test, CreateFromVector) {
    std::vector<int> v = {1, 2, 3, 2};
    auto s = ImmutableHashSet<int>::Create(v);
    EXPECT_EQ(s.getCountProperty(), 3); // duplicate removed
}

TEST(ImmHashSetBatch16Test, SymmetricExcept_OnlyInOne) {
    auto a = ImmutableHashSet<int>::Create({1, 2, 3});
    auto b = ImmutableHashSet<int>::Create({2, 3, 4});
    auto c = a.SymmetricExcept(b);
    EXPECT_EQ(c.getCountProperty(), 2);
    EXPECT_TRUE(c.Contains(1));
    EXPECT_TRUE(c.Contains(4));
    EXPECT_FALSE(c.Contains(2));
    EXPECT_FALSE(c.Contains(3));
}

TEST(ImmHashSetBatch16Test, SetEquals_TrueAndFalse) {
    auto a = ImmutableHashSet<int>::Create({1, 2, 3});
    auto b = ImmutableHashSet<int>::Create({3, 1, 2});
    auto c = ImmutableHashSet<int>::Create({1, 2});
    EXPECT_TRUE(a.SetEquals(b));
    EXPECT_FALSE(a.SetEquals(c));
}

TEST(ImmHashSetBatch16Test, IsProperSubsetOf) {
    auto sub = ImmutableHashSet<int>::Create({2, 3});
    auto sup = ImmutableHashSet<int>::Create({1, 2, 3});
    auto eq  = ImmutableHashSet<int>::Create({2, 3});
    EXPECT_TRUE(sub.IsProperSubsetOf(sup));
    EXPECT_FALSE(sub.IsProperSubsetOf(eq)); // equal, not proper
}

TEST(ImmHashSetBatch16Test, IsProperSupersetOf) {
    auto sup = ImmutableHashSet<int>::Create({1, 2, 3});
    auto sub = ImmutableHashSet<int>::Create({2, 3});
    auto eq  = ImmutableHashSet<int>::Create({1, 2, 3});
    EXPECT_TRUE(sup.IsProperSupersetOf(sub));
    EXPECT_FALSE(sup.IsProperSupersetOf(eq)); // equal, not proper
}

TEST(ImmHashSetBatch16Test, IsSupersetOf) {
    auto sup = ImmutableHashSet<int>::Create({1, 2, 3});
    auto sub = ImmutableHashSet<int>::Create({2, 3});
    EXPECT_TRUE(sup.IsSupersetOf(sub));
    EXPECT_FALSE(sub.IsSupersetOf(sup));
}

TEST(ImmHashSetBatch16Test, Overlaps_TrueAndFalse) {
    auto a = ImmutableHashSet<int>::Create({1, 2});
    auto b = ImmutableHashSet<int>::Create({2, 3});
    auto c = ImmutableHashSet<int>::Create({4, 5});
    EXPECT_TRUE(a.Overlaps(b));
    EXPECT_FALSE(a.Overlaps(c));
}
