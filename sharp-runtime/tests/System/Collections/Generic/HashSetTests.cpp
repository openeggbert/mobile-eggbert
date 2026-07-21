// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Generic/HashSet.hpp"
#include "System/InvalidOperationException.hpp"

using System::Collections::Generic::HashSet;
using SharpRuntime::intcs;

TEST(HashSetTest, DefaultEmpty) {
    HashSet<int> s;
    EXPECT_EQ(s.getCountProperty(), 0);
}

TEST(HashSetTest, AddAndContains) {
    HashSet<int> s;
    EXPECT_TRUE(s.Add(1));
    EXPECT_TRUE(s.Contains(1));
}

TEST(HashSetTest, AddDuplicateReturnsFalse) {
    HashSet<int> s;
    s.Add(5);
    EXPECT_FALSE(s.Add(5));
    EXPECT_EQ(s.getCountProperty(), 1);
}

TEST(HashSetTest, Remove) {
    HashSet<int> s;
    s.Add(3);
    EXPECT_TRUE(s.Remove(3));
    EXPECT_FALSE(s.Contains(3));
}

TEST(HashSetTest, RemoveMissingReturnsFalse) {
    HashSet<int> s;
    EXPECT_FALSE(s.Remove(99));
}

TEST(HashSetTest, Clear) {
    HashSet<int> s;
    s.Add(1); s.Add(2);
    s.Clear();
    EXPECT_EQ(s.getCountProperty(), 0);
}

TEST(HashSetTest, UnionWith) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(2); b.Add(3);
    a.UnionWith(b);
    EXPECT_EQ(a.getCountProperty(), 3);
    EXPECT_TRUE(a.Contains(3));
}

TEST(HashSetTest, IntersectWith) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2); a.Add(3);
    b.Add(2); b.Add(3); b.Add(4);
    a.IntersectWith(b);
    EXPECT_EQ(a.getCountProperty(), 2);
    EXPECT_FALSE(a.Contains(1));
    EXPECT_TRUE(a.Contains(2));
}

TEST(HashSetTest, ExceptWith) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2); a.Add(3);
    b.Add(2); b.Add(3);
    a.ExceptWith(b);
    EXPECT_EQ(a.getCountProperty(), 1);
    EXPECT_TRUE(a.Contains(1));
}

TEST(HashSetTest, SymmetricExceptWith) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(2); b.Add(3);
    a.SymmetricExceptWith(b);
    EXPECT_TRUE(a.Contains(1));
    EXPECT_FALSE(a.Contains(2));
    EXPECT_TRUE(a.Contains(3));
}

// Regression tests for ticket 324: ExceptWith/SymmetricExceptWith erased from set_ while
// iterating other.set_ with no guard for `other` aliasing `this` (e.g. s.ExceptWith(s)) -- when
// aliased, this erases from the exact same std::unordered_set being iterated, which is a real,
// confirmed heap-use-after-free (ASan). Real .NET explicitly special-cases `other == this` in
// both methods ("a set minus/symmetric-differenced with itself is the empty set"); the port had
// no such check for either.
TEST(HashSetTest, ExceptWith_Self_ClearsSet) {
    HashSet<int> a;
    for (int i = 0; i < 200; ++i) a.Add(i); // enough elements to make iterator invalidation likely to crash under ASan
    a.ExceptWith(a);
    EXPECT_EQ(a.getCountProperty(), 0);
}

TEST(HashSetTest, SymmetricExceptWith_Self_ClearsSet) {
    HashSet<int> a;
    for (int i = 0; i < 200; ++i) a.Add(i);
    a.SymmetricExceptWith(a);
    EXPECT_EQ(a.getCountProperty(), 0);
}

TEST(HashSetTest, SetEquals) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(2); b.Add(1);
    EXPECT_TRUE(a.SetEquals(b));
}

TEST(HashSetTest, IsSubsetOf) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(1); b.Add(2); b.Add(3);
    EXPECT_TRUE(a.IsSubsetOf(b));
    EXPECT_FALSE(b.IsSubsetOf(a));
}

TEST(HashSetTest, IsSupersetOf) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2); a.Add(3);
    b.Add(1); b.Add(2);
    EXPECT_TRUE(a.IsSupersetOf(b));
    EXPECT_FALSE(b.IsSupersetOf(a));
}

TEST(HashSetTest, IsProperSubsetOf) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(1); b.Add(2); b.Add(3);
    EXPECT_TRUE(a.IsProperSubsetOf(b));
    EXPECT_FALSE(b.IsProperSubsetOf(a));
}

TEST(HashSetTest, ToArray) {
    HashSet<int> s;
    s.Add(1); s.Add(2);
    auto arr = s.ToArray();
    EXPECT_EQ(static_cast<int>(arr.size()), 2);
}

TEST(HashSetTest, RangeBasedFor) {
    HashSet<int> s;
    s.Add(10); s.Add(20); s.Add(30);
    int sum = 0;
    for (const auto& v : s) sum += v;
    EXPECT_EQ(sum, 60);
}

TEST(HashSetTest, TryGetValue_Found) {
    HashSet<int> s;
    s.Add(5);
    int actual = 0;
    EXPECT_TRUE(s.TryGetValue(5, actual));
    EXPECT_EQ(actual, 5);
}

TEST(HashSetTest, TryGetValue_NotFound) {
    HashSet<int> s;
    int actual = 0;
    EXPECT_FALSE(s.TryGetValue(5, actual));
}

TEST(HashSetTest, RemoveWhere_RemovesMatching) {
    HashSet<int> s;
    s.Add(1); s.Add(2); s.Add(3); s.Add(4);
    intcs removed = s.RemoveWhere([](const int& v) { return v % 2 == 0; });
    EXPECT_EQ(removed, 2);
    EXPECT_TRUE(s.Contains(1));
    EXPECT_FALSE(s.Contains(2));
    EXPECT_TRUE(s.Contains(3));
    EXPECT_FALSE(s.Contains(4));
}

TEST(HashSetTest, Overlaps_True) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(2); b.Add(3);
    EXPECT_TRUE(a.Overlaps(b));
}

TEST(HashSetTest, Overlaps_False) {
    HashSet<int> a, b;
    a.Add(1); a.Add(2);
    b.Add(3); b.Add(4);
    EXPECT_FALSE(a.Overlaps(b));
}

// ---- begin()/end() version-tracking (ticket 1713, post-stabilization-audit finding #4) ----
TEST(HashSetTest, Iteration_NoModification_DoesNotThrow) {
    HashSet<int> s;
    s.Add(1); s.Add(2); s.Add(3);
    int count = 0;
    EXPECT_NO_THROW({ for (auto& x : s) { (void)x; count++; } });
    EXPECT_EQ(count, 3);
}

TEST(HashSetTest, Iteration_AddDuringIteration_Throws) {
    HashSet<int> s;
    s.Add(1); s.Add(2);
    EXPECT_THROW({
        for (auto& x : s) { (void)x; s.Add(99); }
    }, System::InvalidOperationException);
}

TEST(HashSetTest, Iteration_RemoveDuringIteration_Throws) {
    // Deliberate deviation from real .NET's HashSet.Remove (does not bump _version) for memory
    // safety: erasing the element a std::unordered_set iterator currently points at invalidates
    // that iterator, matching the same fix applied to Dictionary<TKey,TValue> for ticket 1713.
    HashSet<int> s;
    s.Add(1); s.Add(2); s.Add(3);
    EXPECT_THROW({
        for (auto& x : s) { (void)x; s.Remove(3); }
    }, System::InvalidOperationException);
}

TEST(HashSetTest, Iteration_ClearDuringIteration_Throws) {
    HashSet<int> s;
    s.Add(1); s.Add(2);
    EXPECT_THROW({
        for (auto& x : s) { (void)x; s.Clear(); }
    }, System::InvalidOperationException);
}
