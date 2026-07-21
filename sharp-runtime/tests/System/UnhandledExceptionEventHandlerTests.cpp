// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UnhandledExceptionEventHandler.hpp"

using System::UnhandledExceptionEventHandler;
using System::UnhandledExceptionEventArgs;

TEST(UnhandledExceptionEventHandlerTest, AssignLambda) {
    bool called = false;
    UnhandledExceptionEventHandler h = [&](void*, UnhandledExceptionEventArgs&) {
        called = true;
    };
    EXPECT_TRUE(static_cast<bool>(h));
}

TEST(UnhandledExceptionEventHandlerTest, Invoke) {
    bool called = false;
    UnhandledExceptionEventHandler h = [&](void*, UnhandledExceptionEventArgs&) {
        called = true;
    };
    UnhandledExceptionEventArgs args(nullptr, false);
    h(nullptr, args);
    EXPECT_TRUE(called);
}

TEST(UnhandledExceptionEventHandlerTest, ReceivesSender) {
    void* received = nullptr;
    int sentinel = 42;
    UnhandledExceptionEventHandler h = [&](void* sender, UnhandledExceptionEventArgs&) {
        received = sender;
    };
    UnhandledExceptionEventArgs args(nullptr, false);
    h(&sentinel, args);
    EXPECT_EQ(received, &sentinel);
}

TEST(UnhandledExceptionEventHandlerTest, ReceivesArgs) {
    bool terminating = false;
    UnhandledExceptionEventHandler h = [&](void*, UnhandledExceptionEventArgs& e) {
        terminating = e.getIsTerminatingProperty();
    };
    UnhandledExceptionEventArgs args(nullptr, true);
    h(nullptr, args);
    EXPECT_TRUE(terminating);
}

TEST(UnhandledExceptionEventHandlerTest, DefaultIsEmpty) {
    UnhandledExceptionEventHandler h;
    EXPECT_FALSE(static_cast<bool>(h));
}
