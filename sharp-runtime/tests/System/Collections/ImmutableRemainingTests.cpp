// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for remaining Immutable collections:
// ImmutableHashSet, ImmutableQueue, ImmutableSortedDictionary,
// ImmutableSortedSet, ImmutableStack.
#include <gtest/gtest.h>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include "System/Collections/Immutable/ImmutableHashSet.hpp"
#include "System/Collections/Immutable/ImmutableQueue.hpp"
#include "System/Collections/Immutable/ImmutableSortedDictionary.hpp"
#include "System/Collections/Immutable/ImmutableSortedSet.hpp"
#include "System/Collections/Immutable/ImmutableStack.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ArgumentException.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"

using System::Collections::Immutable::ImmutableHashSet;
using System::Collections::Immutable::ImmutableQueue;
using System::Collections::Immutable::ImmutableSortedDictionary;
using System::Collections::Immutable::ImmutableSortedSet;
using System::Collections::Immutable::ImmutableStack;

// ===========================================================================
// ImmutableHashSet<T>
// ===========================================================================

TEST(ImmutableHashSetTests, Empty_IsEmptyAndCountZero) {
    auto s = ImmutableHashSet<int>::Empty();
    EXPECT_TRUE(s.getIsEmptyProperty());
    EXPECT_EQ(s.getCountProperty(), 0);
}

TEST(ImmutableHashSetTests, Create_WithItems_CountMatches) {
    auto s = ImmutableHashSet<int>::Create({1, 2, 3});
    EXPECT_EQ(s.getCountProperty(), 3);
    EXPECT_FALSE(s.getIsEmptyProperty());
}

TEST(ImmutableHashSetTests, Contains_True_False) {
    auto s = ImmutableHashSet<int>::Create({10, 20, 30});
    EXPECT_TRUE(s.Contains(20));
    EXPECT_FALSE(s.Contains(99));
}

TEST(ImmutableHashSetTests, Add_ReturnsNewSetWithItem) {
    auto s = ImmutableHashSet<int>::Empty();
    auto s2 = s.Add(42);
    EXPECT_EQ(s.getCountProperty(), 0);
    EXPECT_EQ(s2.getCountProperty(), 1);
    EXPECT_TRUE(s2.Contains(42));
}

TEST(ImmutableHashSetTests, Add_DuplicateNotIncreased) {
    auto s = ImmutableHashSet<int>::Create({5});
    auto s2 = s.Add(5);
    EXPECT_EQ(s2.getCountProperty(), 1);
}

TEST(ImmutableHashSetTests, Remove_ReturnsNewSetWithoutItem) {
    auto s = ImmutableHashSet<int>::Create({1, 2, 3});
    auto s2 = s.Remove(2);
    EXPECT_EQ(s.getCountProperty(), 3);
    EXPECT_EQ(s2.getCountProperty(), 2);
    EXPECT_FALSE(s2.Contains(2));
}

TEST(ImmutableHashSetTests, Remove_NonExistent_SameCount) {
    auto s = ImmutableHashSet<int>::Create({1, 2});
    auto s2 = s.Remove(99);
    EXPECT_EQ(s2.getCountProperty(), 2);
}

TEST(ImmutableHashSetTests, Clear_ReturnsEmpty) {
    auto s = ImmutableHashSet<int>::Create({1, 2, 3});
    auto empty = s.Clear();
    EXPECT_EQ(empty.getCountProperty(), 0);
    EXPECT_TRUE(empty.getIsEmptyProperty());
}

TEST(ImmutableHashSetTests, Union_CombinesBothSets) {
    auto a = ImmutableHashSet<int>::Create({1, 2});
    auto b = ImmutableHashSet<int>::Create({2, 3});
    auto u = a.Union(b);
    EXPECT_EQ(u.getCountProperty(), 3);
    EXPECT_TRUE(u.Contains(1));
    EXPECT_TRUE(u.Contains(2));
    EXPECT_TRUE(u.Contains(3));
}

TEST(ImmutableHashSetTests, Intersect_OnlyCommonItems) {
    auto a = ImmutableHashSet<int>::Create({1, 2, 3});
    auto b = ImmutableHashSet<int>::Create({2, 3, 4});
    auto inter = a.Intersect(b);
    EXPECT_EQ(inter.getCountProperty(), 2);
    EXPECT_TRUE(inter.Contains(2));
    EXPECT_TRUE(inter.Contains(3));
    EXPECT_FALSE(inter.Contains(1));
}

TEST(ImmutableHashSetTests, Except_RemovesOtherItems) {
    auto a = ImmutableHashSet<int>::Create({1, 2, 3});
    auto b = ImmutableHashSet<int>::Create({2, 3});
    auto ex = a.Except(b);
    EXPECT_EQ(ex.getCountProperty(), 1);
    EXPECT_TRUE(ex.Contains(1));
}

TEST(ImmutableHashSetTests, IsSubsetOf_True_False) {
    auto sub = ImmutableHashSet<int>::Create({2, 3});
    auto sup = ImmutableHashSet<int>::Create({1, 2, 3, 4});
    EXPECT_TRUE(sub.IsSubsetOf(sup));
    EXPECT_FALSE(sup.IsSubsetOf(sub));
}

TEST(ImmutableHashSetTests, IsSubsetOf_EmptyIsSubsetOfAny) {
    auto empty = ImmutableHashSet<int>::Empty();
    auto s = ImmutableHashSet<int>::Create({1, 2});
    EXPECT_TRUE(empty.IsSubsetOf(s));
}

// ===========================================================================
// ImmutableQueue<T>
// ===========================================================================

TEST(ImmutableQueueTests, Empty_IsEmptyAndCountZero) {
    auto q = ImmutableQueue<int>::Empty();
    EXPECT_TRUE(q.getIsEmptyProperty());
    EXPECT_EQ(q.getCountProperty(), 0);
}

TEST(ImmutableQueueTests, Enqueue_ReturnsNewQueueWithItem) {
    auto q = ImmutableQueue<int>::Empty();
    auto q2 = q.Enqueue(10);
    EXPECT_EQ(q.getCountProperty(), 0);
    EXPECT_EQ(q2.getCountProperty(), 1);
}

TEST(ImmutableQueueTests, Enqueue_MultipleItems_FIFOOrder) {
    auto q = ImmutableQueue<int>::Empty()
                 .Enqueue(1).Enqueue(2).Enqueue(3);
    EXPECT_EQ(q.getCountProperty(), 3);
    EXPECT_EQ(q.Peek(), 1);
}

TEST(ImmutableQueueTests, Peek_ReturnsFront) {
    auto q = ImmutableQueue<int>::Empty().Enqueue(7).Enqueue(8);
    EXPECT_EQ(q.Peek(), 7);
}

TEST(ImmutableQueueTests, Peek_EmptyQueue_Throws) {
    auto q = ImmutableQueue<int>::Empty();
    EXPECT_THROW(q.Peek(), System::InvalidOperationException);
}

TEST(ImmutableQueueTests, Dequeue_ReturnsNewQueueWithoutFront) {
    auto q = ImmutableQueue<int>::Empty().Enqueue(1).Enqueue(2).Enqueue(3);
    auto q2 = q.Dequeue();
    EXPECT_EQ(q.getCountProperty(), 3);
    EXPECT_EQ(q2.getCountProperty(), 2);
    EXPECT_EQ(q2.Peek(), 2);
}

TEST(ImmutableQueueTests, Dequeue_EmptyQueue_Throws) {
    auto q = ImmutableQueue<int>::Empty();
    EXPECT_THROW(q.Dequeue(), System::InvalidOperationException);
}

TEST(ImmutableQueueTests, DequeueWithValue_FillsValue) {
    auto q = ImmutableQueue<int>::Empty().Enqueue(42).Enqueue(99);
    int val = 0;
    auto q2 = q.Dequeue(val);
    EXPECT_EQ(val, 42);
    EXPECT_EQ(q2.getCountProperty(), 1);
}

TEST(ImmutableQueueTests, Clear_ReturnsEmpty) {
    auto q = ImmutableQueue<int>::Empty().Enqueue(1).Enqueue(2);
    auto empty = q.Clear();
    EXPECT_TRUE(empty.getIsEmptyProperty());
}

// ===========================================================================
// ImmutableSortedDictionary<TKey, TValue>
// ===========================================================================

TEST(ImmutableSortedDictionaryTests, Empty_IsEmptyAndCountZero) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty();
    EXPECT_TRUE(d.getIsEmptyProperty());
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(ImmutableSortedDictionaryTests, Add_IncreasesCount) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty()
                 .Add("a", 1).Add("b", 2);
    EXPECT_EQ(d.getCountProperty(), 2);
}

TEST(ImmutableSortedDictionaryTests, Add_DuplicateKey_Throws) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty().Add("x", 10);
    EXPECT_THROW(d.Add("x", 20), System::ArgumentException);
}

TEST(ImmutableSortedDictionaryTests, Add_DuplicateKeySameValue_IsNoOp) {
    // Verified against ImmutableSortedDictionary_2.Node.SetOrAdd: re-adding an existing
    // key with an equal value is a silent no-op, not a thrown ArgumentException.
    auto d = ImmutableSortedDictionary<std::string, int>::Empty().Add("x", 10);
    ImmutableSortedDictionary<std::string, int> d2;
    EXPECT_NO_THROW(d2 = d.Add("x", 10));
    EXPECT_EQ(d2.getCountProperty(), 1);
    EXPECT_EQ(d2["x"], 10);
}

TEST(ImmutableSortedDictionaryTests, AddRange_DuplicateKeyDifferentValue_Throws) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty().Add("x", 10);
    std::vector<std::pair<std::string, int>> pairs{{"y", 1}, {"x", 99}};
    EXPECT_THROW(d.AddRange(pairs), System::ArgumentException);
}

TEST(ImmutableSortedDictionaryTests, AddRange_DuplicateKeySameValue_IsNoOp) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty().Add("x", 10);
    std::vector<std::pair<std::string, int>> pairs{{"y", 1}, {"x", 10}};
    ImmutableSortedDictionary<std::string, int> d2;
    EXPECT_NO_THROW(d2 = d.AddRange(pairs));
    EXPECT_EQ(d2.getCountProperty(), 2);
    EXPECT_EQ(d2["x"], 10);
    EXPECT_EQ(d2["y"], 1);
}

TEST(ImmutableSortedDictionaryTests, ContainsKey_True_False) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty().Add("key", 42);
    EXPECT_TRUE(d.ContainsKey("key"));
    EXPECT_FALSE(d.ContainsKey("missing"));
}

TEST(ImmutableSortedDictionaryTests, OperatorBracket_ReturnsValue) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty().Add("k", 99);
    EXPECT_EQ(d["k"], 99);
}

TEST(ImmutableSortedDictionaryTests, OperatorBracket_MissingKey_Throws) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty();
    EXPECT_THROW((void)d["nope"], System::Collections::Generic::KeyNotFoundException);
}

TEST(ImmutableSortedDictionaryTests, TryGetValue_Found) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty().Add("z", 55);
    int v = 0;
    EXPECT_TRUE(d.TryGetValue("z", v));
    EXPECT_EQ(v, 55);
}

TEST(ImmutableSortedDictionaryTests, TryGetValue_NotFound_ReturnsFalse) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty();
    int v = 0;
    EXPECT_FALSE(d.TryGetValue("missing", v));
}

TEST(ImmutableSortedDictionaryTests, SetItem_OverwritesExistingKey) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty()
                 .Add("a", 1).SetItem("a", 100);
    EXPECT_EQ(d["a"], 100);
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(ImmutableSortedDictionaryTests, Remove_DecreaseCount) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty()
                 .Add("a", 1).Add("b", 2);
    auto d2 = d.Remove("a");
    EXPECT_EQ(d.getCountProperty(), 2);
    EXPECT_EQ(d2.getCountProperty(), 1);
    EXPECT_FALSE(d2.ContainsKey("a"));
}

TEST(ImmutableSortedDictionaryTests, Clear_ReturnsEmpty) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty()
                 .Add("a", 1).Add("b", 2);
    auto empty = d.Clear();
    EXPECT_EQ(empty.getCountProperty(), 0);
}

// ---------------------------------------------------------------------------
// Custom key comparer support (verified against ImmutableSortedDictionary.cs's
// Create(IComparer<TKey>)/WithComparers)
// ---------------------------------------------------------------------------

TEST(ImmutableSortedDictionaryTests, CustomComparer_ReverseKeyOrder) {
    auto d = ImmutableSortedDictionary<int, std::string>::Create(
        [](const int& a, const int& b) { return a > b; },
        {{1, "one"}, {2, "two"}, {3, "three"}});
    std::vector<int> keys = d.getKeysProperty();
    ASSERT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], 3);
    EXPECT_EQ(keys[2], 1);
}

TEST(ImmutableSortedDictionaryTests, WithComparers_ReSortsExistingEntries) {
    auto d = ImmutableSortedDictionary<int, std::string>::Empty().Add(1, "one").Add(2, "two").Add(3, "three");
    auto reversed = d.WithComparers([](const int& a, const int& b) { return a > b; });
    std::vector<int> keys = reversed.getKeysProperty();
    ASSERT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], 3);
    EXPECT_EQ(keys[2], 1);
    EXPECT_EQ(reversed[2], "two"); // entries themselves are unchanged, only the order
}

TEST(ImmutableSortedDictionaryTests, Clear_PreservesCustomComparer) {
    auto d = ImmutableSortedDictionary<int, std::string>::Create(
        [](const int& a, const int& b) { return a > b; }, {{1, "one"}, {2, "two"}});
    auto cleared = d.Clear();
    auto readded = cleared.Add(1, "one").Add(2, "two").Add(3, "three");
    std::vector<int> keys = readded.getKeysProperty();
    ASSERT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], 3); // reverse order survived Clear()
}

TEST(ImmutableSortedDictionaryTests, Keys_AreSorted) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty()
                 .Add("c", 3).Add("a", 1).Add("b", 2);
    auto keys = d.getKeysProperty();
    ASSERT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], "a");
    EXPECT_EQ(keys[1], "b");
    EXPECT_EQ(keys[2], "c");
}

TEST(ImmutableSortedDictionaryTests, Values_InKeyOrder) {
    auto d = ImmutableSortedDictionary<std::string, int>::Empty()
                 .Add("c", 3).Add("a", 1).Add("b", 2);
    auto vals = d.getValuesProperty();
    ASSERT_EQ(vals.size(), 3u);
    EXPECT_EQ(vals[0], 1);
    EXPECT_EQ(vals[1], 2);
    EXPECT_EQ(vals[2], 3);
}

// ===========================================================================
// ImmutableSortedSet<T>
// ===========================================================================

TEST(ImmutableSortedSetTests, Empty_IsEmptyAndCountZero) {
    auto s = ImmutableSortedSet<int>::Empty();
    EXPECT_TRUE(s.getIsEmptyProperty());
    EXPECT_EQ(s.getCountProperty(), 0);
}

TEST(ImmutableSortedSetTests, Create_CountMatches) {
    auto s = ImmutableSortedSet<int>::Create({3, 1, 2});
    EXPECT_EQ(s.getCountProperty(), 3);
}

TEST(ImmutableSortedSetTests, Contains_True_False) {
    auto s = ImmutableSortedSet<int>::Create({10, 20, 30});
    EXPECT_TRUE(s.Contains(20));
    EXPECT_FALSE(s.Contains(99));
}

TEST(ImmutableSortedSetTests, Add_ReturnsNewSetWithItem) {
    auto s = ImmutableSortedSet<int>::Empty();
    auto s2 = s.Add(5);
    EXPECT_EQ(s.getCountProperty(), 0);
    EXPECT_EQ(s2.getCountProperty(), 1);
    EXPECT_TRUE(s2.Contains(5));
}

TEST(ImmutableSortedSetTests, Add_DuplicateNotIncreased) {
    auto s = ImmutableSortedSet<int>::Create({7});
    auto s2 = s.Add(7);
    EXPECT_EQ(s2.getCountProperty(), 1);
}

TEST(ImmutableSortedSetTests, Remove_ReturnsNewSetWithoutItem) {
    auto s = ImmutableSortedSet<int>::Create({1, 2, 3});
    auto s2 = s.Remove(2);
    EXPECT_EQ(s2.getCountProperty(), 2);
    EXPECT_FALSE(s2.Contains(2));
}

TEST(ImmutableSortedSetTests, Clear_ReturnsEmpty) {
    auto s = ImmutableSortedSet<int>::Create({1, 2});
    EXPECT_EQ(s.Clear().getCountProperty(), 0);
}

TEST(ImmutableSortedSetTests, Min_Max) {
    auto s = ImmutableSortedSet<int>::Create({5, 1, 9, 3});
    EXPECT_EQ(s.getMinProperty(), 1);
    EXPECT_EQ(s.getMaxProperty(), 9);
}

TEST(ImmutableSortedSetTests, Min_Max_Empty_ReturnsDefault) {
    auto s = ImmutableSortedSet<int>::Empty();
    EXPECT_EQ(s.getMinProperty(), 0);
    EXPECT_EQ(s.getMaxProperty(), 0);
}

TEST(ImmutableSortedSetTests, Union_CombinesBothSets) {
    auto a = ImmutableSortedSet<int>::Create({1, 2});
    auto b = ImmutableSortedSet<int>::Create({2, 3});
    auto u = a.Union(b);
    EXPECT_EQ(u.getCountProperty(), 3);
}

TEST(ImmutableSortedSetTests, Intersect_OnlyCommonItems) {
    auto a = ImmutableSortedSet<int>::Create({1, 2, 3});
    auto b = ImmutableSortedSet<int>::Create({2, 3, 4});
    auto inter = a.Intersect(b);
    EXPECT_EQ(inter.getCountProperty(), 2);
    EXPECT_TRUE(inter.Contains(2));
    EXPECT_TRUE(inter.Contains(3));
}

TEST(ImmutableSortedSetTests, Except_RemovesOtherItems) {
    auto a = ImmutableSortedSet<int>::Create({1, 2, 3});
    auto b = ImmutableSortedSet<int>::Create({2, 3});
    auto ex = a.Except(b);
    EXPECT_EQ(ex.getCountProperty(), 1);
    EXPECT_TRUE(ex.Contains(1));
}

TEST(ImmutableSortedSetTests, IsSubsetOf_True_False) {
    auto sub = ImmutableSortedSet<int>::Create({2, 3});
    auto sup = ImmutableSortedSet<int>::Create({1, 2, 3, 4});
    EXPECT_TRUE(sub.IsSubsetOf(sup));
    EXPECT_FALSE(sup.IsSubsetOf(sub));
}

TEST(ImmutableSortedSetTests, Iteration_IsSorted) {
    auto s = ImmutableSortedSet<int>::Create({5, 3, 1, 4, 2});
    std::vector<int> items(s.begin(), s.end());
    ASSERT_EQ(items.size(), 5u);
    for (int i = 0; i < 4; ++i)
        EXPECT_LT(items[i], items[i + 1]);
}

// ===========================================================================
// ImmutableHashSet<T> -- custom comparer support (verified against ImmutableHashSet.cs's
// Create(IEqualityComparer<T>)/WithComparer)
// ===========================================================================

namespace {
    size_t caseInsensitiveHash(const std::string& s) {
        std::string lower = s;
        for (char& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return std::hash<std::string>{}(lower);
    }
    bool caseInsensitiveEqual(const std::string& a, const std::string& b) {
        return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
            return std::tolower(static_cast<unsigned char>(x)) == std::tolower(static_cast<unsigned char>(y));
        });
    }
}

TEST(ImmutableHashSetTests, CustomComparer_CollapsesCaseInsensitiveDuplicates) {
    auto s = ImmutableHashSet<std::string>::Create(caseInsensitiveHash, caseInsensitiveEqual,
                                                     {"Hello", "HELLO", "world"});
    EXPECT_EQ(s.getCountProperty(), 2);
    EXPECT_TRUE(s.Contains("hello"));
    EXPECT_TRUE(s.Contains("WORLD"));
}

TEST(ImmutableHashSetTests, WithComparer_RehashesAndCollapsesExistingDuplicates) {
    auto s = ImmutableHashSet<std::string>::Create({"Hello", "HELLO", "world"});
    EXPECT_EQ(s.getCountProperty(), 3); // distinct under default (case-sensitive) equality
    auto ci = s.WithComparer(caseInsensitiveHash, caseInsensitiveEqual);
    EXPECT_EQ(ci.getCountProperty(), 2);
}

TEST(ImmutableHashSetTests, Intersect_UsesThisSetsComparerNotOthers) {
    // Verified against ImmutableHashSet_1.cs's private Intersect: membership is tested via
    // *this* set's Contains (this set's comparer), not `other`'s -- an earlier draft of
    // this fix tested `other.Contains(x)` instead, which is wrong whenever the two sets use
    // different comparers (caught by this exact test: `b` uses the default case-sensitive
    // comparer, so "HELLO" wouldn't match "Hello" via `b`'s own lookup, even though it
    // should intersect successfully under `a`'s case-insensitive comparer).
    auto a = ImmutableHashSet<std::string>::Create(caseInsensitiveHash, caseInsensitiveEqual, {"Hello", "World"});
    auto b = ImmutableHashSet<std::string>::Create({"HELLO", "there"});
    auto result = a.Intersect(b);
    EXPECT_EQ(result.getCountProperty(), 1);
    EXPECT_TRUE(result.Contains("hello")); // still case-insensitive after Intersect
}

TEST(ImmutableHashSetTests, Except_UsesThisSetsComparerNotOthers) {
    // See Intersect_UsesThisSetsComparerNotOthers's comment -- same verified-against-.NET
    // comparer-precedence rule applies to Except.
    auto a = ImmutableHashSet<std::string>::Create(caseInsensitiveHash, caseInsensitiveEqual,
                                                     {"Hello", "World"});
    auto b = ImmutableHashSet<std::string>::Create({"HELLO"});
    auto result = a.Except(b);
    EXPECT_EQ(result.getCountProperty(), 1);
    EXPECT_TRUE(result.Contains("world"));
    EXPECT_FALSE(result.Contains("hello"));
}

TEST(ImmutableHashSetTests, Clear_PreservesCustomComparer) {
    auto s = ImmutableHashSet<std::string>::Create(caseInsensitiveHash, caseInsensitiveEqual, {"Hello"});
    auto cleared = s.Clear();
    EXPECT_TRUE(cleared.getIsEmptyProperty());
    auto readded = cleared.Add("Hello").Add("HELLO");
    EXPECT_EQ(readded.getCountProperty(), 1); // comparer survived Clear()
}

// ===========================================================================
// ImmutableSortedSet<T> -- custom comparer support (verified against
// ImmutableSortedSet.cs's Create(IComparer<T>)/WithComparer)
// ===========================================================================

TEST(ImmutableSortedSetTests, CustomComparer_ReverseOrder) {
    auto s = ImmutableSortedSet<int>::Create(
        [](const int& a, const int& b) { return a > b; }, {1, 2, 3, 4, 5});
    std::vector<int> items(s.begin(), s.end());
    ASSERT_EQ(items.size(), 5u);
    for (int i = 0; i < 4; ++i)
        EXPECT_GT(items[i], items[i + 1]);
    EXPECT_EQ(s.getMinProperty(), 5); // "smallest" under reverse order is numerically largest
    EXPECT_EQ(s.getMaxProperty(), 1);
}

TEST(ImmutableSortedSetTests, WithComparer_ReSortsExistingElements) {
    auto s = ImmutableSortedSet<int>::Create({3, 1, 2});
    auto reversed = s.WithComparer([](const int& a, const int& b) { return a > b; });
    std::vector<int> items(reversed.begin(), reversed.end());
    ASSERT_EQ(items.size(), 3u);
    EXPECT_EQ(items[0], 3);
    EXPECT_EQ(items[2], 1);
}

TEST(ImmutableSortedSetTests, Intersect_UsesThisSetsComparer) {
    // Verified against ImmutableSortedSet_1.cs's Intersect: membership is tested via *this*
    // set's Contains (this set's comparer/ordering), and the result is built under this
    // set's comparer -- so its iteration order reflects `a`'s reverse ordering, not `b`'s
    // default ordering.
    auto a = ImmutableSortedSet<int>::Create([](const int& x, const int& y) { return x > y; }, {1, 2, 3});
    auto b = ImmutableSortedSet<int>::Create({2, 3, 4});
    auto result = a.Intersect(b);
    std::vector<int> items(result.begin(), result.end());
    ASSERT_EQ(items.size(), 2u);
    EXPECT_EQ(items[0], 3); // reverse order preserved: 3 before 2
    EXPECT_EQ(items[1], 2);
}

// Regression test for ticket 315: SetEquals previously compared the two backing std::sets
// directly via operator==, which is a position-wise comparison after each set's OWN internal
// ordering -- giving a wrong (false-negative) result when the two sets use different comparers
// even though they contain the identical elements. Verified against ImmutableSortedSet_1.cs's
// SetEquals: when comparers differ, `other` is rehashed under *this* set's comparer
// (`new SortedSet<T>(other, this.KeyComparer)`) before comparing -- the same
// rehash-under-this-set's-comparer pattern IsSubsetOf/Intersect/Except already use, just missed
// for SetEquals in the same custom-comparer support pass.
TEST(ImmutableSortedSetTests, SetEquals_SameElementsDifferentComparer_ReturnsTrue) {
    auto ascending = ImmutableSortedSet<int>::Create({1, 2, 3});
    auto descending = ImmutableSortedSet<int>::Create([](const int& a, const int& b) { return a > b; }, {1, 2, 3});
    EXPECT_TRUE(ascending.SetEquals(descending));
    EXPECT_TRUE(descending.SetEquals(ascending));
}
TEST(ImmutableSortedSetTests, SetEquals_DifferentElements_ReturnsFalse) {
    auto a = ImmutableSortedSet<int>::Create({1, 2, 3});
    auto b = ImmutableSortedSet<int>::Create([](const int& x, const int& y) { return x > y; }, {1, 2, 4});
    EXPECT_FALSE(a.SetEquals(b));
}

// Regression test for ticket 338: SymmetricExcept previously toggled directly over `other`'s raw
// elements (iterated under `other`'s own comparer) instead of rehashing them under `this` set's
// comparer first. When two of `other`'s elements are distinct under `other`'s comparer but
// collapse to the same logical element under `this` set's comparer, the toggle ran twice for
// that element and cancelled itself out, silently dropping it from the result -- same bug class
// as the SetEquals fix above, just for the toggle-based set operation. Confirmed via a standalone
// UBSan/ASan repro before the fix (case-insensitive empty set XOR case-sensitive {"A","a"}
// produced an empty result instead of the correct single-element {"A"}).
TEST(ImmutableHashSetTests, SymmetricExcept_OtherHasComparerCollapsingDuplicates) {
    auto empty = ImmutableHashSet<std::string>::Create(caseInsensitiveHash, caseInsensitiveEqual, {});
    auto other = ImmutableHashSet<std::string>::Create({"A", "a"}); // distinct under default comparer
    auto result = empty.SymmetricExcept(other);
    EXPECT_EQ(result.getCountProperty(), 1);
}
TEST(ImmutableSortedSetTests, SymmetricExcept_OtherHasComparerCollapsingDuplicates) {
    auto caseInsensitiveLess = [](const std::string& a, const std::string& b) {
        std::string la = a, lb = b;
        for (char& c : la) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        for (char& c : lb) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return la < lb;
    };
    auto empty = ImmutableSortedSet<std::string>::Create(caseInsensitiveLess);
    auto other = ImmutableSortedSet<std::string>::Create({"A", "a"}); // distinct under default operator<
    auto result = empty.SymmetricExcept(other);
    EXPECT_EQ(result.getCountProperty(), 1);
}

TEST(ImmutableHashSetTests, IsSubsetOf_UsesThisSetsComparer) {
    // Verified against ImmutableHashSet_1.cs's private IsSubsetOf: `other` is rehashed
    // under *this* set's comparer before the subset check, so "Hello" (in `sub`, case-
    // insensitive) correctly matches "HELLO" (in `sup`, default case-sensitive) even though
    // `sup` itself would not consider them equal under its own comparer.
    auto sub = ImmutableHashSet<std::string>::Create(caseInsensitiveHash, caseInsensitiveEqual, {"Hello"});
    auto sup = ImmutableHashSet<std::string>::Create({"HELLO", "World"});
    EXPECT_TRUE(sub.IsSubsetOf(sup));
}

TEST(ImmutableHashSetTests, IsSupersetOf_UsesThisSetsComparer) {
    auto sup = ImmutableHashSet<std::string>::Create(caseInsensitiveHash, caseInsensitiveEqual,
                                                       {"Hello", "World"});
    auto sub = ImmutableHashSet<std::string>::Create({"HELLO"});
    EXPECT_TRUE(sup.IsSupersetOf(sub));
}

// Regression test for ticket 315 (found while fixing the sibling ImmutableSortedSet::SetEquals
// bug -- same class of "missed in the custom-comparer support pass" fix, confirmed by grepping
// this file too): SetEquals previously compared raw sizes and tested membership via `other`'s
// own comparer instead of rehashing `other` under *this* set's comparer first (the same pattern
// IsSubsetOf/IsSupersetOf above already use). A case-insensitive {"Hello"} vs. a case-sensitive
// {"HELLO","hello"} -- which collapses to the identical single logical element once rehashed
// case-insensitively -- previously compared unequal (wrong: real .NET's SetEquals rehashes
// `other` under this's comparer, so they ARE equal).
TEST(ImmutableHashSetTests, SetEquals_UsesThisSetsComparer) {
    auto ciSet = ImmutableHashSet<std::string>::Create(caseInsensitiveHash, caseInsensitiveEqual, {"Hello"});
    auto csSet = ImmutableHashSet<std::string>::Create({"HELLO", "hello"});
    EXPECT_TRUE(ciSet.SetEquals(csSet));
}
TEST(ImmutableHashSetTests, SetEquals_DifferentElements_ReturnsFalse) {
    auto a = ImmutableHashSet<std::string>::Create(caseInsensitiveHash, caseInsensitiveEqual, {"Hello"});
    auto b = ImmutableHashSet<std::string>::Create({"World"});
    EXPECT_FALSE(a.SetEquals(b));
}

TEST(ImmutableSortedSetTests, Clear_PreservesCustomComparer) {
    auto s = ImmutableSortedSet<int>::Create([](const int& a, const int& b) { return a > b; }, {1, 2, 3});
    auto cleared = s.Clear();
    auto readded = cleared.Add(1).Add(2).Add(3);
    std::vector<int> items(readded.begin(), readded.end());
    ASSERT_EQ(items.size(), 3u);
    EXPECT_EQ(items[0], 3); // reverse order survived Clear()
}

// ===========================================================================
// ImmutableStack<T>
// ===========================================================================

TEST(ImmutableStackTests, Empty_IsEmptyAndCountZero) {
    auto st = ImmutableStack<int>::Empty();
    EXPECT_TRUE(st.getIsEmptyProperty());
    EXPECT_EQ(st.getCountProperty(), 0);
}

TEST(ImmutableStackTests, Push_ReturnsNewStackWithItem) {
    auto st = ImmutableStack<int>::Empty();
    auto st2 = st.Push(10);
    EXPECT_EQ(st.getCountProperty(), 0);
    EXPECT_EQ(st2.getCountProperty(), 1);
}

TEST(ImmutableStackTests, Push_MultipleItems_LIFOOrder) {
    auto st = ImmutableStack<int>::Empty().Push(1).Push(2).Push(3);
    EXPECT_EQ(st.Peek(), 3);
}

TEST(ImmutableStackTests, Peek_ReturnsTop) {
    auto st = ImmutableStack<int>::Empty().Push(7).Push(8);
    EXPECT_EQ(st.Peek(), 8);
}

TEST(ImmutableStackTests, Peek_EmptyStack_Throws) {
    auto st = ImmutableStack<int>::Empty();
    EXPECT_THROW(st.Peek(), System::InvalidOperationException);
}

TEST(ImmutableStackTests, Pop_ReturnsNewStackWithoutTop) {
    auto st = ImmutableStack<int>::Empty().Push(1).Push(2).Push(3);
    auto st2 = st.Pop();
    EXPECT_EQ(st.getCountProperty(), 3);
    EXPECT_EQ(st2.getCountProperty(), 2);
    EXPECT_EQ(st2.Peek(), 2);
}

TEST(ImmutableStackTests, Pop_EmptyStack_Throws) {
    auto st = ImmutableStack<int>::Empty();
    EXPECT_THROW(st.Pop(), System::InvalidOperationException);
}

TEST(ImmutableStackTests, PopWithValue_FillsValue) {
    auto st = ImmutableStack<int>::Empty().Push(10).Push(20);
    int val = 0;
    auto st2 = st.Pop(val);
    EXPECT_EQ(val, 20);
    EXPECT_EQ(st2.getCountProperty(), 1);
    EXPECT_EQ(st2.Peek(), 10);
}

TEST(ImmutableStackTests, PopWithValue_EmptyStack_Throws) {
    auto st = ImmutableStack<int>::Empty();
    int val = 0;
    EXPECT_THROW(st.Pop(val), System::InvalidOperationException);
}

TEST(ImmutableStackTests, Clear_ReturnsEmpty) {
    auto st = ImmutableStack<int>::Empty().Push(1).Push(2);
    auto empty = st.Clear();
    EXPECT_TRUE(empty.getIsEmptyProperty());
    EXPECT_EQ(empty.getCountProperty(), 0);
}

TEST(ImmutableStackTests, Iteration_TopToBottom) {
    auto st = ImmutableStack<int>::Empty().Push(1).Push(2).Push(3);
    std::vector<int> items(st.begin(), st.end());
    ASSERT_EQ(items.size(), 3u);
    EXPECT_EQ(items[0], 3);
    EXPECT_EQ(items[1], 2);
    EXPECT_EQ(items[2], 1);
}

TEST(ImmutableStackTests, OriginalUnchangedAfterPush) {
    auto st = ImmutableStack<int>::Empty().Push(5);
    auto st2 = st.Push(10);
    EXPECT_EQ(st.getCountProperty(), 1);
    EXPECT_EQ(st2.getCountProperty(), 2);
}
