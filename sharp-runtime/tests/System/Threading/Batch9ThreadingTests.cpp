// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for newly-added/newly-fixed Threading types: LockCookie, ReaderWriterLock,
// ReaderWriterLockSlim held-state tracking, ExecutionContext, IThreadPoolWorkItem,
// WaitCallback/WaitOrTimerCallback, RegisteredWaitHandle/ThreadPool::RegisterWaitForSingleObject,
// WaitHandle::WaitAll/WaitAny, Barrier post-phase exception wrapping, and Mutex's
// initiallyOwned/timeout fixes.
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>
#include "System/AggregateException.hpp"
#include "System/ApplicationException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Threading/AbandonedMutexException.hpp"
#include "System/Threading/Barrier.hpp"
#include "System/Threading/BarrierPostPhaseException.hpp"
#include "System/Threading/CancellationTokenRegistration.hpp"
#include "System/Threading/CancellationTokenSource.hpp"
#include "System/Threading/ExecutionContext.hpp"
#include "System/Threading/IThreadPoolWorkItem.hpp"
#include "System/Threading/LockCookie.hpp"
#include "System/Threading/Mutex.hpp"
#include "System/Threading/ReaderWriterLock.hpp"
#include "System/Threading/ReaderWriterLockSlim.hpp"
#include "System/Threading/RegisteredWaitHandle.hpp"
#include "System/Threading/Semaphore.hpp"
#include "System/Threading/ThreadPool.hpp"
#include "System/Threading/WaitCallback.hpp"
#include "System/Threading/WaitHandle.hpp"
#include "System/Threading/WaitOrTimerCallback.hpp"

using namespace System::Threading;

// ===========================================================================
// AbandonedMutexException
// ===========================================================================

TEST(AbandonedMutexExceptionTests, LocationAndHandleCtor_SetsMutexAndIndex) {
    Mutex m;
    AbandonedMutexException ex(3, &m);
    EXPECT_EQ(ex.getMutexIndexProperty(), 3);
    EXPECT_EQ(ex.getMutexProperty(), &m);
}

TEST(AbandonedMutexExceptionTests, DefaultCtor_HasNoMutex) {
    AbandonedMutexException ex;
    EXPECT_EQ(ex.getMutexIndexProperty(), -1);
    EXPECT_EQ(ex.getMutexProperty(), nullptr);
}

// ===========================================================================
// CancellationToken::Register / CancellationTokenSource callback invocation
// ===========================================================================

TEST(CancellationTokenRegisterTests, Register_InvokedOnCancel) {
    CancellationTokenSource cts;
    CancellationToken token = cts.getTokenProperty();
    bool called = false;
    auto reg = token.Register([&] { called = true; });
    EXPECT_FALSE(called);
    cts.Cancel();
    EXPECT_TRUE(called);
    reg.Dispose();
}

TEST(CancellationTokenRegisterTests, Register_AfterAlreadyCancelled_InvokesImmediately) {
    CancellationTokenSource cts;
    cts.Cancel();
    bool called = false;
    auto reg = cts.getTokenProperty().Register([&] { called = true; });
    EXPECT_TRUE(called);
    reg.Dispose();
}

TEST(CancellationTokenRegisterTests, DisposedRegistration_NotInvokedOnCancel) {
    CancellationTokenSource cts;
    CancellationToken token = cts.getTokenProperty();
    bool called = false;
    auto reg = token.Register([&] { called = true; });
    reg.Dispose();
    cts.Cancel();
    EXPECT_FALSE(called);
}

TEST(CancellationTokenRegisterTests, Registration_IsActiveUntilDisposedOrCancelled) {
    CancellationTokenSource cts;
    auto reg = cts.getTokenProperty().Register([] {});
    EXPECT_TRUE(reg.getIsActiveProperty());
    cts.Cancel();
    EXPECT_FALSE(reg.getIsActiveProperty());
}

TEST(CancellationTokenRegisterTests, Cancel_ThrowingCallback_RunsAllCallbacksAndThrowsAggregateException) {
    CancellationTokenSource cts;
    CancellationToken token = cts.getTokenProperty();
    bool secondCalled = false;
    bool thirdCalled = false;
    auto reg1 = token.Register([&] { throw std::runtime_error("first"); });
    auto reg2 = token.Register([&] { secondCalled = true; });
    auto reg3 = token.Register([&] { thirdCalled = true; throw std::runtime_error("third"); });
    EXPECT_THROW(cts.Cancel(), System::AggregateException);
    EXPECT_TRUE(secondCalled);
    EXPECT_TRUE(thirdCalled);
    reg1.Dispose();
    reg2.Dispose();
    reg3.Dispose();
}

// Regression test for a wave-3 audit finding: CancellationTokenRegistration::Dispose() was a
// no-op race when the callback was already picked up by a concurrent Cancel() -- it returned
// immediately instead of waiting for the in-flight callback to finish, risking use-after-free
// if the caller tears down a resource the callback references right after Dispose() returns.
// Verified against CancellationTokenRegistration.cs's documented Dispose() contract: "If the
// target callback is currently executing, this method will wait until it completes."
TEST(CancellationTokenRegisterTests, Dispose_WaitsForInFlightCallbackToFinish) {
    CancellationTokenSource cts;
    CancellationToken token = cts.getTokenProperty();

    std::atomic<bool> callbackStarted{false};
    std::atomic<bool> callbackFinished{false};

    auto reg = token.Register([&] {
        callbackStarted = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        callbackFinished = true;
    });

    std::thread canceller([&] { cts.Cancel(); });

    while (!callbackStarted.load()) std::this_thread::yield();
    // The callback is now guaranteed to be mid-execution (sleeping) -- sanity-check the race
    // window is real before trusting the post-Dispose() assertion below.
    bool observedStillRunning = !callbackFinished.load();

    reg.Dispose();
    EXPECT_TRUE(observedStillRunning);
    EXPECT_TRUE(callbackFinished.load());

    canceller.join();
}

// Regression test: the wait-for-completion contract must not deadlock when a callback disposes
// its own registration (the documented "degenerate case" in CancellationTokenRegistration.cs).
TEST(CancellationTokenRegisterTests, SelfUnregisteringCallback_DoesNotDeadlock) {
    CancellationTokenSource cts;
    CancellationToken token = cts.getTokenProperty();
    CancellationTokenRegistration* selfPtr = nullptr;
    bool called = false;
    auto reg = token.Register([&] {
        called = true;
        selfPtr->Dispose();
    });
    selfPtr = &reg;
    cts.Cancel();
    EXPECT_TRUE(called);
}

// ===========================================================================
// LockCookie
// ===========================================================================

TEST(LockCookieTests, DefaultCookies_AreEqual) {
    LockCookie a, b;
    EXPECT_EQ(a, b);
    EXPECT_TRUE(a.Equals(b));
}

// ===========================================================================
// ReaderWriterLock (legacy)
// ===========================================================================

TEST(ReaderWriterLockTests, AcquireReleaseReaderLock_TracksHeldState) {
    ReaderWriterLock lock;
    EXPECT_FALSE(lock.getIsReaderLockHeldProperty());
    lock.AcquireReaderLock(-1);
    EXPECT_TRUE(lock.getIsReaderLockHeldProperty());
    lock.ReleaseReaderLock();
    EXPECT_FALSE(lock.getIsReaderLockHeldProperty());
}

TEST(ReaderWriterLockTests, AcquireReleaseWriterLock_TracksHeldState) {
    ReaderWriterLock lock;
    EXPECT_FALSE(lock.getIsWriterLockHeldProperty());
    lock.AcquireWriterLock(-1);
    EXPECT_TRUE(lock.getIsWriterLockHeldProperty());
    lock.ReleaseWriterLock();
    EXPECT_FALSE(lock.getIsWriterLockHeldProperty());
}

TEST(ReaderWriterLockTests, NestedReaderLock_RequiresMatchingReleases) {
    ReaderWriterLock lock;
    lock.AcquireReaderLock(-1);
    lock.AcquireReaderLock(-1);
    EXPECT_TRUE(lock.getIsReaderLockHeldProperty());
    lock.ReleaseReaderLock();
    EXPECT_TRUE(lock.getIsReaderLockHeldProperty());
    lock.ReleaseReaderLock();
    EXPECT_FALSE(lock.getIsReaderLockHeldProperty());
}

TEST(ReaderWriterLockTests, UpgradeToWriterLock_ThenDowngrade_RestoresReaderLock) {
    ReaderWriterLock lock;
    lock.AcquireReaderLock(-1);
    LockCookie cookie = lock.UpgradeToWriterLock(-1);
    EXPECT_TRUE(lock.getIsWriterLockHeldProperty());
    lock.DowngradeFromWriterLock(cookie);
    EXPECT_FALSE(lock.getIsWriterLockHeldProperty());
    EXPECT_TRUE(lock.getIsReaderLockHeldProperty());
    lock.ReleaseReaderLock();
}

TEST(ReaderWriterLockTests, ReleaseLock_ThenRestoreLock_RoundTrips) {
    ReaderWriterLock lock;
    lock.AcquireWriterLock(-1);
    LockCookie cookie = lock.ReleaseLock();
    EXPECT_FALSE(lock.getIsWriterLockHeldProperty());
    lock.RestoreLock(cookie);
    EXPECT_TRUE(lock.getIsWriterLockHeldProperty());
    lock.ReleaseWriterLock();
}

TEST(ReaderWriterLockTests, WriterSeqNum_IncrementsOnEachRelease) {
    ReaderWriterLock lock;
    SharpRuntime::intcs initial = lock.getWriterSeqNumProperty();
    lock.AcquireWriterLock(-1);
    lock.ReleaseWriterLock();
    EXPECT_GT(lock.getWriterSeqNumProperty(), initial);
}

TEST(ReaderWriterLockTests, AcquireReaderLock_NegativeTimeoutBelowMinusOne_Throws) {
    ReaderWriterLock lock;
    EXPECT_THROW(lock.AcquireReaderLock(-2), System::ArgumentOutOfRangeException);
}

// Regression tests for a wave-3 audit finding: ReleaseReaderLock/ReleaseWriterLock threw
// SynchronizationLockException when the calling thread didn't hold the lock. Verified against
// ReaderWriterLock.cs's GetNotOwnerException(): real .NET's private
// ReaderWriterLockApplicationException derives from ApplicationException, not
// SynchronizationLockException -- the same exception family as the already-correct timeout
// path in this class (AcquireReaderLock/AcquireWriterLock, tested above).
TEST(ReaderWriterLockTests, ReleaseReaderLock_NotHeld_ThrowsApplicationException) {
    ReaderWriterLock lock;
    EXPECT_THROW(lock.ReleaseReaderLock(), System::ApplicationException);
}

TEST(ReaderWriterLockTests, ReleaseWriterLock_NotHeld_ThrowsApplicationException) {
    ReaderWriterLock lock;
    EXPECT_THROW(lock.ReleaseWriterLock(), System::ApplicationException);
}

TEST(ReaderWriterLockTests, AcquireWriterLock_TimesOut_ThrowsApplicationException) {
    ReaderWriterLock lock;
    lock.AcquireWriterLock(-1);
    std::atomic<bool> threw{false};
    std::thread t([&] {
        try {
            lock.AcquireWriterLock(50);
        } catch (const System::ApplicationException&) {
            threw = true;
        }
    });
    t.join();
    EXPECT_TRUE(threw.load());
    EXPECT_TRUE(lock.getIsWriterLockHeldProperty());
    lock.ReleaseWriterLock();
}

TEST(ReaderWriterLockTests, AcquireReaderLock_TimesOut_ThrowsApplicationException) {
    ReaderWriterLock lock;
    lock.AcquireWriterLock(-1);
    std::atomic<bool> threw{false};
    std::thread t([&] {
        try {
            lock.AcquireReaderLock(50);
        } catch (const System::ApplicationException&) {
            threw = true;
        }
    });
    t.join();
    EXPECT_TRUE(threw.load());
    lock.ReleaseWriterLock();
}

// ===========================================================================
// ReaderWriterLockSlim held-state tracking
// ===========================================================================

TEST(ReaderWriterLockSlimTests, IsReadLockHeld_TracksActualState) {
    ReaderWriterLockSlim rwl;
    EXPECT_FALSE(rwl.getIsReadLockHeldProperty());
    rwl.EnterReadLock();
    EXPECT_TRUE(rwl.getIsReadLockHeldProperty());
    rwl.ExitReadLock();
    EXPECT_FALSE(rwl.getIsReadLockHeldProperty());
}

TEST(ReaderWriterLockSlimTests, IsWriteLockHeld_TracksActualState) {
    ReaderWriterLockSlim rwl;
    EXPECT_FALSE(rwl.getIsWriteLockHeldProperty());
    rwl.EnterWriteLock();
    EXPECT_TRUE(rwl.getIsWriteLockHeldProperty());
    rwl.ExitWriteLock();
    EXPECT_FALSE(rwl.getIsWriteLockHeldProperty());
}

TEST(ReaderWriterLockSlimTests, IsUpgradeableReadLockHeld_TracksActualState) {
    ReaderWriterLockSlim rwl;
    EXPECT_FALSE(rwl.getIsUpgradeableReadLockHeldProperty());
    rwl.EnterUpgradeableReadLock();
    EXPECT_TRUE(rwl.getIsUpgradeableReadLockHeldProperty());
    rwl.ExitUpgradeableReadLock();
    EXPECT_FALSE(rwl.getIsUpgradeableReadLockHeldProperty());
}

// ===========================================================================
// Mutex: initiallyOwned + real timeout
// ===========================================================================

TEST(MutexTests, InitiallyOwned_True_IsAlreadyLockedByConstructingThread) {
    Mutex m(true);
    // Recursive: the constructing thread can re-enter without blocking.
    EXPECT_TRUE(m.WaitOne(0));
    m.ReleaseMutex();
    m.ReleaseMutex();
}

TEST(MutexTests, WaitOne_Timeout_WhenHeldByAnotherThread_WaitsFullDuration) {
    Mutex m;
    m.WaitOne();
    std::atomic<bool> acquired{true};
    auto start = std::chrono::steady_clock::now();
    std::thread t([&] { acquired = m.WaitOne(100); });
    t.join();
    auto elapsed = std::chrono::steady_clock::now() - start;
    EXPECT_FALSE(acquired.load());
    EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(), 90);
    m.ReleaseMutex();
}

TEST(MutexTests, WaitOne_TimeoutLessThanNegativeOne_Throws) {
    Mutex m;
    EXPECT_THROW(m.WaitOne(-2), System::ArgumentOutOfRangeException);
}

// Regression test for a wave-3 audit finding: WaitOne(-1) (Timeout.Infinite) used to feed
// -1 straight into std::chrono's try_lock_for, which treats a negative duration as
// already-expired -- so it returned almost immediately instead of blocking forever like
// real .NET. Verified by holding the mutex on the main thread, releasing it only after the
// background thread has been blocked in WaitOne(-1) for longer than any "immediately
// expired" timeout could account for.
TEST(MutexTests, WaitOne_InfiniteTimeout_BlocksUntilReleased) {
    Mutex m;
    m.WaitOne(); // main thread acquires
    std::atomic<bool> acquired{false};
    std::thread t([&] {
        acquired = m.WaitOne(-1); // should block until the main thread releases
        if (acquired) m.ReleaseMutex(); // release from the thread that actually acquired it
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(acquired.load()); // still blocked after 100ms -- proves it didn't return immediately

    m.ReleaseMutex(); // main thread releases its own hold, letting t proceed
    t.join();
    EXPECT_TRUE(acquired.load());
}

TEST(MutexTests, ReleaseMutex_NotOwned_ThrowsApplicationException) {
    Mutex m;
    EXPECT_THROW(m.ReleaseMutex(), System::ApplicationException);
}

TEST(MutexTests, ReleaseMutex_FromNonOwningThread_ThrowsApplicationException) {
    Mutex m;
    m.WaitOne();
    bool threw = false;
    std::thread t([&] {
        try { m.ReleaseMutex(); } catch (const System::ApplicationException&) { threw = true; }
    });
    t.join();
    EXPECT_TRUE(threw);
    m.ReleaseMutex();
}

// ===========================================================================
// Barrier: post-phase exception wrapping
// ===========================================================================

TEST(BarrierTests, PostPhaseActionThrows_WrappedInBarrierPostPhaseException) {
    Barrier b(1, [](Barrier&) { throw std::runtime_error("boom"); });
    EXPECT_THROW(b.SignalAndWait(), BarrierPostPhaseException);
}

TEST(BarrierTests, RemoveParticipant_ToZero_Throws) {
    Barrier b(0);
    EXPECT_THROW(b.RemoveParticipant(), System::ArgumentOutOfRangeException);
}

TEST(BarrierTests, PostPhaseActionThrows_AllParticipantsObserveException) {
    Barrier b(3, [](Barrier&) { throw std::runtime_error("boom"); });
    std::atomic<int> exceptionCount{0};
    std::thread t1([&] {
        try { b.SignalAndWait(); } catch (const BarrierPostPhaseException&) { ++exceptionCount; }
    });
    std::thread t2([&] {
        try { b.SignalAndWait(); } catch (const BarrierPostPhaseException&) { ++exceptionCount; }
    });
    try { b.SignalAndWait(); } catch (const BarrierPostPhaseException&) { ++exceptionCount; }
    t1.join();
    t2.join();
    EXPECT_EQ(exceptionCount.load(), 3);
}

TEST(BarrierTests, CalledFromPostPhaseAction_ThrowsInsteadOfDeadlocking) {
    // A reentrant call from within the post-phase action throws (matching real .NET's
    // Barrier.cs, which rejects such calls with InvalidOperationException) rather than
    // self-deadlocking on the non-recursive mutex the previous implementation held throughout
    // the action's execution. Since the throw happens inside FinishPhase's own try/catch
    // (matching real .NET's finally-block rethrow), it surfaces wrapped in
    // BarrierPostPhaseException, whose inner exception is the InvalidOperationException.
    Barrier* barrier = nullptr;
    Barrier b(1, [&](Barrier&) { barrier->AddParticipant(); });
    barrier = &b;
    try {
        b.SignalAndWait();
        FAIL() << "expected BarrierPostPhaseException";
    } catch (const BarrierPostPhaseException& ex) {
        ASSERT_TRUE(ex.getInnerExceptionProperty() != nullptr);
        EXPECT_THROW(std::rethrow_exception(ex.getInnerExceptionProperty()), System::InvalidOperationException);
    }
}

// ===========================================================================
// ExecutionContext
// ===========================================================================

TEST(ExecutionContextTests, Capture_ReturnsNullptr) {
    EXPECT_EQ(ExecutionContext::Capture(), nullptr);
}

TEST(ExecutionContextTests, Run_InvokesCallbackSynchronously) {
    bool called = false;
    ExecutionContext::Run(nullptr, [&](void*) { called = true; }, nullptr);
    EXPECT_TRUE(called);
}

TEST(ExecutionContextTests, IsFlowSuppressed_DefaultsFalse) {
    EXPECT_FALSE(ExecutionContext::IsFlowSuppressed());
}

TEST(ExecutionContextTests, SuppressFlow_ReturnsUndoableHandle) {
    auto flowControl = ExecutionContext::SuppressFlow();
    EXPECT_NO_THROW(flowControl.Undo());
}

// ===========================================================================
// IThreadPoolWorkItem / WaitCallback / ThreadPool::UnsafeQueueUserWorkItem
// ===========================================================================

namespace {
    class CountingWorkItem : public IThreadPoolWorkItem {
    public:
        std::atomic<int>* counter;
        explicit CountingWorkItem(std::atomic<int>* c) : counter(c) {}
        void Execute() override { counter->fetch_add(1); }
    };
}

TEST(IThreadPoolWorkItemTests, UnsafeQueueUserWorkItem_ExecutesWorkItem) {
    std::atomic<int> counter{0};
    CountingWorkItem item(&counter);
    ThreadPool::UnsafeQueueUserWorkItem(&item, false);
    for (int i = 0; i < 100 && counter.load() == 0; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_EQ(counter.load(), 1);
}

// ===========================================================================
// RegisteredWaitHandle / ThreadPool::RegisterWaitForSingleObject
// ===========================================================================

TEST(RegisteredWaitHandleTests, RegisterWaitForSingleObject_WithSemaphore_InvokesCallback) {
    Semaphore sem(1, 1);
    std::atomic<int> callCount{0};
    std::atomic<bool> lastTimedOut{true};
    auto rwh = ThreadPool::RegisterWaitForSingleObject(
        &sem,
        [&](void*, bool timedOut) { callCount.fetch_add(1); lastTimedOut = timedOut; },
        nullptr, 5000, true);
    for (int i = 0; i < 200 && callCount.load() == 0; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_EQ(callCount.load(), 1);
    EXPECT_FALSE(lastTimedOut.load());
    rwh.Unregister(nullptr);
}

namespace {
    // Test double: never signals, and tracks whether a WaitOne() call is currently in flight,
    // so the regression test below can observe that RegisteredWaitHandle::Unregister() genuinely
    // waits for the background wait thread to leave its WaitOne() call before returning.
    class CountingNeverSignaledWaitHandle : public WaitHandle {
    public:
        std::atomic<int> activeCalls{0};
        bool WaitOne() override { return WaitOne(-1); }
        bool WaitOne(intcs millisecondsTimeout) override {
            activeCalls.fetch_add(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(millisecondsTimeout));
            activeCalls.fetch_sub(1);
            return false;
        }
    };
}

// Regression test for a wave-3 audit finding: Unregister() detached the background wait
// thread instead of joining it, so a caller that destroyed the registered WaitHandle right
// after Unregister() returned could race the thread's in-flight WaitOne() call on that same
// (now freed, non-reference-counted) pointer -- a genuine use-after-free. Real .NET is safe
// without blocking because its wait thread operates on a ref-counted SafeWaitHandle; this port
// has no equivalent ref-counting, so blocking is the simpler, safe alternative.
TEST(RegisteredWaitHandleTests, Unregister_WaitsForInFlightWaitOneToReturn) {
    auto handle = std::make_unique<CountingNeverSignaledWaitHandle>();
    auto rwh = ThreadPool::RegisterWaitForSingleObject(
        handle.get(), [](void*, bool) {}, nullptr, -1, false);

    // Wait until the background thread is confirmed to be inside a WaitOne() call.
    while (handle->activeCalls.load() == 0) std::this_thread::sleep_for(std::chrono::milliseconds(1));

    rwh.Unregister(nullptr);
    // Unregister() must not return while WaitOne() is still in flight.
    EXPECT_EQ(handle->activeCalls.load(), 0);

    handle.reset(); // safe to destroy now; would race a still-detached thread before the fix
}

// ===========================================================================
// WaitHandle::WaitAll / WaitAny
// ===========================================================================

TEST(WaitHandleTests, WaitAll_AllSignaled_ReturnsTrue) {
    Semaphore s1(1, 1), s2(1, 1);
    std::vector<WaitHandle*> handles{&s1, &s2};
    EXPECT_TRUE(WaitHandle::WaitAll(handles));
}

TEST(WaitHandleTests, WaitAny_OneSignaled_ReturnsItsIndex) {
    Semaphore s1(0, 1), s2(1, 1);
    std::vector<WaitHandle*> handles{&s1, &s2};
    EXPECT_EQ(WaitHandle::WaitAny(handles), 1);
}

TEST(WaitHandleTests, WaitAny_NoneSignaled_TimesOutWithWaitTimeout) {
    Semaphore s1(0, 1), s2(0, 1);
    std::vector<WaitHandle*> handles{&s1, &s2};
    EXPECT_EQ(WaitHandle::WaitAny(handles, 50), WaitHandle::WaitTimeout);
}

// Regression test for a wave-3 audit finding shared by both Semaphore::WaitOne(int) and
// WaitHandle::WaitAll(handles, int): passing -1 (Timeout.Infinite) used to compute a
// deadline already in the past (now() + milliseconds(-1)), so both returned/timed-out
// almost immediately instead of blocking forever like real .NET.
TEST(WaitHandleTests, WaitAll_InfiniteTimeout_BlocksUntilAllSignaled) {
    Semaphore s1(0, 1), s2(0, 1);
    std::vector<WaitHandle*> handles{&s1, &s2};
    std::atomic<bool> done{false};
    std::thread t([&] { done = WaitHandle::WaitAll(handles, -1); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(done.load()); // still blocked after 100ms -- neither semaphore is signaled yet

    s1.Release();
    s2.Release();
    t.join();
    EXPECT_TRUE(done.load());
}

TEST(SemaphoreTests, WaitOne_InfiniteTimeout_BlocksUntilReleased) {
    Semaphore s(0, 1);
    std::atomic<bool> acquired{false};
    std::thread t([&] { acquired = s.WaitOne(-1); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(acquired.load());

    s.Release();
    t.join();
    EXPECT_TRUE(acquired.load());
}
