// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Timers/Timer.hpp"
#include <cmath>
#include "System/ArgumentException.hpp"
#include "System/DateTime.hpp"
#include "System/Threading/Timer.hpp"

namespace System::Timers {

namespace {
    constexpr double kMaxInterval = 2147483647.0; // int32 max, matching .NET's validation
}

Timer::Timer() = default;

Timer::Timer(double interval) : Timer() {
    if (interval <= 0 || std::ceil(interval) > kMaxInterval) {
        throw System::ArgumentException("Invalid value for interval: " + std::to_string(interval), "interval");
    }
    interval_ = std::ceil(interval);
}

Timer::Timer(System::TimeSpan interval) : Timer(interval.getTotalMillisecondsProperty()) {}

Timer::~Timer() { Close(); }

void Timer::setAutoResetProperty(bool value) {
    if (autoReset_ != value) {
        autoReset_ = value;
        updateTimer();
    }
}

void Timer::setIntervalProperty(double value) {
    if (value <= 0) {
        throw System::ArgumentException("Invalid value for interval: " + std::to_string(value), "value");
    }
    interval_ = value;
    updateTimer();
}

void Timer::updateTimer() {
    if (!timer_ || !enabled_) return;
    auto i = static_cast<SharpRuntime::intcs>(std::ceil(interval_));
    timer_->Change(i, autoReset_ ? i : -1);
}

void Timer::startTimerThread() {
    auto i = static_cast<SharpRuntime::intcs>(std::ceil(interval_));
    cookie_ = std::make_shared<int>(0);
    auto cookie = cookie_;
    bool autoReset = autoReset_;
    timer_ = std::make_unique<System::Threading::Timer>(
        [this, cookie, autoReset](void* /*state*/) {
            if (cookie != cookie_) return; // stale callback from a since-stopped timer
            if (!autoReset) {
                enabled_ = false;
            }
            ElapsedEventArgs args(System::DateTime::getNowProperty());
            Elapsed.Raise(nullptr, args);
        },
        nullptr, i, autoReset_ ? i : -1);
}

void Timer::setEnabledProperty(bool value) {
    if (initializing_) {
        delayedEnable_ = value;
        return;
    }
    if (enabled_ == value) return;

    if (!value) {
        cookie_ = nullptr;
        timer_.reset();
        enabled_ = false;
    } else {
        enabled_ = true;
        if (!timer_) {
            startTimerThread();
        } else {
            updateTimer();
        }
    }
}

void Timer::BeginInit() {
    Close();
    initializing_ = true;
}

void Timer::EndInit() {
    initializing_ = false;
    setEnabledProperty(delayedEnable_);
}

void Timer::Close() {
    initializing_ = false;
    delayedEnable_ = false;
    enabled_ = false;
    cookie_ = nullptr;
    timer_.reset();
}

} // namespace System::Timers
