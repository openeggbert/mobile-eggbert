// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "System/EventHandler.hpp"
#include "System/EventArgs.hpp"

using System::EventHandler;
using System::EventArgs;

TEST(EventHandlerTests, DefaultCtor_Empty) {
    EventHandler<EventArgs> h;
    EXPECT_TRUE(h.Empty());
    EXPECT_EQ(h.Size(), 0u);
}

TEST(EventHandlerTests, AddOne_SizeIsOne) {
    EventHandler<EventArgs> h;
    h += [](System::Object*, const EventArgs&) {};
    EXPECT_EQ(h.Size(), 1u);
}

TEST(EventHandlerTests, Raise_SingleHandler_IsCalled) {
    EventHandler<EventArgs> h;
    int called = 0;
    h += [&called](System::Object*, const EventArgs&) { ++called; };
    h.Raise(nullptr, EventArgs::Empty);
    EXPECT_EQ(called, 1);
}

TEST(EventHandlerTests, Raise_MultipleHandlers_AllCalled) {
    EventHandler<EventArgs> h;
    int a = 0, b = 0;
    h += [&a](System::Object*, const EventArgs&) { ++a; };
    h += [&b](System::Object*, const EventArgs&) { ++b; };
    h.Raise(nullptr, EventArgs::Empty);
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 1);
}

TEST(EventHandlerTests, Raise_HandlersCalledInOrder) {
    EventHandler<EventArgs> h;
    std::vector<int> order;
    h += [&order](System::Object*, const EventArgs&) { order.push_back(1); };
    h += [&order](System::Object*, const EventArgs&) { order.push_back(2); };
    h += [&order](System::Object*, const EventArgs&) { order.push_back(3); };
    h.Raise(nullptr, EventArgs::Empty);
    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 3);
}

TEST(EventHandlerTests, Invoke_AliasForRaise) {
    EventHandler<EventArgs> h;
    int called = 0;
    h += [&called](System::Object*, const EventArgs&) { ++called; };
    h.Invoke(nullptr, EventArgs::Empty);
    EXPECT_EQ(called, 1);
}

TEST(EventHandlerTests, Remove_ByToken_HandlerNotCalled) {
    EventHandler<EventArgs> h;
    int called = 0;
    auto token = h.Add([&called](System::Object*, const EventArgs&) { ++called; });
    h.Remove(token);
    h.Raise(nullptr, EventArgs::Empty);
    EXPECT_EQ(called, 0);
    EXPECT_TRUE(h.Empty());
}

TEST(EventHandlerTests, Remove_UnknownToken_NoEffect) {
    EventHandler<EventArgs> h;
    int called = 0;
    h += [&called](System::Object*, const EventArgs&) { ++called; };
    h.Remove(9999); // no-op
    h.Raise(nullptr, EventArgs::Empty);
    EXPECT_EQ(called, 1);
}

TEST(EventHandlerTests, Clear_RemovesAllHandlers) {
    EventHandler<EventArgs> h;
    h += [](System::Object*, const EventArgs&) {};
    h += [](System::Object*, const EventArgs&) {};
    h.Clear();
    EXPECT_TRUE(h.Empty());
    EXPECT_EQ(h.Size(), 0u);
}

TEST(EventHandlerTests, Raise_WithCustomEventArgs) {
    struct CountArgs : public EventArgs { int count = 7; };
    EventHandler<CountArgs> h;
    int received = 0;
    h += [&received](System::Object*, const CountArgs& e) { received = e.count; };
    CountArgs args;
    h.Raise(nullptr, args);
    EXPECT_EQ(received, 7);
}

TEST(EventHandlerTests, Raise_Empty_DoesNotThrow) {
    EventHandler<EventArgs> h;
    EXPECT_NO_THROW(h.Raise(nullptr, EventArgs::Empty));
}

// Raise() must invoke a snapshot of the handler list, matching C# multicast delegate semantics:
// a handler that removes itself (or another handler) mid-raise must not corrupt the current
// invocation. Without a snapshot, this used to dereference an already-destroyed std::function
// (undefined behavior, observed as an escaping std::bad_function_call).
TEST(EventHandlerTests, Raise_HandlerRemovesItselfDuringRaise_DoesNotThrowAndStillFiresOthers) {
    EventHandler<EventArgs> h;
    int firedA = 0, firedB = 0;
    EventHandler<EventArgs>::Token tokenA = 0;
    tokenA = h.Add([&](System::Object*, const EventArgs&) {
        ++firedA;
        h.Remove(tokenA);
    });
    h += [&firedB](System::Object*, const EventArgs&) { ++firedB; };

    EXPECT_NO_THROW(h.Raise(nullptr, EventArgs::Empty));
    EXPECT_EQ(firedA, 1);
    EXPECT_EQ(firedB, 1); // still fires in this same Raise() -- the snapshot already included it

    // The removal takes effect for the *next* Raise(), matching C# semantics.
    h.Raise(nullptr, EventArgs::Empty);
    EXPECT_EQ(firedA, 1); // not called again -- it removed itself before the first Raise() finished
    EXPECT_EQ(firedB, 2);
}

// A handler removing a *different*, not-yet-invoked handler mid-raise must not skip or crash on
// that handler either -- the snapshot already captured it before the removal happened.
TEST(EventHandlerTests, Raise_HandlerRemovesAnotherHandlerDuringRaise_StillFiresBoth) {
    EventHandler<EventArgs> h;
    int firedA = 0, firedB = 0;
    EventHandler<EventArgs>::Token tokenB = 0;
    h += [&](System::Object*, const EventArgs&) {
        ++firedA;
        h.Remove(tokenB);
    };
    tokenB = h.Add([&firedB](System::Object*, const EventArgs&) { ++firedB; });

    EXPECT_NO_THROW(h.Raise(nullptr, EventArgs::Empty));
    EXPECT_EQ(firedA, 1);
    EXPECT_EQ(firedB, 1);
    EXPECT_EQ(h.Size(), 1u); // A is still subscribed; only B was removed
}

// SetReplayHook(): models real XNA events like NetworkSession.GamerJoined, which replay already-
// happened state to a handler the instant it subscribes, not just for future occurrences.

TEST(EventHandlerTests, NoReplayHookSet_AddBehavesExactlyAsBefore) {
    EventHandler<EventArgs> h;
    int called = 0;
    h += [&called](System::Object*, const EventArgs&) { ++called; };
    EXPECT_EQ(called, 0); // Add() alone never invokes a handler when no hook is set
    EXPECT_EQ(h.Size(), 1u);
}

TEST(EventHandlerTests, ReplayHook_CalledOnceImmediatelyForEachNewSubscriber) {
    EventHandler<EventArgs> h;
    int replayCount = 0;
    h.SetReplayHook([&](const EventHandler<EventArgs>::HandlerType& handler) {
        ++replayCount;
        handler(nullptr, EventArgs::Empty);
    });

    int firedA = 0;
    h += [&firedA](System::Object*, const EventArgs&) { ++firedA; };
    EXPECT_EQ(replayCount, 1);
    EXPECT_EQ(firedA, 1);

    int firedB = 0;
    h += [&firedB](System::Object*, const EventArgs&) { ++firedB; };
    EXPECT_EQ(replayCount, 2);
    EXPECT_EQ(firedB, 1);
    EXPECT_EQ(firedA, 1); // unaffected by the second subscription's own replay
}

TEST(EventHandlerTests, ReplayHook_DoesNotAffectSizeOrSubsequentRaise) {
    EventHandler<EventArgs> h;
    h.SetReplayHook([](const EventHandler<EventArgs>::HandlerType& handler) {
        handler(nullptr, EventArgs::Empty);
    });

    int called = 0;
    h += [&called](System::Object*, const EventArgs&) { ++called; };
    EXPECT_EQ(called, 1); // from the replay
    EXPECT_EQ(h.Size(), 1u); // the hook call itself is not a second subscriber

    h.Raise(nullptr, EventArgs::Empty);
    EXPECT_EQ(called, 2); // normal Raise() still reaches it exactly once
}

TEST(EventHandlerTests, ReplayHook_ClearedBySettingEmptyFunction) {
    EventHandler<EventArgs> h;
    int replayCount = 0;
    h.SetReplayHook([&](const EventHandler<EventArgs>::HandlerType&) { ++replayCount; });
    h.SetReplayHook(nullptr); // clears it

    h += [](System::Object*, const EventArgs&) {};
    EXPECT_EQ(replayCount, 0);
}
