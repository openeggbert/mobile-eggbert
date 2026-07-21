// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 23:
//   NotifyCollectionChangedAction, NotifyCollectionChangedEventArgs, NotifyCollectionChangedEventHandler
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Specialized/NotifyCollectionChangedAction.hpp"
#include "System/Collections/Specialized/NotifyCollectionChangedEventArgs.hpp"
#include "System/Collections/Specialized/NotifyCollectionChangedEventHandler.hpp"
#include <string>
#include <vector>

using System::Collections::Specialized::NotifyCollectionChangedAction;
using System::Collections::Specialized::NotifyCollectionChangedEventArgs;
using System::Collections::Specialized::NotifyCollectionChangedEventHandler;

TEST(NotifyCollectionChangedActionBatch23Test, Values) {
    EXPECT_EQ(static_cast<int>(NotifyCollectionChangedAction::Add), 0);
    EXPECT_EQ(static_cast<int>(NotifyCollectionChangedAction::Remove), 1);
    EXPECT_EQ(static_cast<int>(NotifyCollectionChangedAction::Replace), 2);
    EXPECT_EQ(static_cast<int>(NotifyCollectionChangedAction::Move), 3);
    EXPECT_EQ(static_cast<int>(NotifyCollectionChangedAction::Reset), 4);
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, Reset_Valid) {
    NotifyCollectionChangedEventArgs<int> args(NotifyCollectionChangedAction::Reset);
    EXPECT_EQ(args.Action, NotifyCollectionChangedAction::Reset);
    EXPECT_TRUE(args.NewItems.empty());
    EXPECT_TRUE(args.OldItems.empty());
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, Reset_WrongAction_Throws) {
    EXPECT_THROW(NotifyCollectionChangedEventArgs<int>(NotifyCollectionChangedAction::Add),
                 System::ArgumentException);
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, Add_SingleItem) {
    NotifyCollectionChangedEventArgs<int> args(NotifyCollectionChangedAction::Add, 42, 3);
    EXPECT_EQ(args.Action, NotifyCollectionChangedAction::Add);
    ASSERT_EQ(args.NewItems.size(), 1u);
    EXPECT_EQ(args.NewItems[0], 42);
    EXPECT_EQ(args.NewStartingIndex, 3);
    EXPECT_TRUE(args.OldItems.empty());
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, Remove_SingleItem) {
    NotifyCollectionChangedEventArgs<std::string> args(NotifyCollectionChangedAction::Remove, std::string("gone"), 1);
    EXPECT_EQ(args.Action, NotifyCollectionChangedAction::Remove);
    ASSERT_EQ(args.OldItems.size(), 1u);
    EXPECT_EQ(args.OldItems[0], "gone");
    EXPECT_EQ(args.OldStartingIndex, 1);
    EXPECT_TRUE(args.NewItems.empty());
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, SingleItem_WrongAction_Throws) {
    EXPECT_THROW(NotifyCollectionChangedEventArgs<int>(NotifyCollectionChangedAction::Move, 1, 0),
                 System::ArgumentException);
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, Add_MultiItem) {
    std::vector<int> items{1, 2, 3};
    NotifyCollectionChangedEventArgs<int> args(NotifyCollectionChangedAction::Add, items, 0);
    EXPECT_EQ(args.NewItems, items);
    EXPECT_EQ(args.NewStartingIndex, 0);
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, Add_MultiItem_StartingIndexMinus1_Allowed) {
    std::vector<int> items{1, 2, 3};
    NotifyCollectionChangedEventArgs<int> args(NotifyCollectionChangedAction::Add, items, -1);
    EXPECT_EQ(args.NewStartingIndex, -1);
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, Add_MultiItem_StartingIndexLessThanMinus1_Throws) {
    std::vector<int> items{1, 2, 3};
    EXPECT_THROW(NotifyCollectionChangedEventArgs<int>(NotifyCollectionChangedAction::Add, items, -2),
                 System::ArgumentOutOfRangeException);
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, Remove_MultiItem_StartingIndexLessThanMinus1_Throws) {
    std::vector<int> items{1, 2, 3};
    EXPECT_THROW(NotifyCollectionChangedEventArgs<int>(NotifyCollectionChangedAction::Remove, items, -5),
                 System::ArgumentOutOfRangeException);
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, Replace_Factory_SingleItem) {
    auto args = NotifyCollectionChangedEventArgs<int>::Replace(9, 1, 2);
    EXPECT_EQ(args.Action, NotifyCollectionChangedAction::Replace);
    ASSERT_EQ(args.NewItems.size(), 1u);
    ASSERT_EQ(args.OldItems.size(), 1u);
    EXPECT_EQ(args.NewItems[0], 9);
    EXPECT_EQ(args.OldItems[0], 1);
    EXPECT_EQ(args.NewStartingIndex, 2);
    EXPECT_EQ(args.OldStartingIndex, 2);
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, Replace_MultiItem_WrongAction_Throws) {
    std::vector<int> a{1}, b{2};
    EXPECT_THROW(NotifyCollectionChangedEventArgs<int>(NotifyCollectionChangedAction::Add, a, b),
                 System::ArgumentException);
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, Move_Factory_SingleItem) {
    auto args = NotifyCollectionChangedEventArgs<int>::Move(7, 2, 0);
    EXPECT_EQ(args.Action, NotifyCollectionChangedAction::Move);
    ASSERT_EQ(args.NewItems.size(), 1u);
    ASSERT_EQ(args.OldItems.size(), 1u);
    EXPECT_EQ(args.NewItems[0], 7);
    EXPECT_EQ(args.OldItems[0], 7);
    EXPECT_EQ(args.NewStartingIndex, 2);
    EXPECT_EQ(args.OldStartingIndex, 0);
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, Move_NegativeIndex_Throws) {
    EXPECT_THROW(NotifyCollectionChangedEventArgs<int>::Move(7, -1, 0),
                 System::ArgumentOutOfRangeException);
}

TEST(NotifyCollectionChangedEventArgsBatch23Test, IntInstantiation_NoConstructorAmbiguity) {
    // Regression check: with T = intcs, single-item Add/Remove ctor (action, T, intcs) must
    // not collide with the Replace factory (which is a static method, not a constructor).
    NotifyCollectionChangedEventArgs<int> add(NotifyCollectionChangedAction::Add, 5, 3);
    EXPECT_EQ(add.NewItems[0], 5);
    EXPECT_EQ(add.NewStartingIndex, 3);
}

TEST(NotifyCollectionChangedEventHandlerBatch23Test, InvokesWithSenderAndArgs) {
    bool invoked = false;
    NotifyCollectionChangedAction seenAction = NotifyCollectionChangedAction::Reset;
    NotifyCollectionChangedEventHandler<int> handler =
        [&](void* sender, const NotifyCollectionChangedEventArgs<int>& args) {
            invoked = (sender != nullptr);
            seenAction = args.Action;
        };
    NotifyCollectionChangedEventArgs<int> args(NotifyCollectionChangedAction::Add, 1, 0);
    int dummy = 0;
    handler(&dummy, args);
    EXPECT_TRUE(invoked);
    EXPECT_EQ(seenAction, NotifyCollectionChangedAction::Add);
}
