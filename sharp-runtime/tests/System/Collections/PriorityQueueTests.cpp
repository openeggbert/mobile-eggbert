// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include "System/Collections/Generic/PriorityQueue.hpp"
#include "System/InvalidOperationException.hpp"

using System::Collections::Generic::PriorityQueue;
using System::InvalidOperationException;

// ---------------------------------------------------------------------------
// Empty queue
// ---------------------------------------------------------------------------

TEST(PriorityQueueTests, EmptyQueueCount) {
    PriorityQueue<std::string, int> pq;
    EXPECT_EQ(pq.getCountProperty(), 0);
}

TEST(PriorityQueueTests, DequeueOnEmptyThrows) {
    PriorityQueue<int, int> pq;
    EXPECT_THROW(pq.Dequeue(), InvalidOperationException);
}

TEST(PriorityQueueTests, PeekOnEmptyThrows) {
    PriorityQueue<int, int> pq;
    EXPECT_THROW(pq.Peek(), InvalidOperationException);
}

TEST(PriorityQueueTests, TryDequeueOnEmptyReturnsFalse) {
    PriorityQueue<int, int> pq;
    int el = -1, pri = -1;
    EXPECT_FALSE(pq.TryDequeue(el, pri));
    EXPECT_EQ(el, -1);  // unchanged
}

TEST(PriorityQueueTests, TryPeekOnEmptyReturnsFalse) {
    PriorityQueue<int, int> pq;
    int el = -1, pri = -1;
    EXPECT_FALSE(pq.TryPeek(el, pri));
}

// ---------------------------------------------------------------------------
// Single element
// ---------------------------------------------------------------------------

TEST(PriorityQueueTests, SingleElementCountIsOne) {
    PriorityQueue<std::string, int> pq;
    pq.Enqueue("a", 1);
    EXPECT_EQ(pq.getCountProperty(), 1);
}

TEST(PriorityQueueTests, PeekDoesNotRemove) {
    PriorityQueue<std::string, int> pq;
    pq.Enqueue("hello", 5);
    EXPECT_EQ(pq.Peek(), "hello");
    EXPECT_EQ(pq.getCountProperty(), 1);
    EXPECT_EQ(pq.Peek(), "hello");
}

TEST(PriorityQueueTests, DequeueRemovesElement) {
    PriorityQueue<std::string, int> pq;
    pq.Enqueue("hello", 5);
    EXPECT_EQ(pq.Dequeue(), "hello");
    EXPECT_EQ(pq.getCountProperty(), 0);
}

TEST(PriorityQueueTests, TryPeekSingleElement) {
    PriorityQueue<std::string, int> pq;
    pq.Enqueue("x", 42);
    std::string el; int pri;
    EXPECT_TRUE(pq.TryPeek(el, pri));
    EXPECT_EQ(el, "x");
    EXPECT_EQ(pri, 42);
    EXPECT_EQ(pq.getCountProperty(), 1);  // not removed
}

TEST(PriorityQueueTests, TryDequeueSingleElement) {
    PriorityQueue<std::string, int> pq;
    pq.Enqueue("x", 42);
    std::string el; int pri;
    EXPECT_TRUE(pq.TryDequeue(el, pri));
    EXPECT_EQ(el, "x");
    EXPECT_EQ(pri, 42);
    EXPECT_EQ(pq.getCountProperty(), 0);
}

// ---------------------------------------------------------------------------
// Min-heap ordering
// ---------------------------------------------------------------------------

TEST(PriorityQueueTests, DequeueInPriorityOrder) {
    PriorityQueue<std::string, int> pq;
    pq.Enqueue("three", 3);
    pq.Enqueue("one",   1);
    pq.Enqueue("two",   2);

    EXPECT_EQ(pq.Dequeue(), "one");
    EXPECT_EQ(pq.Dequeue(), "two");
    EXPECT_EQ(pq.Dequeue(), "three");
    EXPECT_EQ(pq.getCountProperty(), 0);
}

TEST(PriorityQueueTests, EnqueueOutOfOrderStillDequeuesCorrectly) {
    PriorityQueue<int, int> pq;
    for (int v : {5, 1, 4, 2, 3}) pq.Enqueue(v, v);

    for (int expected = 1; expected <= 5; ++expected)
        EXPECT_EQ(pq.Dequeue(), expected);
}

TEST(PriorityQueueTests, PeekAlwaysReturnsLowestPriority) {
    PriorityQueue<int, int> pq;
    pq.Enqueue(10, 10);
    pq.Enqueue(1,  1);
    pq.Enqueue(5,  5);

    EXPECT_EQ(pq.Peek(), 1);
    pq.Dequeue();
    EXPECT_EQ(pq.Peek(), 5);
    pq.Dequeue();
    EXPECT_EQ(pq.Peek(), 10);
}

TEST(PriorityQueueTests, NegativePriorities) {
    PriorityQueue<std::string, int> pq;
    pq.Enqueue("neg", -5);
    pq.Enqueue("pos", 5);
    pq.Enqueue("zero", 0);

    EXPECT_EQ(pq.Dequeue(), "neg");
    EXPECT_EQ(pq.Dequeue(), "zero");
    EXPECT_EQ(pq.Dequeue(), "pos");
}

TEST(PriorityQueueTests, EqualPrioritiesBothDequeued) {
    PriorityQueue<std::string, int> pq;
    pq.Enqueue("a", 1);
    pq.Enqueue("b", 1);

    std::string first  = pq.Dequeue();
    std::string second = pq.Dequeue();
    EXPECT_EQ(pq.getCountProperty(), 0);
    // Both must appear regardless of order
    EXPECT_TRUE((first == "a" && second == "b") || (first == "b" && second == "a"));
}

// ---------------------------------------------------------------------------
// Count tracking
// ---------------------------------------------------------------------------

TEST(PriorityQueueTests, CountTracksEnqueueDequeue) {
    PriorityQueue<int, int> pq;
    EXPECT_EQ(pq.getCountProperty(), 0);
    pq.Enqueue(1, 1); EXPECT_EQ(pq.getCountProperty(), 1);
    pq.Enqueue(2, 2); EXPECT_EQ(pq.getCountProperty(), 2);
    pq.Dequeue();     EXPECT_EQ(pq.getCountProperty(), 1);
    pq.Dequeue();     EXPECT_EQ(pq.getCountProperty(), 0);
}

// ---------------------------------------------------------------------------
// EnqueueRange and Clear
// ---------------------------------------------------------------------------

TEST(PriorityQueueTests, EnqueueRange) {
    PriorityQueue<std::string, int> pq;
    pq.EnqueueRange({{"b", 2}, {"a", 1}, {"c", 3}});

    EXPECT_EQ(pq.getCountProperty(), 3);
    EXPECT_EQ(pq.Dequeue(), "a");
    EXPECT_EQ(pq.Dequeue(), "b");
    EXPECT_EQ(pq.Dequeue(), "c");
}

TEST(PriorityQueueTests, ClearEmptiesQueue) {
    PriorityQueue<int, int> pq;
    pq.Enqueue(1, 1);
    pq.Enqueue(2, 2);
    pq.Clear();

    EXPECT_EQ(pq.getCountProperty(), 0);
    EXPECT_THROW(pq.Dequeue(), InvalidOperationException);
}

TEST(PriorityQueueTests, ClearThenReuse) {
    PriorityQueue<int, int> pq;
    pq.Enqueue(10, 10);
    pq.Clear();
    pq.Enqueue(5, 5);
    EXPECT_EQ(pq.Dequeue(), 5);
}

// ---------------------------------------------------------------------------
// Large input
// ---------------------------------------------------------------------------

TEST(PriorityQueueTests, LargeInputDequeuesInOrder) {
    PriorityQueue<int, int> pq;
    const int N = 1000;
    // Insert in reverse order
    for (int i = N; i >= 1; --i) pq.Enqueue(i, i);
    EXPECT_EQ(pq.getCountProperty(), N);

    for (int expected = 1; expected <= N; ++expected)
        EXPECT_EQ(pq.Dequeue(), expected);

    EXPECT_EQ(pq.getCountProperty(), 0);
}
