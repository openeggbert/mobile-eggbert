// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)

#include "System/DateTime.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace System {

    // -------------------------------------------------------------------------
    // Private helpers
    // -------------------------------------------------------------------------

    // Converts ticks_ to a UTC std::tm via the C library.
    // Uses int64 time_t so pre-1970 dates (negative Unix timestamp) work on
    // 64-bit Linux/MSVC builds.
    std::tm DateTime::toTm() const {
        const longcs unixTicks = ticks_ - UnixEpochTicks;
        // Floor division toward -inf (C++ truncates toward 0, which is wrong
        // for negative values — e.g. pre-1970 dates lose 1 second).
        longcs q = unixTicks / TicksPerSecond;
        const longcs r = unixTicks % TicksPerSecond;
        if (r < 0) --q;
        const time_t unixSec = static_cast<time_t>(q);
        std::tm result{};
#ifdef _WIN32
        gmtime_s(&result, &unixSec);
#else
        gmtime_r(&unixSec, &result);
#endif
        return result;
    }

    // Direct Gregorian → ticks formula (mirrors .NET internals).
    // Days are counted from 0001-01-01 (day 0 in .NET).
    longcs DateTime::dateToTicks(int year, int month, int day,
                                  int hour, int minute, int second, int millisecond)
    {
        if (year < 1 || year > 9999 || month < 1 || month > 12 || day < 1)
            throw System::ArgumentOutOfRangeException("year", "DateTime: date component out of range");

        // Days-to-month tables for non-leap and leap years
        static const int d365[] = {0,31,59,90,120,151,181,212,243,273,304,334,365};
        static const int d366[] = {0,31,60,91,121,152,182,213,244,274,305,335,366};

        const bool leap = (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0);
        const int* d = leap ? d366 : d365;

        if (day > d[month] - d[month - 1])
            throw System::ArgumentOutOfRangeException("day", "DateTime: day out of range for given month");

        const int y = year - 1;
        const longcs days = static_cast<longcs>(y) * 365
                          + y / 4 - y / 100 + y / 400
                          + d[month - 1] + day - 1;

        return days      * TicksPerDay
             + hour      * TicksPerHour
             + minute    * TicksPerMinute
             + second    * TicksPerSecond
             + millisecond * TicksPerMillisecond;
    }

    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------

    DateTime::DateTime()
        : ticks_(0) {}

    DateTime::DateTime(longcs ticks)
        : ticks_(ticks) {
        if (ticks < 0 || ticks > MaxTicks)
            throw System::ArgumentOutOfRangeException("ticks", "Ticks must be between DateTime.MinValue.Ticks and DateTime.MaxValue.Ticks.");
    }

    DateTime::DateTime(int year, int month, int day)
        : ticks_(dateToTicks(year, month, day)) {}

    DateTime::DateTime(int year, int month, int day, int hour, int minute, int second)
        : ticks_(dateToTicks(year, month, day, hour, minute, second)) {}

    DateTime::DateTime(int year, int month, int day,
                       int hour, int minute, int second, int millisecond)
        : ticks_(dateToTicks(year, month, day, hour, minute, second, millisecond)) {}

    // -------------------------------------------------------------------------
    // Properties — tick-level decomposition
    // -------------------------------------------------------------------------

    longcs DateTime::getTicksProperty() const { return ticks_; }

    int DateTime::getYearProperty()        const { return toTm().tm_year + 1900; }
    int DateTime::getMonthProperty()       const { return toTm().tm_mon  + 1;    }
    int DateTime::getDayProperty()         const { return toTm().tm_mday;         }
    int DateTime::getHourProperty()        const { return toTm().tm_hour;         }
    int DateTime::getMinuteProperty()      const { return toTm().tm_min;          }
    int DateTime::getSecondProperty()      const { return toTm().tm_sec;          }
    int DateTime::getMillisecondProperty() const {
        return static_cast<int>((ticks_ % TicksPerSecond) / TicksPerMillisecond);
    }

    DayOfWeek DateTime::getDayOfWeekProperty() const {
        return static_cast<DayOfWeek>(toTm().tm_wday); // 0=Sunday … 6=Saturday
    }

    int DateTime::getDayOfYearProperty() const {
        return toTm().tm_yday + 1; // tm_yday is 0-based, .NET is 1-based
    }

    // -------------------------------------------------------------------------
    // Arithmetic
    // -------------------------------------------------------------------------

    DateTime DateTime::Add(const TimeSpan& value) const {
        return AddTicks(value.getTicksProperty());
    }

    DateTime DateTime::AddDays(int days) const {
        if (days < -MaxDays || days > MaxDays)
            throw System::ArgumentOutOfRangeException("value", "Value to add was out of range.");
        return AddTicks(static_cast<longcs>(days) * TicksPerDay);
    }

    DateTime DateTime::AddHours(int hours) const {
        if (hours < -MaxHours || hours > MaxHours)
            throw System::ArgumentOutOfRangeException("value", "Value to add was out of range.");
        return AddTicks(static_cast<longcs>(hours) * TicksPerHour);
    }

    DateTime DateTime::AddMinutes(int minutes) const {
        // No upfront Max* bound check needed here (unlike AddDays/AddHours): MaxTicks /
        // TicksPerMinute exceeds intcs's representable range, so no int argument can make
        // this multiplication overflow int64.
        return AddTicks(static_cast<longcs>(minutes) * TicksPerMinute);
    }

    DateTime DateTime::AddSeconds(int seconds) const {
        return AddTicks(static_cast<longcs>(seconds) * TicksPerSecond);
    }

    DateTime DateTime::AddMilliseconds(int milliseconds) const {
        return AddTicks(static_cast<longcs>(milliseconds) * TicksPerMillisecond);
    }

    DateTime DateTime::Subtract(const TimeSpan& value) const {
        // Matches real .NET's DateTime.Subtract(TimeSpan): `ulong ticks = (ulong)(Ticks -
        // value._ticks); if (ticks > MaxTicks) throw;`. Unsigned arithmetic wraps on
        // overflow/underflow by well-defined C++ semantics (unlike signed, which would be UB
        // here for a TimeSpan near TimeSpan::MinValue/MaxValue), and the single unsigned
        // comparison catches every invalid case -- wrapped-negative-to-huge or genuinely
        // out-of-range -- the same way the constructor's own range check does.
        const auto diff = static_cast<SharpRuntime::ulongcs>(ticks_) - static_cast<SharpRuntime::ulongcs>(value.getTicksProperty());
        if (diff > static_cast<SharpRuntime::ulongcs>(MaxTicks))
            throw System::ArgumentOutOfRangeException("value", "Value to add was out of range.");
        return DateTime(static_cast<longcs>(diff));
    }

    TimeSpan DateTime::Subtract(const DateTime& value) const {
        return TimeSpan(ticks_ - value.ticks_);
    }

    DateTime DateTime::AddTicks(longcs value) const {
        // Matches real .NET's DateTime.AddTicks: `ulong ticks = (ulong)(Ticks + value); if
        // (ticks > MaxTicks) throw;`. Computing `ticks_ + value` directly in signed int64
        // arithmetic (the previous version of this method) is undefined behavior in C++ when
        // it overflows -- confirmed via UBSan for e.g. ticks_ == MaxTicks, value ==
        // Int64::MaxValue. Unsigned addition wraps by well-defined C++ semantics instead, and
        // the single unsigned comparison against MaxTicks catches every invalid case.
        const auto newTicks = static_cast<SharpRuntime::ulongcs>(ticks_) + static_cast<SharpRuntime::ulongcs>(value);
        if (newTicks > static_cast<SharpRuntime::ulongcs>(MaxTicks))
            throw System::ArgumentOutOfRangeException("value", "Value to add was out of range.");
        return DateTime(static_cast<longcs>(newTicks));
    }

    DateTime DateTime::AddMonths(int months) const {
        if (months < -120000 || months > 120000)
            throw System::ArgumentOutOfRangeException("months", "DateTime: months out of range");

        const int year  = getYearProperty();
        const int month = getMonthProperty();
        const int day   = getDayProperty();

        int m = month + months;
        const int q = (m > 0) ? (m - 1) / 12 : m / 12 - 1;
        const int y = year + q;
        m -= q * 12;

        if (y < 1 || y > 9999)
            throw System::ArgumentOutOfRangeException("months", "DateTime: resulting year out of range");

        const int d = std::min(day, DaysInMonth(y, m));
        const longcs timeOfDay = ticks_ % TicksPerDay;
        return DateTime(dateToTicks(y, m, d) + timeOfDay);
    }

    DateTime DateTime::AddYears(int value) const {
        if (value < -10000 || value > 10000)
            throw System::ArgumentOutOfRangeException("value", "DateTime: years out of range");
        return AddMonths(value * 12);
    }

    bool DateTime::IsLeapYear(int year) {
        if (year < 1 || year > 9999)
            throw System::ArgumentOutOfRangeException("year", "DateTime: year out of range");
        return (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0);
    }

    int DateTime::DaysInMonth(int year, int month) {
        if (month < 1 || month > 12)
            throw System::ArgumentOutOfRangeException("month", "DateTime: month out of range");
        static const int dpm365[] = {31,28,31,30,31,30,31,31,30,31,30,31};
        static const int dpm366[] = {31,29,31,30,31,30,31,31,30,31,30,31};
        return (IsLeapYear(year) ? dpm366 : dpm365)[month - 1];
    }

    // -------------------------------------------------------------------------
    // Static factories
    // -------------------------------------------------------------------------

    const DateTime DateTime::MinValue{0LL};
    const DateTime DateTime::MaxValue{DateTime::MaxTicks};
    const DateTime DateTime::UnixEpoch{DateTime::UnixEpochTicks};

    DateTime DateTime::getNowProperty() {
        using namespace std::chrono;
        const auto now      = system_clock::now();
        const auto duration = now.time_since_epoch();
        const auto secs     = duration_cast<seconds>(duration);
        const auto sub      = duration - secs;
        const longcs ticks  = UnixEpochTicks
            + static_cast<longcs>(secs.count()) * TicksPerSecond
            + static_cast<longcs>(duration_cast<nanoseconds>(sub).count() / 100);
        return DateTime(ticks);
    }

    DateTime DateTime::getTodayProperty() {
        const DateTime now = getNowProperty();
        // Truncate to midnight
        return DateTime(now.ticks_ - (now.ticks_ % TicksPerDay));
    }

    TimeSpan DateTime::getTimeOfDayProperty() const {
        return TimeSpan(ticks_ % TicksPerDay);
    }

    // -------------------------------------------------------------------------
    // String
    // -------------------------------------------------------------------------

    std::string DateTime::ToString() const {
        const std::tm t = toTm();
        char buf[32];
        std::snprintf(buf, sizeof(buf),
            "%04d-%02d-%02d %02d:%02d:%02d",
            t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
            t.tm_hour, t.tm_min, t.tm_sec);
        return buf;
    }

    std::string DateTime::ToString(const std::string& format) const
    {
        std::string result;
        result.reserve(format.size() + 8);
        int yr  = getYearProperty(),   mo  = getMonthProperty(),  dy  = getDayProperty();
        int hr  = getHourProperty(),   mn  = getMinuteProperty(), sc  = getSecondProperty();
        int ms  = getMillisecondProperty();

        auto pad = [](int n, int w) -> std::string {
            std::string s = std::to_string(n);
            while (static_cast<int>(s.size()) < w) s = "0" + s;
            return s;
        };

        size_t i = 0;
        while (i < format.size()) {
            char c = format[i];
            auto run = [&](char ch) {
                size_t k = i + 1;
                while (k < format.size() && format[k] == ch) ++k;
                return static_cast<int>(k - i);
            };
            if (c == 'y') {
                int n = run('y');
                result += (n >= 4) ? pad(yr, 4) : pad(yr % 100, 2);
                i += n;
            } else if (c == 'M') {
                int n = run('M');
                static constexpr const char* abbrevMonths[13] = {
                    "", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
                static constexpr const char* fullMonths[13] = {
                    "", "January", "February", "March", "April", "May", "June",
                    "July", "August", "September", "October", "November", "December"};
                if (n >= 4) result += fullMonths[mo];
                else if (n == 3) result += abbrevMonths[mo];
                else result += (n >= 2) ? pad(mo, 2) : std::to_string(mo);
                i += n;
            } else if (c == 'd') {
                int n = run('d');
                static constexpr const char* abbrevDays[7] = {
                    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
                static constexpr const char* fullDays[7] = {
                    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
                if (n >= 4) result += fullDays[static_cast<int>(getDayOfWeekProperty())];
                else if (n == 3) result += abbrevDays[static_cast<int>(getDayOfWeekProperty())];
                else result += (n >= 2) ? pad(dy, 2) : std::to_string(dy);
                i += n;
            } else if (c == 'H') {
                int n = run('H');
                result += (n >= 2) ? pad(hr, 2) : std::to_string(hr);
                i += n;
            } else if (c == 'h') {
                int n = run('h');
                int h12 = hr % 12; if (h12 == 0) h12 = 12;
                result += (n >= 2) ? pad(h12, 2) : std::to_string(h12);
                i += n;
            } else if (c == 'm') {
                int n = run('m');
                result += (n >= 2) ? pad(mn, 2) : std::to_string(mn);
                i += n;
            } else if (c == 's') {
                int n = run('s');
                result += (n >= 2) ? pad(sc, 2) : std::to_string(sc);
                i += n;
            } else if (c == 'f') {
                int n = run('f');
                std::string mss = pad(ms, 3);
                result += mss.substr(0, static_cast<size_t>(std::min(n, 3)));
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

    bool DateTime::TryParse(const std::string& s, DateTime& result)
    {
        int yr = 0, mo = 0, dy = 0, hr = 0, mn = 0, sc = 0, ms = 0;
        // Require at least "yyyy-MM-dd" (10 chars with dashes at [4] and [7])
        if (s.size() < 10 || s[4] != '-' || s[7] != '-') return false;
        if (std::sscanf(s.c_str(), "%d-%d-%d", &yr, &mo, &dy) != 3) return false;
        if (s.size() >= 19 && (s[10] == ' ' || s[10] == 'T')) {
            if (std::sscanf(s.c_str() + 11, "%d:%d:%d", &hr, &mn, &sc) != 3)
                hr = mn = sc = 0;
            if (s.size() > 20 && s[19] == '.') {
                // Note: the gate is "at least 1 char after the dot", not "at least 3" -- a
                // 1-2 digit fraction (e.g. ".5" or ".5Z") is valid and must be scaled up to
                // milliseconds below, not silently skipped because the string happens to be
                // short.
                std::sscanf(s.c_str() + 20, "%d", &ms);
                // Count only the actual fractional-digit characters -- a trailing
                // 'Z'/'+hh:mm'/'-hh:mm' timezone marker (common on ISO-8601/wire-format
                // timestamps such as "...ss.123Z") is not part of the fraction and must not
                // be counted as if it were extra digits, or the millisecond normalisation
                // below divides by the wrong power of ten (e.g. ".123Z" would previously be
                // read as 4 fractional digits and truncated from 123ms to 12ms).
                size_t fracLen = 0;
                while (20 + fracLen < s.size() && s[20 + fracLen] >= '0' && s[20 + fracLen] <= '9')
                    ++fracLen;
                while (fracLen < 3) { ms *= 10; ++fracLen; }
                if (fracLen > 3) { for (size_t k = 3; k < fracLen; ++k) ms /= 10; }
            }
        }
        try {
            result = DateTime(yr, mo, dy, hr, mn, sc, ms);
            return true;
        } catch (...) { return false; }
    }

    DateTime DateTime::Parse(const std::string& s)
    {
        DateTime result;
        if (!TryParse(s, result))
            throw FormatException("String was not recognized as a valid DateTime: " + s);
        return result;
    }

    // -------------------------------------------------------------------------
    // Comparison operators
    // -------------------------------------------------------------------------

    bool DateTime::operator==(const DateTime& o) const { return ticks_ == o.ticks_; }
    bool DateTime::operator!=(const DateTime& o) const { return ticks_ != o.ticks_; }
    bool DateTime::operator< (const DateTime& o) const { return ticks_ <  o.ticks_; }
    bool DateTime::operator<=(const DateTime& o) const { return ticks_ <= o.ticks_; }
    bool DateTime::operator> (const DateTime& o) const { return ticks_ >  o.ticks_; }
    bool DateTime::operator>=(const DateTime& o) const { return ticks_ >= o.ticks_; }

    GetTypeNameCPP(DateTime, "System::DateTime")

} // namespace System
