// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Regression tests for ticket 1713 (POST_STABILIZATION_AUDIT.md finding #4): SortedDictionary,
// SortedSet, and OrderedDictionary's enumerators must detect structural modification during
// iteration and throw System::InvalidOperationException, matching real .NET's fail-fast
// enumerator contract. (PriorityQueue<TElement,TPriority> has no enumeration surface at all, so
// this bug does not apply to it -- verified during this ticket's audit, not covered here.)
#include <gtest/gtest.h>

#include "System/InvalidOperationException.hpp"
#include "System/Collections/Generic/SortedDictionary.hpp"
#include "System/Collections/Generic/SortedSet.hpp"
#include "System/Collections/Generic/OrderedDictionary.hpp"

using namespace System::Collections::Generic;

// ---------------------------------------------------------------------------
// SortedDictionary<TKey,TValue>
// ---------------------------------------------------------------------------

TEST(SortedDictionaryVersionTrackingTests, Enumeration_NoModification_Succeeds) {
    SortedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20); d.Add(3, 30);
    int sum = 0;
    for (const auto& kv : d) sum += kv.second;
    EXPECT_EQ(sum, 60);
}

TEST(SortedDictionaryVersionTrackingTests, Enumeration_AddDuringIteration_Throws) {
    SortedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20);
    EXPECT_THROW({
        for (const auto& kv : d) { (void)kv; d.Add(99, 999); }
    }, System::InvalidOperationException);
}

TEST(SortedDictionaryVersionTrackingTests, Enumeration_RemoveDuringIteration_Throws) {
    SortedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20); d.Add(3, 30);
    EXPECT_THROW({
        for (const auto& kv : d) { (void)kv; d.Remove(2); }
    }, System::InvalidOperationException);
}

TEST(SortedDictionaryVersionTrackingTests, Enumeration_ClearDuringIteration_Throws) {
    SortedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20);
    EXPECT_THROW({
        for (const auto& kv : d) { (void)kv; d.Clear(); }
    }, System::InvalidOperationException);
}

TEST(SortedDictionaryVersionTrackingTests, Enumeration_IndexerSetNewKeyDuringIteration_Throws) {
    SortedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20);
    EXPECT_THROW({
        for (const auto& kv : d) { (void)kv; d[99] = 999; }
    }, System::InvalidOperationException);
}

TEST(SortedDictionaryVersionTrackingTests, Enumeration_IndexerUpdateExistingKey_DoesNotThrow) {
    // Updating an EXISTING key's value is not a structural change -- must not false-positive.
    SortedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20);
    EXPECT_NO_THROW({
        for (const auto& kv : d) { (void)kv; d[1] = 100; }
    });
}

// ---------------------------------------------------------------------------
// SortedSet<T>
// ---------------------------------------------------------------------------

TEST(SortedSetVersionTrackingTests, Enumeration_NoModification_Succeeds) {
    SortedSet<int> s;
    s.Add(1); s.Add(2); s.Add(3);
    int sum = 0;
    for (const auto& x : s) sum += x;
    EXPECT_EQ(sum, 6);
}

TEST(SortedSetVersionTrackingTests, Enumeration_AddDuringIteration_Throws) {
    SortedSet<int> s;
    s.Add(1); s.Add(2);
    EXPECT_THROW({
        for (const auto& x : s) { (void)x; s.Add(99); }
    }, System::InvalidOperationException);
}

TEST(SortedSetVersionTrackingTests, Enumeration_RemoveDuringIteration_Throws) {
    SortedSet<int> s;
    s.Add(1); s.Add(2); s.Add(3);
    EXPECT_THROW({
        for (const auto& x : s) { (void)x; s.Remove(2); }
    }, System::InvalidOperationException);
}

TEST(SortedSetVersionTrackingTests, Enumeration_ClearDuringIteration_Throws) {
    SortedSet<int> s;
    s.Add(1); s.Add(2);
    EXPECT_THROW({
        for (const auto& x : s) { (void)x; s.Clear(); }
    }, System::InvalidOperationException);
}

TEST(SortedSetVersionTrackingTests, GetViewBetween_StillWorks_AfterVersioningChange) {
    // GetViewBetween's internal snapshot copy uses raw std::set iteration on `data_`, not the
    // versioned public Iterator -- confirm it's unaffected by the version-tracking addition.
    SortedSet<int> s;
    s.Add(1); s.Add(2); s.Add(3); s.Add(4);
    auto view = s.GetViewBetween(2, 3);
    EXPECT_EQ(view.getCountProperty(), 2);
    EXPECT_TRUE(view.Contains(2));
    EXPECT_TRUE(view.Contains(3));
}

// ---------------------------------------------------------------------------
// OrderedDictionary<TKey,TValue>
// ---------------------------------------------------------------------------

TEST(OrderedDictionaryVersionTrackingTests, Enumeration_NoModification_Succeeds) {
    OrderedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20); d.Add(3, 30);
    int sum = 0;
    for (const auto& kv : d) sum += kv.second;
    EXPECT_EQ(sum, 60);
}

TEST(OrderedDictionaryVersionTrackingTests, Enumeration_AddDuringIteration_Throws) {
    OrderedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20);
    EXPECT_THROW({
        for (const auto& kv : d) { (void)kv; d.Add(99, 999); }
    }, System::InvalidOperationException);
}

TEST(OrderedDictionaryVersionTrackingTests, Enumeration_RemoveDuringIteration_Throws) {
    OrderedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20); d.Add(3, 30);
    EXPECT_THROW({
        for (const auto& kv : d) { (void)kv; d.Remove(2); }
    }, System::InvalidOperationException);
}

TEST(OrderedDictionaryVersionTrackingTests, Enumeration_RemoveAtDuringIteration_Throws) {
    OrderedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20); d.Add(3, 30);
    EXPECT_THROW({
        for (const auto& kv : d) { (void)kv; d.RemoveAt(0); }
    }, System::InvalidOperationException);
}

TEST(OrderedDictionaryVersionTrackingTests, Enumeration_InsertDuringIteration_Throws) {
    OrderedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20);
    EXPECT_THROW({
        for (const auto& kv : d) { (void)kv; d.Insert(0, 99, 999); }
    }, System::InvalidOperationException);
}

TEST(OrderedDictionaryVersionTrackingTests, Enumeration_ClearDuringIteration_Throws) {
    OrderedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20);
    EXPECT_THROW({
        for (const auto& kv : d) { (void)kv; d.Clear(); }
    }, System::InvalidOperationException);
}

TEST(OrderedDictionaryVersionTrackingTests, Enumeration_ManyAdds_ReallocationSafe) {
    // Regression for the specific "vector reallocation dangles a raw iterator" failure mode
    // documented in the audit -- the index-based Iterator design must survive many additions
    // (guaranteed reallocation) inside a caught exception without crashing under ASan.
    OrderedDictionary<int, int> d;
    for (int i = 0; i < 4; i++) d.Add(i, i * 10);
    EXPECT_THROW({
        for (const auto& kv : d) {
            (void)kv;
            for (int i = 4; i < 200; i++) d.Add(i, i * 10);  // forces reallocation
        }
    }, System::InvalidOperationException);
    EXPECT_EQ(d.getCountProperty(), 200);
}

// Regression test (fixed 2026-07-14, duplicated-implementation audit finding): EnsureCapacity
// previously never bumped version_ at all, unlike its sibling Dictionary::EnsureCapacity and
// real .NET's own OrderedDictionary.EnsureCapacity (verified against OrderedDictionary.cs: bumps
// _version only when capacity actually grows, inside the `if (Capacity < capacity)` branch).
TEST(OrderedDictionaryVersionTrackingTests, Enumeration_EnsureCapacityGrows_DuringIteration_Throws) {
    OrderedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20);
    EXPECT_THROW({
        for (const auto& kv : d) { (void)kv; d.EnsureCapacity(1000); }
    }, System::InvalidOperationException);
}

TEST(OrderedDictionaryVersionTrackingTests, EnsureCapacity_NoActualGrowth_DoesNotBumpVersion) {
    // Matching real .NET exactly: EnsureCapacity(capacity) that doesn't actually need to grow
    // must NOT invalidate a live enumerator -- only genuine growth is a structural mutation.
    OrderedDictionary<int, int> d;
    d.Add(1, 10); d.Add(2, 20);
    d.EnsureCapacity(1000); // grows once, establishing a large capacity
    int sum = 0;
    for (const auto& kv : d) {
        sum += kv.second;
        d.EnsureCapacity(1); // already satisfied -- must be a no-op, no throw expected
    }
    EXPECT_EQ(sum, 30);
}
