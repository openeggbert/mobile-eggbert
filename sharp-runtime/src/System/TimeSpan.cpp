// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Created by robertvokac on 5/30/25.
//

#include "System/TimeSpan.hpp"

#include <cmath>
#include <cstdio>

#include "System/Int64.hpp"
#include "System/OverflowException.hpp"

namespace System {

    const TimeSpan TimeSpan::Zero = TimeSpan(0);
    const TimeSpan TimeSpan::MaxValue = TimeSpan(SharpRuntime::LONGCS_MAX);

    TimeSpan::TimeSpan() : ticks_internal(0) {
    }

    TimeSpan::TimeSpan(longcs ticks): ticks_internal(ticks) {
    }

    TimeSpan::TimeSpan(const TimeSpan& other) : ticks_internal(other.ticks_internal) {
    }

    TimeSpan::TimeSpan(TimeSpan&& other) noexcept : ticks_internal(other.ticks_internal) {
    }

    TimeSpan &TimeSpan::operator=(const TimeSpan &other) {
        if (this != &other) {  // Prevent self-assignment
            ticks_internal = other.ticks_internal;  // Copy internal data
        }
        return *this;
    }

    TimeSpan& TimeSpan::operator=(TimeSpan&& other) noexcept {
        if (this != &other) {  // Prevent self-assignment
            ticks_internal = other.ticks_internal;  // Transfer ownership
        }
        return *this;
    }

    longcs TimeSpan::getTicksProperty() const {
        return ticks_internal;
    }

    [[nodiscard]] double TimeSpan::getTotalMillisecondsProperty() const {
        double temp = (double) ticks_internal / TicksPerMillisecond;
        if (temp > MaxMilliSeconds)
            return (double) MaxMilliSeconds;

        if (temp < MinMilliSeconds)
            return (double) MinMilliSeconds;

        return temp;
    }

    [[nodiscard]] double TimeSpan::getTotalSecondsProperty() const { return (double) ticks_internal / TicksPerSecond; }

    TimeSpan TimeSpan::Add(const TimeSpan &ts) const {
        if ((ts.ticks_internal > 0 && ticks_internal > std::numeric_limits<int64_t>::max() - ts.ticks_internal) ||
            (ts.ticks_internal < 0 && ticks_internal < std::numeric_limits<int64_t>::min() - ts.ticks_internal)) {
            throw OverflowException("TimeSpan overflowed because the duration is too long.");
        }

        return {ticks_internal + ts.ticks_internal};
    }

    intcs TimeSpan::Compare(const TimeSpan &t1, const TimeSpan &t2) {
        if (t1.ticks_internal > t2.ticks_internal) return 1;
        if (t1.ticks_internal < t2.ticks_internal) return -1;
        return 0;
    }

    intcs TimeSpan::CompareTo(const TimeSpan &value) const {
        return Compare(*this, value);
    }

    bool TimeSpan::Equals(const TimeSpan &obj) const {
        return Equals(*this, obj);
    }

    bool TimeSpan::Equals(const TimeSpan &t1, const TimeSpan &t2) {
        return t1.ticks_internal == t2.ticks_internal;
    }

    TimeSpan TimeSpan::Interval(double value, double scale) {
        if (std::isnan(value)) {
            throw ArgumentException("TimeSpan does not accept floating point Not-a-Number values.");
        }
        return IntervalFromDoubleTicks(value * scale);
    }

    TimeSpan TimeSpan::IntervalFromDoubleTicks(double ticks) {
        if (std::isnan(ticks) || (ticks > static_cast<double>(SharpRuntime::LONGCS_MAX)) || (ticks < static_cast<double>(SharpRuntime::LONGCS_MIN)))
            throw OverflowException("TimeSpan overflowed because the duration is too long.");
        if (ticks == static_cast<double>(Int64::MaxValue))
            return MaxValue;
        return {(longcs) ticks};
    }

    TimeSpan TimeSpan::FromMilliseconds(double value) {
        return Interval(value, TicksPerMillisecond);
    }

    TimeSpan TimeSpan::FromSeconds(double value) {
        return Interval(value, TicksPerSecond);
    }

    TimeSpan TimeSpan::Subtract(const TimeSpan &ts) const {
        const longcs result = ticks_internal - ts.ticks_internal;

        constexpr int signShift = sizeof(longcs) * 8 - 1;

        const bool overflow = ((ticks_internal >> signShift) != (ts.ticks_internal >> signShift)) &&
                              ((ticks_internal >> signShift) != (result >> signShift));

        if (overflow) {
            throw OverflowException("TimeSpan overflowed because the duration is too long.");
        }

        return TimeSpan(result);
    }

    TimeSpan TimeSpan::FromTicks(longcs value) {
        return TimeSpan(value);
    }

    TimeSpan TimeSpan::operator-(const TimeSpan &t2) const { return Subtract(t2); }

    TimeSpan TimeSpan::operator+(const TimeSpan &t2) const { return Add(t2); }

    bool TimeSpan::operator==(const TimeSpan &t2) const { return ticks_internal == t2.ticks_internal; }

    bool TimeSpan::operator!=(const TimeSpan &t2) const { return ticks_internal != t2.ticks_internal; }

    bool TimeSpan::operator>(const TimeSpan &t2) const { return ticks_internal > t2.ticks_internal; }

    bool TimeSpan::operator>=(const TimeSpan &t2) const { return ticks_internal >= t2.ticks_internal; }
}
