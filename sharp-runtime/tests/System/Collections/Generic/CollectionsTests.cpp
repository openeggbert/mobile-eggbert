// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <vector>

#include "System/Collections/Generic/List.hpp"
#include "System/Collections/Generic/Dictionary.hpp"
#include "System/Collections/Generic/HashSet.hpp"
#include "System/Collections/Generic/SortedDictionary.hpp"
#include "System/Collections/Generic/SortedList.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"

using System::Collections::Generic::List;
using System::Collections::Generic::Dictionary;
using System::Collections::Generic::HashSet;
using System::Collections::Generic::SortedDictionary;
using System::Collections::Generic::SortedList;
using System::Collections::Generic::KeyNotFoundException;

// ---------------------------------------------------------------------------
// List<T>
// ---------------------------------------------------------------------------

TEST(ListTests, DefaultCtorIsEmpty) {
    List<int> lst;
    EXPECT_EQ(lst.getCountProperty(), 0);
}

TEST(ListTests, AddIncreasesCount) {
    List<int> lst;
    lst.Add(1);
    lst.Add(2);
    lst.Add(3);
    EXPECT_EQ(lst.getCountProperty(), 3);
}

TEST(ListTests, ContainsAfterAdd) {
    List<std::string> lst;
    lst.Add(std::string("hello"));
    EXPECT_TRUE(lst.Contains(std::string("hello")));
}

TEST(ListTests, ContainsReturnsFalseIfAbsent) {
    List<int> lst;
    lst.Add(1);
    EXPECT_FALSE(lst.Contains(99));
}

TEST(ListTests, OperatorBracketRead) {
    List<int> lst;
    lst.Add(10); lst.Add(20); lst.Add(30);
    EXPECT_EQ(lst[0], 10);
    EXPECT_EQ(lst[1], 20);
    EXPECT_EQ(lst[2], 30);
}

TEST(ListTests, OperatorBracketMutation) {
    List<int> lst;
    lst.Add(0);
    lst[0] = 42;
    EXPECT_EQ(lst[0], 42);
}

TEST(ListTests, RemoveReturnsTrueIfFound) {
    List<int> lst;
    lst.Add(5); lst.Add(10);
    EXPECT_TRUE(lst.Remove(5));
    EXPECT_EQ(lst.getCountProperty(), 1);
    EXPECT_EQ(lst[0], 10);
}

TEST(ListTests, RemoveReturnsFalseIfAbsent) {
    List<int> lst;
    lst.Add(1);
    EXPECT_FALSE(lst.Remove(99));
    EXPECT_EQ(lst.getCountProperty(), 1);
}

TEST(ListTests, RemoveFirstOccurrenceOnly) {
    List<int> lst;
    lst.Add(7); lst.Add(7); lst.Add(7);
    lst.Remove(7);
    EXPECT_EQ(lst.getCountProperty(), 2);
}

TEST(ListTests, IndexOfFound) {
    List<int> lst;
    lst.Add(10); lst.Add(20); lst.Add(30);
    EXPECT_EQ(lst.IndexOf(20), 1);
}

TEST(ListTests, IndexOfNotFound) {
    List<int> lst;
    lst.Add(1); lst.Add(2);
    EXPECT_EQ(lst.IndexOf(99), -1);
}

TEST(ListTests, Insert) {
    List<int> lst;
    lst.Add(1); lst.Add(3);
    lst.Insert(1, 2); // insert 2 between 1 and 3
    EXPECT_EQ(lst.getCountProperty(), 3);
    EXPECT_EQ(lst[0], 1);
    EXPECT_EQ(lst[1], 2);
    EXPECT_EQ(lst[2], 3);
}

TEST(ListTests, RemoveAt) {
    List<int> lst;
    lst.Add(10); lst.Add(20); lst.Add(30);
    lst.RemoveAt(1); // remove 20
    EXPECT_EQ(lst.getCountProperty(), 2);
    EXPECT_EQ(lst[0], 10);
    EXPECT_EQ(lst[1], 30);
}

TEST(ListTests, ClearResetsCount) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    lst.Clear();
    EXPECT_EQ(lst.getCountProperty(), 0);
    EXPECT_FALSE(lst.Contains(1));
}

TEST(ListTests, ToVector) {
    List<int> lst;
    lst.Add(4); lst.Add(5); lst.Add(6);
    const std::vector<int>& v = lst.ToVector();
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 4);
    EXPECT_EQ(v[2], 6);
}

TEST(ListTests, CtorFromVector) {
    std::vector<int> src = {7, 8, 9};
    List<int> lst(src);
    EXPECT_EQ(lst.getCountProperty(), 3);
    EXPECT_EQ(lst[0], 7);
    EXPECT_EQ(lst[2], 9);
}

TEST(ListTests, RangeForIteration) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    int sum = 0;
    for (int x : lst) sum += x;
    EXPECT_EQ(sum, 6);
}

TEST(ListTests, RangeForIterationOrder) {
    List<std::string> lst;
    lst.Add(std::string("a")); lst.Add(std::string("b")); lst.Add(std::string("c"));
    std::vector<std::string> collected;
    for (const auto& s : lst) collected.push_back(s);
    EXPECT_EQ(collected[0], "a");
    EXPECT_EQ(collected[1], "b");
    EXPECT_EQ(collected[2], "c");
}

TEST(ListTests, StringList) {
    List<std::string> lst;
    lst.Add(std::string("foo"));
    lst.Add(std::string("bar"));
    EXPECT_EQ(lst.getCountProperty(), 2);
    EXPECT_EQ(lst[0], "foo");
    EXPECT_EQ(lst[1], "bar");
    EXPECT_TRUE(lst.Contains(std::string("bar")));
    EXPECT_FALSE(lst.Contains(std::string("baz")));
}

// ---------------------------------------------------------------------------
// Dictionary<TKey, TValue>
// ---------------------------------------------------------------------------

TEST(DictionaryTests, DefaultCtorIsEmpty) {
    Dictionary<std::string, int> d;
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(DictionaryTests, AddAndContainsKey) {
    Dictionary<std::string, int> d;
    d.Add(std::string("a"), 1);
    EXPECT_TRUE(d.ContainsKey(std::string("a")));
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(DictionaryTests, ContainsKeyReturnsFalseIfAbsent) {
    Dictionary<std::string, int> d;
    d.Add(std::string("x"), 0);
    EXPECT_FALSE(d.ContainsKey(std::string("y")));
}

TEST(DictionaryTests, AddDuplicateThrows) {
    Dictionary<std::string, int> d;
    d.Add(std::string("k"), 1);
    EXPECT_THROW(d.Add(std::string("k"), 2), System::ArgumentException);
}

TEST(DictionaryTests, TryGetValueFound) {
    Dictionary<int, std::string> d;
    d.Add(42, std::string("forty-two"));
    std::string out;
    bool found = d.TryGetValue(42, out);
    EXPECT_TRUE(found);
    EXPECT_EQ(out, "forty-two");
}

TEST(DictionaryTests, TryGetValueNotFound) {
    Dictionary<int, std::string> d;
    std::string out = "unchanged";
    bool found = d.TryGetValue(99, out);
    EXPECT_FALSE(found);
}

TEST(DictionaryTests, OperatorBracketWrite) {
    Dictionary<std::string, int> d;
    d[std::string("key")] = 100;
    EXPECT_TRUE(d.ContainsKey(std::string("key")));
    EXPECT_EQ(d[std::string("key")], 100);
}

TEST(DictionaryTests, OperatorBracketOverwrite) {
    Dictionary<std::string, int> d;
    d.Add(std::string("k"), 1);
    d[std::string("k")] = 2;
    EXPECT_EQ(d[std::string("k")], 2);
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(DictionaryTests, ConstOperatorBracketThrowsOnMissing) {
    const Dictionary<std::string, int> d;
    EXPECT_THROW((void)d[std::string("missing")], KeyNotFoundException);
}

TEST(DictionaryTests, RemoveReturnsTrueIfFound) {
    Dictionary<std::string, int> d;
    d.Add(std::string("k"), 5);
    EXPECT_TRUE(d.Remove(std::string("k")));
    EXPECT_EQ(d.getCountProperty(), 0);
    EXPECT_FALSE(d.ContainsKey(std::string("k")));
}

TEST(DictionaryTests, RemoveReturnsFalseIfAbsent) {
    Dictionary<std::string, int> d;
    EXPECT_FALSE(d.Remove(std::string("nope")));
}

TEST(DictionaryTests, CountAfterMultipleAdds) {
    Dictionary<int, int> d;
    for (int i = 0; i < 10; ++i) d.Add(i, i * i);
    EXPECT_EQ(d.getCountProperty(), 10);
}

TEST(DictionaryTests, ClearResetsCount) {
    Dictionary<int, int> d;
    d.Add(1, 1); d.Add(2, 4); d.Add(3, 9);
    d.Clear();
    EXPECT_EQ(d.getCountProperty(), 0);
    EXPECT_FALSE(d.ContainsKey(1));
}

TEST(DictionaryTests, RangeForIterationCoversAllEntries) {
    Dictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20); d.Add(3, 30);
    int keySum = 0, valSum = 0;
    for (const auto& [k, v] : d) { keySum += k; valSum += v; }
    EXPECT_EQ(keySum, 6);
    EXPECT_EQ(valSum, 60);
}

TEST(DictionaryTests, IntToStringMap) {
    Dictionary<int, std::string> d;
    d.Add(1, std::string("one"));
    d.Add(2, std::string("two"));
    std::string out;
    EXPECT_TRUE(d.TryGetValue(1, out));
    EXPECT_EQ(out, "one");
    EXPECT_TRUE(d.TryGetValue(2, out));
    EXPECT_EQ(out, "two");
}

// ---------------------------------------------------------------------------
// HashSet<T>
// ---------------------------------------------------------------------------

TEST(HashSetTests, DefaultCtorIsEmpty) {
    HashSet<int> hs;
    EXPECT_EQ(hs.getCountProperty(), 0);
}

TEST(HashSetTests, AddReturnsTrueFirstTime) {
    HashSet<int> hs;
    EXPECT_TRUE(hs.Add(1));
    EXPECT_EQ(hs.getCountProperty(), 1);
}

TEST(HashSetTests, AddReturnsFalseIfDuplicate) {
    HashSet<int> hs;
    hs.Add(1);
    EXPECT_FALSE(hs.Add(1));
    EXPECT_EQ(hs.getCountProperty(), 1);
}

TEST(HashSetTests, ContainsAfterAdd) {
    HashSet<int> hs;
    hs.Add(42);
    EXPECT_TRUE(hs.Contains(42));
}

TEST(HashSetTests, ContainsReturnsFalseIfAbsent) {
    HashSet<int> hs;
    hs.Add(1);
    EXPECT_FALSE(hs.Contains(99));
}

TEST(HashSetTests, RemoveReturnsTrueIfFound) {
    HashSet<int> hs;
    hs.Add(5);
    EXPECT_TRUE(hs.Remove(5));
    EXPECT_EQ(hs.getCountProperty(), 0);
    EXPECT_FALSE(hs.Contains(5));
}

TEST(HashSetTests, RemoveReturnsFalseIfAbsent) {
    HashSet<int> hs;
    EXPECT_FALSE(hs.Remove(99));
}

TEST(HashSetTests, CountAfterAddRemove) {
    HashSet<int> hs;
    hs.Add(1); hs.Add(2); hs.Add(3);
    hs.Remove(2);
    EXPECT_EQ(hs.getCountProperty(), 2);
    EXPECT_TRUE(hs.Contains(1));
    EXPECT_FALSE(hs.Contains(2));
    EXPECT_TRUE(hs.Contains(3));
}

TEST(HashSetTests, ClearResetsCount) {
    HashSet<int> hs;
    hs.Add(1); hs.Add(2); hs.Add(3);
    hs.Clear();
    EXPECT_EQ(hs.getCountProperty(), 0);
    EXPECT_FALSE(hs.Contains(1));
}

TEST(HashSetTests, UnionWith) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(2); b.Add(3);
    a.UnionWith(b);
    EXPECT_EQ(a.getCountProperty(), 3);
    EXPECT_TRUE(a.Contains(1));
    EXPECT_TRUE(a.Contains(2));
    EXPECT_TRUE(a.Contains(3));
}

TEST(HashSetTests, UnionWithDisjoint) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(3); b.Add(4);
    a.UnionWith(b);
    EXPECT_EQ(a.getCountProperty(), 4);
}

TEST(HashSetTests, IntersectWith) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2); a.Add(3);
    b.Add(2); b.Add(3); b.Add(4);
    a.IntersectWith(b);
    EXPECT_EQ(a.getCountProperty(), 2);
    EXPECT_FALSE(a.Contains(1));
    EXPECT_TRUE(a.Contains(2));
    EXPECT_TRUE(a.Contains(3));
    EXPECT_FALSE(a.Contains(4));
}

TEST(HashSetTests, IntersectWithDisjoint) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(3); b.Add(4);
    a.IntersectWith(b);
    EXPECT_EQ(a.getCountProperty(), 0);
}

TEST(HashSetTests, ExceptWith) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2); a.Add(3);
    b.Add(2); b.Add(3);
    a.ExceptWith(b);
    EXPECT_EQ(a.getCountProperty(), 1);
    EXPECT_TRUE(a.Contains(1));
    EXPECT_FALSE(a.Contains(2));
    EXPECT_FALSE(a.Contains(3));
}

TEST(HashSetTests, ExceptWithNoOverlap) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(3);
    a.ExceptWith(b);
    EXPECT_EQ(a.getCountProperty(), 2);
}

TEST(HashSetTests, ToArray) {
    HashSet<int> hs;
    hs.Add(10); hs.Add(20); hs.Add(30);
    std::vector<int> arr = hs.ToArray();
    EXPECT_EQ(arr.size(), 3u);
    // order is unspecified; just verify all elements are present
    std::sort(arr.begin(), arr.end());
    EXPECT_EQ(arr[0], 10);
    EXPECT_EQ(arr[1], 20);
    EXPECT_EQ(arr[2], 30);
}

TEST(HashSetTests, RangeForIteration) {
    HashSet<int> hs;
    hs.Add(1); hs.Add(2); hs.Add(3);
    int sum = 0;
    for (int x : hs) sum += x;
    EXPECT_EQ(sum, 6);
}

TEST(HashSetTests, StringHashSet) {
    HashSet<std::string> hs;
    EXPECT_TRUE(hs.Add(std::string("alpha")));
    EXPECT_TRUE(hs.Add(std::string("beta")));
    EXPECT_FALSE(hs.Add(std::string("alpha")));
    EXPECT_EQ(hs.getCountProperty(), 2);
    EXPECT_TRUE(hs.Contains(std::string("beta")));
    hs.Remove(std::string("beta"));
    EXPECT_FALSE(hs.Contains(std::string("beta")));
}

// ---------------------------------------------------------------------------
// List<T> — new methods (session 57)
// ---------------------------------------------------------------------------

TEST(ListTests, Sort_DefaultOrder) {
    List<int> lst;
    lst.Add(3); lst.Add(1); lst.Add(4); lst.Add(1); lst.Add(5);
    lst.Sort();
    EXPECT_EQ(lst[0], 1);
    EXPECT_EQ(lst[4], 5);
}

TEST(ListTests, Sort_WithComparison_Descending) {
    List<int> lst;
    lst.Add(3); lst.Add(1); lst.Add(2);
    lst.Sort([](const int& a, const int& b) { return b - a; });
    EXPECT_EQ(lst[0], 3);
    EXPECT_EQ(lst[2], 1);
}

TEST(ListTests, Reverse_ReversesOrder) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    lst.Reverse();
    EXPECT_EQ(lst[0], 3);
    EXPECT_EQ(lst[2], 1);
}

TEST(ListTests, AddRange_FromVector) {
    List<int> lst;
    lst.Add(1);
    lst.AddRange(std::vector<int>{2, 3, 4});
    EXPECT_EQ(lst.getCountProperty(), 4);
    EXPECT_EQ(lst[3], 4);
}

TEST(ListTests, AddRange_FromList) {
    List<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(3); b.Add(4);
    a.AddRange(b);
    EXPECT_EQ(a.getCountProperty(), 4);
    EXPECT_EQ(a[3], 4);
}

TEST(ListTests, InsertRange) {
    List<int> lst;
    lst.Add(1); lst.Add(4);
    lst.InsertRange(1, std::vector<int>{2, 3});
    EXPECT_EQ(lst.getCountProperty(), 4);
    EXPECT_EQ(lst[1], 2);
    EXPECT_EQ(lst[2], 3);
}

TEST(ListTests, GetRange) {
    List<int> lst;
    for (int i = 0; i < 5; ++i) lst.Add(i);
    auto sub = lst.GetRange(1, 3);
    EXPECT_EQ(sub.getCountProperty(), 3);
    EXPECT_EQ(sub[0], 1);
    EXPECT_EQ(sub[2], 3);
}

TEST(ListTests, ToArray_ReturnsCopy) {
    List<int> lst;
    lst.Add(10); lst.Add(20);
    auto arr = lst.ToArray();
    EXPECT_EQ(arr.size(), 2u);
    EXPECT_EQ(arr[1], 20);
}

TEST(ListTests, Find_Found) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    EXPECT_EQ(lst.Find([](const int& x) { return x > 1; }), 2);
}

TEST(ListTests, Find_NotFound_ReturnsDefault) {
    List<int> lst;
    lst.Add(1); lst.Add(2);
    EXPECT_EQ(lst.Find([](const int& x) { return x > 10; }), 0);
}

TEST(ListTests, FindAll) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3); lst.Add(4);
    auto evens = lst.FindAll([](const int& x) { return x % 2 == 0; });
    EXPECT_EQ(evens.getCountProperty(), 2);
    EXPECT_EQ(evens[0], 2);
    EXPECT_EQ(evens[1], 4);
}

TEST(ListTests, FindIndex_Found) {
    List<int> lst;
    lst.Add(10); lst.Add(20); lst.Add(30);
    EXPECT_EQ(lst.FindIndex([](const int& x) { return x == 20; }), 1);
}

TEST(ListTests, FindIndex_NotFound) {
    List<int> lst;
    lst.Add(1); lst.Add(2);
    EXPECT_EQ(lst.FindIndex([](const int& x) { return x == 99; }), -1);
}

TEST(ListTests, FindLastIndex) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(2); lst.Add(3);
    EXPECT_EQ(lst.FindLastIndex([](const int& x) { return x == 2; }), 2);
}

TEST(ListTests, RemoveAll_RemovesMatching) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3); lst.Add(4);
    int removed = lst.RemoveAll([](const int& x) { return x % 2 == 0; });
    EXPECT_EQ(removed, 2);
    EXPECT_EQ(lst.getCountProperty(), 2);
    EXPECT_EQ(lst[0], 1);
    EXPECT_EQ(lst[1], 3);
}

TEST(ListTests, ForEach_VisitsAll) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    int sum = 0;
    lst.ForEach([&sum](const int& x) { sum += x; });
    EXPECT_EQ(sum, 6);
}

TEST(ListTests, Exists_True) {
    List<int> lst;
    lst.Add(1); lst.Add(5); lst.Add(3);
    EXPECT_TRUE(lst.Exists([](const int& x) { return x == 5; }));
}

TEST(ListTests, Exists_False) {
    List<int> lst;
    lst.Add(1); lst.Add(2);
    EXPECT_FALSE(lst.Exists([](const int& x) { return x == 99; }));
}

TEST(ListTests, TrueForAll_True) {
    List<int> lst;
    lst.Add(2); lst.Add(4); lst.Add(6);
    EXPECT_TRUE(lst.TrueForAll([](const int& x) { return x % 2 == 0; }));
}

TEST(ListTests, TrueForAll_False) {
    List<int> lst;
    lst.Add(2); lst.Add(3);
    EXPECT_FALSE(lst.TrueForAll([](const int& x) { return x % 2 == 0; }));
}

TEST(ListTests, BinarySearch_Found) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3); lst.Add(4); lst.Add(5);
    EXPECT_EQ(lst.BinarySearch(3), 2);
}

TEST(ListTests, BinarySearch_NotFound_NegativeResult) {
    List<int> lst;
    lst.Add(1); lst.Add(3); lst.Add(5);
    EXPECT_LT(lst.BinarySearch(2), 0);
}

// ---------------------------------------------------------------------------
// Dictionary<K,V> — new methods (session 58)
// ---------------------------------------------------------------------------

TEST(DictionaryTests, TryAdd_AddsWhenAbsent) {
    Dictionary<std::string, int> d;
    EXPECT_TRUE(d.TryAdd("x", 1));
    EXPECT_EQ(d["x"], 1);
}

TEST(DictionaryTests, TryAdd_ReturnsFalseWhenPresent) {
    Dictionary<std::string, int> d;
    d.Add("x", 1);
    EXPECT_FALSE(d.TryAdd("x", 99));
    EXPECT_EQ(d["x"], 1);
}

TEST(DictionaryTests, ContainsValue_Found) {
    Dictionary<std::string, int> d;
    d.Add("a", 42);
    EXPECT_TRUE(d.ContainsValue(42));
}

TEST(DictionaryTests, ContainsValue_NotFound) {
    Dictionary<std::string, int> d;
    d.Add("a", 1);
    EXPECT_FALSE(d.ContainsValue(99));
}

TEST(DictionaryTests, GetValueOrDefault_Present) {
    Dictionary<std::string, int> d;
    d.Add("k", 7);
    EXPECT_EQ(d.GetValueOrDefault("k", 0), 7);
}

TEST(DictionaryTests, GetValueOrDefault_Absent) {
    Dictionary<std::string, int> d;
    EXPECT_EQ(d.GetValueOrDefault("missing", 42), 42);
}

TEST(DictionaryTests, GetValueOrDefault_AbsentDefaultDefault) {
    Dictionary<std::string, int> d;
    EXPECT_EQ(d.GetValueOrDefault("missing"), 0);
}

TEST(DictionaryTests, Keys_ContainsAllKeys) {
    Dictionary<std::string, int> d;
    d.Add("a", 1); d.Add("b", 2);
    auto keys = d.getKeysProperty();
    EXPECT_EQ(keys.size(), 2u);
    bool hasA = std::find(keys.begin(), keys.end(), "a") != keys.end();
    bool hasB = std::find(keys.begin(), keys.end(), "b") != keys.end();
    EXPECT_TRUE(hasA);
    EXPECT_TRUE(hasB);
}

TEST(DictionaryTests, Values_ContainsAllValues) {
    Dictionary<std::string, int> d;
    d.Add("a", 10); d.Add("b", 20);
    auto vals = d.getValuesProperty();
    EXPECT_EQ(vals.size(), 2u);
    bool has10 = std::find(vals.begin(), vals.end(), 10) != vals.end();
    bool has20 = std::find(vals.begin(), vals.end(), 20) != vals.end();
    EXPECT_TRUE(has10);
    EXPECT_TRUE(has20);
}

// ---------------------------------------------------------------------------
// Remove(key, out value)
// ---------------------------------------------------------------------------
TEST(DictionaryTests, Remove_WithValue_ReturnsValue) {
    Dictionary<std::string, int> d;
    d.Add("x", 42);
    int v = 0;
    bool removed = d.Remove(std::string("x"), v);
    EXPECT_TRUE(removed);
    EXPECT_EQ(v, 42);
    EXPECT_FALSE(d.ContainsKey("x"));
}
TEST(DictionaryTests, Remove_WithValue_NotFound_ReturnsFalse) {
    Dictionary<std::string, int> d;
    int v = 99;
    bool removed = d.Remove(std::string("missing"), v);
    EXPECT_FALSE(removed);
    EXPECT_EQ(v, 99);
}

// ---------------------------------------------------------------------------
// EnsureCapacity / TrimExcess
// ---------------------------------------------------------------------------
TEST(DictionaryTests, EnsureCapacity_AllowsInserts) {
    Dictionary<int, int> d;
    d.EnsureCapacity(100);
    for (int i = 0; i < 100; ++i) d.Add(i, i * 2);
    EXPECT_EQ(d.getCountProperty(), 100);
}
TEST(DictionaryTests, TrimExcess_DoesNotLoseEntries) {
    Dictionary<int, int> d;
    d.EnsureCapacity(1000);
    d.Add(1, 10); d.Add(2, 20);
    d.TrimExcess();
    EXPECT_EQ(d.getCountProperty(), 2);
    EXPECT_TRUE(d.ContainsKey(1));
    EXPECT_TRUE(d.ContainsKey(2));
}

// ===========================================================================
// HashSet<T> — additional set operations
// ===========================================================================

TEST(HashSetTests, SetEquals_SameElements) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2); a.Add(3);
    b.Add(3); b.Add(1); b.Add(2);
    EXPECT_TRUE(a.SetEquals(b));
    EXPECT_TRUE(b.SetEquals(a));
}

TEST(HashSetTests, SetEquals_DifferentElements) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(1); b.Add(3);
    EXPECT_FALSE(a.SetEquals(b));
}

TEST(HashSetTests, SymmetricExceptWith) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2); a.Add(3);
    b.Add(2); b.Add(3); b.Add(4);
    a.SymmetricExceptWith(b);
    EXPECT_TRUE(a.Contains(1));
    EXPECT_TRUE(a.Contains(4));
    EXPECT_FALSE(a.Contains(2));
    EXPECT_FALSE(a.Contains(3));
}

TEST(HashSetTests, IsSubsetOf_True) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(1); b.Add(2); b.Add(3);
    EXPECT_TRUE(a.IsSubsetOf(b));
    EXPECT_FALSE(b.IsSubsetOf(a));
}

TEST(HashSetTests, IsSupersetOf_True) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2); a.Add(3);
    b.Add(1); b.Add(2);
    EXPECT_TRUE(a.IsSupersetOf(b));
    EXPECT_FALSE(b.IsSupersetOf(a));
}

TEST(HashSetTests, IsProperSubsetOf) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(1); b.Add(2); b.Add(3);
    EXPECT_TRUE(a.IsProperSubsetOf(b));
    EXPECT_FALSE(b.IsProperSubsetOf(a));
    // Equal sets are NOT proper subsets
    HashSet<int> c;
    c.Add(1); c.Add(2);
    EXPECT_FALSE(a.IsProperSubsetOf(c));
}

TEST(HashSetTests, IsProperSupersetOf) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2); a.Add(3);
    b.Add(1); b.Add(2);
    EXPECT_TRUE(a.IsProperSupersetOf(b));
    EXPECT_FALSE(b.IsProperSupersetOf(a));
}

// ---------------------------------------------------------------------------
// EnsureCapacity / TrimExcess
// ---------------------------------------------------------------------------
TEST(HashSetTests, EnsureCapacity_AllowsInserts) {
    HashSet<int> s;
    s.EnsureCapacity(200);
    for (int i = 0; i < 200; ++i) s.Add(i);
    EXPECT_EQ(s.getCountProperty(), 200);
}
TEST(HashSetTests, TrimExcess_DoesNotLoseEntries) {
    HashSet<int> s;
    s.EnsureCapacity(1000);
    s.Add(1); s.Add(2); s.Add(3);
    s.TrimExcess();
    EXPECT_EQ(s.getCountProperty(), 3);
    EXPECT_TRUE(s.Contains(1));
    EXPECT_TRUE(s.Contains(3));
}

// Regression: EnsureCapacity previously returned void, but real .NET's
// HashSet<T>.EnsureCapacity(int) returns the resulting capacity -- added to match that
// signature.
TEST(HashSetTests, EnsureCapacity_ReturnsResultingCapacity) {
    HashSet<int> s;
    SharpRuntime::intcs cap = s.EnsureCapacity(200);
    EXPECT_GE(cap, 200);
}

// ===========================================================================
// SortedDictionary
// ===========================================================================

TEST(SortedDictionaryTests, Add_And_ContainsKey) {
    SortedDictionary<int, std::string> sd;
    sd.Add(1, "one");
    EXPECT_TRUE(sd.ContainsKey(1));
    EXPECT_FALSE(sd.ContainsKey(2));
}

TEST(SortedDictionaryTests, ContainsValue) {
    SortedDictionary<int, std::string> sd;
    sd.Add(1, "one");
    sd.Add(2, "two");
    EXPECT_TRUE(sd.ContainsValue("one"));
    EXPECT_FALSE(sd.ContainsValue("three"));
}

TEST(SortedDictionaryTests, TryAdd_ReturnsTrueOnNew) {
    SortedDictionary<int, std::string> sd;
    EXPECT_TRUE(sd.TryAdd(1, "one"));
    EXPECT_FALSE(sd.TryAdd(1, "again"));
    EXPECT_EQ(sd.getCountProperty(), 1);
}

TEST(SortedDictionaryTests, GetValueOrDefault_Found) {
    SortedDictionary<int, std::string> sd;
    sd.Add(5, "five");
    EXPECT_EQ(sd.GetValueOrDefault(5, "?"), "five");
}

TEST(SortedDictionaryTests, GetValueOrDefault_Missing) {
    SortedDictionary<int, std::string> sd;
    EXPECT_EQ(sd.GetValueOrDefault(99, "missing"), "missing");
}

TEST(SortedDictionaryTests, getKeysProperty_SortedOrder) {
    SortedDictionary<int, std::string> sd;
    sd.Add(3, "c"); sd.Add(1, "a"); sd.Add(2, "b");
    auto keys = sd.getKeysProperty();
    EXPECT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], 1);
    EXPECT_EQ(keys[1], 2);
    EXPECT_EQ(keys[2], 3);
}

TEST(SortedDictionaryTests, getValuesProperty_SortedByKey) {
    SortedDictionary<int, std::string> sd;
    sd.Add(3, "c"); sd.Add(1, "a"); sd.Add(2, "b");
    auto vals = sd.getValuesProperty();
    EXPECT_EQ(vals[0], "a");
    EXPECT_EQ(vals[1], "b");
    EXPECT_EQ(vals[2], "c");
}

// ===========================================================================
// SortedList
// ===========================================================================

TEST(SortedListTests, Add_And_ContainsKey) {
    SortedList<int, std::string> sl;
    sl.Add(10, "ten");
    EXPECT_TRUE(sl.ContainsKey(10));
    EXPECT_FALSE(sl.ContainsKey(20));
}

TEST(SortedListTests, TryAdd_ReturnsTrueOnNew) {
    SortedList<int, std::string> sl;
    EXPECT_TRUE(sl.TryAdd(1, "one"));
    EXPECT_FALSE(sl.TryAdd(1, "again"));
    EXPECT_EQ(sl.getCountProperty(), 1);
}

TEST(SortedListTests, GetValueOrDefault_Found) {
    SortedList<int, std::string> sl;
    sl.Add(7, "seven");
    EXPECT_EQ(sl.GetValueOrDefault(7, "?"), "seven");
}

TEST(SortedListTests, GetValueOrDefault_Missing) {
    SortedList<int, std::string> sl;
    EXPECT_EQ(sl.GetValueOrDefault(0, "nope"), "nope");
}

TEST(SortedListTests, getKeysProperty_SortedOrder) {
    SortedList<int, std::string> sl;
    sl.Add(30, "c"); sl.Add(10, "a"); sl.Add(20, "b");
    auto keys = sl.getKeysProperty();
    EXPECT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], 10);
    EXPECT_EQ(keys[1], 20);
    EXPECT_EQ(keys[2], 30);
}

TEST(SortedListTests, getValuesProperty_SortedByKey) {
    SortedList<int, std::string> sl;
    sl.Add(30, "c"); sl.Add(10, "a"); sl.Add(20, "b");
    auto vals = sl.getValuesProperty();
    EXPECT_EQ(vals[0], "a");
    EXPECT_EQ(vals[1], "b");
    EXPECT_EQ(vals[2], "c");
}

// Regression tests for ticket 351: GetKeyAtIndex/GetValueAtIndex/SetValueAtIndex previously
// didn't exist on this port at all, despite being core public API in real .NET's
// SortedList<TKey,TValue> (a type whose whole documented purpose is "sorted by key and
// accessible by key or index").
TEST(SortedListTests, GetKeyAtIndex_ReturnsKeyInSortedOrder) {
    SortedList<int, std::string> sl;
    sl.Add(30, "c"); sl.Add(10, "a"); sl.Add(20, "b");
    EXPECT_EQ(sl.GetKeyAtIndex(0), 10);
    EXPECT_EQ(sl.GetKeyAtIndex(1), 20);
    EXPECT_EQ(sl.GetKeyAtIndex(2), 30);
}

TEST(SortedListTests, GetValueAtIndex_ReturnsValueInSortedKeyOrder) {
    SortedList<int, std::string> sl;
    sl.Add(30, "c"); sl.Add(10, "a"); sl.Add(20, "b");
    EXPECT_EQ(sl.GetValueAtIndex(0), "a");
    EXPECT_EQ(sl.GetValueAtIndex(1), "b");
    EXPECT_EQ(sl.GetValueAtIndex(2), "c");
}

TEST(SortedListTests, GetKeyAtIndex_OutOfRange_Throws) {
    SortedList<int, std::string> sl;
    sl.Add(1, "a");
    EXPECT_THROW(sl.GetKeyAtIndex(-1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(sl.GetKeyAtIndex(1), System::ArgumentOutOfRangeException);
}

TEST(SortedListTests, SetValueAtIndex_ReplacesValueKeepingKey) {
    SortedList<int, std::string> sl;
    sl.Add(10, "a"); sl.Add(20, "b");
    sl.SetValueAtIndex(0, "z");
    EXPECT_EQ(sl.GetKeyAtIndex(0), 10);
    EXPECT_EQ(sl.GetValueAtIndex(0), "z");
}

TEST(SortedListTests, SetValueAtIndex_OutOfRange_Throws) {
    SortedList<int, std::string> sl;
    EXPECT_THROW(sl.SetValueAtIndex(0, "x"), System::ArgumentOutOfRangeException);
}

// ===========================================================================
// List<T> — IndexOf(startIndex) and LastIndexOf
// ===========================================================================

TEST(ListTests, IndexOf_StartIndex_Found) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3); lst.Add(2);
    EXPECT_EQ(lst.IndexOf(2, 2), 3);
}
TEST(ListTests, IndexOf_StartIndex_NotFound) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    EXPECT_EQ(lst.IndexOf(1, 1), -1);
}
TEST(ListTests, LastIndexOf_Found) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(1);
    EXPECT_EQ(lst.LastIndexOf(1), 2);
}
TEST(ListTests, LastIndexOf_NotFound) {
    List<int> lst;
    lst.Add(1); lst.Add(2);
    EXPECT_EQ(lst.LastIndexOf(9), -1);
}
TEST(ListTests, LastIndexOf_WithStartIndex) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(1); lst.Add(2);
    EXPECT_EQ(lst.LastIndexOf(2, 2), 1);
}

// ---------------------------------------------------------------------------
// Capacity management
// ---------------------------------------------------------------------------
TEST(ListTests, EnsureCapacity_SetsAtLeast) {
    List<int> lst;
    lst.EnsureCapacity(100);
    EXPECT_GE(lst.getCapacityProperty(), 100);
}
TEST(ListTests, EnsureCapacity_SmallerThanCurrent_NoOp) {
    List<int> lst;
    lst.EnsureCapacity(50);
    int cap = lst.getCapacityProperty();
    lst.EnsureCapacity(10);
    EXPECT_EQ(lst.getCapacityProperty(), cap);
}
TEST(ListTests, TrimExcess_ReducesCapacity) {
    List<int> lst;
    lst.EnsureCapacity(1000);
    lst.Add(1); lst.Add(2);
    lst.TrimExcess();
    EXPECT_LE(lst.getCapacityProperty(), 4);
}

// ---------------------------------------------------------------------------
// ConvertAll
// ---------------------------------------------------------------------------
TEST(ListTests, ConvertAll_IntToDouble) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3);
    auto result = lst.ConvertAll<double>([](const int& x) { return static_cast<double>(x) * 1.5; });
    EXPECT_EQ(result.getCountProperty(), 3);
    EXPECT_NEAR(result[0], 1.5, 1e-9);
    EXPECT_NEAR(result[1], 3.0, 1e-9);
    EXPECT_NEAR(result[2], 4.5, 1e-9);
}

// ---------------------------------------------------------------------------
// AsReadOnly
// ---------------------------------------------------------------------------
TEST(ListTests, AsReadOnly_CountMatches) {
    List<int> lst;
    lst.Add(10); lst.Add(20);
    const auto ro = lst.AsReadOnly();
    EXPECT_EQ(ro.getCountProperty(), 2);
    EXPECT_EQ(ro[0], 10);
    EXPECT_EQ(ro[1], 20);
}
TEST(ListTests, AsReadOnly_IsReadOnly) {
    List<int> lst;
    lst.Add(1);
    auto ro = lst.AsReadOnly();
    EXPECT_TRUE(ro.getIsReadOnlyProperty());
}

// ---------------------------------------------------------------------------
// FindLast
// ---------------------------------------------------------------------------
TEST(ListTests, FindLast_Found) {
    List<int> lst;
    lst.Add(1); lst.Add(3); lst.Add(5); lst.Add(2);
    int v = lst.FindLast([](const int& x) { return x % 2 != 0; });
    EXPECT_EQ(v, 5);
}
TEST(ListTests, FindLast_NotFound_ReturnsDefault) {
    List<int> lst;
    lst.Add(2); lst.Add(4);
    int v = lst.FindLast([](const int& x) { return x % 2 != 0; });
    EXPECT_EQ(v, 0);
}

// ---------------------------------------------------------------------------
// RemoveRange
// ---------------------------------------------------------------------------
TEST(ListTests, RemoveRange_Middle) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3); lst.Add(4); lst.Add(5);
    lst.RemoveRange(1, 3);
    EXPECT_EQ(lst.getCountProperty(), 2);
    EXPECT_EQ(lst[0], 1);
    EXPECT_EQ(lst[1], 5);
}

// ---------------------------------------------------------------------------
// Reverse(int, int)
// ---------------------------------------------------------------------------
TEST(ListTests, Reverse_Subrange) {
    List<int> lst;
    lst.Add(1); lst.Add(2); lst.Add(3); lst.Add(4); lst.Add(5);
    lst.Reverse(1, 3);
    EXPECT_EQ(lst[0], 1);
    EXPECT_EQ(lst[1], 4);
    EXPECT_EQ(lst[2], 3);
    EXPECT_EQ(lst[3], 2);
    EXPECT_EQ(lst[4], 5);
}
