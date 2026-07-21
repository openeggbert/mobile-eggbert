// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "System/Collections/Concurrent/ConcurrentDictionary.hpp"
#include "System/Collections/Concurrent/ConcurrentQueue.hpp"
#include "System/Collections/Concurrent/ConcurrentStack.hpp"

using System::Collections::Concurrent::ConcurrentDictionary;
using System::Collections::Concurrent::ConcurrentQueue;
using System::Collections::Concurrent::ConcurrentStack;

// ===========================================================================
// ConcurrentDictionary
// ===========================================================================

TEST(ConcurrentDictionaryTests, DefaultCtor_IsEmpty) {
    ConcurrentDictionary<std::string, int> d;
    EXPECT_TRUE(d.getIsEmptyProperty());
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(ConcurrentDictionaryTests, TryAdd_Succeeds) {
    ConcurrentDictionary<std::string, int> d;
    EXPECT_TRUE(d.TryAdd("key", 42));
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(ConcurrentDictionaryTests, TryAdd_DuplicateKey_ReturnsFalse) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("k", 1);
    EXPECT_FALSE(d.TryAdd("k", 2));
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(ConcurrentDictionaryTests, TryGetValue_Found) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("x", 99);
    int v = 0;
    EXPECT_TRUE(d.TryGetValue("x", v));
    EXPECT_EQ(v, 99);
}

TEST(ConcurrentDictionaryTests, TryGetValue_NotFound_ReturnsFalse) {
    ConcurrentDictionary<std::string, int> d;
    int v = 0;
    EXPECT_FALSE(d.TryGetValue("missing", v));
}

TEST(ConcurrentDictionaryTests, TryRemove_RemovesEntry) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("a", 10);
    int v = 0;
    EXPECT_TRUE(d.TryRemove("a", v));
    EXPECT_EQ(v, 10);
    EXPECT_EQ(d.getCountProperty(), 0);
}

TEST(ConcurrentDictionaryTests, TryUpdate_MatchingValue_Updates) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("n", 5);
    EXPECT_TRUE(d.TryUpdate("n", 10, 5));
    int v = 0;
    d.TryGetValue("n", v);
    EXPECT_EQ(v, 10);
}

TEST(ConcurrentDictionaryTests, TryUpdate_WrongComparison_ReturnsFalse) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("n", 5);
    EXPECT_FALSE(d.TryUpdate("n", 10, 999));
}

TEST(ConcurrentDictionaryTests, GetOrAdd_NewKey_ReturnsDefault) {
    ConcurrentDictionary<std::string, int> d;
    EXPECT_EQ(d.GetOrAdd("k", 7), 7);
    EXPECT_EQ(d.getCountProperty(), 1);
}

TEST(ConcurrentDictionaryTests, GetOrAdd_ExistingKey_ReturnsExisting) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("k", 42);
    EXPECT_EQ(d.GetOrAdd("k", 99), 42);
}

TEST(ConcurrentDictionaryTests, AddOrUpdate_NewKey_Adds) {
    ConcurrentDictionary<std::string, int> d;
    auto v = d.AddOrUpdate("k", 1, [](const std::string&, int old) { return old + 1; });
    EXPECT_EQ(v, 1);
}

TEST(ConcurrentDictionaryTests, AddOrUpdate_ExistingKey_Updates) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("k", 10);
    auto v = d.AddOrUpdate("k", 0, [](const std::string&, int old) { return old + 5; });
    EXPECT_EQ(v, 15);
}

TEST(ConcurrentDictionaryTests, ContainsKey_True_False) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("present", 1);
    EXPECT_TRUE(d.ContainsKey("present"));
    EXPECT_FALSE(d.ContainsKey("absent"));
}

TEST(ConcurrentDictionaryTests, Clear_EmptiesMap) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("a", 1);
    d.TryAdd("b", 2);
    d.Clear();
    EXPECT_TRUE(d.getIsEmptyProperty());
}

TEST(ConcurrentDictionaryTests, OperatorBracket_WritesAndReads) {
    ConcurrentDictionary<std::string, int> d;
    d["alpha"] = 100;
    EXPECT_EQ(d["alpha"], 100);
}

TEST(ConcurrentDictionaryTests, Keys_ReturnsAllKeys) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("a", 1);
    d.TryAdd("b", 2);
    auto keys = d.getKeysProperty();
    EXPECT_EQ(keys.size(), 2u);
}

TEST(ConcurrentDictionaryTests, Values_ReturnsAllValues) {
    ConcurrentDictionary<std::string, int> d;
    d.TryAdd("x", 10);
    auto vals = d.getValuesProperty();
    EXPECT_EQ(vals.size(), 1u);
    EXPECT_EQ(vals[0], 10);
}

// ===========================================================================
// ConcurrentQueue
// ===========================================================================

TEST(ConcurrentQueueTests, DefaultCtor_IsEmpty) {
    ConcurrentQueue<int> q;
    EXPECT_TRUE(q.getIsEmptyProperty());
    EXPECT_EQ(q.getCountProperty(), 0);
}

TEST(ConcurrentQueueTests, Enqueue_IncreasesCount) {
    ConcurrentQueue<int> q;
    q.Enqueue(1);
    q.Enqueue(2);
    EXPECT_EQ(q.getCountProperty(), 2);
    EXPECT_FALSE(q.getIsEmptyProperty());
}

TEST(ConcurrentQueueTests, TryDequeue_FIFO_Order) {
    ConcurrentQueue<int> q;
    q.Enqueue(10);
    q.Enqueue(20);
    int v = 0;
    EXPECT_TRUE(q.TryDequeue(v));
    EXPECT_EQ(v, 10);
    EXPECT_TRUE(q.TryDequeue(v));
    EXPECT_EQ(v, 20);
}

TEST(ConcurrentQueueTests, TryDequeue_Empty_ReturnsFalse) {
    ConcurrentQueue<int> q;
    int v = 0;
    EXPECT_FALSE(q.TryDequeue(v));
}

TEST(ConcurrentQueueTests, TryPeek_DoesNotRemove) {
    ConcurrentQueue<int> q;
    q.Enqueue(7);
    int v = 0;
    EXPECT_TRUE(q.TryPeek(v));
    EXPECT_EQ(v, 7);
    EXPECT_EQ(q.getCountProperty(), 1);
}

TEST(ConcurrentQueueTests, Clear_EmptiesQueue) {
    ConcurrentQueue<int> q;
    q.Enqueue(1);
    q.Enqueue(2);
    q.Clear();
    EXPECT_TRUE(q.getIsEmptyProperty());
}

// ===========================================================================
// ConcurrentStack
// ===========================================================================

TEST(ConcurrentStackTests, DefaultCtor_IsEmpty) {
    ConcurrentStack<int> s;
    EXPECT_TRUE(s.getIsEmptyProperty());
    EXPECT_EQ(s.getCountProperty(), 0);
}

TEST(ConcurrentStackTests, Push_IncreasesCount) {
    ConcurrentStack<int> s;
    s.Push(1);
    s.Push(2);
    EXPECT_EQ(s.getCountProperty(), 2);
}

TEST(ConcurrentStackTests, TryPop_LIFO_Order) {
    ConcurrentStack<int> s;
    s.Push(1);
    s.Push(2);
    int v = 0;
    EXPECT_TRUE(s.TryPop(v));
    EXPECT_EQ(v, 2);
    EXPECT_TRUE(s.TryPop(v));
    EXPECT_EQ(v, 1);
}

TEST(ConcurrentStackTests, TryPop_Empty_ReturnsFalse) {
    ConcurrentStack<int> s;
    int v = 0;
    EXPECT_FALSE(s.TryPop(v));
}

TEST(ConcurrentStackTests, TryPeek_DoesNotRemove) {
    ConcurrentStack<int> s;
    s.Push(42);
    int v = 0;
    EXPECT_TRUE(s.TryPeek(v));
    EXPECT_EQ(v, 42);
    EXPECT_EQ(s.getCountProperty(), 1);
}

TEST(ConcurrentStackTests, TryPopRange_PopsMultiple) {
    ConcurrentStack<int> s;
    s.Push(1);
    s.Push(2);
    s.Push(3);
    std::vector<int> items;
    int popped = s.TryPopRange(items, 2);
    EXPECT_EQ(popped, 2);
    EXPECT_EQ(items.size(), 2u);
    EXPECT_EQ(s.getCountProperty(), 1);
}

TEST(ConcurrentStackTests, Clear_EmptiesStack) {
    ConcurrentStack<int> s;
    s.Push(1);
    s.Push(2);
    s.Clear();
    EXPECT_TRUE(s.getIsEmptyProperty());
}
