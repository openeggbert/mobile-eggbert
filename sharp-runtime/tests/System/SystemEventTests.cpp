// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for event-related types: EventArgs, AssemblyLoadEventArgs,
// ResolveEventArgs, UnhandledExceptionEventArgs, ThreadExceptionEventArgs,
// and event handler type aliases.
#include <gtest/gtest.h>
#include <stdexcept>
#include "System/EventArgs.hpp"
#include "System/AssemblyLoadEventArgs.hpp"
#include "System/ResolveEventArgs.hpp"
#include "System/UnhandledExceptionEventArgs.hpp"
#include "System/UnhandledExceptionEventHandler.hpp"
#include "System/ResolveEventHandler.hpp"
#include "System/AsyncCallback.hpp"
#include "System/Threading/ThreadExceptionEventArgs.hpp"

using System::EventArgs;
using System::AssemblyLoadEventArgs;
using System::ResolveEventArgs;
using System::UnhandledExceptionEventArgs;

// ===========================================================================
// EventArgs
// ===========================================================================

TEST(EventArgsTests, DefaultCtor_NoThrow) {
    EXPECT_NO_THROW(EventArgs{});
}

TEST(EventArgsTests, Empty_IsSameInstanceOnEveryAccess) {
    const EventArgs& a = EventArgs::Empty;
    const EventArgs& b = EventArgs::Empty;
    EXPECT_EQ(&a, &b);
}

// ===========================================================================
// AssemblyLoadEventArgs
// ===========================================================================

TEST(AssemblyLoadEventArgsTests, Constructor_StoresName) {
    AssemblyLoadEventArgs args("MyAssembly");
    EXPECT_EQ(args.getLoadedAssemblyProperty(), "MyAssembly");
}

TEST(AssemblyLoadEventArgsTests, IsA_EventArgs) {
    AssemblyLoadEventArgs args("X");
    EventArgs& base = args;
    (void)base;
    SUCCEED();
}

// ===========================================================================
// ResolveEventArgs
// ===========================================================================

TEST(ResolveEventArgsTests, SingleArgCtor_StoresName) {
    ResolveEventArgs args("System.Foo");
    EXPECT_EQ(args.getNameProperty(), "System.Foo");
    EXPECT_TRUE(args.getRequestingAssemblyNameProperty().empty());
}

TEST(ResolveEventArgsTests, TwoArgCtor_StoresBoth) {
    ResolveEventArgs args("System.Foo", "MyApp");
    EXPECT_EQ(args.getNameProperty(), "System.Foo");
    EXPECT_EQ(args.getRequestingAssemblyNameProperty(), "MyApp");
}

TEST(ResolveEventArgsTests, IsA_EventArgs) {
    ResolveEventArgs args("System.Foo");
    EXPECT_NE(dynamic_cast<System::EventArgs*>(&args), nullptr);
}

TEST(ResolveEventArgsTests, RequestingAssembly_DefaultEmpty) {
    ResolveEventArgs args("MyType");
    EXPECT_TRUE(args.getRequestingAssemblyNameProperty().empty());
}

// ===========================================================================
// UnhandledExceptionEventArgs
// ===========================================================================

TEST(UnhandledExceptionEventArgsTests, IsTerminating_True) {
    auto ex = std::make_exception_ptr(std::runtime_error("boom"));
    UnhandledExceptionEventArgs args(ex, true);
    EXPECT_TRUE(args.getIsTerminatingProperty());
}

TEST(UnhandledExceptionEventArgsTests, IsTerminating_False) {
    UnhandledExceptionEventArgs args(nullptr, false);
    EXPECT_FALSE(args.getIsTerminatingProperty());
}

TEST(UnhandledExceptionEventArgsTests, ExceptionObject_Stored) {
    auto ex = std::make_exception_ptr(std::runtime_error("test"));
    UnhandledExceptionEventArgs args(ex, false);
    EXPECT_NE(args.getExceptionObjectProperty(), nullptr);
}

TEST(UnhandledExceptionEventArgsTests, NullException_Stored) {
    UnhandledExceptionEventArgs args(nullptr, false);
    EXPECT_EQ(args.getExceptionObjectProperty(), nullptr);
}
TEST(UnhandledExceptionEventArgsTests, IsA_EventArgs) {
    auto ex = std::make_exception_ptr(std::runtime_error("x"));
    UnhandledExceptionEventArgs args(ex, false);
    EXPECT_NE(dynamic_cast<System::EventArgs*>(&args), nullptr);
}
TEST(UnhandledExceptionEventHandlerTests, Invokable_ReceivesSenderAndArgs) {
    void* capturedSender = nullptr;
    bool  capturedTerminating = false;
    System::UnhandledExceptionEventHandler h =
        [&](void* sender, UnhandledExceptionEventArgs& e) {
            capturedSender      = sender;
            capturedTerminating = e.getIsTerminatingProperty();
        };
    int dummy = 0;
    auto ex = std::make_exception_ptr(std::runtime_error("err"));
    UnhandledExceptionEventArgs args(ex, true);
    h(&dummy, args);
    EXPECT_EQ(capturedSender, &dummy);
    EXPECT_TRUE(capturedTerminating);
}
TEST(UnhandledExceptionEventHandlerTests, NullHandler_IsFalsy) {
    System::UnhandledExceptionEventHandler h;
    EXPECT_FALSE(static_cast<bool>(h));
}

// ===========================================================================
// ThreadExceptionEventArgs
// ===========================================================================

TEST(ThreadExceptionEventArgsTests, Constructor_StoresException) {
    auto ex = std::make_exception_ptr(std::runtime_error("thread error"));
    System::Threading::ThreadExceptionEventArgs args(ex);
    EXPECT_NE(args.getExceptionProperty(), nullptr);
}

TEST(ThreadExceptionEventArgsTests, NullException_Stored) {
    System::Threading::ThreadExceptionEventArgs args(nullptr);
    EXPECT_EQ(args.getExceptionProperty(), nullptr);
}

TEST(ThreadExceptionEventArgsTests, IsA_EventArgs) {
    auto ex = std::make_exception_ptr(std::runtime_error("err"));
    System::Threading::ThreadExceptionEventArgs args(ex);
    System::EventArgs& base = args;
    (void)base;
    SUCCEED();
}

// ===========================================================================
// Event handler type aliases (compile-time check)
// ===========================================================================

TEST(EventHandlerTypeTests, UnhandledExceptionEventHandler_Callable) {
    bool called = false;
    System::UnhandledExceptionEventHandler handler = [&](void*, System::UnhandledExceptionEventArgs&) {
        called = true;
    };
    System::UnhandledExceptionEventArgs args(nullptr, false);
    handler(nullptr, args);
    EXPECT_TRUE(called);
}

TEST(EventHandlerTypeTests, ResolveEventHandler_Callable) {
    System::ResolveEventHandler handler = [](void*, System::ResolveEventArgs& a) -> std::string {
        return a.getNameProperty();
    };
    System::ResolveEventArgs args("MyAssembly");
    EXPECT_EQ(handler(nullptr, args), "MyAssembly");
}
