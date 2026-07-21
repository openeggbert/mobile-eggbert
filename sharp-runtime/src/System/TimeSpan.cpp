// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Created by robertvokac on 5/30/25.
//

#include "System/TimeSpan.hpp"

#include <cstdio>
#include <iomanip>

#include "System/FormatException.hpp"
#include "System/Int64.hpp"
#include "System/OverflowException.hpp"

namespace System {

    std::atomic<int> TimeSpan::copy_count{0};
    std::atomic<int> TimeSpan::move_count{0};

    int TimeSpan::getCopyCount() { return copy_count.load(std::memory_order_relaxed); }
    int TimeSpan::getMoveCount() { return move_count.load(std::memory_order_relaxed); }

    void TimeSpan::resetCopyCount() {
        copy_count.store(0, std::memory_order_relaxed);
    }

    void TimeSpan::resetMoveCount() {
        move_count.store(0, std::memory_order_relaxed);
    }

    const TimeSpan TimeSpan::Zero = TimeSpan(0);
    const TimeSpan TimeSpan::MaxValue = TimeSpan(SharpRuntime::LONGCS_MAX);

    const TimeSpan TimeSpan::MinValue = TimeSpan(SharpRuntime::LONGCS_MIN);


    TimeSpan::TimeSpan() : ticks_internal(0) {
    }

    TimeSpan::TimeSpan(longcs ticks): ticks_internal(ticks) {
    }

    TimeSpan::TimeSpan(intcs hours, intcs minutes, intcs seconds): ticks_internal(
        TimeToTicks(hours, minutes, seconds)) {
    }

    TimeSpan::TimeSpan(intcs days, intcs hours, intcs minutes, intcs seconds, intcs milliseconds,
                       intcs microseconds): ticks_internal(
        TimeToTicks(days, hours, minutes, seconds, milliseconds, microseconds) * TicksPerMicrosecond) {
    }

    TimeSpan::TimeSpan(const TimeSpan& other) : ticks_internal(other.ticks_internal) {
        copy_count.fetch_add(1, std::memory_order_relaxed);
    }

    TimeSpan::TimeSpan(TimeSpan&& other) noexcept : ticks_internal(other.ticks_internal) {
        move_count.fetch_add(1, std::memory_order_relaxed);
    }

    longcs TimeSpan::TimeToTicks(intcs days, intcs hours, intcs minutes, intcs seconds, intcs milliseconds,
                                 intcs microseconds) {
        // Computed in uint64_t (defined wraparound on overflow), not signed int64_t like the
        // rest of this file: verified with a standalone UBSan repro that the previous
        // signed-arithmetic version invoked real signed-integer-overflow UB for extreme
        // component values (e.g. days == INT32_MAX) -- the microsecond scale factor here is
        // 1000x finer than TimeToTicks(hour,minute,second)'s seconds granularity below, which
        // .NET's own source comments confirm can't overflow int64; this one can. Real .NET's
        // `long` arithmetic here is unchecked and wraps silently (defined in C#), so computing
        // in unsigned and converting back to signed reproduces that exact behavior instead of
        // C++ signed overflow's undefined one -- static_cast from uint64_t to a same-width
        // signed type for an out-of-range value is well-defined (2's-complement wraparound) as
        // of C++20, which this project already requires.
        std::uint64_t totalMicroseconds =
            ((static_cast<std::uint64_t>(days) * 86400ull
            + static_cast<std::uint64_t>(hours) * 3600ull
            + static_cast<std::uint64_t>(minutes) * 60ull
            + static_cast<std::uint64_t>(seconds))
            * 1000ull + static_cast<std::uint64_t>(milliseconds)) * 1000ull
            + static_cast<std::uint64_t>(microseconds);
        longcs result = static_cast<longcs>(totalMicroseconds);
        if (result > MaxMicroSeconds || result < MinMicroSeconds)
            throw ArgumentOutOfRangeException("", "TimeSpan overflowed because the duration is too long.");
        return result;
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
            move_count.fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
    }

    longcs TimeSpan::getTicksProperty() const {
        return ticks_internal;
    }


    [[nodiscard]] intcs TimeSpan::getDaysProperty() const { return (intcs) (ticks_internal / TicksPerDay); }


    [[nodiscard]] intcs TimeSpan::getHoursProperty() const { return (intcs) ((ticks_internal / TicksPerHour) % 24); }


    [[nodiscard]] intcs TimeSpan::getMillisecondsProperty() const {
        return (intcs) ((ticks_internal / TicksPerMillisecond) % 1000);
    }


    [[nodiscard]] intcs TimeSpan::getMicrosecondsProperty() const {
        return (intcs) ((ticks_internal / TicksPerMicrosecond) % 1000);
    }

    /**
     * @brief Retrieves the nanosecond portion of the time span.
     *
     * Provides access to the nanosecond component of the current TimeSpan instance.
     *
     * @return The number of whole nanoseconds in the time span.
     *
     * @details The `Nanoseconds` property returns only full nanoseconds, while
     * `TotalNanoseconds` provides both complete and fractional values.
     */


    [[nodiscard]] intcs TimeSpan::getNanosecondsProperty() const {
        return (intcs) ((ticks_internal % TicksPerMicrosecond) * 100);
    }


    [[nodiscard]] intcs TimeSpan::getMinutesProperty() const {
        return (intcs) ((ticks_internal / TicksPerMinute) % 60);
    }


    [[nodiscard]] intcs TimeSpan::getSecondsProperty() const {
        return (intcs) ((ticks_internal / TicksPerSecond) % 60);
    }


    [[nodiscard]] double TimeSpan::getTotalDaysProperty() const { return ((double) ticks_internal) / TicksPerDay; }


    [[nodiscard]] double TimeSpan::getTotalHoursProperty() const { return (double) ticks_internal / TicksPerHour; }


    [[nodiscard]] double TimeSpan::getTotalMillisecondsProperty() const {
        double temp = (double) ticks_internal / TicksPerMillisecond;
        if (temp > MaxMilliSeconds)
            return (double) MaxMilliSeconds;

        if (temp < MinMilliSeconds)
            return (double) MinMilliSeconds;

        return temp;
    }

    [[nodiscard]] double TimeSpan::getTotalMicrosecondsProperty() const {
        return (double) ticks_internal / TicksPerMicrosecond;
    }


    [[nodiscard]] double TimeSpan::getTotalNanosecondsProperty() const {
        return (double) ticks_internal * NanosecondsPerTick;
    }

    [[nodiscard]] double TimeSpan::getTotalMinutesProperty() const { return (double) ticks_internal / TicksPerMinute; }

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

    TimeSpan TimeSpan::FromDays(double value) {
        return Interval(value, TicksPerDay);
    }

    TimeSpan TimeSpan::Duration() const {
        if (getTicksProperty() == MinValue.getTicksProperty())
            throw OverflowException("The duration cannot be returned for TimeSpan.MinValue because the absolute value of TimeSpan.MinValue exceeds the value of TimeSpan.MaxValue.");
        return {ticks_internal >= 0 ? ticks_internal : -ticks_internal};
    }

    bool TimeSpan::Equals(const TimeSpan &obj) const {
        return Equals(*this, obj);
    }

    bool TimeSpan::Equals(const TimeSpan &t1, const TimeSpan &t2) {
        return t1.ticks_internal == t2.ticks_internal;
    }

    intcs TimeSpan::GetHashCode() const noexcept {
        return static_cast<intcs>(ticks_internal) ^ static_cast<intcs>(static_cast<uint64_t>(ticks_internal) >> 32);
    }

    TimeSpan TimeSpan::FromHours(double value) {
        return Interval(value, TicksPerHour);
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

    TimeSpan TimeSpan::FromMicroseconds(double value) {
        return Interval(value, TicksPerMicrosecond);
    }

    TimeSpan TimeSpan::FromMinutes(double value) {
        return Interval(value, TicksPerMinute);
    }

    TimeSpan TimeSpan::Negate() const {
        if (getTicksProperty() == MinValue.getTicksProperty())
            throw OverflowException("Negating the minimum value of a twos complement number is invalid.");
        return TimeSpan(-ticks_internal);
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

    TimeSpan TimeSpan::Multiply(double factor) const { return (*this) * factor; }

    TimeSpan TimeSpan::Divide(double divisor) const { return (*this) / divisor; }

    double TimeSpan::Divide(const TimeSpan &ts) const { return (*this) / ts; }

    TimeSpan TimeSpan::FromTicks(longcs value) {
        return TimeSpan(value);
    }

    longcs TimeSpan::TimeToTicks(intcs hour, intcs minute, intcs second) {
        longcs totalSeconds = (longcs) hour * 3600 + (longcs) minute * 60 + (longcs) second;
        if (totalSeconds > MaxSeconds || totalSeconds < MinSeconds)
            throw ArgumentOutOfRangeException("", "TimeSpan overflowed because the duration is too long.");
        return totalSeconds * TicksPerSecond;
    }

    std::string TimeSpan::ToString() const {
        longcs ticks = ticks_internal;
        bool negative = ticks < 0;

        std::uint64_t magnitude;
        if (negative) {
            magnitude = static_cast<std::uint64_t>(-(ticks + 1)) + 1;
        } else {
            magnitude = static_cast<std::uint64_t>(ticks);
        }

        const std::uint64_t days = magnitude / static_cast<std::uint64_t>(TicksPerDay);
        magnitude %= static_cast<std::uint64_t>(TicksPerDay);

        const std::uint64_t hours = magnitude / static_cast<std::uint64_t>(TicksPerHour);
        magnitude %= static_cast<std::uint64_t>(TicksPerHour);

        const std::uint64_t minutes = magnitude / static_cast<std::uint64_t>(TicksPerMinute);
        magnitude %= static_cast<std::uint64_t>(TicksPerMinute);

        const std::uint64_t seconds = magnitude / static_cast<std::uint64_t>(TicksPerSecond);
        magnitude %= static_cast<std::uint64_t>(TicksPerSecond);

        std::ostringstream oss;
        oss.imbue(std::locale::classic());
        if (negative) {
            oss << '-';
        }
        if (days != 0) {
            oss << days << '.'
                << std::setw(2) << std::setfill('0') << hours;
        } else {
            oss << hours;
        }
        oss << ":"
            << std::setw(2) << std::setfill('0') << minutes << ":"
            << std::setw(2) << std::setfill('0') << seconds << "."
            << std::setw(7) << std::setfill('0') << magnitude;

        return oss.str();
    }


    std::string TimeSpan::ToString(const std::string& format) const {
        longcs ticks = ticks_internal;
        bool negative = ticks < 0;
        std::uint64_t magnitude;
        if (negative) magnitude = static_cast<std::uint64_t>(-(ticks + 1)) + 1;
        else           magnitude = static_cast<std::uint64_t>(ticks);

        const auto days    = static_cast<long long>(magnitude / static_cast<std::uint64_t>(TicksPerDay));
        magnitude %= static_cast<std::uint64_t>(TicksPerDay);
        const int hours   = static_cast<int>(magnitude / static_cast<std::uint64_t>(TicksPerHour));
        magnitude %= static_cast<std::uint64_t>(TicksPerHour);
        const int minutes = static_cast<int>(magnitude / static_cast<std::uint64_t>(TicksPerMinute));
        magnitude %= static_cast<std::uint64_t>(TicksPerMinute);
        const int seconds = static_cast<int>(magnitude / static_cast<std::uint64_t>(TicksPerSecond));
        magnitude %= static_cast<std::uint64_t>(TicksPerSecond);
        // magnitude is now sub-second ticks (0–9 999 999)

        auto pad = [](long long n, int w) -> std::string {
            std::string s = std::to_string(n < 0 ? -n : n);
            while (static_cast<int>(s.size()) < w) s = "0" + s;
            return s;
        };

        std::string result;
        if (negative) result += '-';
        size_t i = 0;
        while (i < format.size()) {
            char c = format[i];
            auto run = [&](char ch) {
                size_t k = i + 1;
                while (k < format.size() && format[k] == ch) ++k;
                return static_cast<int>(k - i);
            };
            if (c == 'd') {
                int n = run('d');
                result += (n >= 2) ? pad(days, n) : std::to_string(days < 0 ? -days : days);
                i += n;
            } else if (c == 'h') {
                int n = run('h');
                result += (n >= 2) ? pad(hours, 2) : std::to_string(hours);
                i += n;
            } else if (c == 'm') {
                int n = run('m');
                result += (n >= 2) ? pad(minutes, 2) : std::to_string(minutes);
                i += n;
            } else if (c == 's') {
                int n = run('s');
                result += (n >= 2) ? pad(seconds, 2) : std::to_string(seconds);
                i += n;
            } else if (c == 'f') {
                int n = run('f');
                std::string frac = pad(static_cast<long long>(magnitude), 7);
                result += frac.substr(0, static_cast<size_t>(std::min(n, 7)));
                i += n;
            } else if (c == '\'') {
                ++i;
                while (i < format.size() && format[i] != '\'') result += format[i++];
                if (i < format.size()) ++i;
            } else {
                result += c;
                ++i;
            }
        }
        return result;
    }

    bool TimeSpan::TryParse(const std::string& s, TimeSpan& result) {
        if (s.empty()) return false;
        const char* p = s.c_str();
        bool negative = (*p == '-');
        if (negative) ++p;

        // Determine if there's a days component: look for '.' before the first ':'
        const char* pScan = p;
        bool hasDot = false;
        while (*pScan && *pScan != ':') {
            if (*pScan == '.') { hasDot = true; break; }
            ++pScan;
        }

        int days = 0, hours = 0, minutes = 0, seconds = 0;
        int matched = 0;
        if (hasDot) {
            matched = std::sscanf(p, "%d.%d:%d:%d", &days, &hours, &minutes, &seconds);
            if (matched != 4) return false;
        } else {
            matched = std::sscanf(p, "%d:%d:%d", &hours, &minutes, &seconds);
            if (matched != 3) return false;
        }
        if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59 || seconds < 0 || seconds > 59)
            return false;

        // Find position after the seconds digits to look for fractional part
        const char* afterSec = p;
        int colons = 0;
        while (*afterSec && colons < 2) { if (*afterSec == ':') ++colons; ++afterSec; }
        while (*afterSec >= '0' && *afterSec <= '9') ++afterSec;

        long long subsecTicks = 0;
        if (*afterSec == '.') {
            ++afterSec;
            int fracDigits = 0;
            long long fracVal = 0;
            while (*afterSec >= '0' && *afterSec <= '9' && fracDigits < 7) {
                fracVal = fracVal * 10 + (*afterSec - '0');
                ++fracDigits; ++afterSec;
            }
            while (fracDigits < 7) { fracVal *= 10; ++fracDigits; }
            subsecTicks = fracVal;
        }

        // sscanf() above only checks that a matching prefix exists, not that the whole input
        // was consumed -- e.g. "12:34:56garbage" previously parsed successfully as 12:34:56,
        // silently discarding "garbage" instead of being rejected like real .NET's
        // TimeSpan.Parse (a FormatException) would. Reject any unconsumed trailing content.
        if (*afterSec != '\0') return false;

        longcs ticks = static_cast<longcs>(days)    * TicksPerDay
                     + static_cast<longcs>(hours)   * TicksPerHour
                     + static_cast<longcs>(minutes) * TicksPerMinute
                     + static_cast<longcs>(seconds) * TicksPerSecond
                     + subsecTicks;
        if (negative) ticks = -ticks;
        result = TimeSpan(ticks);
        return true;
    }

    TimeSpan TimeSpan::Parse(const std::string& s) {
        TimeSpan result;
        if (!TryParse(s, result))
            throw FormatException("String was not recognized as a valid TimeSpan: " + s);
        return result;
    }

    TimeSpan TimeSpan::operator-() const {
        if (ticks_internal == MinValue.ticks_internal)
            throw OverflowException("Negating the minimum value of a twos complement number is invalid.");
        return TimeSpan(-ticks_internal);
    }

    TimeSpan TimeSpan::operator-(const TimeSpan &t2) const { return Subtract(t2); }

    TimeSpan TimeSpan::operator+() const { return *this; }

    TimeSpan TimeSpan::operator+(const TimeSpan &t2) const { return Add(t2); }


    TimeSpan TimeSpan::operator*(double factor) const {
        if (std::isnan(factor)) {
            throw ArgumentException("TimeSpan does not accept floating point Not-a-Number values.", "factor");
        }
        const double ticks = std::round(getTicksProperty() * factor);
        return IntervalFromDoubleTicks(ticks);
    }

    TimeSpan TimeSpan::operator/(double divisor) const {
        if (std::isnan(divisor)) {
            throw ArgumentException("TimeSpan does not accept floating point Not-a-Number values.", "divisor");
        }
        const double ticks = std::round(getTicksProperty() / divisor);
        return IntervalFromDoubleTicks(ticks);
    }

    // Performing division using floating-point arithmetic allows the result to be infinity,
    // which makes sense when dividing a non-zero TimeSpan by TimeSpan.Zero.
    // For example, TimeSpan.FromHours(1) / TimeSpan.Zero asks how many zero-length intervals fit into one hour,
    // and mathematically, the answer is infinity.
    // Dividing TimeSpan.Zero by TimeSpan.Zero returns NaN, which may be less practical,
    // but it is no less valid than throwing an exception.


    double TimeSpan::operator/(const TimeSpan &t2) const { return getTicksProperty() / (double) t2.getTicksProperty(); }


    bool TimeSpan::operator==(const TimeSpan &t2) const { return ticks_internal == t2.ticks_internal; }


    bool TimeSpan::operator!=(const TimeSpan &t2) const { return ticks_internal != t2.ticks_internal; }


    bool TimeSpan::operator<(const TimeSpan &t2) const { return ticks_internal < t2.ticks_internal; }


    bool TimeSpan::operator<=(const TimeSpan &t2) const { return ticks_internal <= t2.ticks_internal; }


    bool TimeSpan::operator>(const TimeSpan &t2) const { return ticks_internal > t2.ticks_internal; }


    bool TimeSpan::operator>=(const TimeSpan &t2) const { return ticks_internal >= t2.ticks_internal; }


    TimeSpan operator*(double factor, const TimeSpan &timeSpan) { return timeSpan * factor; }
}