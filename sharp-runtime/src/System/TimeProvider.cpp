// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/TimeProvider.hpp"

namespace {

    class SystemTimeProvider : public System::TimeProvider {
    public:
        SystemTimeProvider() = default;
    };

    // ITimer wrapper around System::Threading::Timer
    class SystemTimeProviderTimer : public System::Threading::ITimer {
        System::Threading::Timer timer_;
    public:
        SystemTimeProviderTimer(System::Threading::TimerCallback cb, void* state,
                                System::TimeSpan dueTime, System::TimeSpan period)
            : timer_(std::move(cb), state,
                     static_cast<SharpRuntime::intcs>(dueTime.getTotalMillisecondsProperty()),
                     static_cast<SharpRuntime::intcs>(period.getTotalMillisecondsProperty()))
        {}

        bool Change(System::TimeSpan dueTime, System::TimeSpan period) override {
            timer_.Change(
                static_cast<SharpRuntime::intcs>(dueTime.getTotalMillisecondsProperty()),
                static_cast<SharpRuntime::intcs>(period.getTotalMillisecondsProperty()));
            return true;
        }

        void Dispose() override { timer_.Dispose(); }
    };

} // anonymous namespace

namespace System {

    TimeProvider& TimeProvider::getSystemProperty() {
        static SystemTimeProvider instance;
        return instance;
    }

    std::unique_ptr<Threading::ITimer> TimeProvider::CreateTimer(
        Threading::TimerCallback callback, void* state,
        TimeSpan dueTime, TimeSpan period)
    {
        return std::make_unique<SystemTimeProviderTimer>(
            std::move(callback), state, dueTime, period);
    }

} // namespace System
