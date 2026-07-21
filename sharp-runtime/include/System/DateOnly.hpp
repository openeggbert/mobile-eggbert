// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/DayOfWeek.hpp"

namespace System {

    class DateTime; // forward declaration for FromDateTime/ToDateTime
    class TimeOnly; // forward declaration for ToDateTime

    using SharpRuntime::intcs;

    /**
     * @brief Represents a date (year, month, day) with no time or time-zone information.
     *
     * C++ counterpart of .NET System.DateOnly.
     */
    class DateOnly {
        intcs year_  = 1;
        intcs month_ = 1;
        intcs day_   = 1;
    public:
        /** @brief The smallest possible value of DateOnly (January 1, 0001). */
        static const DateOnly MinValue;
        /** @brief The largest possible value of DateOnly (December 31, 9999). */
        static const DateOnly MaxValue;

        /** @brief Initializes a new DateOnly with year=1, month=1, day=1. */
        DateOnly() = default;

        /**
         * @brief Initializes a new DateOnly with the specified year, month, and day.
         * @param year  The year (1–9999).
         * @param month The month (1–12).
         * @param day   The day (1–28/29/30/31 depending on month/year).
         * @throws System::ArgumentOutOfRangeException if the year/month/day combination is not a valid date.
         */
        DateOnly(intcs year, intcs month, intcs day);

        /** @brief Returns the year component of this date. */
        [[nodiscard]] intcs getYearProperty()  const { return year_; }

        /** @brief Returns the month component of this date (1–12). */
        [[nodiscard]] intcs getMonthProperty() const { return month_; }

        /** @brief Returns the day-of-month component of this date (1–31). */
        [[nodiscard]] intcs getDayProperty()   const { return day_; }

        /** @brief Returns the day of the week represented by this instance. */
        [[nodiscard]] DayOfWeek getDayOfWeekProperty() const;

        /** @brief Returns the day of the year represented by this instance (1–366). */
        [[nodiscard]] intcs getDayOfYearProperty() const;

        /** @brief Returns the number of days since January 1, 0001 (DateOnly.MinValue). */
        [[nodiscard]] intcs getDayNumberProperty() const;

        /**
         * @brief Creates a DateOnly from the number of days since January 1, 0001.
         *
         * C++ counterpart of .NET DateOnly.FromDayNumber(int).
         * @param dayNumber Zero-based day count from MinValue.
         */
        [[nodiscard]] static DateOnly FromDayNumber(intcs dayNumber);

        /**
         * @brief Returns a string representation in the form "yyyy-MM-dd".
         *
         * C++ counterpart of .NET DateOnly.ToString().
         */
        [[nodiscard]] std::string ToString() const {
            std::ostringstream oss;
            oss.imbue(std::locale::classic());
            oss << year_ << '-'
                << std::setw(2) << std::setfill('0') << month_ << '-'
                << std::setw(2) << std::setfill('0') << day_;
            return oss.str();
        }

        /**
         * @brief Returns the date formatted according to @p format.
         *
         * C++ counterpart of .NET DateOnly.ToString(string).
         * Tokens: yyyy/yy (year), MM/M (month), dd/d (day). Literals in single quotes.
         * @param format The format string.
         */
        [[nodiscard]] std::string ToString(const std::string& format) const;

        /**
         * @brief Compares this instance to another DateOnly.
         *
         * C++ counterpart of .NET DateOnly.CompareTo(DateOnly).
         * @return Negative, zero, or positive.
         */
        [[nodiscard]] intcs CompareTo(const DateOnly& other) const;

        /**
         * @brief Returns true if this instance equals another DateOnly.
         *
         * C++ counterpart of .NET DateOnly.Equals(DateOnly).
         */
        [[nodiscard]] bool Equals(const DateOnly& other) const { return *this == other; }

        /**
         * @brief Returns a hash code for this DateOnly.
         *
         * C++ counterpart of .NET DateOnly.GetHashCode().
         */
        [[nodiscard]] intcs GetHashCode() const { return getDayNumberProperty(); }

        /**
         * @brief Deconstructs this instance into year, month, and day components.
         *
         * C++ counterpart of .NET DateOnly.Deconstruct(out int, out int, out int).
         */
        void Deconstruct(intcs& year, intcs& month, intcs& day) const {
            year = year_; month = month_; day = day_;
        }

        /**
         * @brief Returns a new DateOnly with @p n days added (may be negative).
         *
         * C++ counterpart of .NET DateOnly.AddDays(int).
         */
        [[nodiscard]] DateOnly AddDays(intcs n) const;

        /**
         * @brief Returns a new DateOnly with @p n months added (may be negative).
         *
         * C++ counterpart of .NET DateOnly.AddMonths(int).
         * Clamps the day to the end of the resulting month when needed.
         */
        [[nodiscard]] DateOnly AddMonths(intcs n) const;

        /**
         * @brief Returns a new DateOnly with @p n years added (may be negative).
         *
         * C++ counterpart of .NET DateOnly.AddYears(int).
         */
        [[nodiscard]] DateOnly AddYears(intcs n) const;

        /**
         * @brief Extracts the date part from the specified DateTime.
         *
         * C++ counterpart of .NET DateOnly.FromDateTime(DateTime).
         * @param dt The DateTime to extract the date from.
         */
        [[nodiscard]] static DateOnly FromDateTime(const DateTime& dt);

        /**
         * @brief Returns a DateTime set to the date of this instance and the time of @p time.
         *
         * C++ counterpart of .NET DateOnly.ToDateTime(TimeOnly).
         * @param time The time of day.
         */
        [[nodiscard]] DateTime ToDateTime(const TimeOnly& time) const;

        /**
         * @brief Parses a date string in the form "yyyy-MM-dd".
         *
         * C++ counterpart of .NET DateOnly.Parse(string).
         * @throws System::FormatException on failure.
         */
        [[nodiscard]] static DateOnly Parse(const std::string& s);

        /**
         * @brief Tries to parse a date string in the form "yyyy-MM-dd".
         *
         * C++ counterpart of .NET DateOnly.TryParse(string, out DateOnly).
         * @return false if parsing fails.
         */
        static bool TryParse(const std::string& s, DateOnly& result);

        /** @brief Returns true if this date equals the specified date. */
        bool operator==(const DateOnly& o) const { return year_==o.year_ && month_==o.month_ && day_==o.day_; }
        /** @brief Returns true if this date does not equal the specified date. */
        bool operator!=(const DateOnly& o) const { return !(*this==o); }
        /** @brief Returns true if this date is earlier than the specified date. */
        bool operator< (const DateOnly& o) const { return toDays() <  o.toDays(); }
        /** @brief Returns true if this date is earlier than or equal to the specified date. */
        bool operator<=(const DateOnly& o) const { return toDays() <= o.toDays(); }
        /** @brief Returns true if this date is later than the specified date. */
        bool operator> (const DateOnly& o) const { return toDays() >  o.toDays(); }
        /** @brief Returns true if this date is later than or equal to the specified date. */
        bool operator>=(const DateOnly& o) const { return toDays() >= o.toDays(); }

    private:
        intcs toDays() const { return year_*10000 + month_*100 + day_; }
    };

} // namespace System
