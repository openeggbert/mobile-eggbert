// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "System/Object.hpp"
#include "System/TimeSpan.hpp"
#include "System/DayOfWeek.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    using SharpRuntime::longcs;
    using SharpRuntime::intcs;

    /**
     * @brief Represents an instant in time, expressed as the number of 100-nanosecond
     * ticks since the .NET epoch (0001-01-01 00:00:00).
     *
     * Partial C++ counterpart of .NET System.DateTime.
     *
     * @note Status: Partial — DateTimeKind is not stored/tracked (all values behave as
     *   Unspecified/UTC); ToLocalTime/ToUniversalTime/SpecifyKind are therefore not
     *   provided. OLE Automation date, FILETIME, and binary-serialization conversions
     *   (ToOADate/FromOADate, ToFileTime/FromFileTime, ToBinary/FromBinary) are out of
     *   scope. Culture-aware ToString/Parse overloads (IFormatProvider) are not provided;
     *   ToString(format)/Parse/TryParse use invariant numeric tokens only.
     */
    class DateTime : public Object {
    public:
        static constexpr longcs TicksPerMillisecond = 10000LL;
        static constexpr longcs TicksPerSecond      = 10000000LL;
        static constexpr longcs TicksPerMinute      = 600000000LL;
        static constexpr longcs TicksPerHour        = 36000000000LL;
        static constexpr longcs TicksPerDay         = 864000000000LL;
        /** @brief Ticks from the .NET epoch (0001-01-01) to the Unix epoch (1970-01-01). */
        static constexpr longcs UnixEpochTicks      = 621355968000000000LL;
        /** @brief The maximum tick value representable (9999-12-31 23:59:59.9999999). */
        static constexpr longcs MaxTicks            = 3155378975999999999LL;

    private:
        // Matches real .NET's private MaxDays/MaxHours constants (DateTime.cs: `MaxTicks /
        // TimeSpan.TicksPerDay` etc.) -- AddDays/AddHours must reject a unit count whose
        // product with TicksPer{Day,Hour} would itself overflow int64 *before* multiplying,
        // since `static_cast<longcs>(days) * TicksPerDay` for a merely large (not even
        // extreme) `int` day count is real signed-overflow UB in C++ otherwise (confirmed via
        // UBSan: 1e9 days or 1e9 hours both overflow). AddMinutes/AddSeconds/AddMilliseconds
        // don't need an equivalent guard: their own MaxMinutes/MaxSeconds/MaxMilliseconds
        // bounds exceed intcs's range, so no representable int argument can reach the
        // multiplication-overflow threshold for those units.
        static constexpr longcs MaxDays  = MaxTicks / TicksPerDay;
        static constexpr longcs MaxHours = MaxTicks / TicksPerHour;

        longcs ticks_;

        /**
         * @brief Decomposes ticks_ into a UTC std::tm using the C standard library.
         *
         * Uses floor division so that pre-1970 (negative Unix-timestamp) dates are
         * decomposed correctly.
         */
        [[nodiscard]] std::tm toTm() const;

        /**
         * @brief Converts a calendar date and time to a tick count measured from the
         * .NET epoch (0001-01-01 00:00:00).
         *
         * @param year         Year (1–9999).
         * @param month        Month (1–12).
         * @param day          Day of month (1–31, validated against the given month).
         * @param hour         Hour (0–23). Default 0.
         * @param minute       Minute (0–59). Default 0.
         * @param second       Second (0–59). Default 0.
         * @param millisecond  Millisecond (0–999). Default 0.
         * @throws std::out_of_range if any component is out of the valid range.
         */
        static longcs dateToTicks(int year, int month, int day,
                                   int hour = 0, int minute = 0,
                                   int second = 0, int millisecond = 0);

    public:
        /**
         * @brief Initializes a new instance with zero ticks (0001-01-01 00:00:00).
         */
        DateTime();

        /**
         * @brief Initializes a new instance with the specified number of ticks.
         *
         * @param ticks A date and time expressed in 100-nanosecond ticks since
         *              the .NET epoch (0001-01-01 00:00:00).
         * @throws System::ArgumentOutOfRangeException if @p ticks is negative or greater than MaxTicks.
         */
        explicit DateTime(longcs ticks);

        /**
         * @brief Initializes a new instance with the specified year, month, and day.
         *
         * @param year   Year (1–9999).
         * @param month  Month (1–12).
         * @param day    Day of month (1–max for the given month/year).
         * @throws std::out_of_range if any component is out of range.
         */
        DateTime(intcs year, intcs month, intcs day);

        /**
         * @brief Initializes a new instance with date and time components.
         *
         * @param year    Year (1–9999).
         * @param month   Month (1–12).
         * @param day     Day of month.
         * @param hour    Hour (0–23).
         * @param minute  Minute (0–59).
         * @param second  Second (0–59).
         * @throws std::out_of_range if any component is out of range.
         */
        DateTime(intcs year, intcs month, intcs day, intcs hour, intcs minute, intcs second);

        /**
         * @brief Initializes a new instance with date, time, and millisecond components.
         *
         * @param year         Year (1–9999).
         * @param month        Month (1–12).
         * @param day          Day of month.
         * @param hour         Hour (0–23).
         * @param minute       Minute (0–59).
         * @param second       Second (0–59).
         * @param millisecond  Millisecond (0–999).
         * @throws std::out_of_range if any component is out of range.
         */
        DateTime(intcs year, intcs month, intcs day,
                 intcs hour, intcs minute, intcs second, intcs millisecond);

        /** @brief The smallest possible value of DateTime (0001-01-01 00:00:00). */
        static const DateTime MinValue;
        /** @brief The largest possible value of DateTime (9999-12-31 23:59:59.9999999). */
        static const DateTime MaxValue;
        /** @brief Represents the point in time when Unix time begins (1970-01-01 00:00:00 UTC). */
        static const DateTime UnixEpoch;

        /**
         * @brief Gets the number of 100-nanosecond ticks since the .NET epoch.
         *
         * @return Tick count (0 = 0001-01-01 00:00:00).
         */
        [[nodiscard]] longcs getTicksProperty() const;

        /**
         * @brief Gets the year component of this instance (1–9999).
         */
        [[nodiscard]] intcs getYearProperty() const;

        /**
         * @brief Gets the month component of this instance (1–12).
         */
        [[nodiscard]] intcs getMonthProperty() const;

        /**
         * @brief Gets the day-of-month component of this instance (1–31).
         */
        [[nodiscard]] intcs getDayProperty() const;

        /**
         * @brief Gets the hour component of this instance (0–23).
         */
        [[nodiscard]] intcs getHourProperty() const;

        /**
         * @brief Gets the minute component of this instance (0–59).
         */
        [[nodiscard]] intcs getMinuteProperty() const;

        /**
         * @brief Gets the second component of this instance (0–59).
         */
        [[nodiscard]] intcs getSecondProperty() const;

        /**
         * @brief Gets the millisecond component of this instance (0–999).
         */
        [[nodiscard]] intcs getMillisecondProperty() const;

        /**
         * @brief Gets the day of the week represented by this instance.
         *
         * @return DayOfWeek enumeration value (Sunday = 0, …, Saturday = 6).
         */
        [[nodiscard]] DayOfWeek getDayOfWeekProperty() const;

        /**
         * @brief Gets the day of the year represented by this instance (1–366).
         */
        [[nodiscard]] intcs getDayOfYearProperty() const;

        /**
         * @brief Adds the specified time span to this instance.
         *
         * @param value Time span to add.
         * @return A new DateTime that is the sum of this instance and @p value.
         */
        [[nodiscard]] DateTime Add(const TimeSpan& value) const;

        /**
         * @brief Returns a new DateTime with the specified number of whole days added.
         *
         * @param days Number of days to add (may be negative).
         * @return A new DateTime.
         */
        [[nodiscard]] DateTime AddDays(intcs days) const;

        /**
         * @brief Returns a new DateTime with the specified number of hours added.
         *
         * @param hours Number of hours to add (may be negative).
         * @return A new DateTime.
         */
        [[nodiscard]] DateTime AddHours(intcs hours) const;

        /**
         * @brief Returns a new DateTime with the specified number of minutes added.
         *
         * @param minutes Number of minutes to add (may be negative).
         * @return A new DateTime.
         */
        [[nodiscard]] DateTime AddMinutes(intcs minutes) const;

        /**
         * @brief Returns a new DateTime with the specified number of seconds added.
         *
         * @param seconds Number of seconds to add (may be negative).
         * @return A new DateTime.
         */
        [[nodiscard]] DateTime AddSeconds(intcs seconds) const;

        /**
         * @brief Returns a new DateTime with the specified number of milliseconds added.
         *
         * @param milliseconds Number of milliseconds to add (may be negative).
         * @return A new DateTime.
         */
        [[nodiscard]] DateTime AddMilliseconds(intcs milliseconds) const;

        /**
         * @brief Returns a new DateTime with the specified number of 100-nanosecond ticks added.
         *
         * @param value Number of ticks to add (may be negative).
         * @return A new DateTime.
         * @throws std::out_of_range if the result is outside the representable range.
         */
        [[nodiscard]] DateTime AddTicks(longcs value) const;

        /**
         * @brief Returns a new DateTime with the specified number of months added.
         *
         * If the resulting day would be invalid for the resulting month, the day is
         * clamped to the last valid day of that month (e.g. Jan 31 + 1 month = Feb 28/29).
         *
         * @param months Number of months to add (may be negative; must be in [-120000, 120000]).
         * @return A new DateTime.
         * @throws std::out_of_range if @p months is out of range or the resulting year is outside [1, 9999].
         */
        [[nodiscard]] DateTime AddMonths(intcs months) const;

        /**
         * @brief Returns a new DateTime with the specified number of years added.
         *
         * If the current date is February 29 and the resulting year is not a leap year,
         * the result is clamped to February 28.
         *
         * @param value Number of years to add (may be negative; must be in [-10000, 10000]).
         * @return A new DateTime.
         * @throws std::out_of_range if @p value is out of range or the resulting year is outside [1, 9999].
         */
        [[nodiscard]] DateTime AddYears(intcs value) const;

        /**
         * @brief Subtracts the specified time span from this instance.
         *
         * @param value Time span to subtract.
         * @return A new DateTime that is this instance minus @p value.
         */
        [[nodiscard]] DateTime Subtract(const TimeSpan& value) const;

        /**
         * @brief Subtracts another DateTime from this instance.
         *
         * @param value The DateTime to subtract.
         * @return A TimeSpan representing the interval between the two values.
         */
        [[nodiscard]] TimeSpan Subtract(const DateTime& value) const;

        /**
         * @brief Gets the current local date and time.
         *
         * @return Current local DateTime expressed in .NET-compatible ticks.
         * @note DateTimeKind is not stored; the value reflects UTC-based system time.
         */
        [[nodiscard]] static DateTime getNowProperty();

        /**
         * @brief Gets the current date with the time component set to midnight (00:00:00).
         *
         * @return Today's date at 00:00:00.
         */
        [[nodiscard]] static DateTime getTodayProperty();

        /**
         * @brief Gets the time-of-day component of this instance as a TimeSpan.
         *
         * @return A TimeSpan representing the time elapsed since midnight.
         */
        [[nodiscard]] TimeSpan getTimeOfDayProperty() const;

        /**
         * @brief Returns an ISO-8601-style string representation: "YYYY-MM-DD HH:MM:SS".
         *
         * @return Formatted date/time string.
         */
        [[nodiscard]] std::string ToString() const override;

        /**
         * @brief Returns the date/time formatted according to @p format.
         *
         * C++ counterpart of .NET DateTime.ToString(string).
         * Tokens: yyyy, yy, MMMM, MMM, MM, M, dddd, ddd, dd, d, HH, H, hh, h, mm, m, ss, s, fff,
         * ff, f. ddd/dddd and MMM/MMMM use fixed invariant-culture English names (no locale
         * support). Literal text can be enclosed in single quotes.
         * @param format The format string.
         */
        [[nodiscard]] std::string ToString(const std::string& format) const;

        /**
         * @brief Parses a date/time string in ISO-8601 style.
         *
         * C++ counterpart of .NET DateTime.Parse(string).
         * Accepts "yyyy-MM-dd", "yyyy-MM-dd HH:mm:ss", "yyyy-MM-ddTHH:mm:ss",
         * or with optional ".fff" millisecond suffix.
         * @throws System::FormatException if the string cannot be parsed.
         */
        [[nodiscard]] static DateTime Parse(const std::string& s);

        /**
         * @brief Tries to parse a date/time string; returns false without throwing on failure.
         *
         * C++ counterpart of .NET DateTime.TryParse(string, out DateTime).
         */
        static bool TryParse(const std::string& s, DateTime& result);

        using Object::Equals;

        /**
         * @brief Determines whether this instance is equal to @p value.
         *
         * C++ counterpart of .NET DateTime.Equals(DateTime).
         */
        [[nodiscard]] bool Equals(const DateTime& value) const { return *this == value; }

        /**
         * @brief Compares this instance to a specified DateTime value.
         *
         * C++ counterpart of .NET DateTime.CompareTo(DateTime).
         * @return Less than zero if this instance is earlier than @p value, zero if equal,
         *         greater than zero if this instance is later than @p value.
         */
        [[nodiscard]] intcs CompareTo(const DateTime& value) const {
            if (ticks_ > value.ticks_) return 1;
            if (ticks_ < value.ticks_) return -1;
            return 0;
        }

        /**
         * @brief Returns a hash code for this DateTime.
         *
         * C++ counterpart of .NET DateTime.GetHashCode().
         */
        [[nodiscard]] intcs GetHashCode() const override {
            return static_cast<intcs>(ticks_) ^ static_cast<intcs>(ticks_ >> 32);
        }

        /**
         * @brief Determines whether the specified year is a leap year.
         *
         * C++ counterpart of .NET DateTime.IsLeapYear(int).
         * @throws std::out_of_range if @p year is outside [1, 9999].
         */
        [[nodiscard]] static bool IsLeapYear(intcs year);

        /**
         * @brief Returns the number of days in the specified month and year.
         *
         * C++ counterpart of .NET DateTime.DaysInMonth(int, int).
         * @throws std::out_of_range if @p month is outside [1, 12] or @p year is outside [1, 9999].
         */
        [[nodiscard]] static intcs DaysInMonth(intcs year, intcs month);

        [[nodiscard]] bool operator==(const DateTime& other) const;
        [[nodiscard]] bool operator!=(const DateTime& other) const;
        [[nodiscard]] bool operator<(const DateTime& other)  const;
        [[nodiscard]] bool operator<=(const DateTime& other) const;
        [[nodiscard]] bool operator>(const DateTime& other)  const;
        [[nodiscard]] bool operator>=(const DateTime& other) const;

        /** @brief Returns a new DateTime that is this instance plus @p value. */
        [[nodiscard]] DateTime operator+(const TimeSpan& value) const { return Add(value); }
        /** @brief Returns a new DateTime that is this instance minus @p value. */
        [[nodiscard]] DateTime operator-(const TimeSpan& value) const { return Subtract(value); }
        /** @brief Returns the TimeSpan elapsed between @p other and this instance. */
        [[nodiscard]] TimeSpan operator-(const DateTime& other) const { return Subtract(other); }

        GetTypeNameHPP()
    };

} // namespace System
