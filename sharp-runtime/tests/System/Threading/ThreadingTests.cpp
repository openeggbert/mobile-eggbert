// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/Threading/Thread.hpp"
#include "System/Threading/Interlocked.hpp"
#include "System/Threading/LockRecursionException.hpp"
#include "System/Threading/Monitor.hpp"
#include "System/Threading/SynchronizationLockException.hpp"
#include "System/Threading/Mutex.hpp"
#include "System/Threading/Semaphore.hpp"
#include "System/Threading/SemaphoreSlim.hpp"
#include "System/Threading/ManualResetEvent.hpp"
#include "System/Threading/AutoResetEvent.hpp"
#include "System/Threading/CancellationToken.hpp"
#include "System/Threading/CancellationTokenRegistration.hpp"
#include "System/Threading/CancellationTokenSource.hpp"
#include "System/Threading/SpinLock.hpp"
#include "System/Threading/Volatile.hpp"
#include "System/Threading/Timeout.hpp"
#include "System/Threading/ThreadStateException.hpp"

using namespace System::Threading;

// ---------------------------------------------------------------------------
// Thread
// ---------------------------------------------------------------------------

TEST(ThreadingTests, Thread_Sleep_ZeroMs_DoesNotThrow) {
    EXPECT_NO_THROW(Thread::Sleep(0));
}

TEST(ThreadingTests, Thread_CurrentThread_ReturnsManagedThreadId) {
    auto proxy = Thread::CurrentThread();
    auto id = proxy.getManagedThreadIdProperty();
    (void)id; // any value is valid — just verifying no throw
}

TEST(ThreadingTests, Thread_CurrentThread_IsNotBackground) {
    EXPECT_FALSE(Thread::CurrentThread().getIsBackgroundProperty());
}

TEST(ThreadingTests, Thread_Ctor_ExecutesLambda) {
    std::atomic<int> counter{0};
    Thread t([&counter]{ counter.fetch_add(1); });
    t.Start();
    t.Join();
    EXPECT_EQ(counter.load(), 1);
}

TEST(ThreadingTests, Thread_Join_AfterCompletion_IsNotAlive) {
    Thread t([]{ Thread::Sleep(0); });
    t.Start();
    t.Join();
    EXPECT_FALSE(t.getIsAliveProperty());
}

TEST(ThreadingTests, Thread_NameProperty_RoundTrip) {
    Thread t([]{ Thread::Sleep(0); });
    t.setNameProperty("worker");
    EXPECT_EQ(t.getNameProperty(), "worker");
    t.Start();
    t.Join();
}

TEST(ThreadingTests, Thread_IsBackground_RoundTrip) {
    Thread t([]{ Thread::Sleep(0); });
    t.setIsBackgroundProperty(true);
    EXPECT_TRUE(t.getIsBackgroundProperty());
    t.Start();
    t.Join();
}

TEST(ThreadingTests, Thread_Start_StartsOnce_SecondThrows) {
    Thread t([]{ Thread::Sleep(0); });
    EXPECT_NO_THROW(t.Start());
    EXPECT_THROW(t.Start(), System::Threading::ThreadStateException);
    t.Join();
}

TEST(ThreadingTests, Thread_Sleep_LessThanNegativeOne_Throws) {
    EXPECT_THROW(Thread::Sleep(-2), System::ArgumentOutOfRangeException);
}

TEST(ThreadingTests, Thread_Join_LessThanNegativeOne_Throws) {
    Thread t([]{ Thread::Sleep(0); });
    t.Start();
    EXPECT_THROW(t.Join(-2), System::ArgumentOutOfRangeException);
    t.Join();
}

TEST(ThreadingTests, Thread_Join_InfiniteTimeout_BlocksUntilCompletion) {
    // Regression: Join(-1) previously computed a deadline in the past and returned
    // almost immediately instead of blocking forever (Timeout.Infinite semantics).
    std::atomic<bool> finished{false};
    Thread t([&finished]{
        Thread::Sleep(50);
        finished.store(true);
    });
    t.Start();
    EXPECT_TRUE(t.Join(-1));
    EXPECT_TRUE(finished.load());
}

TEST(ThreadingTests, Thread_Join_NoArg_OnUnstartedThread_Throws) {
    Thread t([]{ Thread::Sleep(0); });
    EXPECT_THROW(t.Join(), System::Threading::ThreadStateException);
}

TEST(ThreadingTests, Thread_Join_WithTimeout_OnUnstartedThread_Throws) {
    Thread t([]{ Thread::Sleep(0); });
    EXPECT_THROW(t.Join(100), System::Threading::ThreadStateException);
}

TEST(ThreadingTests, Thread_CurrentThread_InsideStartedThread_MatchesOwnManagedThreadId) {
    intcs idSeenFromInside = -1;
    Thread worker([&]{ idSeenFromInside = Thread::CurrentThread().getManagedThreadIdProperty(); });
    worker.Start();
    worker.Join();
    EXPECT_EQ(idSeenFromInside, worker.getManagedThreadIdProperty());
}

TEST(ThreadingTests, Thread_CurrentThread_InsideBackgroundThread_ReportsBackground) {
    bool sawBackground = false;
    Thread worker([&]{ sawBackground = Thread::CurrentThread().getIsBackgroundProperty(); });
    worker.setIsBackgroundProperty(true);
    worker.Start();
    worker.Join();
    EXPECT_TRUE(sawBackground);
}

TEST(ThreadingTests, Thread_ManagedThreadId_IsPositive) {
    Thread t([]{ Thread::Sleep(0); });
    EXPECT_GT(t.getManagedThreadIdProperty(), 0);
    t.Start();
    t.Join();
}

TEST(ThreadingTests, Thread_ManagedThreadId_UniqueAcrossThreads) {
    Thread t1([]{ Thread::Sleep(0); });
    Thread t2([]{ Thread::Sleep(0); });
    EXPECT_NE(t1.getManagedThreadIdProperty(), t2.getManagedThreadIdProperty());
    t1.Start();
    t2.Start();
    t1.Join();
    t2.Join();
}

TEST(ThreadingTests, Thread_IsAlive_TrueWhileRunning) {
    std::atomic<bool> hold{true};
    Thread t([&hold]{ while (hold.load()) Thread::Sleep(1); });
    t.Start();
    EXPECT_TRUE(t.getIsAliveProperty());
    hold.store(false);
    t.Join();
}

// ---------------------------------------------------------------------------
// Interlocked — int32
// ---------------------------------------------------------------------------

TEST(ThreadingTests, Interlocked_Increment_Int_ReturnsNewValue) {
    SharpRuntime::intcs v = 0;
    EXPECT_EQ(Interlocked::Increment(v), 1);
    EXPECT_EQ(v, 1);
}

TEST(ThreadingTests, Interlocked_Decrement_Int_ReturnsNewValue) {
    SharpRuntime::intcs v = 5;
    EXPECT_EQ(Interlocked::Decrement(v), 4);
    EXPECT_EQ(v, 4);
}

TEST(ThreadingTests, Interlocked_Add_Int_ReturnsSum) {
    SharpRuntime::intcs v = 10;
    EXPECT_EQ(Interlocked::Add(v, 3), 13);
    EXPECT_EQ(v, 13);
}

TEST(ThreadingTests, Interlocked_Exchange_Int_ReturnsOldValue) {
    SharpRuntime::intcs v = 7;
    SharpRuntime::intcs old = Interlocked::Exchange(v, 42);
    EXPECT_EQ(old, 7);
    EXPECT_EQ(v, 42);
}

TEST(ThreadingTests, Interlocked_CompareExchange_Match_Exchanges) {
    SharpRuntime::intcs v = 10;
    SharpRuntime::intcs ret = Interlocked::CompareExchange(v, 99, 10);
    EXPECT_EQ(ret, 10);
    EXPECT_EQ(v, 99);
}

TEST(ThreadingTests, Interlocked_CompareExchange_NoMatch_NoChange) {
    SharpRuntime::intcs v = 10;
    Interlocked::CompareExchange(v, 99, 55); // 55 != 10 → no change
    EXPECT_EQ(v, 10);
}

TEST(ThreadingTests, Interlocked_Read_Int_ReturnsValue) {
    SharpRuntime::intcs v = 123;
    EXPECT_EQ(Interlocked::Read(v), 123);
}

// ---------------------------------------------------------------------------
// Interlocked — int64
// ---------------------------------------------------------------------------

TEST(ThreadingTests, Interlocked_Increment_Long_ReturnsNewValue) {
    SharpRuntime::longcs v = 0LL;
    EXPECT_EQ(Interlocked::Increment(v), 1LL);
}

TEST(ThreadingTests, Interlocked_Decrement_Long_ReturnsNewValue) {
    SharpRuntime::longcs v = 100LL;
    EXPECT_EQ(Interlocked::Decrement(v), 99LL);
}

TEST(ThreadingTests, Interlocked_Add_Long_ReturnsSum) {
    SharpRuntime::longcs v = 1000LL;
    EXPECT_EQ(Interlocked::Add(v, 500LL), 1500LL);
}

// ---------------------------------------------------------------------------
// Monitor (stub — all no-ops that return true)
// ---------------------------------------------------------------------------

TEST(ThreadingTests, Monitor_Enter_DoesNotThrow) {
    int obj = 0;
    EXPECT_NO_THROW(Monitor::Enter(&obj));
    Monitor::Exit(&obj);
}

TEST(ThreadingTests, Monitor_Exit_AfterEnter_DoesNotThrow) {
    int obj = 0;
    Monitor::Enter(&obj);
    EXPECT_NO_THROW(Monitor::Exit(&obj));
}

TEST(ThreadingTests, Monitor_TryEnter_ReturnsTrue) {
    int obj = 0;
    EXPECT_TRUE(Monitor::TryEnter(&obj));
    Monitor::Exit(&obj);
}

TEST(ThreadingTests, Monitor_Enter_SetsLockTaken) {
    int obj = 0;
    bool taken = false;
    Monitor::Enter(&obj, taken);
    EXPECT_TRUE(taken);
    Monitor::Exit(&obj);
}

TEST(ThreadingTests, Monitor_EnterExit_ProvidesRealMutualExclusion) {
    int obj = 0;
    int counter = 0;
    constexpr int kIterations = 2000;
    auto worker = [&] {
        for (int i = 0; i < kIterations; ++i) {
            Monitor::Enter(&obj);
            ++counter;
            Monitor::Exit(&obj);
        }
    };
    std::thread t1(worker);
    std::thread t2(worker);
    t1.join();
    t2.join();
    EXPECT_EQ(counter, 2 * kIterations);
}

TEST(ThreadingTests, Monitor_TryEnter_WhileHeldByAnotherThread_ReturnsFalse) {
    int obj = 0;
    Monitor::Enter(&obj);
    std::atomic<bool> acquired{true};
    std::thread t([&] { acquired = Monitor::TryEnter(&obj); });
    t.join();
    EXPECT_FALSE(acquired.load());
    Monitor::Exit(&obj);
}

TEST(ThreadingTests, Monitor_Exit_NotHeld_ThrowsSynchronizationLockException) {
    int obj = 0;
    EXPECT_THROW(Monitor::Exit(&obj), System::Threading::SynchronizationLockException);
}

TEST(ThreadingTests, Monitor_Wait_NotHeld_ThrowsSynchronizationLockException) {
    int obj = 0;
    EXPECT_THROW(Monitor::Wait(&obj), System::Threading::SynchronizationLockException);
}

TEST(ThreadingTests, Monitor_Pulse_NotHeld_ThrowsSynchronizationLockException) {
    int obj = 0;
    EXPECT_THROW(Monitor::Pulse(&obj), System::Threading::SynchronizationLockException);
}

TEST(ThreadingTests, Monitor_Exit_FromNonOwningThread_ThrowsSynchronizationLockException) {
    int obj = 0;
    Monitor::Enter(&obj);
    bool threw = false;
    std::thread t([&] {
        try { Monitor::Exit(&obj); } catch (const System::Threading::SynchronizationLockException&) { threw = true; }
    });
    t.join();
    EXPECT_TRUE(threw);
    Monitor::Exit(&obj);
}

TEST(ThreadingTests, Monitor_AnotherThread_CanEnter_WhileFirstThreadIsWaiting) {
    // Verifies owner/depth tracking is correctly transferred during Wait()'s internal
    // unlock/relock window: while the waiter is blocked inside Wait(), a second thread
    // must be able to Enter() and later Exit() without a spurious SynchronizationLockException.
    int obj = 0;
    std::atomic<bool> waiterEntered{false};
    std::atomic<bool> signaled{false};
    std::thread waiter([&] {
        Monitor::Enter(&obj);
        waiterEntered = true;
        Monitor::Wait(&obj);
        Monitor::Exit(&obj);
    });
    while (!waiterEntered.load()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); // let waiter reach cv.wait()
    EXPECT_NO_THROW(Monitor::Enter(&obj));
    Monitor::PulseAll(&obj);
    EXPECT_NO_THROW(Monitor::Exit(&obj));
    waiter.join();
}

TEST(ThreadingTests, Monitor_PulseAll_WakesWaitingThread) {
    // Classic monitor predicate-loop pattern: `signaled` is only ever read/written while
    // holding the monitor lock, so there is no lost-wakeup window regardless of whether the
    // waiter reaches Wait() before or after the signaling thread sets the predicate.
    int obj = 0;
    bool signaled = false;
    std::atomic<bool> woke{false};
    std::thread waiter([&] {
        Monitor::Enter(&obj);
        while (!signaled) Monitor::Wait(&obj);
        woke = true;
        Monitor::Exit(&obj);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    Monitor::Enter(&obj);
    signaled = true;
    Monitor::PulseAll(&obj);
    Monitor::Exit(&obj);
    waiter.join();
    EXPECT_TRUE(woke.load());
}

// ---------------------------------------------------------------------------
// Mutex
// ---------------------------------------------------------------------------

TEST(ThreadingTests, Mutex_DefaultCtor_DoesNotThrow) {
    EXPECT_NO_THROW(Mutex m);
}

TEST(ThreadingTests, Mutex_WaitOneAndRelease_DoNotThrow) {
    Mutex m;
    EXPECT_NO_THROW(m.WaitOne());
    EXPECT_NO_THROW(m.ReleaseMutex());
}

TEST(ThreadingTests, Mutex_TryLock_WhenFree_ReturnsTrue) {
    Mutex m;
    bool acquired = m.WaitOne(0);
    if (acquired) m.ReleaseMutex();
    EXPECT_TRUE(acquired);
}

// ---------------------------------------------------------------------------
// Semaphore
// ---------------------------------------------------------------------------

TEST(ThreadingTests, Semaphore_Ctor_DoesNotThrow) {
    EXPECT_NO_THROW(Semaphore s(1, 2));
}

TEST(ThreadingTests, Semaphore_WaitOne_WhenSignaled_ReturnsTrue) {
    Semaphore s(1, 2);
    EXPECT_TRUE(s.WaitOne(0));
}

TEST(ThreadingTests, Semaphore_Release_ReturnsPreviousCount) {
    Semaphore s(0, 5);
    int prev = s.Release(3);
    EXPECT_EQ(prev, 0);
}

TEST(ThreadingTests, Semaphore_WaitOne_TimeoutLessThanNegativeOne_Throws) {
    Semaphore s(1, 2);
    EXPECT_THROW(s.WaitOne(-2), System::ArgumentOutOfRangeException);
}

// Regression tests for a wave-3 audit finding: the constructor threw
// ArgumentOutOfRangeException when initialCount > maximumCount (both individually valid).
// Verified against Semaphore.cs's ValidateArguments: real .NET throws a plain ArgumentException
// (Argument_SemaphoreInitialMaximum) for exactly this case; ArgumentOutOfRangeException is only
// for each count being individually out of range (negative initialCount, non-positive
// maximumCount).
TEST(ThreadingTests, Semaphore_Ctor_InitialGreaterThanMaximum_ThrowsArgumentException) {
    EXPECT_THROW(Semaphore(5, 2), System::ArgumentException);
}

TEST(ThreadingTests, Semaphore_Ctor_NegativeInitialCount_ThrowsArgumentOutOfRangeException) {
    EXPECT_THROW(Semaphore(-1, 2), System::ArgumentOutOfRangeException);
}

TEST(ThreadingTests, Semaphore_Ctor_NonPositiveMaximumCount_ThrowsArgumentOutOfRangeException) {
    EXPECT_THROW(Semaphore(0, 0), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// SemaphoreSlim
// ---------------------------------------------------------------------------

TEST(ThreadingTests, SemaphoreSlim_InitialCount_IsCorrect) {
    SemaphoreSlim ss(3, 5);
    EXPECT_EQ(ss.getCurrentCountProperty(), 3);
}

TEST(ThreadingTests, SemaphoreSlim_Wait_WhenReady_DecreasesCount) {
    SemaphoreSlim ss(2, 5);
    ss.Wait(0);
    EXPECT_EQ(ss.getCurrentCountProperty(), 1);
}

TEST(ThreadingTests, SemaphoreSlim_Release_IncreasesCount) {
    SemaphoreSlim ss(0, 5);
    int prev = ss.Release();
    EXPECT_EQ(prev, 0);
    EXPECT_EQ(ss.getCurrentCountProperty(), 1);
}

TEST(ThreadingTests, SemaphoreSlim_Release_Multiple_IncreasesCount) {
    SemaphoreSlim ss(0, 10);
    ss.Release(4);
    EXPECT_EQ(ss.getCurrentCountProperty(), 4);
}

TEST(ThreadingTests, SemaphoreSlim_Wait_TimeoutLessThanNegativeOne_Throws) {
    SemaphoreSlim ss(1, 2);
    EXPECT_THROW(ss.Wait(-2), System::ArgumentOutOfRangeException);
}

// Regression tests for a wave-3 audit finding: SemaphoreSlim had no disposed_ flag at all --
// Dispose() was a true no-op and Wait/Release never checked disposal state, an inconsistency
// with sibling primitives (Barrier, CountdownEvent, ManualResetEventSlim, ReaderWriterLockSlim)
// that all already guard against post-Dispose() use. Verified against SemaphoreSlim.cs's
// CheckDispose(): Wait(int) validates its timeout argument *before* checking disposal, while
// Release(int) checks disposal *before* validating releaseCount -- each in its own .NET
// source's order, not a uniform rule across both methods.
TEST(ThreadingTests, SemaphoreSlim_AfterDispose_Wait_ThrowsObjectDisposedException) {
    SemaphoreSlim ss(1, 2);
    ss.Dispose();
    EXPECT_THROW(ss.Wait(), System::ObjectDisposedException);
    EXPECT_THROW(ss.Wait(10), System::ObjectDisposedException);
}

TEST(ThreadingTests, SemaphoreSlim_AfterDispose_Release_ThrowsObjectDisposedException) {
    SemaphoreSlim ss(1, 2);
    ss.Dispose();
    EXPECT_THROW(ss.Release(), System::ObjectDisposedException);
    EXPECT_THROW(ss.Release(1), System::ObjectDisposedException);
}

TEST(ThreadingTests, SemaphoreSlim_Wait_InvalidTimeoutCheckedBeforeDisposal) {
    SemaphoreSlim ss(1, 2);
    ss.Dispose();
    // Matches SemaphoreSlim.cs's Wait(int): timeout validation happens before CheckDispose(),
    // so an already-invalid timeout still throws ArgumentOutOfRangeException even when disposed.
    EXPECT_THROW(ss.Wait(-2), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// ManualResetEvent
// ---------------------------------------------------------------------------

TEST(ThreadingTests, ManualResetEvent_InitiallyNotSignaled_WaitTimesOut) {
    ManualResetEvent e(false);
    EXPECT_FALSE(e.WaitOne(0));
}

TEST(ThreadingTests, ManualResetEvent_InitiallySignaled_WaitSucceeds) {
    ManualResetEvent e(true);
    EXPECT_TRUE(e.WaitOne(0));
}

TEST(ThreadingTests, ManualResetEvent_SetThenWait_Succeeds) {
    ManualResetEvent e(false);
    e.Set();
    EXPECT_TRUE(e.WaitOne(0));
}

TEST(ThreadingTests, ManualResetEvent_SetThenReset_WaitTimesOut) {
    ManualResetEvent e(false);
    e.Set();
    e.Reset();
    EXPECT_FALSE(e.WaitOne(0));
}

TEST(ThreadingTests, ManualResetEvent_StaysSignaledAfterMultipleWaits) {
    ManualResetEvent e(true);
    EXPECT_TRUE(e.WaitOne(0));
    EXPECT_TRUE(e.WaitOne(0)); // still signaled
}

TEST(ThreadingTests, ManualResetEvent_WaitOne_TimeoutLessThanNegativeOne_Throws) {
    ManualResetEvent e(false);
    EXPECT_THROW(e.WaitOne(-2), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// AutoResetEvent
// ---------------------------------------------------------------------------

TEST(ThreadingTests, AutoResetEvent_InitiallyNotSignaled_WaitTimesOut) {
    AutoResetEvent e(false);
    EXPECT_FALSE(e.WaitOne(0));
}

// Regression test for a wave-3 audit finding: WaitOne(intcs) was missing the
// WaitHandle::ValidateTimeout(...) call present on every sibling wait-handle type in this
// codebase (ManualResetEvent, EventWaitHandle, Mutex, Semaphore, etc.) -- a timeout below -1
// silently reached cv_.wait_for with a negative duration instead of throwing.
TEST(ThreadingTests, AutoResetEvent_WaitOne_TimeoutLessThanNegativeOne_Throws) {
    AutoResetEvent e(false);
    EXPECT_THROW(e.WaitOne(-2), System::ArgumentOutOfRangeException);
}

TEST(ThreadingTests, AutoResetEvent_InitiallySignaled_WaitSucceeds) {
    AutoResetEvent e(true);
    EXPECT_TRUE(e.WaitOne(0));
}

TEST(ThreadingTests, AutoResetEvent_AfterWait_AutoResets) {
    AutoResetEvent e(true);
    e.WaitOne(0);             // consumes the signal
    EXPECT_FALSE(e.WaitOne(0)); // auto-reset — no longer signaled
}

TEST(ThreadingTests, AutoResetEvent_SetThenWait_Succeeds) {
    AutoResetEvent e(false);
    e.Set();
    EXPECT_TRUE(e.WaitOne(0));
}

// ---------------------------------------------------------------------------
// CancellationToken / CancellationTokenSource
// ---------------------------------------------------------------------------

TEST(ThreadingTests, CancellationToken_None_NotCancelled) {
    EXPECT_FALSE(CancellationToken::None().getIsCancellationRequestedProperty());
}

TEST(ThreadingTests, CancellationTokenSource_Initially_NotCancelled) {
    CancellationTokenSource cts;
    EXPECT_FALSE(cts.getIsCancellationRequestedProperty());
    EXPECT_FALSE(cts.getTokenProperty().getIsCancellationRequestedProperty());
}

TEST(ThreadingTests, CancellationTokenSource_Cancel_PropagatesToken) {
    CancellationTokenSource cts;
    CancellationToken tok = cts.getTokenProperty();
    cts.Cancel();
    EXPECT_TRUE(cts.getIsCancellationRequestedProperty());
    EXPECT_TRUE(tok.getIsCancellationRequestedProperty());
}

TEST(ThreadingTests, CancellationTokenSource_AfterDispose_Cancel_ThrowsObjectDisposedException) {
    CancellationTokenSource cts;
    cts.Dispose();
    EXPECT_THROW(cts.Cancel(), System::ObjectDisposedException);
}

TEST(ThreadingTests, CancellationTokenSource_AfterDispose_GetToken_ThrowsObjectDisposedException) {
    CancellationTokenSource cts;
    cts.Dispose();
    EXPECT_THROW(cts.getTokenProperty(), System::ObjectDisposedException);
}

TEST(ThreadingTests, CancellationTokenSource_ConcurrentDisposeAndAccess_NoDataRace) {
    // Regression: disposed_ was previously a plain (non-atomic) bool read by getTokenProperty/
    // Cancel's ThrowIfDisposed() check and written by Dispose() with no synchronization -- a
    // genuine data race (undefined behavior) under concurrent access, e.g. under a thread
    // sanitizer. This test doesn't assert a specific outcome (either "still valid" or
    // "ObjectDisposedException" are both acceptable depending on timing) -- it only exercises
    // the concurrent access pattern so a race would be caught by TSan/ASan if reintroduced.
    for (int iter = 0; iter < 200; ++iter) {
        CancellationTokenSource cts;
        std::thread disposer([&] { cts.Dispose(); });
        std::thread accessor([&] {
            try { cts.getTokenProperty(); } catch (const System::ObjectDisposedException&) {}
            try { (void)cts.getIsCancellationRequestedProperty(); } catch (...) {}
        });
        disposer.join();
        accessor.join();
    }
}

TEST(ThreadingTests, CancellationTokenSource_Cancel_RunsCallbacksInLifoOrder) {
    // Regression: callbacks were stored in a std::unordered_map, so Cancel()'s collection loop
    // iterated in arbitrary bucket order unrelated to registration order. Verified against
    // CancellationTokenSource.cs's ExecuteCallbackHandlers: real .NET fires callbacks in LIFO
    // order (most-recently-registered first, "deepest first") so nested/child registrations run
    // before their parents'.
    CancellationTokenSource cts;
    CancellationToken tok = cts.getTokenProperty();
    std::vector<int> order;
    tok.Register([&] { order.push_back(1); });
    tok.Register([&] { order.push_back(2); });
    tok.Register([&] { order.push_back(3); });
    cts.Cancel();
    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], 3);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 1);
}

// ---------------------------------------------------------------------------
// SpinLock
// ---------------------------------------------------------------------------

TEST(ThreadingTests, SpinLock_Enter_SetsLockTaken) {
    SpinLock sl;
    bool taken = false;
    sl.Enter(taken);
    EXPECT_TRUE(taken);
    sl.Exit();
}

TEST(ThreadingTests, SpinLock_TryEnter_WhenFree_ReturnsTrue) {
    SpinLock sl;
    bool taken = false;
    bool ok = sl.TryEnter(taken);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(taken);
    sl.Exit();
}

TEST(ThreadingTests, SpinLock_Exit_DoesNotThrow) {
    SpinLock sl;
    bool taken = false;
    sl.Enter(taken);
    EXPECT_NO_THROW(sl.Exit());
}

// Regression tests for a wave-3 audit finding: SpinLock was a bare atomic_flag with no thread
// ownership tracking at all -- getIsHeldProperty() always returned false (a hard-coded stub),
// Exit() never validated ownership, and there was no re-entrancy or IsHeldByCurrentThread
// support, unlike real SpinLock.cs. Verified against SpinLock.cs's default constructor (thread
// ownership tracking enabled), Enter()'s LockRecursionException on same-thread re-entry, and
// ExitSlowPath()'s SynchronizationLockException when the calling thread doesn't own the lock.

TEST(ThreadingTests, SpinLock_IsHeld_ReflectsActualState) {
    SpinLock sl;
    EXPECT_FALSE(sl.getIsHeldProperty());
    bool taken = false;
    sl.Enter(taken);
    EXPECT_TRUE(sl.getIsHeldProperty());
    sl.Exit();
    EXPECT_FALSE(sl.getIsHeldProperty());
}

TEST(ThreadingTests, SpinLock_IsHeldByCurrentThread_TracksOwner) {
    SpinLock sl;
    bool taken = false;
    sl.Enter(taken);
    EXPECT_TRUE(sl.getIsHeldByCurrentThreadProperty());
    sl.Exit();
    EXPECT_FALSE(sl.getIsHeldByCurrentThreadProperty());
}

TEST(ThreadingTests, SpinLock_IsHeldByCurrentThread_ThrowsWhenTrackingDisabled) {
    SpinLock sl(false);
    EXPECT_THROW(sl.getIsHeldByCurrentThreadProperty(), System::InvalidOperationException);
}

TEST(ThreadingTests, SpinLock_Enter_SameThreadReentry_ThrowsLockRecursionException) {
    SpinLock sl;
    bool taken1 = false;
    sl.Enter(taken1);
    bool taken2 = false;
    EXPECT_THROW(sl.Enter(taken2), System::Threading::LockRecursionException);
    sl.Exit();
}

TEST(ThreadingTests, SpinLock_Enter_LockTakenAlreadyTrue_ThrowsArgumentException) {
    SpinLock sl;
    bool taken = true;
    EXPECT_THROW(sl.Enter(taken), System::ArgumentException);
}

TEST(ThreadingTests, SpinLock_Exit_WithoutOwnership_ThrowsSynchronizationLockException) {
    SpinLock sl;
    std::thread other([&sl]() {
        bool taken = false;
        sl.Enter(taken);
    });
    other.join();
    // sl is now held by the (finished) "other" thread; this thread never entered it.
    EXPECT_THROW(sl.Exit(), System::Threading::SynchronizationLockException);
}

TEST(ThreadingTests, SpinLock_TryEnter_WhenHeldByAnotherThread_TimesOutAndReturnsFalse) {
    SpinLock sl;
    bool taken1 = false;
    sl.Enter(taken1);
    std::atomic<bool> ok{true};
    std::thread other([&]() {
        bool taken2 = false;
        ok = sl.TryEnter(20, taken2);
    });
    other.join();
    EXPECT_FALSE(ok.load());
    sl.Exit();
}

TEST(ThreadingTests, SpinLock_TryEnter_NegativeTimeoutOtherThanInfinite_ThrowsArgumentOutOfRangeException) {
    SpinLock sl;
    bool taken = false;
    EXPECT_THROW(sl.TryEnter(-2, taken), System::ArgumentOutOfRangeException);
}

TEST(ThreadingTests, SpinLock_ThreadOwnerTrackingDisabled_ExitNeverThrows) {
    SpinLock sl(false);
    EXPECT_FALSE(sl.getIsThreadOwnerTrackingEnabledProperty());
    bool taken = false;
    sl.Enter(taken);
    EXPECT_NO_THROW(sl.Exit());
}

// ---------------------------------------------------------------------------
// Volatile
// ---------------------------------------------------------------------------

TEST(ThreadingTests, Volatile_WriteAndRead_Int_RoundTrip) {
    int v = 0;
    Volatile::Write(v, 42);
    EXPECT_EQ(Volatile::Read(v), 42);
}

TEST(ThreadingTests, Volatile_WriteAndRead_Long_RoundTrip) {
    long v = 0L;
    Volatile::Write(v, 12345L);
    EXPECT_EQ(Volatile::Read(v), 12345L);
}

// ---------------------------------------------------------------------------
// Timeout constants
// ---------------------------------------------------------------------------

TEST(ThreadingTests, Timeout_Infinite_IsMinusOne) {
    EXPECT_EQ(Timeout::Infinite, -1);
}
