// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <atomic>
#include <thread>
#include <vector>
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Concurrent/ConcurrentStack.hpp"

using System::Collections::Concurrent::ConcurrentStack;

TEST(ConcurrentStackTest, DefaultEmpty) {
    ConcurrentStack<int> s;
    EXPECT_TRUE(s.getIsEmptyProperty());
    EXPECT_EQ(s.getCountProperty(), 0);
}

TEST(ConcurrentStackTest, PushAndCount) {
    ConcurrentStack<int> s;
    s.Push(1);
    s.Push(2);
    EXPECT_EQ(s.getCountProperty(), 2);
    EXPECT_FALSE(s.getIsEmptyProperty());
}

TEST(ConcurrentStackTest, TryPopLIFO) {
    ConcurrentStack<int> s;
    s.Push(10);
    s.Push(20);
    int v = 0;
    EXPECT_TRUE(s.TryPop(v));
    EXPECT_EQ(v, 20);
    EXPECT_TRUE(s.TryPop(v));
    EXPECT_EQ(v, 10);
}

TEST(ConcurrentStackTest, TryPopEmptyReturnsFalse) {
    ConcurrentStack<int> s;
    int v = 0;
    EXPECT_FALSE(s.TryPop(v));
}

TEST(ConcurrentStackTest, TryPeekDoesNotRemove) {
    ConcurrentStack<int> s;
    s.Push(5);
    int v = 0;
    EXPECT_TRUE(s.TryPeek(v));
    EXPECT_EQ(v, 5);
    EXPECT_EQ(s.getCountProperty(), 1);
}

TEST(ConcurrentStackTest, Clear) {
    ConcurrentStack<int> s;
    s.Push(1);
    s.Push(2);
    s.Clear();
    EXPECT_TRUE(s.getIsEmptyProperty());
}

TEST(ConcurrentStackTest, PushRange) {
    ConcurrentStack<int> s;
    s.PushRange({1, 2, 3});
    int v = 0;
    EXPECT_TRUE(s.TryPop(v));
    EXPECT_EQ(v, 3);
}

// .NET ConcurrentStack<T>.PushRange(T[], int, int) validates count/startIndex with
// ArgumentOutOfRangeException, and the combined startIndex+count-exceeds-length case with
// ArgumentException (ConcurrentStack.cs's ValidatePushPopRangeInput).
TEST(ConcurrentStackTest, PushRange_NegativeCount_ThrowsArgumentOutOfRange) {
    ConcurrentStack<int> s;
    std::vector<int> items{1, 2, 3};
    EXPECT_THROW(s.PushRange(items, 0, -1), System::ArgumentOutOfRangeException);
}
TEST(ConcurrentStackTest, PushRange_NegativeStartIndex_ThrowsArgumentOutOfRange) {
    ConcurrentStack<int> s;
    std::vector<int> items{1, 2, 3};
    EXPECT_THROW(s.PushRange(items, -1, 1), System::ArgumentOutOfRangeException);
}
TEST(ConcurrentStackTest, PushRange_CountExceedsLength_ThrowsArgumentException) {
    ConcurrentStack<int> s;
    std::vector<int> items{1, 2, 3};
    EXPECT_THROW(s.PushRange(items, 1, 5), System::ArgumentException);
}

TEST(ConcurrentStackTest, TryPopRange) {
    ConcurrentStack<int> s;
    s.Push(1);
    s.Push(2);
    s.Push(3);
    std::vector<int> out;
    int n = s.TryPopRange(out, 2);
    EXPECT_EQ(n, 2);
    EXPECT_EQ(out[0], 3);
    EXPECT_EQ(out[1], 2);
    EXPECT_EQ(s.getCountProperty(), 1);
}

TEST(ConcurrentStackTest, ToArray) {
    ConcurrentStack<int> s;
    s.Push(1);
    s.Push(2);
    s.Push(3);
    auto arr = s.ToArray();
    ASSERT_EQ(static_cast<int>(arr.size()), 3);
    EXPECT_EQ(arr[0], 3);
    EXPECT_EQ(arr[1], 2);
    EXPECT_EQ(arr[2], 1);
}

TEST(ConcurrentStackTest, ConstructFromCollection) {
    ConcurrentStack<int> s({10, 20, 30});
    int v = 0;
    EXPECT_TRUE(s.TryPop(v));
    EXPECT_EQ(v, 30);
    EXPECT_EQ(s.getCountProperty(), 2);
}

TEST(ConcurrentStackTest, TryAdd_PushesOnTop) {
    ConcurrentStack<int> s;
    s.Push(1);
    EXPECT_TRUE(s.TryAdd(2));
    int v = 0;
    s.TryPop(v);
    EXPECT_EQ(v, 2);
}

TEST(ConcurrentStackTest, TryTake_EquivalentToTryPop) {
    ConcurrentStack<int> s;
    s.Push(9);
    int v = 0;
    EXPECT_TRUE(s.TryTake(v));
    EXPECT_EQ(v, 9);
    EXPECT_TRUE(s.getIsEmptyProperty());
}

TEST(ConcurrentStackTest, CopyTo_TooSmall_Throws) {
    ConcurrentStack<int> s;
    s.Push(1);
    s.Push(2);
    std::vector<int> dest(1);
    EXPECT_THROW(s.CopyTo(dest, 0), System::ArgumentException);
}

// .NET ConcurrentStack<T>.CopyTo throws ArgumentOutOfRangeException specifically for a
// negative index, distinct from the ArgumentException thrown for a too-small array.
TEST(ConcurrentStackTest, CopyTo_NegativeIndex_ThrowsArgumentOutOfRange) {
    ConcurrentStack<int> s;
    s.Push(1);
    std::vector<int> dest(2);
    EXPECT_THROW(s.CopyTo(dest, -1), System::ArgumentOutOfRangeException);
}

TEST(ConcurrentStackTest, CopyTo_ExactSize_Succeeds) {
    ConcurrentStack<int> s;
    s.Push(1);
    s.Push(2);
    std::vector<int> dest(2);
    s.CopyTo(dest, 0);
    EXPECT_EQ(dest[0], 2);
    EXPECT_EQ(dest[1], 1);
}

TEST(ConcurrentStackTest, GetEnumerator_IteratesTopToBottom) {
    ConcurrentStack<int> s;
    s.Push(1);
    s.Push(2);
    auto* e = s.GetEnumerator();
    ASSERT_TRUE(e->MoveNext());
    EXPECT_EQ(e->Current(), 2);
    ASSERT_TRUE(e->MoveNext());
    EXPECT_EQ(e->Current(), 1);
    EXPECT_FALSE(e->MoveNext());
    delete e;
}

// The class's own doc-comment promises "All public members are thread-safe and may be used
// concurrently from multiple threads" -- no prior test actually exercised concurrent access
// (all 19 original tests were single-threaded). Verifies the mutex-protected implementation
// doesn't lose pushes/pops or corrupt state under real concurrent load; run under ThreadSanitizer
// in CI to catch any data race, not just wrong final counts.
TEST(ConcurrentStackTest, ConcurrentPushPop_NoLostUpdatesOrCorruption) {
    ConcurrentStack<int> s;
    constexpr int kThreads = 8;
    constexpr int kPerThread = 2000;

    std::vector<std::thread> pushers;
    for (int t = 0; t < kThreads; ++t) {
        pushers.emplace_back([&s, t]() {
            for (int i = 0; i < kPerThread; ++i) s.Push(t * kPerThread + i);
        });
    }
    for (auto& th : pushers) th.join();
    EXPECT_EQ(s.getCountProperty(), kThreads * kPerThread);

    std::atomic<int> poppedCount{0};
    std::vector<std::thread> poppers;
    for (int t = 0; t < kThreads; ++t) {
        poppers.emplace_back([&s, &poppedCount]() {
            int value;
            while (s.TryPop(value)) poppedCount.fetch_add(1, std::memory_order_relaxed);
        });
    }
    for (auto& th : poppers) th.join();

    EXPECT_EQ(poppedCount.load(), kThreads * kPerThread);
    EXPECT_TRUE(s.getIsEmptyProperty());
    EXPECT_EQ(s.getCountProperty(), 0);
}
