// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "System/MulticastAction.hpp"

using System::MulticastAction;

TEST(MulticastActionTests, DefaultConstructedIsEmptyAndInvokeIsNoOp) {
    MulticastAction<int> action;
    EXPECT_FALSE(static_cast<bool>(action));
    EXPECT_TRUE(action.Empty());
    EXPECT_NO_THROW(action(42)); // no subscribers -> no-op
}

TEST(MulticastActionTests, AssignmentSetsASingleSubscriber) {
    MulticastAction<int> action;
    int seen = 0;
    action = [&seen](int v) { seen = v; };
    EXPECT_TRUE(static_cast<bool>(action));
    action(7);
    EXPECT_EQ(seen, 7);
}

TEST(MulticastActionTests, PlusEqualsAddsSubscribersInvokedInOrder) {
    MulticastAction<int> action;
    std::vector<int> order;
    action += [&order](int) { order.push_back(1); };
    action += [&order](int v) { order.push_back(10 + v); };
    action(5);
    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 15);
}

TEST(MulticastActionTests, AssignmentReplacesTheWholeInvocationList) {
    MulticastAction<int> action;
    int added = 0;
    int assigned = 0;
    action += [&added](int) { ++added; };
    action = [&assigned](int) { ++assigned; }; // replaces, does not append
    action(0);
    EXPECT_EQ(added, 0);
    EXPECT_EQ(assigned, 1);
}

TEST(MulticastActionTests, NullptrAssignmentClearsAllSubscribers) {
    MulticastAction<int> action;
    int calls = 0;
    action += [&calls](int) { ++calls; };
    action = nullptr;
    EXPECT_FALSE(static_cast<bool>(action));
    action(0);
    EXPECT_EQ(calls, 0);
}

TEST(MulticastActionTests, SupportsReferenceAndMultipleParameters) {
    MulticastAction<const std::string&, int> action;
    std::string lastText;
    int lastNum = 0;
    action += [&](const std::string& t, int n) { lastText = t; lastNum = n; };
    action("hello", 3);
    EXPECT_EQ(lastText, "hello");
    EXPECT_EQ(lastNum, 3);
}

TEST(MulticastActionTests, SnapshotSemanticsSubscribeDuringInvokeAffectsOnlyNextInvoke) {
    MulticastAction<> action;
    int calls = 0;
    action += [&]() {
        ++calls;
        // Subscribing during invocation must not run the new handler in this same invocation.
        action += [&]() { ++calls; };
    };
    action();            // only the original handler runs
    EXPECT_EQ(calls, 1);
    action();            // now both handlers run
    EXPECT_EQ(calls, 3); // 1 (prev) + 2 (this invoke)
}

TEST(MulticastActionTests, AddReturnsATokenAndSubscribesLikePlusEquals) {
    MulticastAction<int> action;
    int seen = 0;
    const auto token = action.Add([&seen](int v) { seen = v; });
    EXPECT_NE(token, MulticastAction<int>::InvalidToken);
    action(9);
    EXPECT_EQ(seen, 9);
}

TEST(MulticastActionTests, AddOfEmptyHandlerReturnsInvalidTokenAndDoesNotSubscribe) {
    MulticastAction<int> action;
    const auto token = action.Add(nullptr);
    EXPECT_EQ(token, MulticastAction<int>::InvalidToken);
    EXPECT_TRUE(action.Empty());
}

TEST(MulticastActionTests, RemoveRemovesOnlyTheIdentifiedSubscription) {
    MulticastAction<> action;
    int firstCalls = 0;
    int secondCalls = 0;
    const auto firstToken = action.Add([&]() { ++firstCalls; });
    action.Add([&]() { ++secondCalls; }); // never removed

    EXPECT_TRUE(action.Remove(firstToken));
    action();

    EXPECT_EQ(firstCalls, 0);
    EXPECT_EQ(secondCalls, 1);
}

TEST(MulticastActionTests, RemoveOfUnknownOrAlreadyRemovedTokenIsANoOpReturningFalse) {
    MulticastAction<> action;
    const auto token = action.Add([]() {});

    EXPECT_TRUE(action.Remove(token));
    EXPECT_FALSE(action.Remove(token)); // already removed
    EXPECT_FALSE(action.Remove(MulticastAction<>::InvalidToken));
}

TEST(MulticastActionTests, ResubscribingAfterRemoveGetsAFreshToken) {
    // Motivating use case: CNA::Extended::BaseTransform re-subscribes to every ancestor's
    // TransformBecameDirty event whenever Parent changes, unsubscribing the old chain first.
    MulticastAction<> action;
    int calls = 0;
    const auto firstToken = action.Add([&]() { ++calls; });
    ASSERT_TRUE(action.Remove(firstToken));

    const auto secondToken = action.Add([&]() { ++calls; });
    EXPECT_NE(secondToken, firstToken);

    action();
    EXPECT_EQ(calls, 1);
}
