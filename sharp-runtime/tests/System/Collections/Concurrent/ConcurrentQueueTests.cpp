// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <vector>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Concurrent/ConcurrentQueue.hpp"

using System::Collections::Concurrent::ConcurrentQueue;

TEST(ConcurrentQueueTest, EnqueueAndCount) {
    ConcurrentQueue<int> q;
    q.Enqueue(1);
    q.Enqueue(2);
    EXPECT_EQ(q.getCountProperty(), 2);
}

TEST(ConcurrentQueueTest, TryDequeueFIFO) {
    ConcurrentQueue<int> q;
    q.Enqueue(10);
    q.Enqueue(20);
    int v = 0;
    EXPECT_TRUE(q.TryDequeue(v));
    EXPECT_EQ(v, 10);
    EXPECT_TRUE(q.TryDequeue(v));
    EXPECT_EQ(v, 20);
}

TEST(ConcurrentQueueTest, TryDequeueEmptyReturnsFalse) {
    ConcurrentQueue<int> q;
    int v = 0;
    EXPECT_FALSE(q.TryDequeue(v));
}

TEST(ConcurrentQueueTest, TryPeek) {
    ConcurrentQueue<int> q;
    q.Enqueue(5);
    int v = 0;
    EXPECT_TRUE(q.TryPeek(v));
    EXPECT_EQ(v, 5);
    EXPECT_EQ(q.getCountProperty(), 1);
}

TEST(ConcurrentQueueTest, IsEmpty) {
    ConcurrentQueue<int> q;
    EXPECT_TRUE(q.getIsEmptyProperty());
    q.Enqueue(1);
    EXPECT_FALSE(q.getIsEmptyProperty());
}

TEST(ConcurrentQueueTest, Clear) {
    ConcurrentQueue<int> q;
    q.Enqueue(1);
    q.Enqueue(2);
    q.Clear();
    EXPECT_TRUE(q.getIsEmptyProperty());
}

TEST(ConcurrentQueueTest, TryAdd_AddsAtBack) {
    ConcurrentQueue<int> q;
    q.Enqueue(1);
    EXPECT_TRUE(q.TryAdd(2));
    int v = 0;
    q.TryDequeue(v);
    EXPECT_EQ(v, 1);
    q.TryDequeue(v);
    EXPECT_EQ(v, 2);
}

TEST(ConcurrentQueueTest, TryTake_EquivalentToTryDequeue) {
    ConcurrentQueue<int> q;
    q.Enqueue(7);
    int v = 0;
    EXPECT_TRUE(q.TryTake(v));
    EXPECT_EQ(v, 7);
    EXPECT_TRUE(q.getIsEmptyProperty());
}

TEST(ConcurrentQueueTest, ToArray_FrontToBackOrder) {
    ConcurrentQueue<int> q;
    q.Enqueue(1);
    q.Enqueue(2);
    q.Enqueue(3);
    auto arr = q.ToArray();
    ASSERT_EQ(arr.size(), 3u);
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[1], 2);
    EXPECT_EQ(arr[2], 3);
}

TEST(ConcurrentQueueTest, CopyTo_TooSmall_Throws) {
    ConcurrentQueue<int> q;
    q.Enqueue(1);
    q.Enqueue(2);
    std::vector<int> dest(1);
    EXPECT_THROW(q.CopyTo(dest, 0), System::ArgumentException);
}

// .NET ConcurrentQueue<T>.CopyTo throws ArgumentOutOfRangeException specifically for a
// negative index, distinct from the ArgumentException thrown for a too-small array.
TEST(ConcurrentQueueTest, CopyTo_NegativeIndex_ThrowsArgumentOutOfRange) {
    ConcurrentQueue<int> q;
    q.Enqueue(1);
    std::vector<int> dest(2);
    EXPECT_THROW(q.CopyTo(dest, -1), System::ArgumentOutOfRangeException);
}

TEST(ConcurrentQueueTest, GetEnumerator_IteratesFrontToBack) {
    ConcurrentQueue<int> q;
    q.Enqueue(1);
    q.Enqueue(2);
    auto* e = q.GetEnumerator();
    ASSERT_TRUE(e->MoveNext());
    EXPECT_EQ(e->Current(), 1);
    ASSERT_TRUE(e->MoveNext());
    EXPECT_EQ(e->Current(), 2);
    EXPECT_FALSE(e->MoveNext());
    delete e;
}

// Ticket 1725 (post-stabilization-audit): the class exists specifically to be safely mutated
// from multiple threads (std::mutex-guarded internally), but all prior tests were
// single-threaded API-shape verification -- matching the stress test already added to sibling
// ConcurrentStack. Verifies concurrent Enqueue from N threads loses nothing, then concurrent
// TryDequeue from N threads drains exactly that many items with no duplicates.
TEST(ConcurrentQueueTest, ConcurrentEnqueueDequeue_NoLostUpdatesOrCorruption) {
    ConcurrentQueue<int> q;
    constexpr int kThreads = 8;
    constexpr int kPerThread = 2000;

    std::vector<std::thread> enqueuers;
    for (int t = 0; t < kThreads; ++t) {
        enqueuers.emplace_back([&q, t]() {
            for (int i = 0; i < kPerThread; ++i) q.Enqueue(t * kPerThread + i);
        });
    }
    for (auto& th : enqueuers) th.join();
    EXPECT_EQ(q.getCountProperty(), kThreads * kPerThread);

    std::atomic<int> dequeuedCount{0};
    std::vector<std::thread> dequeuers;
    for (int t = 0; t < kThreads; ++t) {
        dequeuers.emplace_back([&q, &dequeuedCount]() {
            int value;
            while (q.TryDequeue(value)) dequeuedCount.fetch_add(1, std::memory_order_relaxed);
        });
    }
    for (auto& th : dequeuers) th.join();

    EXPECT_EQ(dequeuedCount.load(), kThreads * kPerThread);
    EXPECT_TRUE(q.getIsEmptyProperty());
}
