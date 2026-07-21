// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Collections/Concurrent/ConcurrentDictionary.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"
#include <atomic>
#include <string>
#include <thread>
#include <vector>

using System::Collections::Concurrent::ConcurrentDictionary;

TEST(ConcurrentDictionaryTest, TryAddAndCount) {
    ConcurrentDictionary<std::string, int> d;
    EXPECT_TRUE(d.TryAdd("a", 1));
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(ConcurrentDictionaryTest, TryAddDuplicateReturnsFalse) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("x", 1);
    EXPECT_FALSE(d.TryAdd("x", 2));
}

TEST(ConcurrentDictionaryTest, TryGetValue) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("k", 42);
    int v = 0;
    EXPECT_TRUE(d.TryGetValue("k", v));
    EXPECT_EQ(v, 42);
}

TEST(ConcurrentDictionaryTest, TryGetValueMissing) {
    ConcurrentDictionary<std::string, int> d;
    int v = 0;
    EXPECT_FALSE(d.TryGetValue("missing", v));
}

TEST(ConcurrentDictionaryTest, TryRemove) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("r", 99);
    int v = 0;
    EXPECT_TRUE(d.TryRemove("r", v));
    EXPECT_EQ(v, 99);
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(ConcurrentDictionaryTest, ContainsKey) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("yes", 1);
    EXPECT_TRUE(d.ContainsKey("yes"));
    EXPECT_FALSE(d.ContainsKey("no"));
}

TEST(ConcurrentDictionaryTest, GetOrAdd) {
    ConcurrentDictionary<std::string, int> d;
    int v = d.GetOrAdd("key", 7);
    EXPECT_EQ(v, 7);
    int v2 = d.GetOrAdd("key", 99);
    EXPECT_EQ(v2, 7);
}

TEST(ConcurrentDictionaryTest, TryUpdate) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("u", 10);
    EXPECT_TRUE(d.TryUpdate("u", 20, 10));
    int v = 0;
    d.TryGetValue("u", v);
    EXPECT_EQ(v, 20);
}

TEST(ConcurrentDictionaryTest, AddOrUpdate) {
    ConcurrentDictionary<std::string, int> d;
    d.AddOrUpdate("au", 1, [](const std::string&, int old){ return old + 1; });
    d.AddOrUpdate("au", 1, [](const std::string&, int old){ return old + 1; });
    int v = 0;
    d.TryGetValue("au", v);
    EXPECT_EQ(v, 2);
}

TEST(ConcurrentDictionaryTest, Clear) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("a", 1);
    d.Clear();
    EXPECT_TRUE(d.getIsEmptyProperty());
}

TEST(ConcurrentDictionaryTest, Keys) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("a", 1);
    d.TryAdd("b", 2);
    auto keys = d.getKeysProperty();
    EXPECT_EQ(static_cast<int>(keys.size()), 2);
}

TEST(ConcurrentDictionaryTest, AddOrUpdate_WithAddFactory_NewKey_Adds) {
    ConcurrentDictionary<std::string, int> d;
    int result = d.AddOrUpdate("a", [](const std::string&) { return 5; },
                                [](const std::string&, int v) { return v + 1; });
    EXPECT_EQ(result, 5);
}

TEST(ConcurrentDictionaryTest, AddOrUpdate_WithAddFactory_ExistingKey_Updates) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("a", 5);
    int result = d.AddOrUpdate("a", [](const std::string&) { return 100; },
                                [](const std::string&, int v) { return v + 1; });
    EXPECT_EQ(result, 6);
}

// .NET's ConcurrentDictionary<TKey,TValue> indexer getter throws KeyNotFoundException for
// an absent key (ConcurrentDictionary.cs:1069-1072) -- it does NOT insert a default value,
// unlike std::unordered_map::operator[] or this type's own prior (buggy) implementation.
TEST(ConcurrentDictionaryTest, Indexer_MissingKey_Throws) {
    ConcurrentDictionary<std::string, int> d;
    EXPECT_THROW({ int v = d["missing"]; (void)v; }, System::Collections::Generic::KeyNotFoundException);
    EXPECT_TRUE(d.getIsEmptyProperty());
}

TEST(ConcurrentDictionaryTest, Indexer_Get_ExistingKey_ReturnsValue) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("a", 42);
    int v = d["a"];
    EXPECT_EQ(v, 42);
}

TEST(ConcurrentDictionaryTest, Indexer_Set_NewKey_Inserts) {
    ConcurrentDictionary<std::string, int> d;
    d["a"] = 7;
    EXPECT_EQ(d.getCountProperty(), 1);
    EXPECT_EQ(static_cast<int>(d["a"]), 7);
}

TEST(ConcurrentDictionaryTest, Indexer_Set_ExistingKey_Overwrites) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("a", 1);
    d["a"] = 2;
    EXPECT_EQ(d.getCountProperty(), 1);
    EXPECT_EQ(static_cast<int>(d["a"]), 2);
}

// Regression tests for POST_STABILIZATION_AUDIT.md finding #7 / ticket 1716: GetOrAdd and
// AddOrUpdate previously held the internal mutex across the entire user-supplied factory
// callback, so a factory that reentrantly called back into the same ConcurrentDictionary
// instance (a realistic memoization pattern) deadlocked the thread against itself
// (std::mutex is non-recursive). Real .NET's documented contract explicitly permits and
// expects the factory to run without the lock held. Each test below intentionally has the
// callback call another ConcurrentDictionary method reentrantly; a hang here (rather than a
// clean pass) indicates the deadlock has regressed.

TEST(ConcurrentDictionaryTest, GetOrAdd_ReentrantFactory_DoesNotDeadlock) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("related", 10);
    int result = d.GetOrAdd("key", [&](const std::string&) {
        // Reentrant call into the same instance while "key" is still being computed.
        int related = d.GetOrAdd("related", 999);
        return related + 1;
    });
    EXPECT_EQ(result, 11);
    EXPECT_EQ(d.getCountProperty(), 2);
}

TEST(ConcurrentDictionaryTest, AddOrUpdate_ReentrantUpdateFactory_DoesNotDeadlock) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("a", 1);
    d.TryAdd("b", 5);
    int result = d.AddOrUpdate("a", 0, [&](const std::string&, int old) {
        int b = d.GetOrAdd("b", -1);
        return old + b;
    });
    EXPECT_EQ(result, 6);
}

TEST(ConcurrentDictionaryTest, AddOrUpdate_ReentrantAddFactory_DoesNotDeadlock) {
    ConcurrentDictionary<std::string, int> d;
    int result = d.AddOrUpdate(
        "a",
        [&](const std::string&) { return d.GetOrAdd("b", 3); },
        [](const std::string&, int old) { return old + 1; });
    EXPECT_EQ(result, 3);
    EXPECT_TRUE(d.ContainsKey("a"));
    EXPECT_TRUE(d.ContainsKey("b"));
}

TEST(ConcurrentDictionaryTest, GetOrAdd_ConcurrentContention_RaceDiscardsLosingFactory) {
    ConcurrentDictionary<std::string, int> d;
    int result1 = d.GetOrAdd("shared", [](const std::string&) { return 100; });
    int result2 = d.GetOrAdd("shared", [](const std::string&) { return 200; });
    // Second call's factory result must be discarded since the key already existed.
    EXPECT_EQ(result1, 100);
    EXPECT_EQ(result2, 100);
    EXPECT_EQ(d.getCountProperty(), 1);
}

// Ticket 1725 (post-stabilization-audit): the reentrancy tests above (GetOrAdd/AddOrUpdate
// deadlock scenarios, fixed by ticket 1716) exercise the lock-release-around-callback logic
// specifically, but none exercised ORDINARY concurrent access from real OS threads. Matches the
// stress test pattern already established for sibling ConcurrentStack/ConcurrentQueue: N threads
// each TryAdd distinct keys (no key contention between threads, so no losing-factory-discard
// noise), verifying no lost updates; then N threads each TryRemove their own keys concurrently,
// verifying every key is removed and the dictionary ends up empty.
TEST(ConcurrentDictionaryTest, ConcurrentAddRemove_DistinctKeys_NoLostUpdatesOrCorruption) {
    ConcurrentDictionary<std::string, int> d;
    constexpr int kThreads = 8;
    constexpr int kPerThread = 500;

    std::vector<std::thread> adders;
    for (int t = 0; t < kThreads; ++t) {
        adders.emplace_back([&d, t]() {
            for (int i = 0; i < kPerThread; ++i) {
                std::string key = "t" + std::to_string(t) + "_" + std::to_string(i);
                EXPECT_TRUE(d.TryAdd(key, t * kPerThread + i));
            }
        });
    }
    for (auto& th : adders) th.join();
    EXPECT_EQ(d.getCountProperty(), kThreads * kPerThread);

    std::atomic<int> removedCount{0};
    std::vector<std::thread> removers;
    for (int t = 0; t < kThreads; ++t) {
        removers.emplace_back([&d, &removedCount, t]() {
            for (int i = 0; i < kPerThread; ++i) {
                std::string key = "t" + std::to_string(t) + "_" + std::to_string(i);
                int value;
                if (d.TryRemove(key, value)) {
                    EXPECT_EQ(value, t * kPerThread + i);
                    removedCount.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }
    for (auto& th : removers) th.join();

    EXPECT_EQ(removedCount.load(), kThreads * kPerThread);
    EXPECT_EQ(d.getCountProperty(), 0);
}
