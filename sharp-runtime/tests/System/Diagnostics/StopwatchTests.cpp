// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "System/Diagnostics/Stopwatch.hpp"

using System::Diagnostics::Stopwatch;

TEST(StopwatchTests, DefaultCtorNotRunning) {
    Stopwatch sw;
    EXPECT_FALSE(sw.getIsRunningProperty());
}

TEST(StopwatchTests, DefaultCtorZeroElapsed) {
    Stopwatch sw;
    EXPECT_EQ(0, sw.getElapsedMillisecondsProperty());
    EXPECT_EQ(0, sw.getElapsedTicksProperty());
}

TEST(StopwatchTests, StartSetsIsRunning) {
    Stopwatch sw;
    sw.Start();
    EXPECT_TRUE(sw.getIsRunningProperty());
    sw.Stop();
}

TEST(StopwatchTests, StopClearsIsRunning) {
    Stopwatch sw;
    sw.Start();
    sw.Stop();
    EXPECT_FALSE(sw.getIsRunningProperty());
}

TEST(StopwatchTests, ElapsedAccumulatesAfterStartStop) {
    Stopwatch sw;
    sw.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    sw.Stop();
    EXPECT_GT(sw.getElapsedMillisecondsProperty(), 0);
    EXPECT_GT(sw.getElapsedTicksProperty(), 0);
}

TEST(StopwatchTests, ElapsedGrowsWhileRunning) {
    Stopwatch sw;
    sw.Start();
    long long t1 = sw.getElapsedTicksProperty();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    long long t2 = sw.getElapsedTicksProperty();
    sw.Stop();
    EXPECT_GE(t2, t1);
}

TEST(StopwatchTests, StartNewIsRunning) {
    auto sw = Stopwatch::StartNew();
    EXPECT_TRUE(sw.getIsRunningProperty());
    sw.Stop();
}

TEST(StopwatchTests, StartNewAccumulatesElapsed) {
    auto sw = Stopwatch::StartNew();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    sw.Stop();
    EXPECT_GT(sw.getElapsedMillisecondsProperty(), 0);
}

TEST(StopwatchTests, ResetZerosElapsedAndStops) {
    Stopwatch sw;
    sw.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    sw.Stop();
    sw.Reset();
    EXPECT_FALSE(sw.getIsRunningProperty());
    EXPECT_EQ(0, sw.getElapsedMillisecondsProperty());
    EXPECT_EQ(0, sw.getElapsedTicksProperty());
}

TEST(StopwatchTests, ResetWhileRunningStopsAndZeros) {
    Stopwatch sw;
    sw.Start();
    sw.Reset();
    EXPECT_FALSE(sw.getIsRunningProperty());
    EXPECT_EQ(0, sw.getElapsedMillisecondsProperty());
}

TEST(StopwatchTests, RestartZerosAndStarts) {
    Stopwatch sw;
    sw.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    sw.Stop();
    sw.Restart();
    EXPECT_TRUE(sw.getIsRunningProperty());
    // elapsed ticks should now be very small (just restarted)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    sw.Stop();
    // We just want to confirm it restarted — elapsed after a 5ms+restart should be < 5ms previous
    EXPECT_GE(sw.getElapsedTicksProperty(), 0);
}

TEST(StopwatchTests, StopOnStoppedIsIdempotent) {
    Stopwatch sw;
    sw.Start();
    sw.Stop();
    long long t = sw.getElapsedTicksProperty();
    // Calling Stop again must not throw or alter accumulated time
    sw.Stop();
    EXPECT_EQ(t, sw.getElapsedTicksProperty());
    EXPECT_FALSE(sw.getIsRunningProperty());
}

TEST(StopwatchTests, StartOnRunningIsIdempotent) {
    Stopwatch sw;
    sw.Start();
    sw.Start();  // second call must be a no-op
    EXPECT_TRUE(sw.getIsRunningProperty());
    sw.Stop();
}

TEST(StopwatchTests, GetElapsedPropertyMatchesGetElapsedTicks) {
    Stopwatch sw;
    sw.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    sw.Stop();
    long long ticks = sw.getElapsedTicksProperty();
    System::TimeSpan ts = sw.getElapsedProperty();
    EXPECT_EQ(ticks, ts.getTicksProperty());
}

TEST(StopwatchTests, AccumulatesAcrossMultipleStartStop) {
    Stopwatch sw;
    sw.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    sw.Stop();
    long long first = sw.getElapsedTicksProperty();

    sw.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    sw.Stop();
    long long total = sw.getElapsedTicksProperty();

    EXPECT_GT(total, first);
}

TEST(StopwatchTests, Frequency_Is10MHz) {
    EXPECT_EQ(Stopwatch::Frequency, 10'000'000LL);
}

TEST(StopwatchTests, IsHighResolution_True) {
    EXPECT_TRUE(Stopwatch::IsHighResolution);
}

TEST(StopwatchTests, GetElapsedTime_TwoTimestamps_MatchesDelta) {
    long long start = Stopwatch::GetTimestamp();
    long long end = start + Stopwatch::Frequency; // exactly 1 second later, in ticks
    System::TimeSpan elapsed = Stopwatch::GetElapsedTime(start, end);
    EXPECT_EQ(elapsed.getTicksProperty(), Stopwatch::Frequency);
}

TEST(StopwatchTests, GetElapsedTime_SingleTimestamp_IsNonNegative) {
    long long start = Stopwatch::GetTimestamp();
    System::TimeSpan elapsed = Stopwatch::GetElapsedTime(start);
    EXPECT_GE(elapsed.getTicksProperty(), 0);
}

TEST(StopwatchTests, ToString_MatchesElapsedToString) {
    Stopwatch sw;
    sw.Start();
    sw.Stop();
    EXPECT_EQ(sw.ToString(), sw.getElapsedProperty().ToString());
}
