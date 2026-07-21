// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for remaining Threading types:
// enums (ApartmentState, EventResetMode, LazyThreadSafetyMode, LockRecursionPolicy,
//         ThreadPriority, ThreadState),
// AsyncLocal, Barrier, CountdownEvent, EventWaitHandle, LazyInitializer, Lock,
// ManualResetEventSlim, ReaderWriterLockSlim, SpinWait, ThreadLocal, ThreadPool, Timer,
// and Threading exceptions.
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <future>
#include <limits>
#include <new>
#include <string>
#include <thread>
#include "System/ApplicationException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/Threading/ApartmentState.hpp"
#include "System/Threading/EventResetMode.hpp"
#include "System/Threading/LazyThreadSafetyMode.hpp"
#include "System/Threading/LockRecursionPolicy.hpp"
#include "System/Threading/ThreadPriority.hpp"
#include "System/Threading/ThreadState.hpp"
#include "System/Threading/AsyncLocal.hpp"
#include "System/Threading/AsyncLocalValueChangedArgs.hpp"
#include "System/Threading/Barrier.hpp"
#include "System/Threading/CountdownEvent.hpp"
#include "System/Threading/EventWaitHandle.hpp"
#include "System/Threading/LazyInitializer.hpp"
#include "System/Threading/Lock.hpp"
#include "System/Threading/ManualResetEventSlim.hpp"
#include "System/Threading/ReaderWriterLockSlim.hpp"
#include "System/Threading/SpinWait.hpp"
#include "System/Threading/ThreadLocal.hpp"
#include "System/Threading/ThreadPool.hpp"
#include "System/Threading/AbandonedMutexException.hpp"
#include "System/Threading/LockRecursionException.hpp"
#include "System/Threading/SemaphoreFullException.hpp"
#include "System/Threading/SynchronizationLockException.hpp"
#include "System/Threading/ThreadAbortException.hpp"
#include "System/Threading/ThreadInterruptedException.hpp"
#include "System/Threading/ThreadStateException.hpp"
#include "System/Threading/WaitHandleCannotBeOpenedException.hpp"
#include "System/Exception.hpp"

using namespace System::Threading;

// ===========================================================================
// ApartmentState enum
// ===========================================================================

TEST(ApartmentStateTests, STA_IsZero)     { EXPECT_EQ(static_cast<int>(ApartmentState::STA), 0); }
TEST(ApartmentStateTests, MTA_IsOne)      { EXPECT_EQ(static_cast<int>(ApartmentState::MTA), 1); }
TEST(ApartmentStateTests, Unknown_IsTwo)  { EXPECT_EQ(static_cast<int>(ApartmentState::Unknown), 2); }

// ===========================================================================
// EventResetMode enum
// ===========================================================================

TEST(EventResetModeTests, AutoReset_IsZero)   { EXPECT_EQ(static_cast<int>(EventResetMode::AutoReset), 0); }
TEST(EventResetModeTests, ManualReset_IsOne)  { EXPECT_EQ(static_cast<int>(EventResetMode::ManualReset), 1); }

// ===========================================================================
// LazyThreadSafetyMode enum
// ===========================================================================

TEST(LazyThreadSafetyModeTests, None_IsZero)                    { EXPECT_EQ(static_cast<int>(LazyThreadSafetyMode::None), 0); }
TEST(LazyThreadSafetyModeTests, ExecutionAndPublication_IsTwo)  { EXPECT_EQ(static_cast<int>(LazyThreadSafetyMode::ExecutionAndPublication), 2); }

// ===========================================================================
// LockRecursionPolicy enum
// ===========================================================================

TEST(LockRecursionPolicyTests, NoRecursion_IsZero)        { EXPECT_EQ(static_cast<int>(LockRecursionPolicy::NoRecursion), 0); }
TEST(LockRecursionPolicyTests, SupportsRecursion_IsOne)   { EXPECT_EQ(static_cast<int>(LockRecursionPolicy::SupportsRecursion), 1); }

// ===========================================================================
// ThreadPriority enum
// ===========================================================================

TEST(ThreadPriorityTests, Lowest_IsZero)   { EXPECT_EQ(static_cast<int>(ThreadPriority::Lowest), 0); }
TEST(ThreadPriorityTests, Normal_IsTwo)    { EXPECT_EQ(static_cast<int>(ThreadPriority::Normal), 2); }
TEST(ThreadPriorityTests, Highest_IsFour) { EXPECT_EQ(static_cast<int>(ThreadPriority::Highest), 4); }

// ===========================================================================
// ThreadState enum
// ===========================================================================

TEST(ThreadStateTests, Running_IsZero)     { EXPECT_EQ(static_cast<int>(ThreadState::Running), 0); }
TEST(ThreadStateTests, Unstarted_IsEight)  { EXPECT_EQ(static_cast<int>(ThreadState::Unstarted), 8); }
TEST(ThreadStateTests, Stopped_Is16)       { EXPECT_EQ(static_cast<int>(ThreadState::Stopped), 16); }
TEST(ThreadStateTests, OrOperator) {
    auto combined = ThreadState::Background | ThreadState::Unstarted;
    EXPECT_NE(static_cast<int>(combined & ThreadState::Background), 0);
    EXPECT_NE(static_cast<int>(combined & ThreadState::Unstarted), 0);
}

// ===========================================================================
// AsyncLocal<T>
// ===========================================================================

TEST(AsyncLocalTests, DefaultValue_IsDefaultConstructed) {
    AsyncLocal<int> al;
    EXPECT_EQ(al.getValueProperty(), 0);
}
TEST(AsyncLocalTests, SetAndGet_Value) {
    AsyncLocal<int> al;
    al.setValueProperty(42);
    EXPECT_EQ(al.getValueProperty(), 42);
}
TEST(AsyncLocalTests, ValueChangedHandler_Called) {
    int callCount = 0;
    int lastPrevious = -1, lastCurrent = -1;
    AsyncLocal<int> al([&](const System::Threading::AsyncLocalValueChangedArgs<int>& args) {
        ++callCount;
        lastPrevious = args.getPreviousValueProperty();
        lastCurrent = args.getCurrentValueProperty();
    });
    al.setValueProperty(1);
    al.setValueProperty(2);
    EXPECT_EQ(callCount, 2);
    EXPECT_EQ(lastPrevious, 1);
    EXPECT_EQ(lastCurrent, 2);
}
TEST(AsyncLocalTests, SetSameValue_IsNoOp) {
    int callCount = 0;
    AsyncLocal<int> al([&](const System::Threading::AsyncLocalValueChangedArgs<int>&) { ++callCount; });
    al.setValueProperty(5);
    EXPECT_EQ(callCount, 1);
    al.setValueProperty(5); // same value: must not fire the handler again
    EXPECT_EQ(callCount, 1);
}
TEST(AsyncLocalTests, TwoInstances_SameType_AreIndependent) {
    AsyncLocal<int> a;
    AsyncLocal<int> b;
    a.setValueProperty(1);
    b.setValueProperty(2);
    EXPECT_EQ(a.getValueProperty(), 1);
    EXPECT_EQ(b.getValueProperty(), 2);
}
TEST(AsyncLocalTests, InstanceDestroyedOnDifferentThread_ThenNewInstanceAtSameAddress_DoesNotLeakStaleValue) {
    // Reproduces the exact scenario the ID-based-keying fix targets: a worker thread's
    // thread_local map is populated by instance A; A is then destroyed by a *different*
    // thread (only cleans that other thread's own map), and a new instance B is placement-new'd
    // at the identical address. The old this-pointer-keyed implementation would have the worker
    // thread's map lookup for B's address collide with A's still-present stale entry.
    alignas(AsyncLocal<int>) unsigned char buffer[sizeof(AsyncLocal<int>)];
    std::promise<void> constructed;
    std::shared_future<void> constructedFuture = constructed.get_future().share();
    std::promise<void> replaced;
    std::shared_future<void> replacedFuture = replaced.get_future().share();

    std::thread worker([&] {
        auto* a = new (buffer) AsyncLocal<int>();
        a->setValueProperty(111); // populates this worker thread's thread_local map
        constructed.set_value();  // real synchronization: main thread must not touch buffer yet
        replacedFuture.wait();    // block until the main thread has destroyed A and placement-new'd B
        auto* b = reinterpret_cast<AsyncLocal<int>*>(buffer);
        EXPECT_EQ(b->getValueProperty(), 0); // must not see A's leftover 111
    });

    // A fixed sleep_for() here (the original approach) gives no actual happens-before guarantee
    // between the worker's buffer writes above and the main thread's destroy/reconstruct below --
    // confirmed as a genuine ThreadSanitizer-flagged data race (2026-07-14), even though it
    // virtually always "worked" in practice since 20ms is far longer than the worker needs.
    // std::promise::set_value()/std::future::wait() is a real synchronizes-with relationship.
    constructedFuture.wait();
    reinterpret_cast<AsyncLocal<int>*>(buffer)->~AsyncLocal<int>(); // destroyed on the MAIN thread
    new (buffer) AsyncLocal<int>();                                 // instance B at the identical address
    replaced.set_value();
    worker.join();
    reinterpret_cast<AsyncLocal<int>*>(buffer)->~AsyncLocal<int>();
}

TEST(AsyncLocalValueChangedArgsTests, PropertiesReflectConstructorArgs) {
    System::Threading::AsyncLocalValueChangedArgs<int> args(10, 20, true);
    EXPECT_EQ(args.getPreviousValueProperty(), 10);
    EXPECT_EQ(args.getCurrentValueProperty(), 20);
    EXPECT_TRUE(args.getThreadContextChangedProperty());
}

// ===========================================================================
// Barrier
// ===========================================================================

TEST(BarrierTests, Constructor_StoresParticipantCount) {
    Barrier b(3);
    EXPECT_EQ(b.getParticipantCountProperty(), 3);
}
TEST(BarrierTests, NegativeCount_Throws) {
    EXPECT_THROW(Barrier(-1), System::ArgumentOutOfRangeException);
}
TEST(BarrierTests, SingleParticipant_SignalAndWait_AdvancesPhase) {
    Barrier b(1);
    EXPECT_EQ(b.getCurrentPhaseNumberProperty(), 0L);
    b.SignalAndWait();
    EXPECT_EQ(b.getCurrentPhaseNumberProperty(), 1L);
}
TEST(BarrierTests, AddParticipant_IncreasesCount) {
    Barrier b(1);
    b.AddParticipant();
    EXPECT_EQ(b.getParticipantCountProperty(), 2);
}
TEST(BarrierTests, RemoveParticipant_DecreasesCount) {
    Barrier b(2);
    b.RemoveParticipant();
    EXPECT_EQ(b.getParticipantCountProperty(), 1);
}
TEST(BarrierTests, PostPhaseAction_CalledAfterPhase) {
    int phasesCalled = 0;
    Barrier b(1, [&](Barrier&) { ++phasesCalled; });
    b.SignalAndWait();
    EXPECT_EQ(phasesCalled, 1);
}

// Regression tests for a wave-3 audit finding: Dispose() was a true no-op with no disposed_
// flag at all -- every method remained fully usable after disposal. Verified against
// Barrier.cs: SignalAndWait/AddParticipants/RemoveParticipants all call
// ObjectDisposedException.ThrowIf(_disposed, this) as their first check.
TEST(BarrierTests, SignalAndWait_AfterDispose_ThrowsObjectDisposedException) {
    Barrier b(1);
    b.Dispose();
    EXPECT_THROW(b.SignalAndWait(), System::ObjectDisposedException);
}

TEST(BarrierTests, AddParticipant_AfterDispose_ThrowsObjectDisposedException) {
    Barrier b(1);
    b.Dispose();
    EXPECT_THROW(b.AddParticipant(), System::ObjectDisposedException);
}

TEST(BarrierTests, RemoveParticipant_AfterDispose_ThrowsObjectDisposedException) {
    Barrier b(2);
    b.Dispose();
    EXPECT_THROW(b.RemoveParticipant(), System::ObjectDisposedException);
}

TEST(BarrierTests, Dispose_CalledTwice_DoesNotThrow) {
    Barrier b(1);
    b.Dispose();
    EXPECT_NO_THROW(b.Dispose());
}

// ===========================================================================
// CountdownEvent
// ===========================================================================

TEST(CountdownEventTests, Constructor_StoresInitialCount) {
    CountdownEvent ce(5);
    EXPECT_EQ(ce.getInitialCountProperty(), 5);
    EXPECT_EQ(ce.getCurrentCountProperty(), 5);
}
TEST(CountdownEventTests, NegativeCount_Throws) {
    EXPECT_THROW(CountdownEvent(-1), System::ArgumentOutOfRangeException);
}
TEST(CountdownEventTests, IsSet_FalseInitially) {
    CountdownEvent ce(2);
    EXPECT_FALSE(ce.getIsSetProperty());
}
TEST(CountdownEventTests, Signal_DecrementsCount) {
    CountdownEvent ce(3);
    ce.Signal();
    EXPECT_EQ(ce.getCurrentCountProperty(), 2);
}
TEST(CountdownEventTests, Signal_ToZero_IsSet) {
    CountdownEvent ce(1);
    bool set = ce.Signal();
    EXPECT_TRUE(set);
    EXPECT_TRUE(ce.getIsSetProperty());
}
TEST(CountdownEventTests, Signal_AlreadyZero_Throws) {
    CountdownEvent ce(1);
    ce.Signal();
    EXPECT_THROW(ce.Signal(), System::InvalidOperationException);
}
TEST(CountdownEventTests, AddCount_AlreadyZero_Throws) {
    CountdownEvent ce(1);
    ce.Signal();
    EXPECT_THROW(ce.AddCount(), System::InvalidOperationException);
}
TEST(CountdownEventTests, AfterDispose_Wait_ThrowsObjectDisposedException) {
    CountdownEvent ce(1);
    ce.Dispose();
    EXPECT_THROW(ce.Wait(), System::ObjectDisposedException);
}
TEST(CountdownEventTests, AddCount_IncreasesCount) {
    CountdownEvent ce(2);
    ce.AddCount(3);
    EXPECT_EQ(ce.getCurrentCountProperty(), 5);
}
TEST(CountdownEventTests, Reset_RestoresToInitial) {
    CountdownEvent ce(3);
    ce.Signal(2);
    ce.Reset();
    EXPECT_EQ(ce.getCurrentCountProperty(), 3);
}
TEST(CountdownEventTests, Reset_WithNewCount) {
    CountdownEvent ce(3);
    ce.Reset(10);
    EXPECT_EQ(ce.getInitialCountProperty(), 10);
    EXPECT_EQ(ce.getCurrentCountProperty(), 10);
}

// Regression test for a wave-3 audit finding: Reset(intcs count = -1) used -1 as a sentinel
// meaning "use InitialCount", so an explicit Reset(-1) call silently reset to InitialCount
// instead of throwing. Verified against CountdownEvent.cs's Reset(int): real .NET has no
// negative-sentinel concept -- Reset(int count) rejects *any* negative count, including -1,
// with ArgumentOutOfRangeException.
TEST(CountdownEventTests, Reset_NegativeCount_ThrowsArgumentOutOfRangeException) {
    CountdownEvent ce(3);
    EXPECT_THROW(ce.Reset(-1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(ce.Reset(-5), System::ArgumentOutOfRangeException);
}

// Regression test for a wave-3 audit finding: Reset() never called ThrowIfDisposed() at all,
// so calling it after Dispose() silently succeeded instead of throwing. Verified against
// CountdownEvent.cs's Reset(int): ObjectDisposedException.ThrowIf(_disposed, this).
TEST(CountdownEventTests, Reset_AfterDispose_ThrowsObjectDisposedException) {
    CountdownEvent ce(3);
    ce.Dispose();
    EXPECT_THROW(ce.Reset(), System::ObjectDisposedException);
    EXPECT_THROW(ce.Reset(5), System::ObjectDisposedException);
}
TEST(CountdownEventTests, Wait_AlreadySet_ReturnsImmediately) {
    CountdownEvent ce(0);
    EXPECT_NO_THROW(ce.Wait());
}
TEST(CountdownEventTests, WaitWithTimeout_AlreadySet_ReturnsTrue) {
    CountdownEvent ce(0);
    EXPECT_TRUE(ce.Wait(100));
}
TEST(CountdownEventTests, AddCount_ZeroOrNegativeSignalCount_Throws) {
    CountdownEvent ce(1);
    EXPECT_THROW(ce.AddCount(0), System::ArgumentOutOfRangeException);
    EXPECT_THROW(ce.AddCount(-1), System::ArgumentOutOfRangeException);
}
TEST(CountdownEventTests, Signal_ZeroOrNegativeSignalCount_Throws) {
    CountdownEvent ce(1);
    EXPECT_THROW(ce.Signal(0), System::ArgumentOutOfRangeException);
    EXPECT_THROW(ce.Signal(-1), System::ArgumentOutOfRangeException);
}
TEST(CountdownEventTests, AddCount_WouldOverflow_ThrowsInvalidOperationException) {
    CountdownEvent ce(std::numeric_limits<intcs>::max() - 1);
    EXPECT_THROW(ce.AddCount(3), System::InvalidOperationException);
    EXPECT_EQ(ce.getCurrentCountProperty(), std::numeric_limits<intcs>::max() - 1);
}
TEST(CountdownEventTests, AddCount_UpToMax_Succeeds) {
    CountdownEvent ce(std::numeric_limits<intcs>::max() - 1);
    EXPECT_NO_THROW(ce.AddCount(1));
    EXPECT_EQ(ce.getCurrentCountProperty(), std::numeric_limits<intcs>::max());
}

// ===========================================================================
// EventWaitHandle
// ===========================================================================

TEST(EventWaitHandleTests, AutoReset_InitiallySet_WaitOne_ReturnsTrue) {
    EventWaitHandle ewh(true, EventResetMode::AutoReset);
    EXPECT_TRUE(ewh.WaitOne(0));
}
TEST(EventWaitHandleTests, AutoReset_NotSet_WaitOneWithTimeout_ReturnsFalse) {
    EventWaitHandle ewh(false, EventResetMode::AutoReset);
    EXPECT_FALSE(ewh.WaitOne(1));
}
TEST(EventWaitHandleTests, ManualReset_Set_MultipleWaitOnes) {
    EventWaitHandle ewh(true, EventResetMode::ManualReset);
    EXPECT_TRUE(ewh.WaitOne(0));
    EXPECT_TRUE(ewh.WaitOne(0));
}
TEST(EventWaitHandleTests, Set_Then_Reset_NotSet) {
    EventWaitHandle ewh(false, EventResetMode::ManualReset);
    ewh.Set();
    ewh.Reset();
    EXPECT_FALSE(ewh.WaitOne(1));
}
TEST(EventWaitHandleTests, WaitOne_TimeoutLessThanNegativeOne_Throws) {
    EventWaitHandle ewh(false, EventResetMode::ManualReset);
    EXPECT_THROW(ewh.WaitOne(-2), System::ArgumentOutOfRangeException);
}

// ===========================================================================
// LazyInitializer
// ===========================================================================

TEST(LazyInitializerTests, EnsureInitialized_Default_CreatesObject) {
    int* ptr = nullptr;
    LazyInitializer::EnsureInitialized(ptr);
    EXPECT_NE(ptr, nullptr);
    delete ptr;
}
TEST(LazyInitializerTests, EnsureInitialized_AlreadySet_NoChange) {
    int* ptr = new int(42);
    int* original = ptr;
    LazyInitializer::EnsureInitialized(ptr);
    EXPECT_EQ(ptr, original);
    delete ptr;
}
TEST(LazyInitializerTests, EnsureInitialized_WithFactory) {
    int* ptr = nullptr;
    LazyInitializer::EnsureInitialized<int>(ptr, []() { return new int(99); });
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, 99);
    delete ptr;
}
TEST(LazyInitializerTests, EnsureInitialized_NullFactory_ThrowsInvalidOperationException) {
    int* ptr = nullptr;
    EXPECT_THROW(
        LazyInitializer::EnsureInitialized<int>(ptr, []() -> int* { return nullptr; }),
        System::InvalidOperationException);
}
TEST(LazyInitializerTests, EnsureInitialized_ReentrantSameType_DoesNotDeadlock) {
    // A factory for one `int*` target that itself initializes a second, distinct `int*`
    // target of the same type T=int on the same thread. The previous per-T-instantiation
    // static std::mutex would self-deadlock here since std::mutex isn't recursive.
    int* outer = nullptr;
    int* inner = nullptr;
    LazyInitializer::EnsureInitialized<int>(outer, [&]() {
        LazyInitializer::EnsureInitialized<int>(inner, []() { return new int(7); });
        return new int(3);
    });
    ASSERT_NE(outer, nullptr);
    ASSERT_NE(inner, nullptr);
    EXPECT_EQ(*outer, 3);
    EXPECT_EQ(*inner, 7);
    delete outer;
    delete inner;
}

// ===========================================================================
// Lock
// ===========================================================================

TEST(LockTests, TryEnter_Succeeds_ThenExit) {
    Lock lk;
    EXPECT_TRUE(lk.TryEnter());
    EXPECT_NO_THROW(lk.Exit());
}
TEST(LockTests, Enter_And_Exit_NoThrow) {
    Lock lk;
    EXPECT_NO_THROW(lk.Enter());
    EXPECT_NO_THROW(lk.Exit());
}
TEST(LockTests, Exit_NotHeld_ThrowsSynchronizationLockException) {
    Lock lk;
    EXPECT_THROW(lk.Exit(), System::Threading::SynchronizationLockException);
}
TEST(LockTests, Exit_FromNonOwningThread_ThrowsSynchronizationLockException) {
    Lock lk;
    lk.Enter();
    bool threw = false;
    std::thread t([&] {
        try { lk.Exit(); } catch (const System::Threading::SynchronizationLockException&) { threw = true; }
    });
    t.join();
    EXPECT_TRUE(threw);
    lk.Exit();
}
TEST(LockTests, IsHeldByCurrentThread_ReflectsState) {
    Lock lk;
    EXPECT_FALSE(lk.getIsHeldByCurrentThreadProperty());
    lk.Enter();
    EXPECT_TRUE(lk.getIsHeldByCurrentThreadProperty());
    lk.Exit();
    EXPECT_FALSE(lk.getIsHeldByCurrentThreadProperty());
}
TEST(LockTests, Enter_IsReentrant) {
    Lock lk;
    lk.Enter();
    EXPECT_NO_THROW(lk.Enter());
    lk.Exit();
    EXPECT_TRUE(lk.getIsHeldByCurrentThreadProperty());
    lk.Exit();
    EXPECT_FALSE(lk.getIsHeldByCurrentThreadProperty());
}
TEST(LockTests, EnterScope_RAII_ReleasesOnDestruction) {
    Lock lk;
    {
        auto scope = lk.EnterScope();
    }
    EXPECT_TRUE(lk.TryEnter());
    lk.Exit();
}

// Regression test for a wave-3 audit finding: TryEnter(TimeSpan) didn't special-case
// Timeout::InfiniteTimeSpan the way the intcs overload above does -- it fell through to
// try_lock_for with a negative duration, which behaves like a non-blocking try_lock() (returns
// almost instantly) instead of blocking indefinitely. Verified against Lock.cs's doc comment.
// This blocks a contending thread on an already-held lock and confirms TryEnter only succeeds
// once the lock is actually released, rather than returning early.
TEST(LockTests, TryEnter_InfiniteTimeSpan_BlocksUntilReleased) {
    Lock lk;
    lk.Enter();
    std::atomic<bool> acquired{false};
    std::atomic<bool> started{false};
    // The acquiring thread must also be the one to Exit() -- Lock's ownership tracking (like
    // the underlying std::recursive_timed_mutex) is thread-affine, matching real .NET's Lock.
    std::thread t([&] {
        started = true;
        acquired = lk.TryEnter(System::TimeSpan(System::Threading::Timeout::InfiniteTimeSpan));
        if (acquired.load()) lk.Exit();
    });
    while (!started.load()) std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    EXPECT_FALSE(acquired.load()); // still blocked -- lock hasn't been released yet
    lk.Exit();
    t.join();
    EXPECT_TRUE(acquired.load());
}

// Regression tests for a wave-3 audit finding: TryEnter(intcs)/TryEnter(TimeSpan) had no
// timeout validation at all -- Lock.hpp didn't even include WaitHandle.hpp, unlike every
// sibling wait primitive in this namespace. A negative timeout other than -1 (e.g. -5) fell
// straight through to try_lock_for, which treats a negative duration as already-expired --
// silently behaving like a non-blocking try_lock() instead of throwing
// ArgumentOutOfRangeException as real .NET's Lock.cs does.
TEST(LockTests, TryEnter_Intcs_InvalidNegativeTimeout_Throws) {
    Lock lk;
    EXPECT_THROW(lk.TryEnter(-2), System::ArgumentOutOfRangeException);
    EXPECT_THROW(lk.TryEnter(-5), System::ArgumentOutOfRangeException);
}

TEST(LockTests, TryEnter_Intcs_NegativeOne_StillMeansInfinite) {
    Lock lk;
    EXPECT_TRUE(lk.TryEnter(-1));
    lk.Exit();
}

TEST(LockTests, TryEnter_TimeSpan_InvalidNegativeTimeout_Throws) {
    Lock lk;
    EXPECT_THROW(lk.TryEnter(System::TimeSpan::FromMilliseconds(-2)), System::ArgumentOutOfRangeException);
}

// ===========================================================================
// ManualResetEventSlim
// ===========================================================================

TEST(ManualResetEventSlimTests, DefaultCtor_NotSet) {
    ManualResetEventSlim mre;
    EXPECT_FALSE(mre.getIsSetProperty());
}
TEST(ManualResetEventSlimTests, InitiallySet_IsSet) {
    ManualResetEventSlim mre(true);
    EXPECT_TRUE(mre.getIsSetProperty());
}
TEST(ManualResetEventSlimTests, Set_SetsFlag) {
    ManualResetEventSlim mre;
    mre.Set();
    EXPECT_TRUE(mre.getIsSetProperty());
}
TEST(ManualResetEventSlimTests, Reset_ClearsFlag) {
    ManualResetEventSlim mre(true);
    mre.Reset();
    EXPECT_FALSE(mre.getIsSetProperty());
}
TEST(ManualResetEventSlimTests, Wait_AlreadySet_ReturnsImmediately) {
    ManualResetEventSlim mre(true);
    EXPECT_NO_THROW(mre.Wait());
}
TEST(ManualResetEventSlimTests, Wait_WithTimeout_NotSet_ReturnsFalse) {
    ManualResetEventSlim mre(false);
    EXPECT_FALSE(mre.Wait(1));
}
TEST(ManualResetEventSlimTests, Wait_WithTimeout_AlreadySet_ReturnsTrue) {
    ManualResetEventSlim mre(true);
    EXPECT_TRUE(mre.Wait(100));
}
TEST(ManualResetEventSlimTests, Wait_TimeoutLessThanNegativeOne_Throws) {
    ManualResetEventSlim mre;
    EXPECT_THROW(mre.Wait(-2), System::ArgumentOutOfRangeException);
}
TEST(ManualResetEventSlimTests, AfterDispose_Wait_ThrowsObjectDisposedException) {
    ManualResetEventSlim mre;
    mre.Dispose();
    EXPECT_THROW(mre.Wait(), System::ObjectDisposedException);
}
TEST(ManualResetEventSlimTests, AfterDispose_Reset_ThrowsObjectDisposedException) {
    ManualResetEventSlim mre(true);
    mre.Dispose();
    EXPECT_THROW(mre.Reset(), System::ObjectDisposedException);
}

// ===========================================================================
// ReaderWriterLockSlim
// ===========================================================================

TEST(ReaderWriterLockSlimTests, DefaultCtor_NoThrow) {
    EXPECT_NO_THROW(ReaderWriterLockSlim rw);
}
TEST(ReaderWriterLockSlimTests, EnterExitReadLock_NoThrow) {
    ReaderWriterLockSlim rw;
    EXPECT_NO_THROW(rw.EnterReadLock());
    EXPECT_NO_THROW(rw.ExitReadLock());
}
TEST(ReaderWriterLockSlimTests, EnterExitWriteLock_NoThrow) {
    ReaderWriterLockSlim rw;
    EXPECT_NO_THROW(rw.EnterWriteLock());
    EXPECT_NO_THROW(rw.ExitWriteLock());
}
TEST(ReaderWriterLockSlimTests, TryEnterReadLock_Succeeds) {
    ReaderWriterLockSlim rw;
    EXPECT_TRUE(rw.TryEnterReadLock(0));
    rw.ExitReadLock();
}
TEST(ReaderWriterLockSlimTests, TryEnterWriteLock_Succeeds) {
    ReaderWriterLockSlim rw;
    EXPECT_TRUE(rw.TryEnterWriteLock(0));
    rw.ExitWriteLock();
}
TEST(ReaderWriterLockSlimTests, Dispose_NoThrow) {
    ReaderWriterLockSlim rw;
    EXPECT_NO_THROW(rw.Dispose());
}
TEST(ReaderWriterLockSlimTests, UpgradeableToWrite_DoesNotDeadlock) {
    // Previously EnterWriteLock() called mtx_.lock() while the same thread already held
    // mtx_.lock_shared() via EnterUpgradeableReadLock() -- undefined behavior on
    // std::shared_mutex, manifesting as a deadlock. Run on a background thread with a
    // bounded join so a regression hangs the test instead of the whole suite.
    ReaderWriterLockSlim rw;
    std::promise<void> upgraded;
    std::thread t([&] {
        rw.EnterUpgradeableReadLock();
        rw.EnterWriteLock();
        upgraded.set_value();
        rw.ExitWriteLock();
        rw.ExitUpgradeableReadLock();
    });
    auto fut = upgraded.get_future();
    ASSERT_EQ(fut.wait_for(std::chrono::seconds(2)), std::future_status::ready);
    t.join();
}
TEST(ReaderWriterLockSlimTests, UpgradeableToWrite_WaitsForConcurrentReaderToDrain) {
    ReaderWriterLockSlim rw;
    rw.EnterReadLock();
    std::promise<void> upgraded;
    std::thread t([&] {
        rw.EnterUpgradeableReadLock();
        rw.EnterWriteLock();
        upgraded.set_value();
        rw.ExitWriteLock();
        rw.ExitUpgradeableReadLock();
    });
    auto fut = upgraded.get_future();
    // The write upgrade must not complete while the concurrent reader is still active.
    EXPECT_EQ(fut.wait_for(std::chrono::milliseconds(200)), std::future_status::timeout);
    rw.ExitReadLock();
    ASSERT_EQ(fut.wait_for(std::chrono::seconds(2)), std::future_status::ready);
    t.join();
}
TEST(ReaderWriterLockSlimTests, ExitReadLock_NotHeld_Throws) {
    ReaderWriterLockSlim rw;
    EXPECT_THROW(rw.ExitReadLock(), System::Threading::SynchronizationLockException);
}
TEST(ReaderWriterLockSlimTests, ExitWriteLock_NotHeld_Throws) {
    ReaderWriterLockSlim rw;
    EXPECT_THROW(rw.ExitWriteLock(), System::Threading::SynchronizationLockException);
}
TEST(ReaderWriterLockSlimTests, ExitUpgradeableReadLock_NotHeld_Throws) {
    ReaderWriterLockSlim rw;
    EXPECT_THROW(rw.ExitUpgradeableReadLock(), System::Threading::SynchronizationLockException);
}
TEST(ReaderWriterLockSlimTests, EnterReadLock_AfterDispose_Throws) {
    ReaderWriterLockSlim rw;
    rw.Dispose();
    EXPECT_THROW(rw.EnterReadLock(), System::ObjectDisposedException);
}
TEST(ReaderWriterLockSlimTests, DefaultRecursionPolicy_IsNoRecursion) {
    ReaderWriterLockSlim rw;
    EXPECT_EQ(rw.getRecursionPolicyProperty(), LockRecursionPolicy::NoRecursion);
}
TEST(ReaderWriterLockSlimTests, TryEnterWriteLock_HonorsRealTimeout) {
    // Previously the millisecondsTimeout parameter was discarded entirely (a single
    // non-blocking attempt regardless of value). A held write lock must now make a contending
    // TryEnterWriteLock actually block for approximately the requested duration before
    // returning false, not return immediately.
    ReaderWriterLockSlim rw;
    rw.EnterWriteLock();
    std::atomic<bool> result{true};
    auto start = std::chrono::steady_clock::now();
    std::thread t([&] { result = rw.TryEnterWriteLock(100); });
    t.join();
    auto elapsed = std::chrono::steady_clock::now() - start;
    EXPECT_FALSE(result.load());
    EXPECT_GE(elapsed, std::chrono::milliseconds(80));
    rw.ExitWriteLock();
}
TEST(ReaderWriterLockSlimTests, TryEnterWriteLock_SucceedsWithinTimeoutOnceReleased) {
    ReaderWriterLockSlim rw;
    rw.EnterWriteLock();
    std::atomic<bool> result{false};
    std::thread t([&] {
        result = rw.TryEnterWriteLock(2000);
        if (result.load()) rw.ExitWriteLock(); // release on the same thread that acquired it
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    rw.ExitWriteLock();
    t.join();
    EXPECT_TRUE(result.load());
}
TEST(ReaderWriterLockSlimTests, NoRecursion_RecursiveReadLock_ThrowsInsteadOfDeadlocking) {
    ReaderWriterLockSlim rw;
    rw.EnterReadLock();
    EXPECT_THROW(rw.EnterReadLock(), System::Threading::LockRecursionException);
    rw.ExitReadLock();
}
TEST(ReaderWriterLockSlimTests, NoRecursion_RecursiveWriteLock_ThrowsInsteadOfDeadlocking) {
    ReaderWriterLockSlim rw;
    rw.EnterWriteLock();
    EXPECT_THROW(rw.EnterWriteLock(), System::Threading::LockRecursionException);
    rw.ExitWriteLock();
}
TEST(ReaderWriterLockSlimTests, NoRecursion_RecursiveUpgradeableLock_ThrowsInsteadOfDeadlocking) {
    ReaderWriterLockSlim rw;
    rw.EnterUpgradeableReadLock();
    EXPECT_THROW(rw.EnterUpgradeableReadLock(), System::Threading::LockRecursionException);
    rw.ExitUpgradeableReadLock();
}
TEST(ReaderWriterLockSlimTests, NoRecursion_WriteAfterRead_ThrowsInsteadOfDeadlocking) {
    ReaderWriterLockSlim rw;
    rw.EnterReadLock();
    EXPECT_THROW(rw.EnterWriteLock(), System::Threading::LockRecursionException);
    rw.ExitReadLock();
}
TEST(ReaderWriterLockSlimTests, NoRecursion_UpgradeAfterRead_ThrowsInsteadOfDeadlocking) {
    ReaderWriterLockSlim rw;
    rw.EnterReadLock();
    EXPECT_THROW(rw.EnterUpgradeableReadLock(), System::Threading::LockRecursionException);
    rw.ExitReadLock();
}
TEST(ReaderWriterLockSlimTests, NoRecursion_UpgradeAfterWrite_ThrowsInsteadOfDeadlocking) {
    ReaderWriterLockSlim rw;
    rw.EnterWriteLock();
    EXPECT_THROW(rw.EnterUpgradeableReadLock(), System::Threading::LockRecursionException);
    rw.ExitWriteLock();
}
TEST(ReaderWriterLockSlimTests, SupportsRecursion_NestedReadLock_BothExitsSucceed_AndWriterNotStarved) {
    // Previously reader ownership was tracked via set *membership*, not a count: a second
    // ExitReadLock() after a legitimately nested EnterReadLock()/EnterReadLock() threw
    // SynchronizationLockException (membership already removed by the first exit), and the
    // internal reader tally never reached zero, permanently starving any waiting writer.
    ReaderWriterLockSlim rw(LockRecursionPolicy::SupportsRecursion);
    rw.EnterReadLock();
    rw.EnterReadLock();
    EXPECT_TRUE(rw.getIsReadLockHeldProperty());
    EXPECT_NO_THROW(rw.ExitReadLock());
    EXPECT_TRUE(rw.getIsReadLockHeldProperty()); // still held once more
    EXPECT_NO_THROW(rw.ExitReadLock());
    EXPECT_FALSE(rw.getIsReadLockHeldProperty());
    // A writer must now be able to acquire the lock -- proves the reader tally actually
    // reached zero, not left permanently nonzero by the nested acquisition.
    EXPECT_TRUE(rw.TryEnterWriteLock(2000));
    rw.ExitWriteLock();
}
TEST(ReaderWriterLockSlimTests, SupportsRecursion_NestedWriteLock_BothExitsSucceed) {
    ReaderWriterLockSlim rw(LockRecursionPolicy::SupportsRecursion);
    rw.EnterWriteLock();
    rw.EnterWriteLock();
    EXPECT_NO_THROW(rw.ExitWriteLock());
    EXPECT_TRUE(rw.getIsWriteLockHeldProperty());
    EXPECT_NO_THROW(rw.ExitWriteLock());
    EXPECT_FALSE(rw.getIsWriteLockHeldProperty());
}
TEST(ReaderWriterLockSlimTests, SupportsRecursion_ExplicitCtor_RoundTripsPolicy) {
    ReaderWriterLockSlim rw(LockRecursionPolicy::SupportsRecursion);
    EXPECT_EQ(rw.getRecursionPolicyProperty(), LockRecursionPolicy::SupportsRecursion);
}

// ===========================================================================
// SpinWait
// ===========================================================================

TEST(SpinWaitTests, InitialCount_IsZero) {
    SpinWait sw;
    EXPECT_EQ(sw.getCountProperty(), 0);
}
TEST(SpinWaitTests, SpinOnce_IncrementsCount) {
    SpinWait sw;
    sw.SpinOnce();
    EXPECT_EQ(sw.getCountProperty(), 1);
}
TEST(SpinWaitTests, NextSpinWillYield_FalseBeforeThreshold) {
    SpinWait sw;
    EXPECT_FALSE(sw.getNextSpinWillYieldProperty());
}
TEST(SpinWaitTests, NextSpinWillYield_TrueAfterThreshold) {
    SpinWait sw;
    for (int i = 0; i < 10; ++i) sw.SpinOnce();
    EXPECT_TRUE(sw.getNextSpinWillYieldProperty());
}
TEST(SpinWaitTests, Reset_SetsCountToZero) {
    SpinWait sw;
    sw.SpinOnce();
    sw.Reset();
    EXPECT_EQ(sw.getCountProperty(), 0);
}
TEST(SpinWaitTests, SpinUntil_ConditionAlreadyTrue) {
    EXPECT_NO_THROW(SpinWait::SpinUntil([]() { return true; }));
}
TEST(SpinWaitTests, SpinUntil_WithTimeout_True_ReturnsTrue) {
    EXPECT_TRUE(SpinWait::SpinUntil([]() { return true; }, 100));
}
TEST(SpinWaitTests, SpinUntil_WithTimeout_NeverTrue_ReturnsFalse) {
    EXPECT_FALSE(SpinWait::SpinUntil([]() { return false; }, 1));
}

// Regression test for a wave-3 audit finding: SpinUntil(condition, -1) (Timeout.Infinite)
// used to compute a deadline already in the past (now() + milliseconds(-1)), so it returned
// false almost immediately instead of spinning until the condition became true, like real
// .NET (verified against SpinWait.cs: the 0-arg SpinUntil(condition) overload delegates to
// this one with Timeout.Infinite).
TEST(SpinWaitTests, SpinUntil_InfiniteTimeout_BlocksUntilConditionTrue) {
    std::atomic<bool> flag{false};
    std::atomic<bool> done{false};
    std::thread t([&] { done = SpinWait::SpinUntil([&] { return flag.load(); }, -1); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(done.load()); // still spinning after 100ms -- proves it didn't return immediately

    flag.store(true);
    t.join();
    EXPECT_TRUE(done.load());
}

// ===========================================================================
// ThreadLocal<T>
// ===========================================================================

TEST(ThreadLocalTests, DefaultCtor_IsValueCreated_False) {
    ThreadLocal<int> tl;
    EXPECT_FALSE(tl.getIsValueCreatedProperty());
}
TEST(ThreadLocalTests, GetValue_DefaultConstructsValue) {
    ThreadLocal<int> tl;
    EXPECT_EQ(tl.getValueProperty(), 0);
    EXPECT_TRUE(tl.getIsValueCreatedProperty());
}
TEST(ThreadLocalTests, SetValue_StoresValue) {
    ThreadLocal<int> tl;
    tl.setValueProperty(42);
    EXPECT_EQ(tl.getValueProperty(), 42);
}
// Uses a distinct struct type so it gets its own static thread_local slot
struct TLFactoryTag { int v = 0; };
TEST(ThreadLocalTests, Factory_UsedOnFirstAccess) {
    ThreadLocal<TLFactoryTag> tl(std::function<TLFactoryTag()>([]() { return TLFactoryTag{99}; }));
    EXPECT_EQ(tl.getValueProperty().v, 99);
}
TEST(ThreadLocalTests, Value_AliasForGetValue) {
    ThreadLocal<std::string> tl(std::function<std::string()>([]() { return std::string("hello"); }));
    EXPECT_EQ(tl.Value(), "hello");
}
TEST(ThreadLocalTests, Dispose_NoThrow) {
    ThreadLocal<int> tl;
    tl.getValueProperty();
    EXPECT_NO_THROW(tl.Dispose());
}
TEST(ThreadLocalTests, TwoInstances_SameType_AreIndependent) {
    ThreadLocal<int> a;
    ThreadLocal<int> b;
    a.setValueProperty(1);
    b.setValueProperty(2);
    EXPECT_EQ(a.getValueProperty(), 1);
    EXPECT_EQ(b.getValueProperty(), 2);
}
TEST(ThreadLocalTests, AfterDispose_GetValue_ThrowsObjectDisposedException) {
    ThreadLocal<int> tl;
    tl.Dispose();
    EXPECT_THROW(tl.getValueProperty(), System::ObjectDisposedException);
}
TEST(ThreadLocalTests, AfterDispose_SetValue_ThrowsObjectDisposedException) {
    ThreadLocal<int> tl;
    tl.Dispose();
    EXPECT_THROW(tl.setValueProperty(5), System::ObjectDisposedException);
}
TEST(ThreadLocalTests, InstanceDestroyedOnDifferentThread_ThenNewInstanceAtSameAddress_DoesNotLeakStaleValue) {
    alignas(ThreadLocal<int>) unsigned char buffer[sizeof(ThreadLocal<int>)];
    std::promise<void> constructed;
    std::shared_future<void> constructedFuture = constructed.get_future().share();
    std::promise<void> replaced;
    std::shared_future<void> replacedFuture = replaced.get_future().share();

    std::thread worker([&] {
        auto* a = new (buffer) ThreadLocal<int>();
        a->setValueProperty(222); // populates this worker thread's thread_local map
        constructed.set_value();  // real synchronization: main thread must not touch buffer yet
        replacedFuture.wait();    // block until the main thread has destroyed A and placement-new'd B
        auto* b = reinterpret_cast<ThreadLocal<int>*>(buffer);
        EXPECT_FALSE(b->getIsValueCreatedProperty()); // must not see A's leftover entry at all
        EXPECT_EQ(b->getValueProperty(), 0);
    });

    // See the identical AsyncLocalTests case above for why sleep_for() was replaced with a real
    // synchronizes-with relationship (ThreadSanitizer-flagged data race, 2026-07-14).
    constructedFuture.wait();
    reinterpret_cast<ThreadLocal<int>*>(buffer)->~ThreadLocal<int>(); // destroyed on the MAIN thread
    new (buffer) ThreadLocal<int>();                                  // instance B at the identical address
    replaced.set_value();
    worker.join();
    reinterpret_cast<ThreadLocal<int>*>(buffer)->~ThreadLocal<int>();
}
TEST(ThreadLocalTests, Factory_ReentrantAccess_ThrowsInvalidOperationException) {
    ThreadLocal<int>* self = nullptr;
    ThreadLocal<int> tl(std::function<int()>([&]() { return self->getValueProperty(); }));
    self = &tl;
    EXPECT_THROW(tl.getValueProperty(), System::InvalidOperationException);
    // Verify the guard was released, not left permanently stuck for this thread/instance.
    EXPECT_THROW(tl.getValueProperty(), System::InvalidOperationException);
}

// ===========================================================================
// ThreadPool
// ===========================================================================

TEST(ThreadPoolTests, QueueUserWorkItem_ReturnsTrue) {
    std::atomic<bool> ran{false};
    bool result = ThreadPool::QueueUserWorkItem([&ran]() { ran = true; });
    EXPECT_TRUE(result);
    // Give the detached thread a moment to run
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_TRUE(ran.load());
}
TEST(ThreadPoolTests, QueueUserWorkItem_NullCallback_Throws) {
    // Regression: an empty std::function invoked inside the detached thread previously
    // threw std::bad_function_call with no handler, crashing the process via
    // std::terminate() instead of throwing synchronously before the thread was spawned.
    EXPECT_THROW(ThreadPool::QueueUserWorkItem(std::function<void()>()), System::ArgumentNullException);
}
TEST(ThreadPoolTests, QueueUserWorkItem_WithState_NullCallback_Throws) {
    int state = 0;
    EXPECT_THROW(ThreadPool::QueueUserWorkItem(std::function<void(void*)>(), &state), System::ArgumentNullException);
}
TEST(ThreadPoolTests, UnsafeQueueUserWorkItem_NullCallback_Throws) {
    EXPECT_THROW(ThreadPool::UnsafeQueueUserWorkItem(nullptr, false), System::ArgumentNullException);
}
TEST(ThreadPoolTests, GetMinThreads_NonNegative) {
    int w = 0, c = 0;
    ThreadPool::GetMinThreads(w, c);
    EXPECT_GE(w, 0);
    EXPECT_GE(c, 0);
}
TEST(ThreadPoolTests, GetMaxThreads_AtLeastMin) {
    int wMin = 0, cMin = 0, wMax = 0, cMax = 0;
    ThreadPool::GetMinThreads(wMin, cMin);
    ThreadPool::GetMaxThreads(wMax, cMax);
    EXPECT_GE(wMax, wMin);
}
TEST(ThreadPoolTests, SetMinMaxThreads_ReturnsTrue) {
    EXPECT_TRUE(ThreadPool::SetMinThreads(1, 1));
    EXPECT_TRUE(ThreadPool::SetMaxThreads(8, 8));
}

// Timer: not unit-tested here — the implementation uses a detached std::thread
// that captures `this` by raw pointer, leading to dangling-pointer UB when the
// Timer object is destroyed before the thread wakes up.  Functional testing of
// Timer requires a fix to the implementation (e.g. a wakeup CV + join on Dispose).

// ===========================================================================
// Threading exceptions
// ===========================================================================

#define THREADING_EXCEPT_SIMPLE(ExType) \
    TEST(ExType##Tests, DefaultCtor_WhatNotEmpty) { \
        ExType ex; \
        EXPECT_FALSE(std::string(ex.what()).empty()); \
    } \
    TEST(ExType##Tests, IsA_Exception) { \
        EXPECT_THROW(throw ExType(), System::Exception); \
    }

THREADING_EXCEPT_SIMPLE(AbandonedMutexException)
THREADING_EXCEPT_SIMPLE(LockRecursionException)
THREADING_EXCEPT_SIMPLE(SemaphoreFullException)
THREADING_EXCEPT_SIMPLE(SynchronizationLockException)
THREADING_EXCEPT_SIMPLE(ThreadAbortException)
THREADING_EXCEPT_SIMPLE(ThreadInterruptedException)
THREADING_EXCEPT_SIMPLE(ThreadStateException)
THREADING_EXCEPT_SIMPLE(WaitHandleCannotBeOpenedException)

TEST(WaitHandleCannotBeOpenedExceptionTests, IsA_ApplicationException) {
    EXPECT_THROW(throw WaitHandleCannotBeOpenedException(), System::ApplicationException);
}
