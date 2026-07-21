// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include "System/Globalization/Calendar.hpp"

namespace System::Globalization {

/**
 * @brief Represents the Julian calendar.
 *
 * C++ counterpart of .NET System.Globalization.JulianCalendar.
 * In the Julian calendar every year divisible by 4 is a leap year;
 * there is no century exception (unlike Gregorian). This produces a drift
 * of approximately one day every 128 years relative to the solar year.
 *
 * Gregorian 0001/01/01 is Julian 0001/01/03 (JulianCalendar.cs): all date-conversion
 * members (GetYear/GetMonth/GetDayOfMonth/GetDayOfYear/ToDateTime/AddMonths/AddYears)
 * apply that fixed 2-day offset via the same day-number algorithm .NET uses, so this is
 * an actual proleptic Julian calendar rather than the underlying Gregorian DateTime
 * relabeled with a Julian-flavored leap rule.
 */
class JulianCalendar : public Calendar {
public:
    static constexpr intcs JulianEra = 1; ///< The only era value for this calendar.

    /**
     * @brief Gets the algorithm type for this calendar.
     *
     * C++ counterpart of .NET JulianCalendar.AlgorithmType.
     * @return Always CalendarAlgorithmType::SolarCalendar.
     */
    [[nodiscard]] CalendarAlgorithmType getAlgorithmTypeProperty() const override {
        return CalendarAlgorithmType::SolarCalendar;
    }

    /**
     * @brief Gets the earliest date supported by JulianCalendar.
     *
     * C++ counterpart of .NET JulianCalendar.MinSupportedDateTime.
     * @return DateTime(1, 1, 1).
     */
    [[nodiscard]] System::DateTime getMinSupportedDateTimeProperty() const override {
        return System::DateTime(1, 1, 1);
    }

    /**
     * @brief Gets the latest date supported by JulianCalendar.
     *
     * C++ counterpart of .NET JulianCalendar.MaxSupportedDateTime.
     * @return DateTime(9999, 12, 31).
     */
    [[nodiscard]] System::DateTime getMaxSupportedDateTimeProperty() const override {
        return System::DateTime(9999, 12, 31);
    }

    /**
     * @brief Gets the last two-digit year that maps into the range of this calendar.
     *
     * C++ counterpart of .NET JulianCalendar.TwoDigitYearMax.
     * @return The maximum two-digit year (default 2049).
     */
    [[nodiscard]] intcs getTwoDigitYearMaxProperty() const override { return twoDigitYearMax_; }

    /**
     * @brief Sets the last two-digit year that maps into the range of this calendar.
     *
     * C++ counterpart of .NET JulianCalendar.TwoDigitYearMax setter.
     * @param value The new maximum two-digit year.
     * @throws System::InvalidOperationException if this instance is read-only.
     */
    void setTwoDigitYearMaxProperty(intcs value) override {
        VerifyWritable();
        twoDigitYearMax_ = value;
    }

    /**
     * @brief Returns the era for the given DateTime.
     *
     * C++ counterpart of .NET JulianCalendar.GetEra(DateTime).
     * @return Always JulianEra (1).
     */
    [[nodiscard]] intcs GetEra(const System::DateTime& /*time*/) const override { return JulianEra; }

    /**
     * @brief Returns the number of eras in this calendar.
     *
     * @return Always 1; the Julian calendar has a single era.
     */
    [[nodiscard]] intcs GetErasCount() const override { return 1; }

    /**
     * @brief Gets the list of era identifiers supported by this calendar.
     *
     * C++ counterpart of .NET JulianCalendar.Eras.
     * @return A vector containing {JulianEra}.
     */
    [[nodiscard]] std::vector<intcs> getErasProperty() const override { return {JulianEra}; }

    /**
     * @brief Determines whether the specified year is a Julian leap year.
     *
     * C++ counterpart of .NET JulianCalendar.IsLeapYear(int, int).
     * Every year divisible by 4 is a leap year (no century exception).
     * @param year The year to check.
     * @param era  The era (ignored; always Julian).
     * @return true if @p year is divisible by 4; otherwise false.
     */
    [[nodiscard]] bool IsLeapYear(intcs year, intcs /*era*/ = CurrentEra) const override {
        return year % 4 == 0;
    }

    /**
     * @brief Returns the number of days in the specified Julian month.
     *
     * C++ counterpart of .NET JulianCalendar.GetDaysInMonth(int, int, int).
     * Uses the Julian leap year rule: every year divisible by 4 gives February 29 days.
     * @param year  The year.
     * @param month The month (1–12).
     * @param era   The era (ignored).
     * @return The number of days in the specified month.
     */
    [[nodiscard]] intcs GetDaysInMonth(intcs year, intcs month, intcs /*era*/ = CurrentEra) const override {
        static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (month == 2 && IsLeapYear(year)) return 29;
        return days[month];
    }

    /**
     * @brief Returns the number of days in the specified Julian year.
     *
     * C++ counterpart of .NET JulianCalendar.GetDaysInYear(int, int).
     * @param year The year.
     * @param era  The era (ignored).
     * @return 366 for a leap year; otherwise 365.
     */
    [[nodiscard]] intcs GetDaysInYear(intcs year, intcs /*era*/ = CurrentEra) const override {
        return IsLeapYear(year) ? 366 : 365;
    }

    /**
     * @brief Returns the Julian calendar year for the given DateTime.
     * C++ counterpart of .NET JulianCalendar.GetYear(DateTime).
     */
    [[nodiscard]] intcs GetYear(const System::DateTime& time) const override {
        return GetDatePart(time.getTicksProperty(), DatePartYear);
    }

    /**
     * @brief Returns the Julian calendar month for the given DateTime.
     * C++ counterpart of .NET JulianCalendar.GetMonth(DateTime).
     */
    [[nodiscard]] intcs GetMonth(const System::DateTime& time) const override {
        return GetDatePart(time.getTicksProperty(), DatePartMonth);
    }

    /**
     * @brief Returns the Julian calendar day-of-month for the given DateTime.
     * C++ counterpart of .NET JulianCalendar.GetDayOfMonth(DateTime).
     */
    [[nodiscard]] intcs GetDayOfMonth(const System::DateTime& time) const override {
        return GetDatePart(time.getTicksProperty(), DatePartDay);
    }

    /**
     * @brief Returns the Julian calendar day-of-year for the given DateTime.
     * C++ counterpart of .NET JulianCalendar.GetDayOfYear(DateTime).
     */
    [[nodiscard]] intcs GetDayOfYear(const System::DateTime& time) const override {
        return GetDatePart(time.getTicksProperty(), DatePartDayOfYear);
    }

    /**
     * @brief Returns a DateTime from the Julian date and time components.
     *
     * C++ counterpart of .NET JulianCalendar.ToDateTime(int, int, int, int, int, int, int, int).
     * @param year        The Julian year.
     * @param month       The Julian month (1-12).
     * @param day         The Julian day of month.
     * @param hour        Hour (0-23).
     * @param minute      Minute (0-59).
     * @param second      Second (0-59).
     * @param millisecond Millisecond (0-999).
     * @param era         The era (ignored).
     * @return The Gregorian-DateTime equivalent of the given Julian date components.
     */
    System::DateTime ToDateTime(intcs year, intcs month, intcs day, intcs hour, intcs minute,
                                intcs second, intcs millisecond, intcs /*era*/ = CurrentEra) const override {
        SharpRuntime::longcs ticks = DateToTicks(year, month, day)
            + hour * System::DateTime::TicksPerHour
            + minute * System::DateTime::TicksPerMinute
            + second * System::DateTime::TicksPerSecond
            + static_cast<SharpRuntime::longcs>(millisecond) * System::DateTime::TicksPerMillisecond;
        return System::DateTime(ticks);
    }

    /**
     * @brief Returns a DateTime that is the specified number of months from the given DateTime,
     * performing the arithmetic in the Julian calendar system.
     * C++ counterpart of .NET JulianCalendar.AddMonths(DateTime, int).
     *
     * As a virtual override, this replaces (does not augment) Calendar::AddMonths's own
     * |months| > 120000 bounds check, so it needs its own -- without it, `i = m - 1 + months`
     * a few lines below is real signed-integer-overflow UB in C++ for a months argument as
     * simple as INT_MAX (confirmed via UBSan: "signed integer overflow: 2147483647 + 5 cannot
     * be represented in type 'int'"). Matches the same 120000 bound Calendar::AddMonths and
     * this port's other calendar-specific AddMonths overrides use.
     * @throws System::ArgumentOutOfRangeException if @p months is outside [-120000, 120000].
     */
    System::DateTime AddMonths(const System::DateTime& time, intcs months) const override {
        if (months < -120000 || months > 120000)
            throw System::ArgumentOutOfRangeException("months", std::to_string(months),
                "Valid values are between -120000 and 120000, inclusive.");
        SharpRuntime::longcs ticks = time.getTicksProperty();
        int y = GetDatePart(ticks, DatePartYear);
        int m = GetDatePart(ticks, DatePartMonth);
        int d = GetDatePart(ticks, DatePartDay);
        int i = m - 1 + months;
        if (i >= 0) {
            m = i % 12 + 1;
            y += i / 12;
        } else {
            m = 12 + (i + 1) % 12;
            y += (i - 11) / 12;
        }
        int days = GetDaysInMonth(y, m);
        if (d > days) d = days;
        SharpRuntime::longcs newTicks = DateToTicks(y, m, d) + (ticks % System::DateTime::TicksPerDay);
        return System::DateTime(newTicks);
    }

    /**
     * @brief Returns a DateTime that is the specified number of years from the given DateTime,
     * performing the arithmetic in the Julian calendar system.
     * C++ counterpart of .NET JulianCalendar.AddYears(DateTime, int).
     *
     * `years * 12` computed directly with no upfront bounds check is real signed-integer-
     * overflow UB in C++ for a merely large years argument (same bug class as
     * Calendar::AddYears and PersianCalendar::AddYears). Validated against the equivalent
     * bound AddMonths (above) already enforces (120000 / 12 = 10000) before the multiply.
     * @throws System::ArgumentOutOfRangeException if @p years is outside [-10000, 10000].
     */
    System::DateTime AddYears(const System::DateTime& time, intcs years) const override {
        if (years < -10000 || years > 10000)
            throw System::ArgumentOutOfRangeException("years", std::to_string(years),
                "Valid values are between -10000 and 10000, inclusive.");
        return AddMonths(time, years * 12);
    }

private:
    int twoDigitYearMax_{2049};

    static constexpr int DatePartYear      = 0;
    static constexpr int DatePartDayOfYear = 1;
    static constexpr int DatePartMonth     = 2;
    static constexpr int DatePartDay       = 3;

    static constexpr int JulianDaysPerYear    = 365;
    static constexpr int JulianDaysPer4Years  = JulianDaysPerYear * 4 + 1;

    // Cumulative day count before each month; index 0 is unused (months are 1-based).
    static constexpr int daysToMonth365[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
    static constexpr int daysToMonth366[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};

    // C++ counterpart of .NET JulianCalendar.GetDatePart(long, int) (JulianCalendar.cs).
    // Gregorian ticks -> Julian day-number decomposition via 4-year cycles.
    static int GetDatePart(SharpRuntime::longcs ticks, int part) {
        // Gregorian 1/1/0001 is Julian 1/3/0001; shift Gregorian ticks to Julian ticks.
        SharpRuntime::longcs julianTicks = ticks + System::DateTime::TicksPerDay * 2;
        int n = static_cast<int>(julianTicks / System::DateTime::TicksPerDay);
        int y4 = n / JulianDaysPer4Years;
        n -= y4 * JulianDaysPer4Years;
        int y1 = n / JulianDaysPerYear;
        if (y1 == 4) y1 = 3;
        if (part == DatePartYear) return y4 * 4 + y1 + 1;

        n -= y1 * JulianDaysPerYear;
        if (part == DatePartDayOfYear) return n + 1;

        bool leapYear = (y1 == 3);
        const int* days = leapYear ? daysToMonth366 : daysToMonth365;
        int m = (n >> 5) + 1;
        while (n >= days[m]) ++m;

        if (part == DatePartMonth) return m;
        return n - days[m - 1] + 1;
    }

    // C++ counterpart of .NET JulianCalendar.DateToTicks(int, int, int) (JulianCalendar.cs).
    static SharpRuntime::longcs DateToTicks(int year, int month, int day) {
        const int* days = (year % 4 == 0) ? daysToMonth366 : daysToMonth365;
        int y = year - 1;
        int n = y * 365 + y / 4 + days[month - 1] + day - 1;
        // Julian day-number n -> Gregorian ticks: subtract the fixed 2-day offset.
        return static_cast<SharpRuntime::longcs>(n - 2) * System::DateTime::TicksPerDay;
    }
};

} // namespace System::Globalization
