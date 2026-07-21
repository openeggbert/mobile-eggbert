// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)

#include "System/DateTimeOffset.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

namespace {

    using namespace System;

    // Returns the system's current local UTC offset in ticks. DST-transition history is
    // not modelled; callers apply this as an approximation for any point in time,
    // consistent with System::TimeZoneInfo's own documented limitations.
    longcs currentLocalOffsetTicks() {
        long offset_secs = 0;
#if defined(_WIN32)
        TIME_ZONE_INFORMATION tz{};
        DWORD r = GetTimeZoneInformation(&tz);
        if (r != TIME_ZONE_ID_INVALID)
            offset_secs = -static_cast<long>(tz.Bias) * 60;
#elif defined(__EMSCRIPTEN__)
        offset_secs = 0;
#else
        std::time_t t = std::time(nullptr);
        struct tm local_tm{};
        localtime_r(&t, &local_tm);
        offset_secs = local_tm.tm_gmtoff;
#endif
        return static_cast<longcs>(offset_secs) * TimeSpan::TicksPerSecond;
    }

    // C++ counterpart of .NET DateTimeOffset's private ValidateOffset/ValidateDate helpers.
    void validateOffsetAndRange(const DateTime& clockDateTime, const TimeSpan& offset) {
        constexpr longcs maxOffsetMinutes = 14 * 60;

        if (offset.getTicksProperty() % TimeSpan::TicksPerMinute != 0) {
            throw ArgumentException("Offset must be specified in whole minutes.", "offset");
        }

        const longcs offsetMinutes = offset.getTicksProperty() / TimeSpan::TicksPerMinute;
        if (offsetMinutes < -maxOffsetMinutes || offsetMinutes > maxOffsetMinutes) {
            throw ArgumentOutOfRangeException("offset", "Offset must be within plus or minus 14 hours.");
        }

        const longcs utcTicks = clockDateTime.getTicksProperty() - offset.getTicksProperty();
        if (utcTicks < 0 || utcTicks > DateTime::MaxTicks) {
            throw ArgumentOutOfRangeException("offset",
                "The UTC time represented when the offset is applied must be between year 0 and 10,000.");
        }
    }

} // namespace

namespace System {

    DateTimeOffset::DateTimeOffset()
        : dateTime_(DateTime()), offset_(TimeSpan::Zero) {}

    DateTimeOffset::DateTimeOffset(const DateTime& dateTime, const TimeSpan& offset)
        : dateTime_(dateTime), offset_(offset) {
        validateOffsetAndRange(dateTime_, offset_);
    }

    DateTimeOffset::DateTimeOffset(const DateTime& dateTime)
        : DateTimeOffset(dateTime, TimeSpan::Zero) {}

    DateTimeOffset::DateTimeOffset(longcs ticks, const TimeSpan& offset)
        : DateTimeOffset(DateTime(ticks), offset) {}

    DateTimeOffset::DateTimeOffset(intcs year, intcs month, intcs day,
                                   intcs hour, intcs minute, intcs second,
                                   const TimeSpan& offset)
        : DateTimeOffset(DateTime(year, month, day, hour, minute, second), offset) {}

    DateTimeOffset::DateTimeOffset(intcs year, intcs month, intcs day,
                                   intcs hour, intcs minute, intcs second, intcs millisecond,
                                   const TimeSpan& offset)
        : DateTimeOffset(DateTime(year, month, day, hour, minute, second, millisecond), offset) {}

    // Build these from self-contained temporaries (constexpr tick constants + freshly-constructed
    // TimeSpan/DateTime) rather than the cross-TU globals DateTime::MinValue / TimeSpan::Zero. Those
    // live in other translation units, so referencing them here is a static-initialization-order
    // fiasco: if this TU's dynamic init runs first, the source globals are still zero-initialized
    // (null vptr), and copying a polymorphic TimeSpan/DateTime out of them is undefined behavior
    // (caught by UBSan). The temporaries below run their own constructors in place, so their vptrs are
    // always valid. The resulting values are identical to the previous definitions.
    const DateTimeOffset DateTimeOffset::MinValue{DateTime(0LL), TimeSpan(0LL)};
    const DateTimeOffset DateTimeOffset::MaxValue{DateTime(DateTime::MaxTicks), TimeSpan(0LL)};
    const DateTimeOffset DateTimeOffset::UnixEpoch{DateTime(DateTime::UnixEpochTicks), TimeSpan(0LL)};

    // -------------------------------------------------------------------------
    // Static factory
    // -------------------------------------------------------------------------

    DateTimeOffset DateTimeOffset::getUtcNowProperty() {
        return DateTimeOffset(DateTime::getNowProperty(), TimeSpan::Zero);
    }

    DateTimeOffset DateTimeOffset::getNowProperty() {
        // system_clock::now() is UTC; get the local offset to compute local DateTime
        DateTime utc = DateTime::getNowProperty();
        TimeSpan off(currentLocalOffsetTicks());
        return DateTimeOffset(utc.Add(off), off);
    }

    // -------------------------------------------------------------------------
    // Accessors
    // -------------------------------------------------------------------------

    const DateTime& DateTimeOffset::getDateTimeProperty() const { return dateTime_; }
    const TimeSpan& DateTimeOffset::getOffsetProperty()   const { return offset_; }

    intcs DateTimeOffset::getTotalOffsetMinutesProperty() const {
        return static_cast<intcs>(offset_.getTicksProperty() / TimeSpan::TicksPerMinute);
    }

    longcs DateTimeOffset::getTicksProperty() const {
        return dateTime_.getTicksProperty();
    }

    longcs DateTimeOffset::getUtcTicksProperty() const {
        return dateTime_.getTicksProperty() - offset_.getTicksProperty();
    }

    intcs DateTimeOffset::getYearProperty()        const { return dateTime_.getYearProperty(); }
    intcs DateTimeOffset::getMonthProperty()       const { return dateTime_.getMonthProperty(); }
    intcs DateTimeOffset::getDayProperty()         const { return dateTime_.getDayProperty(); }
    intcs DateTimeOffset::getHourProperty()        const { return dateTime_.getHourProperty(); }
    intcs DateTimeOffset::getMinuteProperty()      const { return dateTime_.getMinuteProperty(); }
    intcs DateTimeOffset::getSecondProperty()      const { return dateTime_.getSecondProperty(); }
    intcs DateTimeOffset::getMillisecondProperty() const { return dateTime_.getMillisecondProperty(); }

    DayOfWeek DateTimeOffset::getDayOfWeekProperty() const { return dateTime_.getDayOfWeekProperty(); }
    intcs DateTimeOffset::getDayOfYearProperty()     const { return dateTime_.getDayOfYearProperty(); }
    TimeSpan DateTimeOffset::getTimeOfDayProperty()  const { return dateTime_.getTimeOfDayProperty(); }

    DateTime DateTimeOffset::getDateProperty() const {
        return DateTime(dateTime_.getYearProperty(), dateTime_.getMonthProperty(), dateTime_.getDayProperty());
    }

    DateTime DateTimeOffset::getUtcDateTimeProperty() const {
        return DateTime(getUtcTicksProperty());
    }

    DateTime DateTimeOffset::getLocalDateTimeProperty() const {
        return ToLocalTime().getDateTimeProperty();
    }

    DateTimeOffset DateTimeOffset::ToOffset(const TimeSpan& offset) const {
        return DateTimeOffset(getUtcDateTimeProperty().Add(offset), offset);
    }

    // -------------------------------------------------------------------------
    // Arithmetic
    // -------------------------------------------------------------------------

    DateTimeOffset DateTimeOffset::Add(const TimeSpan& ts) const {
        return DateTimeOffset(dateTime_.Add(ts), offset_);
    }

    DateTimeOffset DateTimeOffset::AddDays(double days) const {
        return Add(TimeSpan::FromSeconds(days * 86400.0));
    }

    DateTimeOffset DateTimeOffset::AddHours(double hours) const {
        return Add(TimeSpan::FromHours(hours));
    }

    DateTimeOffset DateTimeOffset::AddMinutes(double minutes) const {
        return Add(TimeSpan::FromMinutes(minutes));
    }

    DateTimeOffset DateTimeOffset::AddSeconds(double seconds) const {
        return Add(TimeSpan::FromSeconds(seconds));
    }

    DateTimeOffset DateTimeOffset::AddMilliseconds(double ms) const {
        return Add(TimeSpan::FromSeconds(ms / 1000.0));
    }

    DateTimeOffset DateTimeOffset::AddMonths(intcs months) const {
        // Real .NET delegates entirely to DateTime.AddMonths: `AddMonths(int months) =>
        // Add(ClockDateTime.AddMonths(months))`. The previous version of this method
        // reimplemented month/year arithmetic from scratch and, in doing so, dropped
        // DateTime::AddMonths's own bounds validation (months must be in [-120000, 120000])
        // -- `getMonthProperty() + months` and `years * 12` (in the old AddYears) are both
        // real signed-integer-overflow UB in C++ for a merely large `months`/`years`
        // argument (confirmed via UBSan) with no upfront guard to catch it first. Delegating
        // to the already-fixed, already-validated DateTime::AddMonths matches real .NET and
        // inherits its overflow safety.
        return DateTimeOffset(dateTime_.AddMonths(months), offset_);
    }

    DateTimeOffset DateTimeOffset::AddYears(intcs years) const {
        // Real .NET: `AddYears(int years) => Add(ClockDateTime.AddYears(years))`.
        return DateTimeOffset(dateTime_.AddYears(years), offset_);
    }

    DateTimeOffset DateTimeOffset::AddTicks(longcs ticks) const {
        return DateTimeOffset(dateTime_.AddTicks(ticks), offset_);
    }

    TimeSpan DateTimeOffset::Subtract(const DateTimeOffset& other) const {
        return TimeSpan(getUtcTicksProperty() - other.getUtcTicksProperty());
    }

    DateTimeOffset DateTimeOffset::Subtract(const TimeSpan& ts) const {
        return DateTimeOffset(dateTime_.Subtract(ts), offset_);
    }

    DateTimeOffset DateTimeOffset::ToUniversalTime() const {
        return DateTimeOffset(getUtcDateTimeProperty(), TimeSpan::Zero);
    }

    DateTimeOffset DateTimeOffset::ToLocalTime() const {
        DateTime utc = getUtcDateTimeProperty();
        TimeSpan off(currentLocalOffsetTicks());
        return DateTimeOffset(utc.Add(off), off);
    }

    // -------------------------------------------------------------------------
    // Unix time conversions
    // -------------------------------------------------------------------------

    DateTimeOffset DateTimeOffset::FromUnixTimeSeconds(longcs seconds) {
        constexpr longcs unixEpochSeconds = DateTime::UnixEpochTicks / DateTime::TicksPerSecond;
        constexpr longcs minSeconds = 0 - unixEpochSeconds;
        constexpr longcs maxSeconds = DateTime::MaxTicks / DateTime::TicksPerSecond - unixEpochSeconds;
        if (seconds < minSeconds || seconds > maxSeconds) {
            throw ArgumentOutOfRangeException("seconds", std::to_string(seconds),
                "Valid values are between " + std::to_string(minSeconds) + " and " +
                std::to_string(maxSeconds) + ", inclusive.");
        }
        const longcs ticks = seconds * DateTime::TicksPerSecond + DateTime::UnixEpochTicks;
        return DateTimeOffset(DateTime(ticks), TimeSpan::Zero);
    }

    DateTimeOffset DateTimeOffset::FromUnixTimeMilliseconds(longcs milliseconds) {
        constexpr longcs unixEpochMilliseconds = DateTime::UnixEpochTicks / DateTime::TicksPerMillisecond;
        constexpr longcs minMilliseconds = 0 - unixEpochMilliseconds;
        constexpr longcs maxMilliseconds = DateTime::MaxTicks / DateTime::TicksPerMillisecond - unixEpochMilliseconds;
        if (milliseconds < minMilliseconds || milliseconds > maxMilliseconds) {
            throw ArgumentOutOfRangeException("milliseconds", std::to_string(milliseconds),
                "Valid values are between " + std::to_string(minMilliseconds) + " and " +
                std::to_string(maxMilliseconds) + ", inclusive.");
        }
        const longcs ticks = milliseconds * DateTime::TicksPerMillisecond + DateTime::UnixEpochTicks;
        return DateTimeOffset(DateTime(ticks), TimeSpan::Zero);
    }

    longcs DateTimeOffset::ToUnixTimeSeconds() const {
        constexpr longcs unixEpochSeconds = DateTime::UnixEpochTicks / DateTime::TicksPerSecond;
        return getUtcTicksProperty() / DateTime::TicksPerSecond - unixEpochSeconds;
    }

    longcs DateTimeOffset::ToUnixTimeMilliseconds() const {
        constexpr longcs unixEpochMilliseconds = DateTime::UnixEpochTicks / DateTime::TicksPerMillisecond;
        return getUtcTicksProperty() / DateTime::TicksPerMillisecond - unixEpochMilliseconds;
    }

    // -------------------------------------------------------------------------
    // Parsing
    // -------------------------------------------------------------------------

    bool DateTimeOffset::TryParse(const std::string& s, DateTimeOffset& result) {
        if (s.size() < 10) return false;

        // Find where the offset starts (after the time part)
        // Look for 'Z' or '+'/'-' that appears after position 10
        std::string dtStr = s;
        TimeSpan offset = TimeSpan::Zero;

        // Check trailing 'Z'
        if (!s.empty() && (s.back() == 'Z' || s.back() == 'z')) {
            dtStr = s.substr(0, s.size() - 1);
            offset = TimeSpan::Zero;
        } else {
            // Look for '+' or '-' after position 10 (skip date separators)
            size_t offPos = std::string::npos;
            for (size_t i = 10; i < s.size(); ++i) {
                if (s[i] == '+' || s[i] == '-') { offPos = i; break; }
            }
            if (offPos != std::string::npos) {
                dtStr = s.substr(0, offPos);
                const std::string offStr = s.substr(offPos);
                bool neg = offStr[0] == '-';
                int hh = 0, mm = 0;
                if (std::sscanf(offStr.c_str() + 1, "%d:%d", &hh, &mm) != 2) return false;
                double secs = (hh * 3600.0 + mm * 60.0) * (neg ? -1.0 : 1.0);
                offset = TimeSpan::FromSeconds(secs);
            }
        }

        DateTime dt;
        if (!DateTime::TryParse(dtStr, dt)) return false;
        // The DateTimeOffset(DateTime, TimeSpan) constructor validates the offset (must be a
        // whole number of minutes, within +-14 hours, and produce a UTC instant within
        // DateTime's representable range) and throws ArgumentException/
        // ArgumentOutOfRangeException on violation -- e.g. TryParse("...+15:00", result) parses
        // a syntactically well-formed but out-of-range (>14h) offset successfully up to this
        // point, then this construction throws. TryParse's entire contract is to never throw,
        // only report failure via its bool return, so that exception must not escape here.
        try {
            result = DateTimeOffset(dt, offset);
        } catch (...) {
            return false;
        }
        return true;
    }

    DateTimeOffset DateTimeOffset::Parse(const std::string& s) {
        DateTimeOffset result;
        if (!TryParse(s, result))
            throw System::FormatException("String was not recognized as a valid DateTimeOffset.");
        return result;
    }

    // -------------------------------------------------------------------------
    // ToString
    // -------------------------------------------------------------------------

    static std::string formatOffset(const TimeSpan& off) {
        int totalMin = static_cast<int>(off.getTotalMinutesProperty());
        char sign = (totalMin < 0) ? '-' : '+';
        if (totalMin < 0) totalMin = -totalMin;
        int hh = totalMin / 60, mm = totalMin % 60;
        char buf[8];
        std::snprintf(buf, sizeof(buf), "%c%02d:%02d", sign, hh, mm);
        return buf;
    }

    std::string DateTimeOffset::ToString() const {
        return dateTime_.ToString() + formatOffset(offset_);
    }

    std::string DateTimeOffset::ToString(const std::string& format) const {
        if (format == "O" || format == "o") {
            // ISO 8601 round-trip: yyyy-MM-ddTHH:mm:ss.fffffffzzz. The offset is preserved
            // (not converted to UTC) since round-tripping must reconstruct the exact original
            // DateTimeOffset, including its specific offset -- matches real .NET.
            // Note: DateTime::ToString's own "f" specifier tops out at millisecond (3-digit)
            // precision in this port, not .NET's full 7-digit (100ns tick) precision -- a
            // pre-existing limitation of DateTime's formatting engine, not introduced here.
            // ".fff" is still added (previously the fractional-seconds component was omitted
            // entirely), since that's a real, confirmed loss of round-trip fidelity for any
            // sub-second component -- millisecond precision is strictly better than none.
            return dateTime_.ToString("yyyy-MM-ddTHH:mm:ss.fff") + formatOffset(offset_);
        }
        if (format == "R" || format == "r") {
            // Verified against DateTimeFormat.cs's TryFormatR: real .NET converts to UTC
            // (subtracting the offset) BEFORE formatting, and ALWAYS appends the literal "GMT"
            // -- RFC 1123 has no offset notation, "GMT" is the only valid trailing token
            // regardless of the original offset. The previous code used the ORIGINAL
            // (offset-relative) clock time directly and only emitted "GMT" when the offset
            // happened to already be zero, appending the raw "+HH:MM"/"-HH:MM" offset
            // otherwise -- producing both the wrong TIME and an invalid RFC1123 string for any
            // non-zero offset.
            return getUtcDateTimeProperty().ToString("ddd, dd MMM yyyy HH:mm:ss") + " GMT";
        }
        if (format == "u") {
            // Verified against DateTimeFormat.cs's TryFormatu: real .NET ALSO converts to UTC
            // first. The previous code appended a literal "Z" (implying UTC) to the ORIGINAL,
            // still-offset-relative clock time without actually converting it -- misleading
            // output for any non-zero offset (a "Z"-suffixed timestamp that wasn't actually UTC).
            return getUtcDateTimeProperty().ToString("yyyy-MM-dd HH:mm:ssZ");
        }
        // General format: delegate to DateTime and append offset
        return dateTime_.ToString(format) + formatOffset(offset_);
    }

    // -------------------------------------------------------------------------
    // Comparison
    // -------------------------------------------------------------------------

    intcs DateTimeOffset::CompareTo(const DateTimeOffset& other) const {
        longcs a = getUtcTicksProperty(), b = other.getUtcTicksProperty();
        return (a < b) ? -1 : (a > b) ? 1 : 0;
    }

    intcs DateTimeOffset::Compare(const DateTimeOffset& first, const DateTimeOffset& second) {
        return first.CompareTo(second);
    }

    bool DateTimeOffset::Equals(const DateTimeOffset& other) const {
        return getUtcTicksProperty() == other.getUtcTicksProperty();
    }

    bool DateTimeOffset::Equals(const DateTimeOffset& first, const DateTimeOffset& second) {
        return first.getUtcTicksProperty() == second.getUtcTicksProperty();
    }

    bool DateTimeOffset::EqualsExact(const DateTimeOffset& other) const {
        return getUtcTicksProperty() == other.getUtcTicksProperty() && offset_ == other.offset_;
    }

    intcs DateTimeOffset::GetHashCode() const {
        const longcs t = getUtcTicksProperty();
        return static_cast<intcs>(t) ^ static_cast<intcs>(t >> 32);
    }

    bool DateTimeOffset::operator==(const DateTimeOffset& other) const { return Equals(other); }
    bool DateTimeOffset::operator!=(const DateTimeOffset& other) const { return !Equals(other); }
    bool DateTimeOffset::operator< (const DateTimeOffset& other) const { return getUtcTicksProperty() <  other.getUtcTicksProperty(); }
    bool DateTimeOffset::operator<=(const DateTimeOffset& other) const { return getUtcTicksProperty() <= other.getUtcTicksProperty(); }
    bool DateTimeOffset::operator> (const DateTimeOffset& other) const { return getUtcTicksProperty() >  other.getUtcTicksProperty(); }
    bool DateTimeOffset::operator>=(const DateTimeOffset& other) const { return getUtcTicksProperty() >= other.getUtcTicksProperty(); }

    // -------------------------------------------------------------------------
    // Operators
    // -------------------------------------------------------------------------

    DateTimeOffset DateTimeOffset::operator+(const TimeSpan& ts) const { return Add(ts); }
    DateTimeOffset DateTimeOffset::operator-(const TimeSpan& ts) const { return Subtract(ts); }
    TimeSpan DateTimeOffset::operator-(const DateTimeOffset& other) const { return Subtract(other); }

    GetTypeNameCPP(DateTimeOffset, "System::DateTimeOffset")

} // namespace System
