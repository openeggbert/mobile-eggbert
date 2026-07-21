// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <chrono>
#include "System/TimeProvider.hpp"
#include "System/Diagnostics/Stopwatch.hpp"

using System::TimeProvider;
using System::TimeSpan;
using System::DateTimeOffset;
using System::Diagnostics::Stopwatch;
using SharpRuntime::longcs;

TEST(TimeProviderTests, SystemSingleton_NotNull) {
    TimeProvider& tp = TimeProvider::getSystemProperty();
    (void)tp; // just verify it compiles and doesn't crash
    SUCCEED();
}

TEST(TimeProviderTests, GetUtcNow_ReturnsRecentTime) {
    TimeProvider& tp = TimeProvider::getSystemProperty();
    DateTimeOffset now = tp.GetUtcNow();
    EXPECT_GT(now.getYearProperty(), 2020);
}

TEST(TimeProviderTests, GetLocalNow_ReturnsRecentTime) {
    TimeProvider& tp = TimeProvider::getSystemProperty();
    DateTimeOffset local = tp.GetLocalNow();
    EXPECT_GT(local.getYearProperty(), 2020);
}

TEST(TimeProviderTests, TimestampFrequency_Matches_Stopwatch) {
    TimeProvider& tp = TimeProvider::getSystemProperty();
    EXPECT_EQ(tp.getTimestampFrequencyProperty(), Stopwatch::Frequency);
}

TEST(TimeProviderTests, GetTimestamp_Increases) {
    TimeProvider& tp = TimeProvider::getSystemProperty();
    longcs t1 = tp.GetTimestamp();
    longcs t2 = tp.GetTimestamp();
    EXPECT_GE(t2, t1);
}

TEST(TimeProviderTests, GetElapsedTime_TwoArgs_ZeroForSameTimestamp) {
    TimeProvider& tp = TimeProvider::getSystemProperty();
    longcs t = tp.GetTimestamp();
    TimeSpan elapsed = tp.GetElapsedTime(t, t);
    EXPECT_EQ(elapsed.getTicksProperty(), 0);
}

TEST(TimeProviderTests, GetElapsedTime_OneArg_NonNegative) {
    TimeProvider& tp = TimeProvider::getSystemProperty();
    longcs t = tp.GetTimestamp();
    TimeSpan elapsed = tp.GetElapsedTime(t);
    EXPECT_GE(elapsed.getTicksProperty(), 0);
}

TEST(TimeProviderTests, GetElapsedTime_ReflectsDelay) {
    TimeProvider& tp = TimeProvider::getSystemProperty();
    longcs freq = tp.getTimestampFrequencyProperty();
    // Simulate 1 second: freq ticks = 1 second
    TimeSpan one_sec = tp.GetElapsedTime(0, freq);
    EXPECT_NEAR(one_sec.getTotalSecondsProperty(), 1.0, 1e-6);
}

TEST(TimeProviderTests, CreateTimer_FiresCallback) {
    std::atomic<int> count{0};
    System::Threading::TimerCallback cb = [&](void*) { count++; };
    auto timer = TimeProvider::getSystemProperty().CreateTimer(
        cb, nullptr,
        System::TimeSpan::FromMilliseconds(20),
        System::TimeSpan::FromMilliseconds(-1)); // fire once
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    timer->Dispose();
    EXPECT_GE(count.load(), 1);
}

TEST(TimeProviderTests, CustomProvider_OverridesGetUtcNow) {
    struct FakeProvider : TimeProvider {
        DateTimeOffset GetUtcNow() const override {
            return DateTimeOffset(System::DateTime(2024, 6, 1, 12, 0, 0), System::TimeSpan::Zero);
        }
    } fake;
    DateTimeOffset t = fake.GetUtcNow();
    EXPECT_EQ(t.getYearProperty(), 2024);
    EXPECT_EQ(t.getMonthProperty(), 6);
    EXPECT_EQ(t.getDayProperty(), 1);
}
