// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Progress.hpp"
#include "System/ArgumentNullException.hpp"

using System::Progress;

TEST(ProgressTest, DefaultCtorDoesNotThrow) {
    Progress<int> p;
    EXPECT_NO_THROW(p.Report(42));
}

TEST(ProgressTest, HandlerCtorInvokesHandler) {
    int received = -1;
    Progress<int> p([&](int v) { received = v; });
    p.Report(99);
    EXPECT_EQ(received, 99);
}

TEST(ProgressTest, AddProgressChangedHandlerInvoked) {
    int received = 0;
    Progress<int> p;
    p.addProgressChangedHandler([&](int v) { received = v; });
    p.Report(7);
    EXPECT_EQ(received, 7);
}

TEST(ProgressTest, MultipleHandlersAllInvoked) {
    int a = 0, b = 0;
    Progress<int> p;
    p.addProgressChangedHandler([&](int v) { a = v; });
    p.addProgressChangedHandler([&](int v) { b = v * 2; });
    p.Report(5);
    EXPECT_EQ(a, 5);
    EXPECT_EQ(b, 10);
}

TEST(ProgressTest, ConstructorHandlerAndAddedHandlerBothInvoked) {
    int fromCtor = 0, fromAdd = 0;
    Progress<int> p([&](int v) { fromCtor = v; });
    p.addProgressChangedHandler([&](int v) { fromAdd = v + 1; });
    p.Report(3);
    EXPECT_EQ(fromCtor, 3);
    EXPECT_EQ(fromAdd, 4);
}

TEST(ProgressTest, ReportCalledMultipleTimes) {
    int count = 0;
    Progress<int> p([&](int) { ++count; });
    p.Report(1);
    p.Report(2);
    p.Report(3);
    EXPECT_EQ(count, 3);
}

TEST(ProgressTest, IsIProgress) {
    Progress<double> p;
    System::IProgress<double>& ref = p;
    int called = 0;
    p.addProgressChangedHandler([&](double) { ++called; });
    ref.Report(1.0);
    EXPECT_EQ(called, 1);
}

TEST(ProgressTest, StringProgress) {
    std::string last;
    Progress<std::string> p([&](const std::string& s) { last = s; });
    p.Report("hello");
    EXPECT_EQ(last, "hello");
}

TEST(ProgressTest, NullHandlerCtor_Throws) {
    // .NET's Progress<T>(Action<T>) throws ArgumentNullException for a null handler
    // (verified against Progress.cs: ArgumentNullException.ThrowIfNull(handler)).
    EXPECT_THROW(Progress<int>(std::function<void(int)>{}), System::ArgumentNullException);
}
