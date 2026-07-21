// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include "System/ArgumentException.hpp"
#include "System/DateTime.hpp"
#include "System/Timers/ElapsedEventArgs.hpp"
#include "System/Timers/ElapsedEventHandler.hpp"
#include "System/Timers/Timer.hpp"
#include "System/Timers/TimersDescriptionAttribute.hpp"

using namespace System::Timers;

namespace {
bool waitFor(std::atomic<int>& counter, int atLeast, int timeoutMs) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (std::chrono::steady_clock::now() < deadline) {
        if (counter.load() >= atLeast) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return counter.load() >= atLeast;
}
} // namespace

TEST(TimerTests, DefaultConstructor_HasDefaultIntervalAndDisabled) {
    Timer timer;
    EXPECT_EQ(timer.getIntervalProperty(), 100.0);
    EXPECT_FALSE(timer.getEnabledProperty());
    EXPECT_TRUE(timer.getAutoResetProperty());
}

TEST(TimerTests, ConstructorWithInterval_Throws_WhenNotPositive) {
    EXPECT_THROW(Timer(0.0), System::ArgumentException);
    EXPECT_THROW(Timer(-5.0), System::ArgumentException);
}

TEST(TimerTests, SetInterval_Throws_WhenNotPositive) {
    Timer timer;
    EXPECT_THROW(timer.setIntervalProperty(0.0), System::ArgumentException);
}

TEST(TimerTests, StartStop_TogglesEnabled) {
    Timer timer(50.0);
    EXPECT_FALSE(timer.getEnabledProperty());
    timer.Start();
    EXPECT_TRUE(timer.getEnabledProperty());
    timer.Stop();
    EXPECT_FALSE(timer.getEnabledProperty());
}

TEST(TimerTests, Elapsed_FiresRepeatedlyWithAutoReset) {
    Timer timer(15.0);
    std::atomic<int> count{0};
    timer.Elapsed += [&count](System::Object*, const ElapsedEventArgs&) { count++; };
    timer.Start();
    EXPECT_TRUE(waitFor(count, 3, 2000));
    timer.Stop();
}

TEST(TimerTests, Elapsed_FiresOnceWithoutAutoReset) {
    Timer timer(15.0);
    timer.setAutoResetProperty(false);
    std::atomic<int> count{0};
    timer.Elapsed += [&count](System::Object*, const ElapsedEventArgs&) { count++; };
    timer.Start();
    EXPECT_TRUE(waitFor(count, 1, 2000));
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    EXPECT_EQ(count.load(), 1);
}

TEST(TimerTests, BeginInitEndInit_DelaysEnable) {
    Timer timer(15.0);
    timer.BeginInit();
    timer.setEnabledProperty(true);
    EXPECT_FALSE(timer.getEnabledProperty()); // delayed until EndInit()
    timer.EndInit();
    EXPECT_TRUE(timer.getEnabledProperty());
    timer.Stop();
}

TEST(TimersDescriptionAttributeTests, StoresDescription) {
    TimersDescriptionAttribute attr("Fires when the interval elapses.");
    EXPECT_EQ(attr.getDescriptionProperty(), "Fires when the interval elapses.");
}

TEST(ElapsedEventArgsTests, StoresSignalTime) {
    auto now = System::DateTime::getNowProperty();
    ElapsedEventArgs args(now);
    EXPECT_EQ(args.getSignalTimeProperty(), now);
}
