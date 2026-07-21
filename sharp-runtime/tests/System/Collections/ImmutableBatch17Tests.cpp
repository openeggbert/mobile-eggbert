// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for methods added in Batch 17:
//   ImmutableSortedDictionary: AddRange, SetItems, RemoveRange, Contains(pair), ContainsValue
//   ImmutableSortedSet:        SymmetricExcept, SetEquals, IsProperSubset/Superset,
//                              IsSupersetOf, Overlaps, TryGetValue, Create(vector)
//   Collection:                CopyTo
#include <gtest/gtest.h>
#include "System/Collections/Immutable/ImmutableSortedDictionary.hpp"
#include "System/Collections/Immutable/ImmutableSortedSet.hpp"
#include "System/Collections/ObjectModel/Collection.hpp"
#include "System/ArgumentException.hpp"
#include <string>
#include <utility>
#include <vector>

using System::Collections::Immutable::ImmutableSortedDictionary;
using System::Collections::Immutable::ImmutableSortedSet;
using System::Collections::ObjectModel::Collection;

// ===========================================================================
// ImmutableSortedDictionary — new methods
// ===========================================================================

TEST(ImmSortedDictBatch17Test, AddRange_AddsMultiple) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty();
    std::vector<std::pair<std::string, int>> pairs = {{"b", 2}, {"a", 1}};
    auto d2 = d.AddRange(pairs);
    EXPECT_EQ(d2.getCountProperty(), 2);
    EXPECT_EQ(d2["a"], 1);
    EXPECT_EQ(d2["b"], 2);
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(ImmSortedDictBatch17Test, AddRange_DuplicateKeyThrows) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty().Add("k", 1);
    std::vector<std::pair<std::string, int>> pairs = {{"k", 2}};
    EXPECT_THROW(d.AddRange(pairs), System::ArgumentException);
}

TEST(ImmSortedDictBatch17Test, SetItems_InsertsAndOverwrites) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty().Add("a", 1);
    std::vector<std::pair<std::string, int>> items = {{"a", 99}, {"b", 2}};
    auto d2 = d.SetItems(items);
    EXPECT_EQ(d2["a"], 99);
    EXPECT_EQ(d2["b"], 2);
    EXPECT_EQ(d["a"], 1); // original unchanged
}

TEST(ImmSortedDictBatch17Test, RemoveRange_RemovesMultiple) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty()
                 .Add("a", 1).Add("b", 2).Add("c", 3);
    auto d2 = d.RemoveRange({"a", "c"});
    EXPECT_EQ(d2.getCountProperty(), 1);
    EXPECT_TRUE(d2.ContainsKey("b"));
    EXPECT_FALSE(d2.ContainsKey("a"));
    EXPECT_EQ(d.getCountProperty(), 3); // original unchanged
}

TEST(ImmSortedDictBatch17Test, Contains_Pair_TrueAndFalse) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty().Add("x", 42);
    EXPECT_TRUE(d.Contains({"x", 42}));
    EXPECT_FALSE(d.Contains({"x", 0}));
    EXPECT_FALSE(d.Contains({"y", 42}));
}

TEST(ImmSortedDictBatch17Test, ContainsValue_TrueAndFalse) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty().Add("k", 77);
    EXPECT_TRUE(d.ContainsValue(77));
    EXPECT_FALSE(d.ContainsValue(0));
}

TEST(ImmSortedDictBatch17Test, KeysAreSorted) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty()
                 .Add("c", 3).Add("a", 1).Add("b", 2);
    auto keys = d.getKeysProperty();
    ASSERT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], "a");
    EXPECT_EQ(keys[2], "c");
}

// ===========================================================================
// ImmutableSortedSet — new methods
// ===========================================================================

TEST(ImmSortedSetBatch17Test, CreateFromVector) {
    std::vector<int> v = {3, 1, 2, 1};
    auto s = ImmutableSortedSet<int>::Create(v);
    EXPECT_EQ(s.getCountProperty(), 3); // duplicates removed
    EXPECT_TRUE(s.Contains(1));
    EXPECT_TRUE(s.Contains(2));
    EXPECT_TRUE(s.Contains(3));
}

TEST(ImmSortedSetBatch17Test, TryGetValue_FoundAndNotFound) {
    auto s = ImmutableSortedSet<int>::Create({10, 20, 30});
    int val = 0;
    EXPECT_TRUE(s.TryGetValue(20, val));
    EXPECT_EQ(val, 20);
    EXPECT_FALSE(s.TryGetValue(99, val));
}

TEST(ImmSortedSetBatch17Test, SymmetricExcept_OnlyInOne) {
    auto a = ImmutableSortedSet<int>::Create({1, 2, 3});
    auto b = ImmutableSortedSet<int>::Create({2, 3, 4});
    auto c = a.SymmetricExcept(b);
    EXPECT_EQ(c.getCountProperty(), 2);
    EXPECT_TRUE(c.Contains(1));
    EXPECT_TRUE(c.Contains(4));
    EXPECT_FALSE(c.Contains(2));
}

TEST(ImmSortedSetBatch17Test, SetEquals_TrueAndFalse) {
    auto a = ImmutableSortedSet<int>::Create({1, 2, 3});
    auto b = ImmutableSortedSet<int>::Create({3, 1, 2});
    auto c = ImmutableSortedSet<int>::Create({1, 2});
    EXPECT_TRUE(a.SetEquals(b));
    EXPECT_FALSE(a.SetEquals(c));
}

TEST(ImmSortedSetBatch17Test, IsProperSubsetOf) {
    auto sub = ImmutableSortedSet<int>::Create({2, 3});
    auto sup = ImmutableSortedSet<int>::Create({1, 2, 3});
    auto eq  = ImmutableSortedSet<int>::Create({2, 3});
    EXPECT_TRUE(sub.IsProperSubsetOf(sup));
    EXPECT_FALSE(sub.IsProperSubsetOf(eq));
}

TEST(ImmSortedSetBatch17Test, IsProperSupersetOf) {
    auto sup = ImmutableSortedSet<int>::Create({1, 2, 3});
    auto sub = ImmutableSortedSet<int>::Create({2, 3});
    auto eq  = ImmutableSortedSet<int>::Create({1, 2, 3});
    EXPECT_TRUE(sup.IsProperSupersetOf(sub));
    EXPECT_FALSE(sup.IsProperSupersetOf(eq));
}

TEST(ImmSortedSetBatch17Test, IsSupersetOf) {
    auto sup = ImmutableSortedSet<int>::Create({1, 2, 3});
    auto sub = ImmutableSortedSet<int>::Create({2, 3});
    EXPECT_TRUE(sup.IsSupersetOf(sub));
    EXPECT_FALSE(sub.IsSupersetOf(sup));
}

TEST(ImmSortedSetBatch17Test, Overlaps_TrueAndFalse) {
    auto a = ImmutableSortedSet<int>::Create({1, 2});
    auto b = ImmutableSortedSet<int>::Create({2, 3});
    auto c = ImmutableSortedSet<int>::Create({4, 5});
    EXPECT_TRUE(a.Overlaps(b));
    EXPECT_FALSE(a.Overlaps(c));
}

TEST(ImmSortedSetBatch17Test, OriginalUnchangedAfterSymmetricExcept) {
    auto a = ImmutableSortedSet<int>::Create({1, 2, 3});
    auto b = ImmutableSortedSet<int>::Create({2, 3, 4});
    auto c = a.SymmetricExcept(b);
    EXPECT_EQ(a.getCountProperty(), 3);
    EXPECT_EQ(b.getCountProperty(), 3);
    EXPECT_EQ(c.getCountProperty(), 2);
}

// ===========================================================================
// Collection — CopyTo
// ===========================================================================

TEST(CollectionBatch17Test, CopyTo_CopiesToVector) {
    Collection<int> col;
    col.Add(10); col.Add(20); col.Add(30);
    std::vector<int> dest(5, 0);
    col.CopyTo(dest, 1);
    EXPECT_EQ(dest[0], 0);
    EXPECT_EQ(dest[1], 10);
    EXPECT_EQ(dest[2], 20);
    EXPECT_EQ(dest[3], 30);
    EXPECT_EQ(dest[4], 0);
}

TEST(CollectionBatch17Test, CopyTo_OutOfRangeThrows) {
    Collection<int> col;
    col.Add(1); col.Add(2);
    std::vector<int> dest(1, 0); // too small
    EXPECT_THROW(col.CopyTo(dest, 0), System::ArgumentException);
}

TEST(CollectionBatch17Test, AddAndIndexer) {
    Collection<int> col;
    col.Add(5); col.Add(10);
    EXPECT_EQ(col.getCountProperty(), 2);
    EXPECT_EQ(col[0], 5);
    EXPECT_EQ(col[1], 10);
}

TEST(CollectionBatch17Test, InsertAndRemoveAt) {
    Collection<int> col;
    col.Add(1); col.Add(3);
    col.Insert(1, 2);
    EXPECT_EQ(col[1], 2);
    col.RemoveAt(0);
    EXPECT_EQ(col[0], 2);
    EXPECT_EQ(col.getCountProperty(), 2);
}

TEST(CollectionBatch17Test, IndexOf_ContainsRemove) {
    Collection<std::string> col;
    col.Add("a"); col.Add("b"); col.Add("c");
    EXPECT_EQ(col.IndexOf("b"), 1);
    EXPECT_TRUE(col.Contains("c"));
    EXPECT_TRUE(col.Remove("b"));
    EXPECT_EQ(col.getCountProperty(), 2);
    EXPECT_EQ(col.IndexOf("b"), -1);
}

TEST(CollectionBatch17Test, ClearAndEmpty) {
    Collection<int> col;
    col.Add(1); col.Add(2);
    col.Clear();
    EXPECT_EQ(col.getCountProperty(), 0);
}
