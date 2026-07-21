// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/EventArgs.hpp"

using System::EventArgs;

TEST(EventArgsTests, DefaultCtor_DoesNotThrow) {
    EXPECT_NO_THROW(EventArgs e);
}

TEST(EventArgsTests, Empty_IsEventArgs) {
    const EventArgs& e = EventArgs::Empty;
    EXPECT_NE(dynamic_cast<const EventArgs*>(&e), nullptr);
}

TEST(EventArgsTests, CanDeriveFromEventArgs) {
    struct MyArgs : public EventArgs { int value = 42; };
    MyArgs a;
    EXPECT_EQ(a.value, 42);
    EXPECT_NE(dynamic_cast<EventArgs*>(&a), nullptr);
}

TEST(EventArgsTests, Catchable_AsEventArgs) {
    struct TestArgs : public EventArgs {};
    TestArgs a;
    EventArgs* ptr = &a;
    EXPECT_NE(ptr, nullptr);
}

TEST(EventArgsTests, Empty_CanBeUsed) {
    // Simulate passing EventArgs::Empty to a handler
    auto handler = [](const EventArgs&) { return true; };
    EXPECT_TRUE(handler(EventArgs::Empty));
}

TEST(EventArgsTests, VirtualDestructor_RunsDerivedDestructor) {
    static bool destroyed = false;
    struct TestArgs : public EventArgs {
        ~TestArgs() override { destroyed = true; }
    };
    EventArgs* ptr = new TestArgs();
    delete ptr;
    EXPECT_TRUE(destroyed);
}
