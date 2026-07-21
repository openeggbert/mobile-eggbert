// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/DateOnly.hpp"
#include "System/DateTime.hpp"
#include "System/DayOfWeek.hpp"
#include "System/TimeOnly.hpp"

namespace System::Globalization {

    using SharpRuntime::intcs;

/**
 * @brief Provides static helpers for ISO 8601 week-based calendars.
 *
 * C++ counterpart of .NET System.Globalization.ISOWeek.
 * ISO 8601 weeks start on Monday; week 1 is the week containing the first Thursday
 * of the year. The ISO week-year may differ from the Gregorian year near year boundaries.
 */
class ISOWeek {
public:
    /** @brief Not instantiable — all members are static. */
    ISOWeek() = delete;

    /**
     * @brief Returns the ISO 8601 week number for the given date.
     *
     * C++ counterpart of .NET ISOWeek.GetWeekOfYear(DateTime).
     * Week 1 is the week containing the first Thursday of the year.
     * @param date The date to query.
     * @return The ISO 8601 week number (1–53).
     */
    static intcs GetWeekOfYear(const System::DateTime& date) {
        int week = getWeekNumber(date);
        if (week < MinWeek) return GetWeeksInYear(date.getYearProperty() - 1);
        if (week > 52 && GetWeeksInYear(date.getYearProperty()) == 52) return MinWeek;
        return week;
    }

    /**
     * @brief Returns the ISO 8601 week number for the given date.
     * C++ counterpart of .NET ISOWeek.GetWeekOfYear(DateOnly).
     * @param date The date to query.
     * @return The ISO 8601 week number (1–53).
     */
    static intcs GetWeekOfYear(const System::DateOnly& date) {
        return GetWeekOfYear(date.ToDateTime(System::TimeOnly()));
    }

    /**
     * @brief Returns the ISO 8601 week-year for the given date.
     *
     * C++ counterpart of .NET ISOWeek.GetYear(DateTime).
     * May differ from the Gregorian year near year boundaries.
     * @param date The date to query.
     * @return The ISO 8601 week-year.
     */
    static intcs GetYear(const System::DateTime& date) {
        int week = getWeekNumber(date);
        int year = date.getYearProperty();
        if (week < MinWeek) return year - 1;
        if (week > 52 && GetWeeksInYear(year) == 52) return year + 1;
        return year;
    }

    /**
     * @brief Returns the ISO 8601 week-year for the given date.
     * C++ counterpart of .NET ISOWeek.GetYear(DateOnly).
     * @param date The date to query.
     * @return The ISO 8601 week-year.
     */
    static intcs GetYear(const System::DateOnly& date) {
        return GetYear(date.ToDateTime(System::TimeOnly()));
    }

    /**
     * @brief Returns the number of ISO weeks in the given year (52 or 53).
     *
     * C++ counterpart of .NET ISOWeek.GetWeeksInYear(int).
     * A year is long (53 weeks) if it starts on Thursday, or if it is a leap year
     * ending on Thursday (i.e. starting on Wednesday).
     * @param year The Gregorian year (1–9999).
     * @return 52 or 53.
     * @throws System::ArgumentOutOfRangeException if @p year is outside 1..9999.
     */
    static intcs GetWeeksInYear(intcs year) {
        if (year < MinYear || year > MaxYear) throw System::ArgumentOutOfRangeException("year");
        auto p = [](long long y) -> int {
            long long cent = y / 100;
            return static_cast<int>((y + (y / 4) - cent + cent / 4) % 7);
        };
        if (p(year) == 4 || p(year - 1) == 3) return 53;
        return 52;
    }

    /**
     * @brief Maps an ISO week date to the equivalent Gregorian DateTime.
     *
     * C++ counterpart of .NET ISOWeek.ToDateTime(int, int, DayOfWeek).
     * @param year      The ISO week-numbering year (1–9999).
     * @param week      The ISO week number (1–53).
     * @param dayOfWeek The day of week within the ISO week (Sunday may be given as 0 or 7).
     * @return The Gregorian DateTime equivalent to the given ISO week date.
     * @throws System::ArgumentOutOfRangeException if @p year, @p week, or @p dayOfWeek is out of range.
     */
    static System::DateTime ToDateTime(intcs year, intcs week, System::DayOfWeek dayOfWeek) {
        if (year < MinYear || year > MaxYear) throw System::ArgumentOutOfRangeException("year");
        if (week < MinWeek || week > MaxWeek) throw System::ArgumentOutOfRangeException("week");
        int dowValue = static_cast<int>(dayOfWeek);
        if (dowValue < 0 || dowValue > 7) throw System::ArgumentOutOfRangeException("dayOfWeek");

        System::DateTime jan4(year, 1, 4);
        int correction = isoWeekday(jan4.getDayOfWeekProperty()) + 3;
        int ordinal = (week * 7) + isoWeekday(dayOfWeek) - correction;
        return jan4.AddDays(ordinal - 4);
    }

    /**
     * @brief Maps an ISO week date to the equivalent Gregorian DateOnly.
     *
     * C++ counterpart of .NET ISOWeek.ToDateOnly(int, int, DayOfWeek).
     * @param year      The ISO week-numbering year (1–9999).
     * @param week      The ISO week number (1–53).
     * @param dayOfWeek The day of week within the ISO week (Sunday may be given as 0 or 7).
     * @return The Gregorian DateOnly equivalent to the given ISO week date.
     * @throws System::ArgumentOutOfRangeException if @p year, @p week, or @p dayOfWeek is out of range.
     */
    static System::DateOnly ToDateOnly(intcs year, intcs week, System::DayOfWeek dayOfWeek) {
        return System::DateOnly::FromDateTime(ToDateTime(year, week, dayOfWeek));
    }

    /**
     * @brief Returns the DateTime of the first day (Monday) of ISO week 1 in the given year.
     *
     * C++ counterpart of .NET ISOWeek.GetYearStart(int).
     * @param year The ISO week-year.
     * @return The Monday that begins ISO week 1.
     */
    static System::DateTime GetYearStart(intcs year) {
        return ToDateTime(year, MinWeek, System::DayOfWeek::Monday);
    }

    /**
     * @brief Returns the DateTime of the last day (Sunday) of the last ISO week in the given year.
     *
     * C++ counterpart of .NET ISOWeek.GetYearEnd(int).
     * @param year The ISO week-year.
     * @return The Sunday that ends the last ISO week of @p year.
     */
    static System::DateTime GetYearEnd(intcs year) {
        return ToDateTime(year, GetWeeksInYear(year), System::DayOfWeek::Sunday);
    }

private:
    static constexpr int MinYear = 1;
    static constexpr int MaxYear = 9999;
    static constexpr int MinWeek = 1;
    static constexpr int MaxWeek = 53;

    /** @brief ISO weekday number (Monday=1..Sunday=7); .NET's DayOfWeek::Sunday (0) maps to 7. */
    static int isoWeekday(System::DayOfWeek dayOfWeek) {
        return dayOfWeek == System::DayOfWeek::Sunday ? 7 : static_cast<int>(dayOfWeek);
    }

    /**
     * @brief Raw ISO week number for @p date, not yet normalized to the neighboring year.
     *
     * May be 0 (belongs to the previous ISO year) or >52 (may belong to the next ISO year);
     * callers normalize using GetWeeksInYear(). C++ counterpart of .NET ISOWeek.GetWeekNumber.
     */
    static int getWeekNumber(const System::DateTime& date) {
        return (date.getDayOfYearProperty() - isoWeekday(date.getDayOfWeekProperty()) + 10) / 7;
    }
};

} // namespace System::Globalization
