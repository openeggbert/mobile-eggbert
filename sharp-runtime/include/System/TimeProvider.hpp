// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/DateTimeOffset.hpp"
#include "System/DateTime.hpp"
#include "System/TimeSpan.hpp"
#include "System/TimeZoneInfo.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Diagnostics/Stopwatch.hpp"
#include "System/Threading/ITimer.hpp"
#include "System/Threading/Timer.hpp"
#include <memory>

namespace System {

    using SharpRuntime::longcs;

    /**
     * @brief Provides an abstraction for time.
     * C++ counterpart of .NET System.TimeProvider.
     */
    class TimeProvider {
    public:
        virtual ~TimeProvider() = default;

        /** Returns the system-default TimeProvider (wall clock + high-resolution timer). */
        static TimeProvider& getSystemProperty();

    protected:
        TimeProvider() = default;

    public:
        /** Returns the current UTC date and time. Override to supply a custom clock. */
        virtual DateTimeOffset GetUtcNow() const { return DateTimeOffset::getUtcNowProperty(); }

        /** Returns the current local date and time, converted via LocalTimeZone. */
        DateTimeOffset GetLocalNow() const {
            DateTimeOffset utcNow = GetUtcNow();
            const TimeZoneInfo& zone = getLocalTimeZoneProperty();
            TimeSpan offset = zone.GetUtcOffset(utcNow.getUtcDateTimeProperty());
            if (offset.getTicksProperty() == 0) return utcNow;
            longcs localTicks = utcNow.getUtcTicksProperty() + offset.getTicksProperty();
            constexpr longcs minTicks = 0LL;
            constexpr longcs maxTicks = 3'155'378'975'999'999'999LL;
            if (static_cast<unsigned long long>(localTicks) >
                static_cast<unsigned long long>(maxTicks)) {
                localTicks = localTicks < minTicks ? minTicks : maxTicks;
            }
            return DateTimeOffset(DateTime(localTicks), offset);
        }

        /** Returns the local time zone. Override to supply a fixed zone. */
        virtual const TimeZoneInfo& getLocalTimeZoneProperty() const {
            return TimeZoneInfo::Local();
        }

        /** Returns the frequency of GetTimestamp() in ticks per second. */
        virtual longcs getTimestampFrequencyProperty() const {
            return System::Diagnostics::Stopwatch::Frequency;
        }

        /** Returns the current high-frequency timestamp value. */
        virtual longcs GetTimestamp() const {
            return System::Diagnostics::Stopwatch::GetTimestamp();
        }

        /** Returns elapsed time between two GetTimestamp() values. */
        TimeSpan GetElapsedTime(longcs startingTimestamp, longcs endingTimestamp) const {
            longcs freq = getTimestampFrequencyProperty();
            if (freq <= 0)
                throw InvalidOperationException("The operation cannot be performed when TimeProvider.TimestampFrequency is zero or negative.");
            return TimeSpan(static_cast<longcs>(
                (endingTimestamp - startingTimestamp) *
                (static_cast<double>(TimeSpan::TicksPerSecond) / static_cast<double>(freq))
            ));
        }

        /** Returns elapsed time since startingTimestamp. */
        TimeSpan GetElapsedTime(longcs startingTimestamp) const {
            return GetElapsedTime(startingTimestamp, GetTimestamp());
        }

        /** Creates a new timer that invokes callback after dueTime and then every period. */
        virtual std::unique_ptr<Threading::ITimer> CreateTimer(
            Threading::TimerCallback callback, void* state,
            TimeSpan dueTime, TimeSpan period);
    };

} // namespace System
