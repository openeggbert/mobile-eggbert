// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Generic/Queue.hpp"
#include "System/Collections/Generic/PriorityQueue.hpp"
#include "System/Collections/Generic/OrderedDictionary.hpp"
#include "System/Collections/Generic/ReferenceEqualityComparer.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include <stdexcept>
#include <string>
#include <vector>

using namespace System::Collections::Generic;

// ---- Queue<int> ----
TEST(GenQueueTest, EnqueueDequeue) {
    Queue<int> q;
    q.Enqueue(1); q.Enqueue(2); q.Enqueue(3);
    EXPECT_EQ(q.Dequeue(), 1);
    EXPECT_EQ(q.Dequeue(), 2);
    EXPECT_EQ(q.getCountProperty(), 1);
}

TEST(GenQueueTest, Peek) {
    Queue<int> q;
    q.Enqueue(42);
    EXPECT_EQ(q.Peek(), 42);
    EXPECT_EQ(q.getCountProperty(), 1);
}

TEST(GenQueueTest, TryDequeue) {
    Queue<int> q;
    q.Enqueue(7);
    int v = 0;
    EXPECT_TRUE(q.TryDequeue(v));
    EXPECT_EQ(v, 7);
    EXPECT_FALSE(q.TryDequeue(v));
}

TEST(GenQueueTest, TryPeek) {
    Queue<int> q;
    int v = 0;
    EXPECT_FALSE(q.TryPeek(v));
    q.Enqueue(99);
    EXPECT_TRUE(q.TryPeek(v));
    EXPECT_EQ(v, 99);
    EXPECT_EQ(q.getCountProperty(), 1);
}

TEST(GenQueueTest, Contains) {
    Queue<int> q;
    q.Enqueue(1); q.Enqueue(2);
    EXPECT_TRUE(q.Contains(2));
    EXPECT_FALSE(q.Contains(99));
}

TEST(GenQueueTest, Clear) {
    Queue<int> q;
    q.Enqueue(1); q.Enqueue(2);
    q.Clear();
    EXPECT_EQ(q.getCountProperty(), 0);
}

TEST(GenQueueTest, ToArray) {
    Queue<int> q;
    q.Enqueue(10); q.Enqueue(20); q.Enqueue(30);
    auto v = q.ToArray();
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[2], 30);
}

TEST(GenQueueTest, DequeueEmptyThrows) {
    Queue<int> q;
    EXPECT_THROW(q.Dequeue(), System::InvalidOperationException);
}

// ---- PriorityQueue<string,int> ----
TEST(GenPriorityQueueTest, EnqueueDequeueMinFirst) {
    PriorityQueue<std::string, int> pq;
    pq.Enqueue("low", 10);
    pq.Enqueue("high", 1);
    pq.Enqueue("mid", 5);
    EXPECT_EQ(pq.Dequeue(), "high");
    EXPECT_EQ(pq.Dequeue(), "mid");
    EXPECT_EQ(pq.Dequeue(), "low");
}

TEST(GenPriorityQueueTest, Count) {
    PriorityQueue<int, int> pq;
    pq.Enqueue(1, 1); pq.Enqueue(2, 2);
    EXPECT_EQ(pq.getCountProperty(), 2);
}

TEST(GenPriorityQueueTest, Peek) {
    PriorityQueue<int, int> pq;
    pq.Enqueue(5, 10); pq.Enqueue(3, 1);
    EXPECT_EQ(pq.Peek(), 3);
    EXPECT_EQ(pq.getCountProperty(), 2);
}

TEST(GenPriorityQueueTest, TryDequeue) {
    PriorityQueue<int, int> pq;
    pq.Enqueue(7, 2);
    int el = 0, pri = 0;
    EXPECT_TRUE(pq.TryDequeue(el, pri));
    EXPECT_EQ(el, 7);
    EXPECT_EQ(pri, 2);
    EXPECT_FALSE(pq.TryDequeue(el, pri));
}

TEST(GenPriorityQueueTest, TryPeek) {
    PriorityQueue<int, int> pq;
    int el = 0, pri = 0;
    EXPECT_FALSE(pq.TryPeek(el, pri));
    pq.Enqueue(42, 1);
    EXPECT_TRUE(pq.TryPeek(el, pri));
    EXPECT_EQ(el, 42);
}

TEST(GenPriorityQueueTest, EnqueueRange) {
    PriorityQueue<int, int> pq;
    pq.EnqueueRange({{3, 3}, {1, 1}, {2, 2}});
    EXPECT_EQ(pq.Dequeue(), 1);
}

TEST(GenPriorityQueueTest, Clear) {
    PriorityQueue<int, int> pq;
    pq.Enqueue(1, 1); pq.Enqueue(2, 2);
    pq.Clear();
    EXPECT_EQ(pq.getCountProperty(), 0);
}

// ---- OrderedDictionary<string,int> ----
TEST(OrderedDictionaryTest, AddAndContainsKey) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1); d.Add("b", 2);
    EXPECT_TRUE(d.ContainsKey("a"));
    EXPECT_FALSE(d.ContainsKey("z"));
}

TEST(OrderedDictionaryTest, PreservesInsertionOrder) {
    OrderedDictionary<std::string, int> d;
    d.Add("x", 10); d.Add("y", 20); d.Add("z", 30);
    auto kv0 = d.GetAt(0); auto kv1 = d.GetAt(1); auto kv2 = d.GetAt(2);
    EXPECT_EQ(kv0.Key, "x");
    EXPECT_EQ(kv1.Key, "y");
    EXPECT_EQ(kv2.Key, "z");
}

TEST(OrderedDictionaryTest, IndexOf) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1); d.Add("b", 2);
    EXPECT_EQ(d.IndexOf("b"), 1);
    EXPECT_EQ(d.IndexOf("z"), -1);
}

TEST(OrderedDictionaryTest, TryGetValue) {
    OrderedDictionary<std::string, int> d;
    d.Add("k", 42);
    int v = 0;
    EXPECT_TRUE(d.TryGetValue("k", v));
    EXPECT_EQ(v, 42);
    EXPECT_FALSE(d.TryGetValue("missing", v));
}

TEST(OrderedDictionaryTest, TryAdd) {
    OrderedDictionary<std::string, int> d;
    EXPECT_TRUE(d.TryAdd("a", 1));
    EXPECT_FALSE(d.TryAdd("a", 2));
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(OrderedDictionaryTest, Remove) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1); d.Add("b", 2); d.Add("c", 3);
    EXPECT_TRUE(d.Remove("b"));
    EXPECT_EQ(d.getCountProperty(), 2);
    EXPECT_EQ(d.IndexOf("c"), 1);
}

TEST(OrderedDictionaryTest, RemoveAt) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1); d.Add("b", 2);
    d.RemoveAt(0);
    EXPECT_EQ(d.getCountProperty(), 1);
    EXPECT_EQ(d.GetAt(0).Key, "b");
}

TEST(OrderedDictionaryTest, SetAt) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1);
    d.SetAt(0, 99);
    EXPECT_EQ(d.GetAt(0).Value, 99);
}

TEST(OrderedDictionaryTest, Insert) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1); d.Add("c", 3);
    d.Insert(1, "b", 2);
    EXPECT_EQ(d.GetAt(1).Key, "b");
    EXPECT_EQ(d.getCountProperty(), 3);
}

TEST(OrderedDictionaryTest, AddDuplicateThrows) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1);
    EXPECT_THROW(d.Add("a", 2), System::ArgumentException);
}

namespace {
// A value type whose copy constructor throws on demand -- used to simulate Add()/operator[]
// failing partway through inserting a new entry (e.g. a resource-validating TValue).
struct ThrowsOnSecondCopy {
    static inline int copyCount = 0;
    static inline bool armed = false;
    int v = 0;
    ThrowsOnSecondCopy() = default;
    explicit ThrowsOnSecondCopy(int x) : v(x) {}
    ThrowsOnSecondCopy(const ThrowsOnSecondCopy& o) : v(o.v) {
        if (armed && ++copyCount == 2) throw std::runtime_error("simulated copy failure");
    }
    ThrowsOnSecondCopy& operator=(const ThrowsOnSecondCopy&) = default;
};
}

// Regression test for ticket 310: Add()/TryAdd()/operator[] previously set keyIndex_[key] to
// the new entry's index BEFORE entries_.emplace_back() actually added it. If emplace_back threw
// (e.g. TKey/TValue's copy constructor throwing), keyIndex_ was left pointing one-past-the-end
// of entries_ -- confirmed via a standalone ASan repro as a genuine heap-buffer-overflow on the
// next lookup of that key (operator[] doesn't bounds-check entries_[it->second]). Fixed by
// mutating entries_ before keyIndex_, so a failed Add leaves the dictionary fully consistent.
TEST(OrderedDictionaryTest, Add_ThrowingValueCopy_LeavesDictionaryConsistent) {
    OrderedDictionary<std::string, ThrowsOnSecondCopy> d;
    ThrowsOnSecondCopy::copyCount = 0;
    ThrowsOnSecondCopy::armed = false;
    d.Add("a", ThrowsOnSecondCopy(1));  // 1st copy (into emplace_back) -- succeeds
    ThrowsOnSecondCopy::armed = true;
    EXPECT_THROW(d.Add("b", ThrowsOnSecondCopy(2)), std::runtime_error);  // 2nd copy -- throws
    ThrowsOnSecondCopy::armed = false;

    EXPECT_EQ(d.getCountProperty(), 1);
    EXPECT_FALSE(d.ContainsKey("b"));
    EXPECT_EQ(d.IndexOf("b"), -1);
    // The dictionary must remain fully usable -- no corrupted keyIndex_ entries.
    EXPECT_TRUE(d.ContainsKey("a"));
    EXPECT_NO_THROW(d.Add("c", ThrowsOnSecondCopy(3)));
    EXPECT_EQ(d.getCountProperty(), 2);
}

namespace {
// A key type that can be armed to throw either on its SECOND copy construction (simulating a
// throwing key copy made when unordered_map constructs a new node) or when hashing a specific
// "poisoned" key value (simulating a throwing std::hash<TKey>) -- audit finding A-05,
// 2026-07-14. The prior fix above only protected against a throwing TKey/TValue copy made
// BEFORE entries_ is mutated (ticket 310); this protects the OTHER direction: entries_ is
// mutated first (an established, deliberate ordering -- see Add()'s own comment), so a throw
// from the map-side keyIndex_ update (Add/TryAdd/operator[]'s new-key path) or from
// rebuildIndex() (Insert/Remove/RemoveAt) used to leave keyIndex_ out of sync with entries_,
// confirmed via a standalone repro matching the audit's own: Count reported one more element
// than ContainsKey() could ever find.
struct FaultInjectingKey {
    static inline bool copyArmed = false;
    static inline int copyCount = 0;
    // Which copy (1-based, counted from when copyArmed is turned on) should throw. Different
    // call paths make a different number of copies before reaching the map-side operation
    // under test: Add()/TryAdd() copy once into entries_ then once into keyIndex_ (throw on
    // the 2nd), while `d[key] = value` additionally copies once into the ValueProxy itself
    // before that (throw on the 3rd) -- each test sets this to match its own call path.
    static inline int throwOnCopyNumber = 2;
    static inline std::string poisonedValue; // hashing this value throws when non-empty

    std::string s;
    FaultInjectingKey() = default;
    explicit FaultInjectingKey(std::string v) : s(std::move(v)) {}
    FaultInjectingKey(const FaultInjectingKey& o) : s(o.s) {
        if (copyArmed && ++copyCount == throwOnCopyNumber) throw std::runtime_error("simulated key copy failure");
    }
    FaultInjectingKey& operator=(const FaultInjectingKey&) = default;
    bool operator==(const FaultInjectingKey& o) const { return s == o.s; }
};
} // namespace

namespace std {
template<> struct hash<FaultInjectingKey> {
    std::size_t operator()(const FaultInjectingKey& k) const {
        if (!FaultInjectingKey::poisonedValue.empty() && k.s == FaultInjectingKey::poisonedValue)
            throw std::runtime_error("simulated hash failure");
        return std::hash<std::string>{}(k.s);
    }
};
} // namespace std

TEST(OrderedDictionaryTest, Add_ThrowingKeyCopy_LeavesDictionaryConsistent) {
    FaultInjectingKey::copyArmed = false;
    FaultInjectingKey::copyCount = 0;
    FaultInjectingKey::poisonedValue.clear();
    // Reserved capacity up front so entries_/keyIndex_ never need to grow mid-test -- growth
    // would relocate EXISTING elements (via K's copy constructor, since it has no noexcept
    // move constructor) and could spuriously trip the fault injection on the wrong key,
    // confirmed via a standalone diagnostic before this fix.
    OrderedDictionary<FaultInjectingKey, int> d(8);
    d.Add(FaultInjectingKey("a"), 1);

    FaultInjectingKey::copyArmed = true;
    FaultInjectingKey::copyCount = 0;
    // 1st copy of "b" is entries_.emplace_back's own stored key (succeeds); 2nd is the copy
    // unordered_map makes constructing a node for a not-yet-present key in keyIndex_ (throws).
    EXPECT_THROW(d.Add(FaultInjectingKey("b"), 2), std::runtime_error);
    FaultInjectingKey::copyArmed = false;

    EXPECT_EQ(d.getCountProperty(), 1);
    EXPECT_FALSE(d.ContainsKey(FaultInjectingKey("b")));
    EXPECT_EQ(d.IndexOf(FaultInjectingKey("b")), -1);
    EXPECT_TRUE(d.ContainsKey(FaultInjectingKey("a")));
    EXPECT_NO_THROW(d.Add(FaultInjectingKey("c"), 3));
    EXPECT_EQ(d.getCountProperty(), 2);
}

TEST(OrderedDictionaryTest, TryAdd_ThrowingKeyCopy_LeavesDictionaryConsistent) {
    FaultInjectingKey::copyArmed = false;
    FaultInjectingKey::copyCount = 0;
    FaultInjectingKey::poisonedValue.clear();
    // Reserved capacity up front so entries_/keyIndex_ never need to grow mid-test -- growth
    // would relocate EXISTING elements (via K's copy constructor, since it has no noexcept
    // move constructor) and could spuriously trip the fault injection on the wrong key,
    // confirmed via a standalone diagnostic before this fix.
    OrderedDictionary<FaultInjectingKey, int> d(8);
    d.Add(FaultInjectingKey("a"), 1);

    FaultInjectingKey::copyArmed = true;
    FaultInjectingKey::copyCount = 0;
    EXPECT_THROW(d.TryAdd(FaultInjectingKey("b"), 2), std::runtime_error);
    FaultInjectingKey::copyArmed = false;

    EXPECT_EQ(d.getCountProperty(), 1);
    EXPECT_FALSE(d.ContainsKey(FaultInjectingKey("b")));
}

TEST(OrderedDictionaryTest, IndexerSetter_ThrowingKeyCopy_LeavesDictionaryConsistent) {
    FaultInjectingKey::copyArmed = false;
    FaultInjectingKey::copyCount = 0;
    FaultInjectingKey::poisonedValue.clear();
    // Reserved capacity up front so entries_/keyIndex_ never need to grow mid-test -- growth
    // would relocate EXISTING elements (via K's copy constructor, since it has no noexcept
    // move constructor) and could spuriously trip the fault injection on the wrong key,
    // confirmed via a standalone diagnostic before this fix.
    OrderedDictionary<FaultInjectingKey, int> d(8);
    d.Add(FaultInjectingKey("a"), 1);

    FaultInjectingKey::copyArmed = true;
    FaultInjectingKey::copyCount = 0;
    // `d[key] = value` makes an extra copy (into the ValueProxy itself) before the two copies
    // Add() makes, so the map-side insertion this test targets is the 3rd copy, not the 2nd.
    FaultInjectingKey::throwOnCopyNumber = 3;
    EXPECT_THROW(d[FaultInjectingKey("b")] = 2, std::runtime_error);
    FaultInjectingKey::copyArmed = false;
    FaultInjectingKey::throwOnCopyNumber = 2;

    EXPECT_EQ(d.getCountProperty(), 1);
    EXPECT_FALSE(d.ContainsKey(FaultInjectingKey("b")));
}

TEST(OrderedDictionaryTest, Insert_ThrowingHash_RollsBackInsertion) {
    FaultInjectingKey::copyArmed = false;
    FaultInjectingKey::poisonedValue.clear();
    // Reserved capacity up front so entries_/keyIndex_ never need to grow mid-test -- growth
    // would relocate EXISTING elements (via K's copy constructor, since it has no noexcept
    // move constructor) and could spuriously trip the fault injection on the wrong key,
    // confirmed via a standalone diagnostic before this fix.
    OrderedDictionary<FaultInjectingKey, int> d(8);
    d.Add(FaultInjectingKey("a"), 1);
    d.Add(FaultInjectingKey("c"), 3);

    // Poison an EXISTING key ("a"), not the one being inserted ("b"): Insert()'s own
    // duplicate-key check only hashes "b" (unaffected), so the poisoned hash fires only once
    // rebuildIndex() re-hashes every entry afterward, including "a".
    FaultInjectingKey::poisonedValue = "a";
    EXPECT_THROW(d.Insert(1, FaultInjectingKey("b"), 2), std::runtime_error);
    FaultInjectingKey::poisonedValue.clear();

    EXPECT_EQ(d.getCountProperty(), 2);
    EXPECT_FALSE(d.ContainsKey(FaultInjectingKey("b")));
    EXPECT_EQ(d.IndexOf(FaultInjectingKey("a")), 0);
    EXPECT_EQ(d.IndexOf(FaultInjectingKey("c")), 1);
}

TEST(OrderedDictionaryTest, Remove_ThrowingHash_RollsBackRemoval) {
    FaultInjectingKey::copyArmed = false;
    FaultInjectingKey::poisonedValue.clear();
    // Reserved capacity up front so entries_/keyIndex_ never need to grow mid-test -- growth
    // would relocate EXISTING elements (via K's copy constructor, since it has no noexcept
    // move constructor) and could spuriously trip the fault injection on the wrong key,
    // confirmed via a standalone diagnostic before this fix.
    OrderedDictionary<FaultInjectingKey, int> d(8);
    d.Add(FaultInjectingKey("a"), 1);
    d.Add(FaultInjectingKey("b"), 2);

    FaultInjectingKey::poisonedValue = "b";
    EXPECT_THROW(d.Remove(FaultInjectingKey("a")), std::runtime_error);
    FaultInjectingKey::poisonedValue.clear();

    EXPECT_EQ(d.getCountProperty(), 2);
    EXPECT_TRUE(d.ContainsKey(FaultInjectingKey("a")));
    EXPECT_EQ(d.IndexOf(FaultInjectingKey("a")), 0);
    EXPECT_TRUE(d.ContainsKey(FaultInjectingKey("b")));
}

TEST(OrderedDictionaryTest, RemoveAt_ThrowingHash_RollsBackRemoval) {
    FaultInjectingKey::copyArmed = false;
    FaultInjectingKey::poisonedValue.clear();
    // Reserved capacity up front so entries_/keyIndex_ never need to grow mid-test -- growth
    // would relocate EXISTING elements (via K's copy constructor, since it has no noexcept
    // move constructor) and could spuriously trip the fault injection on the wrong key,
    // confirmed via a standalone diagnostic before this fix.
    OrderedDictionary<FaultInjectingKey, int> d(8);
    d.Add(FaultInjectingKey("a"), 1);
    d.Add(FaultInjectingKey("b"), 2);

    FaultInjectingKey::poisonedValue = "b";
    EXPECT_THROW(d.RemoveAt(0), std::runtime_error);
    FaultInjectingKey::poisonedValue.clear();

    EXPECT_EQ(d.getCountProperty(), 2);
    EXPECT_TRUE(d.ContainsKey(FaultInjectingKey("a")));
    EXPECT_EQ(d.IndexOf(FaultInjectingKey("a")), 0);
}

TEST(OrderedDictionaryTest, IndexerConstMissingKeyThrows) {
    OrderedDictionary<std::string, int> d;
    const auto& cd = d;
    EXPECT_THROW((void)cd["missing"], KeyNotFoundException);
}

// Regression tests (fixed 2026-07-14, duplicated-implementation audit finding): the non-const
// operator[] used to return a plain TValue&, which could not distinguish a read from a write --
// so reading a missing key through a non-const OrderedDictionary silently inserted a
// default-constructed value instead of throwing, unlike the const overload above and every
// sibling dictionary type (Dictionary/SortedDictionary/SortedList/ConcurrentDictionary).

TEST(OrderedDictionaryTest, IndexerNonConstMissingKeyRead_ThrowsAndDoesNotInsert) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1);
    // A plain (void)d["missing"] would NOT trigger the throw here: operator[] returns a
    // ValueProxy BY VALUE (not a reference), and the lookup/throw only happens inside its
    // `operator const TValue&()` conversion -- a void-cast discards the ValueProxy temporary
    // without ever converting it. Must actually convert to TValue to exercise the read path,
    // e.g. via assignment, matching how a real caller would read the value.
    EXPECT_THROW({ int unused = d["missing"]; (void)unused; }, KeyNotFoundException);
    // The failed read must not have inserted anything.
    EXPECT_EQ(d.getCountProperty(), 1);
    EXPECT_FALSE(d.ContainsKey("missing"));
}

TEST(OrderedDictionaryTest, IndexerNonConstExistingKeyRead_ReturnsValueWithoutInserting) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 42);
    int value = d["a"];
    EXPECT_EQ(value, 42);
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(OrderedDictionaryTest, IndexerSetter_InsertsNewKeyAtEnd) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1);
    d["b"] = 2;
    EXPECT_EQ(d.getCountProperty(), 2);
    EXPECT_EQ(static_cast<int>(d["b"]), 2);
    EXPECT_EQ(d.GetAt(1).Value, 2);
}

TEST(OrderedDictionaryTest, IndexerSetter_OverwritesExistingKeyInPlace) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1);
    d.Add("b", 2);
    d["a"] = 99;
    EXPECT_EQ(d.getCountProperty(), 2);
    EXPECT_EQ(static_cast<int>(d["a"]), 99);
    // Order must be unchanged -- overwriting an existing key doesn't move it.
    EXPECT_EQ(d.GetAt(0).Key, "a");
    EXPECT_EQ(d.GetAt(1).Key, "b");
}

TEST(OrderedDictionaryTest, GetAtOutOfRangeThrows) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1);
    EXPECT_THROW(d.GetAt(1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(d.GetAt(-1), System::ArgumentOutOfRangeException);
}

TEST(OrderedDictionaryTest, RemoveAtOutOfRangeThrows) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1);
    EXPECT_THROW(d.RemoveAt(1), System::ArgumentOutOfRangeException);
}

TEST(OrderedDictionaryTest, InsertOutOfRangeThrows) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1);
    EXPECT_THROW(d.Insert(5, "b", 2), System::ArgumentOutOfRangeException);
}

TEST(OrderedDictionaryTest, ContainsValue) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 42);
    EXPECT_TRUE(d.ContainsValue(42));
    EXPECT_FALSE(d.ContainsValue(0));
}

TEST(OrderedDictionaryTest, Clear) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1);
    d.Clear();
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(OrderedDictionaryTest, RangeFor) {
    OrderedDictionary<std::string, int> d;
    d.Add("a", 1); d.Add("b", 2);
    int sum = 0;
    for (const auto& e : d) sum += e.second;
    EXPECT_EQ(sum, 3);
}

// ---- ReferenceEqualityComparer ----
TEST(ReferenceEqualityComparerTest, SamePointerIsEqual) {
    int x = 42;
    auto& cmp = ReferenceEqualityComparer<int>::Instance();
    EXPECT_TRUE(cmp.Equals(&x, &x));
}

TEST(ReferenceEqualityComparerTest, DifferentPointersNotEqual) {
    int x = 1, y = 1;
    auto& cmp = ReferenceEqualityComparer<int>::Instance();
    EXPECT_FALSE(cmp.Equals(&x, &y));
}

TEST(ReferenceEqualityComparerTest, HashIsPointerBased) {
    int x = 0;
    auto& cmp = ReferenceEqualityComparer<int>::Instance();
    EXPECT_EQ(cmp.GetHashCode(&x), cmp.GetHashCode(&x));
}

TEST(ReferenceEqualityComparerTest, NullPointerHash) {
    auto& cmp = ReferenceEqualityComparer<int>::Instance();
    int* p = nullptr;
    EXPECT_EQ(cmp.GetHashCode(p), std::hash<int*>{}(nullptr));
}
