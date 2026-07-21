// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>
#include "System/Threading/Channels/Channel.hpp"
#include "System/Threading/Channels/ChannelClosedException.hpp"

using namespace System::Threading::Channels;

TEST(ChannelTests, UnboundedTryWriteThenTryRead) {
    auto channel = Channel<int>::CreateUnbounded();
    EXPECT_TRUE(channel.Writer->TryWrite(42));
    int value = 0;
    EXPECT_TRUE(channel.Reader->TryRead(value));
    EXPECT_EQ(value, 42);
}

TEST(ChannelTests, TryRead_EmptyChannel_ReturnsFalse) {
    auto channel = Channel<int>::CreateUnbounded();
    int value = 0;
    EXPECT_FALSE(channel.Reader->TryRead(value));
}

TEST(ChannelTests, FifoOrdering) {
    auto channel = Channel<int>::CreateUnbounded();
    channel.Writer->TryWrite(1);
    channel.Writer->TryWrite(2);
    channel.Writer->TryWrite(3);
    int a, b, c;
    channel.Reader->TryRead(a);
    channel.Reader->TryRead(b);
    channel.Reader->TryRead(c);
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 2);
    EXPECT_EQ(c, 3);
}

TEST(ChannelTests, BoundedChannel_RespectsCapacity_WaitMode) {
    auto channel = Channel<int>::CreateBounded(2);
    EXPECT_TRUE(channel.Writer->TryWrite(1));
    EXPECT_TRUE(channel.Writer->TryWrite(2));
    EXPECT_FALSE(channel.Writer->TryWrite(3)); // full, Wait mode rejects synchronous TryWrite
}

TEST(ChannelTests, BoundedChannel_DropOldest) {
    BoundedChannelOptions options(2);
    options.FullMode = BoundedChannelFullMode::DropOldest;
    auto channel = Channel<int>::CreateBounded(options);
    channel.Writer->TryWrite(1);
    channel.Writer->TryWrite(2);
    EXPECT_TRUE(channel.Writer->TryWrite(3)); // drops 1, keeps [2,3]
    int a, b;
    channel.Reader->TryRead(a);
    channel.Reader->TryRead(b);
    EXPECT_EQ(a, 2);
    EXPECT_EQ(b, 3);
}

// Verified against BoundedChannel.cs's DropNewest handling (DequeueTail): the item removed to
// make room is the most-recently-written one, not the oldest -- easy to accidentally swap with
// DropOldest's DequeueHead, so this pins the distinction explicitly rather than relying only on
// DropOldest's existing coverage.
TEST(ChannelTests, BoundedChannel_DropNewest) {
    BoundedChannelOptions options(2);
    options.FullMode = BoundedChannelFullMode::DropNewest;
    auto channel = Channel<int>::CreateBounded(options);
    channel.Writer->TryWrite(1);
    channel.Writer->TryWrite(2);
    EXPECT_TRUE(channel.Writer->TryWrite(3)); // drops 2 (newest), keeps [1,3]
    int a, b;
    channel.Reader->TryRead(a);
    channel.Reader->TryRead(b);
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 3);
}

TEST(ChannelTests, BoundedChannel_DropWrite) {
    BoundedChannelOptions options(1);
    options.FullMode = BoundedChannelFullMode::DropWrite;
    auto channel = Channel<int>::CreateBounded(options);
    channel.Writer->TryWrite(1);
    EXPECT_TRUE(channel.Writer->TryWrite(2)); // "handled" (dropped), item 1 stays
    int a;
    channel.Reader->TryRead(a);
    EXPECT_EQ(a, 1);
    EXPECT_FALSE(channel.Reader->TryRead(a));
}

TEST(ChannelTests, TryComplete_ThenTryWrite_Fails) {
    auto channel = Channel<int>::CreateUnbounded();
    EXPECT_TRUE(channel.Writer->TryComplete());
    EXPECT_FALSE(channel.Writer->TryWrite(1));
    EXPECT_FALSE(channel.Writer->TryComplete()); // already completed
}

TEST(ChannelTests, WaitToReadAsync_UnblocksWhenItemWritten) {
    auto channel = Channel<int>::CreateUnbounded();
    auto waitTask = channel.Reader->WaitToReadAsync();
    std::thread writer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        channel.Writer->TryWrite(99);
    });
    EXPECT_TRUE(waitTask.getResultProperty());
    int value = 0;
    EXPECT_TRUE(channel.Reader->TryRead(value));
    EXPECT_EQ(value, 99);
    writer.join();
}

TEST(ChannelTests, WaitToReadAsync_ReturnsFalse_WhenClosedEmpty) {
    auto channel = Channel<int>::CreateUnbounded();
    channel.Writer->TryComplete();
    EXPECT_FALSE(channel.Reader->WaitToReadAsync().getResultProperty());
}

// Regression test for a wave-3 audit finding: WaitToReadAsync() never inspected closeError, so
// completing the channel with an exception and an empty queue made it return false (as if
// gracefully closed) instead of faulting -- the real error was only observable via the
// separate getCompletionProperty() task, not the primary read path. Verified against
// UnboundedChannel.cs's WaitToReadAsync, which returns Task.FromException when the channel
// completed with a non-null error.
TEST(ChannelTests, WaitToReadAsync_ThrowsCompletionError_WhenClosedWithException) {
    auto channel = Channel<int>::CreateUnbounded();
    channel.Writer->TryComplete(std::make_exception_ptr(std::runtime_error("boom")));
    EXPECT_THROW(channel.Reader->WaitToReadAsync().getResultProperty(), std::runtime_error);
}

TEST(ChannelTests, WaitToWriteAsync_ThrowsCompletionError_WhenClosedWithException) {
    auto channel = Channel<int>::CreateUnbounded();
    channel.Writer->TryComplete(std::make_exception_ptr(std::runtime_error("boom")));
    EXPECT_THROW(channel.Writer->WaitToWriteAsync().getResultProperty(), std::runtime_error);
}

TEST(ChannelTests, ReadAsync_ThrowsChannelClosedException_WhenClosedEmpty) {
    auto channel = Channel<int>::CreateUnbounded();
    channel.Writer->TryComplete();
    EXPECT_THROW(channel.Reader->ReadAsync().getResultProperty(), ChannelClosedException);
}

TEST(ChannelTests, ReadAsync_ReturnsItemWrittenConcurrently) {
    auto channel = Channel<int>::CreateUnbounded();
    auto readTask = channel.Reader->ReadAsync();
    std::thread writer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        channel.Writer->TryWrite(7);
    });
    EXPECT_EQ(readTask.getResultProperty(), 7);
    writer.join();
}

TEST(ChannelTests, WriteAsync_BlocksUntilSpaceAvailable) {
    auto channel = Channel<int>::CreateBounded(1);
    channel.Writer->TryWrite(1);
    auto writeTask = channel.Writer->WriteAsync(2);

    int value = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    channel.Reader->TryRead(value); // frees a slot, allowing the pending WriteAsync to complete
    writeTask.Wait();
    EXPECT_EQ(value, 1);

    int second = 0;
    EXPECT_TRUE(channel.Reader->TryRead(second));
    EXPECT_EQ(second, 2);
}

// Regression test for a wave-3 audit finding: the default ChannelReader<T>::ReadAsync()/
// ChannelWriter<T>::WriteAsync() captured a raw `this` pointer into a lambda that runs on a
// background thread (Task's std::async(std::launch::async, ...)) outlasting the synchronous
// call. If the caller drops every other shared_ptr reference to the Channel/Reader/Writer
// right after issuing the call -- a realistic "fire and forget" pattern -- the background
// thread could still be executing against an already-destroyed object: a genuine
// heap-use-after-free, confirmed via a standalone AddressSanitizer repro against the pre-fix
// code (reliably crashed on the very first iteration). Fixed by having ChannelReader<T>/
// ChannelWriter<T> inherit std::enable_shared_from_this and capture shared_from_this()
// instead of `this`.
TEST(ChannelTests, WriteAsync_ChannelDroppedImmediately_StillCompletesSafely) {
    System::Threading::Tasks::Task task;
    {
        auto channel = Channel<int>::CreateBounded(1);
        task = channel.Writer->WriteAsync(42);
        // `channel` (and its shared_ptr Reader/Writer) is destroyed here. `task` is the only
        // thing keeping the write's background work referenced; before the fix, that
        // background work referenced the (now being destroyed) ChannelWriterImpl directly.
    }
    task.Wait();
    EXPECT_TRUE(task.getIsCompletedProperty());
    EXPECT_FALSE(task.getIsFaultedProperty());
}

static System::Threading::Tasks::TaskT<int> IssueReadAsyncThenDropChannel() {
    auto channel = Channel<int>::CreateUnbounded();
    channel.Writer->TryWrite(99);
    return channel.Reader->ReadAsync();
    // `channel` (and its shared_ptr Reader/Writer) is destroyed here, on function return.
}

TEST(ChannelTests, ReadAsync_ChannelDroppedImmediately_StillCompletesSafely) {
    auto task = IssueReadAsyncThenDropChannel();
    EXPECT_EQ(task.getResultProperty(), 99);
}

TEST(ChannelTests, ZeroCapacityChannel_TryWrite_SucceedsOnceThenBlocksLikeCapacityOne) {
    auto channel = Channel<int>::CreateBounded(0);
    EXPECT_TRUE(channel.Writer->TryWrite(1));
    EXPECT_FALSE(channel.Writer->TryWrite(2)); // full, Wait mode rejects synchronous TryWrite
    int value = 0;
    EXPECT_TRUE(channel.Reader->TryRead(value));
    EXPECT_EQ(value, 1);
    EXPECT_TRUE(channel.Writer->TryWrite(3)); // slot freed, write succeeds again
}

TEST(ChannelTests, ZeroCapacityChannel_WriteAsync_UnblocksOnceReaderDrains) {
    auto channel = Channel<int>::CreateBounded(0);
    channel.Writer->TryWrite(1);
    auto writeTask = channel.Writer->WriteAsync(2);

    int value = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    channel.Reader->TryRead(value); // frees the slot, allowing the pending WriteAsync to complete
    writeTask.Wait();
    EXPECT_EQ(value, 1);

    int second = 0;
    EXPECT_TRUE(channel.Reader->TryRead(second));
    EXPECT_EQ(second, 2);
}

TEST(ChannelTests, Completion_CompletesOnceClosedAndDrained) {
    auto channel = Channel<int>::CreateUnbounded();
    channel.Writer->TryWrite(1);
    channel.Writer->TryComplete();

    std::thread drainer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        int v;
        channel.Reader->TryRead(v);
    });
    channel.Reader->getCompletionProperty().Wait();
    drainer.join();
}

TEST(ChannelTests, CanCountAndCount) {
    auto channel = Channel<int>::CreateUnbounded();
    EXPECT_TRUE(channel.Reader->getCanCountProperty());
    channel.Writer->TryWrite(1);
    channel.Writer->TryWrite(2);
    EXPECT_EQ(channel.Reader->getCountProperty(), 2);
}

TEST(ChannelTests, ChannelClosedException_DefaultMessage) {
    ChannelClosedException ex;
    EXPECT_STREQ(ex.what(), "The channel has been closed.");
}

// Ticket 1726 (post-stabilization-audit): all prior std::thread-using tests above are
// single-writer/single-reader coordination checks. Channel<T> is exactly the kind of primitive
// real .NET code uses for fan-in/fan-out, so this exercises genuine multi-producer/
// multi-consumer contention: N writer threads each TryWrite a distinct block of values (using an
// unbounded channel, so TryWrite always succeeds immediately -- no need to also race
// WaitToWriteAsync backpressure here, that's covered by the existing bounded-channel tests
// above), M reader threads race TryRead against each other and against getIsCompletedProperty()
// once the writers finish and TryComplete() is called, verifying the total items read matches
// total written with no duplicates or losses.
TEST(ChannelTests, ConcurrentMultiWriterMultiReader_NoLostOrDuplicatedItems) {
    auto channel = Channel<int>::CreateUnbounded();
    constexpr int kWriters = 4;
    constexpr int kReaders = 4;
    constexpr int kPerWriter = 2000;
    constexpr int kTotal = kWriters * kPerWriter;

    std::vector<std::thread> writers;
    for (int w = 0; w < kWriters; ++w) {
        writers.emplace_back([&channel, w]() {
            for (int i = 0; i < kPerWriter; ++i) {
                EXPECT_TRUE(channel.Writer->TryWrite(w * kPerWriter + i));
            }
        });
    }
    for (auto& th : writers) th.join();
    channel.Writer->TryComplete();

    std::atomic<int> readCount{0};
    std::vector<int> seenPerValue(kTotal, 0);
    std::mutex seenMutex;
    std::vector<std::thread> readers;
    for (int r = 0; r < kReaders; ++r) {
        readers.emplace_back([&]() {
            int value;
            while (channel.Reader->TryRead(value)) {
                readCount.fetch_add(1, std::memory_order_relaxed);
                std::lock_guard<std::mutex> lock(seenMutex);
                ASSERT_GE(value, 0);
                ASSERT_LT(value, kTotal);
                seenPerValue[static_cast<size_t>(value)]++;
            }
        });
    }
    for (auto& th : readers) th.join();

    EXPECT_EQ(readCount.load(), kTotal);
    for (int v = 0; v < kTotal; ++v) {
        EXPECT_EQ(seenPerValue[static_cast<size_t>(v)], 1) << "value " << v << " seen "
            << seenPerValue[static_cast<size_t>(v)] << " times (expected exactly once)";
    }
}

TEST(ChannelTests, MultipleBlockedReaders_AllUnblockWithinBoundedTime) {
    // Regression test for a lost-wakeup bug (fixed 2026-07-14, confirmed via a standalone
    // repro): TryWrite used to call notify_one(), which wakes at most one blocked
    // WaitToReadAsync/ReadAsync waiter. With multiple readers concurrently blocked, that could
    // permanently starve any reader notify_one() doesn't happen to pick, even though real
    // .NET's WaitToReadAsync contract notifies every currently-pending registration on a write,
    // not just one. Uses a bounded poll-with-timeout (not a plain join()) so a regression fails
    // this test instead of hanging the suite -- any reader thread still running at the deadline
    // is detached, not joined, to avoid std::terminate on a joinable thread's destructor. See
    // PrioritizedChannelTests.MultipleBlockedReaders_AllUnblockWithinBoundedTime for the
    // equivalent coverage of PrioritizedChannelWriterImpl::TryWrite's identical fix.
    auto channel = Channel<int>::CreateUnbounded();
    constexpr int kReaders = 6;

    std::vector<std::atomic<bool>> done(kReaders);
    for (auto& d : done) d = false;
    std::vector<std::thread> readers;
    for (int r = 0; r < kReaders; ++r) {
        readers.emplace_back([&channel, &done, r]() {
            channel.Reader->ReadAsync().getResultProperty();
            done[static_cast<size_t>(r)] = true;
        });
    }

    // Give every reader time to actually block inside WaitToReadAsync's condition_variable::wait().
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    for (int i = 0; i < kReaders; ++i) {
        EXPECT_TRUE(channel.Writer->TryWrite(i));
    }

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    auto allDone = [&] {
        return std::all_of(done.begin(), done.end(), [](const std::atomic<bool>& d) { return d.load(); });
    };
    while (!allDone() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (int r = 0; r < kReaders; ++r) {
        EXPECT_TRUE(done[static_cast<size_t>(r)].load()) << "reader " << r << " never unblocked";
    }
    for (int r = 0; r < kReaders; ++r) {
        if (done[static_cast<size_t>(r)]) readers[static_cast<size_t>(r)].join();
        else readers[static_cast<size_t>(r)].detach();
    }
}

// -----------------------------------------------------------------------
// Channel<T>::CreateUnboundedPrioritized
// -----------------------------------------------------------------------

TEST(PrioritizedChannelTests, DefaultComparer_DequeuesSmallestFirst) {
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    channel.Writer->TryWrite(5);
    channel.Writer->TryWrite(1);
    channel.Writer->TryWrite(3);
    int a, b, c;
    ASSERT_TRUE(channel.Reader->TryRead(a));
    ASSERT_TRUE(channel.Reader->TryRead(b));
    ASSERT_TRUE(channel.Reader->TryRead(c));
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 3);
    EXPECT_EQ(c, 5);
}

TEST(PrioritizedChannelTests, WriteOrderDoesNotAffectReadOrder) {
    // Priority order, not insertion order -- distinguishes this from the plain FIFO channel.
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    for (int v : {10, 2, 8, 4, 6}) channel.Writer->TryWrite(v);
    std::vector<int> got;
    int value;
    while (channel.Reader->TryRead(value)) got.push_back(value);
    EXPECT_EQ(got, (std::vector<int>{2, 4, 6, 8, 10}));
}

TEST(PrioritizedChannelTests, CustomComparer_DescendingOrder) {
    UnboundedPrioritizedChannelOptions<int> options;
    options.Comparer = [](const int& a, const int& b) { return a > b; }; // largest first
    auto channel = Channel<int>::CreateUnboundedPrioritized(options);
    channel.Writer->TryWrite(1);
    channel.Writer->TryWrite(5);
    channel.Writer->TryWrite(3);
    int a, b, c;
    ASSERT_TRUE(channel.Reader->TryRead(a));
    ASSERT_TRUE(channel.Reader->TryRead(b));
    ASSERT_TRUE(channel.Reader->TryRead(c));
    EXPECT_EQ(a, 5);
    EXPECT_EQ(b, 3);
    EXPECT_EQ(c, 1);
}

TEST(PrioritizedChannelTests, TryRead_EmptyChannel_ReturnsFalse) {
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    int value;
    EXPECT_FALSE(channel.Reader->TryRead(value));
}

TEST(PrioritizedChannelTests, TryPeek_DoesNotRemoveItem) {
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    channel.Writer->TryWrite(7);
    channel.Writer->TryWrite(2);
    int peeked = 0;
    EXPECT_TRUE(channel.Reader->TryPeek(peeked));
    EXPECT_EQ(peeked, 2);
    EXPECT_EQ(channel.Reader->getCountProperty(), 2); // peek didn't remove it
    int read = 0;
    EXPECT_TRUE(channel.Reader->TryRead(read));
    EXPECT_EQ(read, 2);
}

TEST(PrioritizedChannelTests, CanPeekAndCanCountAreTrue) {
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    EXPECT_TRUE(channel.Reader->getCanPeekProperty());
    EXPECT_TRUE(channel.Reader->getCanCountProperty());
}

TEST(PrioritizedChannelTests, TryComplete_ThenTryWrite_Fails) {
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    EXPECT_TRUE(channel.Writer->TryComplete());
    EXPECT_FALSE(channel.Writer->TryWrite(1));
    EXPECT_FALSE(channel.Writer->TryComplete()); // already completed
}

TEST(PrioritizedChannelTests, WaitToWriteAsync_AlwaysTrueUntilClosed) {
    // Unbounded: writing never blocks on capacity, matching CreateUnbounded()'s own contract.
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    EXPECT_TRUE(channel.Writer->WaitToWriteAsync().getResultProperty());
    channel.Writer->TryComplete();
    EXPECT_FALSE(channel.Writer->WaitToWriteAsync().getResultProperty());
}

TEST(PrioritizedChannelTests, WaitToReadAsync_UnblocksWhenItemWritten) {
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    std::thread writer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        channel.Writer->TryWrite(9);
    });
    EXPECT_TRUE(channel.Reader->WaitToReadAsync().getResultProperty());
    writer.join();
    int value = 0;
    EXPECT_TRUE(channel.Reader->TryRead(value));
    EXPECT_EQ(value, 9);
}

TEST(PrioritizedChannelTests, WaitToReadAsync_ReturnsFalse_WhenClosedEmpty) {
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    channel.Writer->TryComplete();
    EXPECT_FALSE(channel.Reader->WaitToReadAsync().getResultProperty());
}

TEST(PrioritizedChannelTests, WaitToReadAsync_ThrowsCompletionError_WhenClosedWithException) {
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    channel.Writer->TryComplete(std::make_exception_ptr(std::runtime_error("boom")));
    EXPECT_THROW(channel.Reader->WaitToReadAsync().getResultProperty(), std::runtime_error);
}

TEST(PrioritizedChannelTests, Completion_CompletesOnceClosedAndDrained) {
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    channel.Writer->TryWrite(1);
    channel.Writer->TryComplete();
    auto completion = channel.Reader->getCompletionProperty();
    EXPECT_FALSE(completion.getIsCompletedProperty());
    int value;
    channel.Reader->TryRead(value);
    completion.Wait();
    EXPECT_TRUE(completion.getIsCompletedSuccessfullyProperty());
}

TEST(PrioritizedChannelTests, ReadAsync_ReturnsHighestPriorityItem) {
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    channel.Writer->TryWrite(9);
    channel.Writer->TryWrite(1);
    EXPECT_EQ(channel.Reader->ReadAsync().getResultProperty(), 1);
}

TEST(PrioritizedChannelTests, MultipleBlockedReaders_AllUnblockWithinBoundedTime) {
    // Regression test for a lost-wakeup bug (fixed 2026-07-14, confirmed via a standalone
    // repro): TryWrite used to call notify_one(), which wakes at most one blocked
    // WaitToReadAsync/ReadAsync waiter. With multiple readers concurrently blocked, that could
    // permanently starve any reader notify_one() doesn't happen to pick, even though real
    // .NET's WaitToReadAsync contract notifies every currently-pending registration on a write,
    // not just one. Uses a bounded poll-with-timeout (not a plain join()) so a regression fails
    // this test instead of hanging the suite -- any reader thread still running at the deadline
    // is detached, not joined, to avoid std::terminate on a joinable thread's destructor.
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    constexpr int kReaders = 6;

    std::vector<std::atomic<bool>> done(kReaders);
    for (auto& d : done) d = false;
    std::vector<std::thread> readers;
    for (int r = 0; r < kReaders; ++r) {
        readers.emplace_back([&channel, &done, r]() {
            channel.Reader->ReadAsync().getResultProperty();
            done[static_cast<size_t>(r)] = true;
        });
    }

    // Give every reader time to actually block inside WaitToReadAsync's condition_variable::wait().
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    for (int i = 0; i < kReaders; ++i) {
        EXPECT_TRUE(channel.Writer->TryWrite(i));
    }

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    auto allDone = [&] {
        return std::all_of(done.begin(), done.end(), [](const std::atomic<bool>& d) { return d.load(); });
    };
    while (!allDone() && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (int r = 0; r < kReaders; ++r) {
        EXPECT_TRUE(done[static_cast<size_t>(r)].load()) << "reader " << r << " never unblocked";
    }
    for (int r = 0; r < kReaders; ++r) {
        if (done[static_cast<size_t>(r)]) readers[static_cast<size_t>(r)].join();
        else readers[static_cast<size_t>(r)].detach();
    }
}

TEST(PrioritizedChannelTests, ConcurrentMultiWriterMultiReader_NoLostOrDuplicatedItems) {
    auto channel = Channel<int>::CreateUnboundedPrioritized();
    constexpr int kWriters = 4;
    constexpr int kPerWriter = 250;
    constexpr int kTotal = kWriters * kPerWriter;
    constexpr int kReaders = 4;

    std::vector<std::thread> writers;
    for (int w = 0; w < kWriters; ++w) {
        writers.emplace_back([&, w]() {
            for (int i = 0; i < kPerWriter; ++i) {
                channel.Writer->TryWrite(w * kPerWriter + i);
            }
        });
    }
    for (auto& th : writers) th.join();
    channel.Writer->TryComplete();

    std::atomic<int> readCount{0};
    std::vector<int> seenPerValue(kTotal, 0);
    std::mutex seenMutex;
    std::vector<std::thread> readers;
    for (int r = 0; r < kReaders; ++r) {
        readers.emplace_back([&]() {
            int value;
            while (channel.Reader->TryRead(value)) {
                readCount.fetch_add(1, std::memory_order_relaxed);
                std::lock_guard<std::mutex> lock(seenMutex);
                ASSERT_GE(value, 0);
                ASSERT_LT(value, kTotal);
                seenPerValue[static_cast<size_t>(value)]++;
            }
        });
    }
    for (auto& th : readers) th.join();

    EXPECT_EQ(readCount.load(), kTotal);
    for (int v = 0; v < kTotal; ++v) {
        EXPECT_EQ(seenPerValue[static_cast<size_t>(v)], 1) << "value " << v << " seen "
            << seenPerValue[static_cast<size_t>(v)] << " times (expected exactly once)";
    }
}
