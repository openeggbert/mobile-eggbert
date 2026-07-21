// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Math.hpp"
#include "System/TimeSpan.hpp"
#include "System/Globalization/Calendar.hpp"

namespace System::Globalization {

using SharpRuntime::intcs;

/**
 * @brief Represents the Persian (Solar Hijri) calendar.
 *
 * C++ counterpart of .NET System.Globalization.PersianCalendar.
 * Modern Persian calendar is a solar observation based calendar: each new year begins on
 * the day when the vernal equinox occurs before noon at the Persian observation site
 * (52.5 degrees east, the longitude of Iran Standard Time). Ordinary years have 365 days;
 * leap years have 366 days with the extra day added to the last month (Esfand).
 * There is no Persian year 0.
 *
 * The astronomical vernal-equinox calculation (AstroHelper below) is a faithful port of
 * .NET's internal CalendricalCalculationsHelper -- a fixed 33-year arithmetic leap-year
 * cycle (the rule this port used before this fix) diverges from the real algorithm on
 * roughly 29% of years, so it was replaced rather than kept as a simplification.
 */
class PersianCalendar : public Calendar {
public:
    static constexpr intcs PersianEra = 1; ///< The only era value for this calendar.

    /**
     * @brief Gets the algorithm type for this calendar.
     *
     * C++ counterpart of .NET PersianCalendar.AlgorithmType.
     * @return Always CalendarAlgorithmType::SolarCalendar.
     */
    [[nodiscard]] CalendarAlgorithmType getAlgorithmTypeProperty() const override {
        return CalendarAlgorithmType::SolarCalendar;
    }

    /**
     * @brief Gets the earliest date supported by PersianCalendar.
     *
     * C++ counterpart of .NET PersianCalendar.MinSupportedDateTime.
     * @return DateTime(622, 3, 22) — the Gregorian date of Persian year 1, month 1, day 1.
     */
    [[nodiscard]] System::DateTime getMinSupportedDateTimeProperty() const override {
        return MinDate();
    }

    /**
     * @brief Gets the latest date supported by PersianCalendar.
     *
     * C++ counterpart of .NET PersianCalendar.MaxSupportedDateTime.
     * Verified against PersianCalendar.cs: `s_maxDate = DateTime.MaxValue`, not a
     * calendar-specific boundary -- Persian year 9378, month 10, day 13 is the highest
     * representable date, and it falls short of DateTime.MaxValue, but real .NET still
     * reports the property as DateTime.MaxValue verbatim.
     * @return DateTime::MaxValue.
     */
    [[nodiscard]] System::DateTime getMaxSupportedDateTimeProperty() const override {
        return System::DateTime::MaxValue;
    }

    /**
     * @brief Gets the last two-digit year that maps into the range of this calendar.
     *
     * C++ counterpart of .NET PersianCalendar.TwoDigitYearMax.
     * @return The maximum two-digit year (default 1410).
     */
    [[nodiscard]] intcs getTwoDigitYearMaxProperty() const override { return twoDigitYearMax_; }

    /**
     * @brief Sets the last two-digit year that maps into the range of this calendar.
     *
     * C++ counterpart of .NET PersianCalendar.TwoDigitYearMax setter.
     * @param value The new maximum two-digit year.
     * @throws System::InvalidOperationException if this instance is read-only.
     * @throws System::ArgumentOutOfRangeException if @p value is outside [99, MaxCalendarYear].
     */
    void setTwoDigitYearMaxProperty(intcs value) override {
        VerifyWritable();
        if (value < 99 || value > MaxCalendarYear)
            throw System::ArgumentOutOfRangeException("value", std::to_string(value),
                "Valid values are between 99 and " + std::to_string(MaxCalendarYear) + ", inclusive.");
        twoDigitYearMax_ = value;
    }

    /**
     * @brief Returns the era for the given DateTime.
     *
     * C++ counterpart of .NET PersianCalendar.GetEra(DateTime).
     * @return Always PersianEra (1).
     * @throws System::ArgumentOutOfRangeException if @p time is outside the supported range.
     */
    [[nodiscard]] intcs GetEra(const System::DateTime& time) const override {
        CheckTicksRange(time.getTicksProperty());
        return PersianEra;
    }

    /**
     * @brief Returns the number of eras in this calendar.
     *
     * @return Always 1; the Persian calendar has a single era.
     */
    [[nodiscard]] intcs GetErasCount() const override { return 1; }

    /**
     * @brief Gets the list of era identifiers supported by this calendar.
     *
     * C++ counterpart of .NET PersianCalendar.Eras.
     * @return A vector containing {PersianEra}.
     */
    [[nodiscard]] std::vector<intcs> getErasProperty() const override { return {PersianEra}; }

    /**
     * @brief Returns the Persian year corresponding to the given DateTime.
     *
     * C++ counterpart of .NET PersianCalendar.GetYear(DateTime).
     * @param time The Gregorian DateTime to convert.
     * @return The Persian year.
     */
    [[nodiscard]] intcs GetYear(const System::DateTime& time) const override {
        int y, m, d, doy;
        GetDate(time.getTicksProperty(), y, m, d, doy);
        return y;
    }

    /**
     * @brief Returns the Persian month corresponding to the given DateTime.
     *
     * C++ counterpart of .NET PersianCalendar.GetMonth(DateTime).
     * @param time The Gregorian DateTime to convert.
     * @return The Persian month (1–12).
     */
    [[nodiscard]] intcs GetMonth(const System::DateTime& time) const override {
        int y, m, d, doy;
        GetDate(time.getTicksProperty(), y, m, d, doy);
        return m;
    }

    /**
     * @brief Returns the day of the Persian month corresponding to the given DateTime.
     *
     * C++ counterpart of .NET PersianCalendar.GetDayOfMonth(DateTime).
     * @param time The Gregorian DateTime to convert.
     * @return The day of the Persian month (1–31).
     */
    [[nodiscard]] intcs GetDayOfMonth(const System::DateTime& time) const override {
        int y, m, d, doy;
        GetDate(time.getTicksProperty(), y, m, d, doy);
        return d;
    }

    /**
     * @brief Returns the ordinal day within the Persian year for the given DateTime.
     *
     * C++ counterpart of .NET PersianCalendar.GetDayOfYear(DateTime). The Calendar base
     * class default (Calendar.hpp) returns the DateTime's own (Gregorian) day-of-year,
     * which is wrong for any non-Gregorian calendar; this override returns the Persian
     * ordinal day instead.
     * @param time The Gregorian DateTime to convert.
     * @return The day of the Persian year (1–366).
     */
    [[nodiscard]] intcs GetDayOfYear(const System::DateTime& time) const override {
        int y, m, d, doy;
        GetDate(time.getTicksProperty(), y, m, d, doy);
        return doy;
    }

    /**
     * @brief Determines whether the specified Persian year is a leap year.
     *
     * C++ counterpart of .NET PersianCalendar.IsLeapYear(int, int). A year is a leap year
     * iff the astronomically-determined Persian year that follows it starts 366 days later.
     * @param year The Persian year.
     * @param era  The era (default CurrentEra; must be CurrentEra or PersianEra).
     * @return true if @p year is a Persian leap year; otherwise false.
     * @throws System::ArgumentOutOfRangeException if @p year or @p era is out of range.
     */
    [[nodiscard]] bool IsLeapYear(intcs year, intcs era = CurrentEra) const override {
        CheckYearRange(year, era);
        if (year == MaxCalendarYear) return false;
        return (GetAbsoluteDatePersian(year + 1, 1, 1) - GetAbsoluteDatePersian(year, 1, 1)) == 366;
    }

    /**
     * @brief Returns the number of days in the specified Persian month.
     *
     * C++ counterpart of .NET PersianCalendar.GetDaysInMonth(int, int, int).
     * Months 1–6 have 31 days; months 7–11 have 30 days; month 12 has 29 or 30 days.
     * @param year  The Persian year.
     * @param month The Persian month (1–12).
     * @param era   The era (default CurrentEra).
     * @return The number of days in the specified month.
     * @throws System::ArgumentOutOfRangeException if @p year, @p month, or @p era is out of range.
     */
    [[nodiscard]] intcs GetDaysInMonth(intcs year, intcs month, intcs era = CurrentEra) const override {
        CheckYearMonthRange(year, month, era);
        if (month == MaxCalendarMonth && year == MaxCalendarYear) return MaxCalendarDay;
        int daysInMonth = DaysToMonth[static_cast<size_t>(month)] - DaysToMonth[static_cast<size_t>(month) - 1];
        if (month == 12 && !IsLeapYear(year)) --daysInMonth;
        return daysInMonth;
    }

    /**
     * @brief Returns the number of days in the specified Persian year.
     *
     * C++ counterpart of .NET PersianCalendar.GetDaysInYear(int, int).
     * @param year The Persian year.
     * @param era  The era (default CurrentEra).
     * @return 366 for leap years, 365 otherwise.
     * @throws System::ArgumentOutOfRangeException if @p year or @p era is out of range.
     */
    [[nodiscard]] intcs GetDaysInYear(intcs year, intcs era = CurrentEra) const override {
        CheckYearRange(year, era);
        if (year == MaxCalendarYear)
            return DaysToMonth[static_cast<size_t>(MaxCalendarMonth) - 1] + MaxCalendarDay;
        return IsLeapYear(year, CurrentEra) ? 366 : 365;
    }

    /**
     * @brief Returns the number of months in the specified Persian year.
     *
     * C++ counterpart of .NET PersianCalendar.GetMonthsInYear(int, int).
     * @param year The Persian year.
     * @param era  The era (default CurrentEra).
     * @return 12, except MaxCalendarMonth (10) in the final supported year.
     * @throws System::ArgumentOutOfRangeException if @p year or @p era is out of range.
     */
    [[nodiscard]] intcs GetMonthsInYear(intcs year, intcs era = CurrentEra) const override {
        CheckYearRange(year, era);
        return (year == MaxCalendarYear) ? MaxCalendarMonth : 12;
    }

    /**
     * @brief Returns the leap month for the specified Persian year, or 0 (none).
     *
     * C++ counterpart of .NET PersianCalendar.GetLeapMonth(int, int). The Persian calendar
     * never has a leap month (its leap day is appended to the last ordinary month instead).
     * @param year The Persian year.
     * @param era  The era (default CurrentEra).
     * @return Always 0.
     * @throws System::ArgumentOutOfRangeException if @p year or @p era is out of range.
     */
    [[nodiscard]] intcs GetLeapMonth(intcs year, intcs era = CurrentEra) const override {
        CheckYearRange(year, era);
        return 0;
    }

    /**
     * @brief Determines whether the specified Persian month is a leap month.
     *
     * C++ counterpart of .NET PersianCalendar.IsLeapMonth(int, int, int). Always false;
     * see GetLeapMonth()'s doc comment.
     * @param year  The Persian year.
     * @param month The Persian month (1–12).
     * @param era   The era (default CurrentEra).
     * @return Always false.
     * @throws System::ArgumentOutOfRangeException if @p year, @p month, or @p era is out of range.
     */
    [[nodiscard]] bool IsLeapMonth(intcs year, intcs month, intcs era = CurrentEra) const override {
        CheckYearMonthRange(year, month, era);
        return false;
    }

    /**
     * @brief Determines whether the specified Persian date is the calendar's leap day.
     *
     * C++ counterpart of .NET PersianCalendar.IsLeapDay(int, int, int, int). The Calendar
     * base class default (Calendar.hpp) checks for February 29th, which is meaningless for
     * the Persian calendar; the Persian leap day is Esfand (month 12) day 30.
     * @param year  The Persian year.
     * @param month The Persian month (1–12).
     * @param day   The Persian day.
     * @param era   The era (default CurrentEra).
     * @return true if the date is month 12, day 30 of a leap year; otherwise false.
     * @throws System::ArgumentOutOfRangeException if any component is out of range.
     */
    [[nodiscard]] bool IsLeapDay(intcs year, intcs month, intcs day, intcs era = CurrentEra) const override {
        int daysInMonth = GetDaysInMonth(year, month, era);
        if (day < 1 || day > daysInMonth)
            throw System::ArgumentOutOfRangeException("day", std::to_string(day),
                "Day must be between 1 and " + std::to_string(daysInMonth) + " for month " + std::to_string(month) + ".");
        return IsLeapYear(year, era) && month == 12 && day == 30;
    }

    /**
     * @brief Returns a DateTime from the Persian date and time components.
     *
     * C++ counterpart of .NET PersianCalendar.ToDateTime(int, int, int, int, int, int, int, int).
     * @param year        The Persian year.
     * @param month       The Persian month (1–12).
     * @param day         The Persian day (1–31).
     * @param hour        Hour (0–23).
     * @param minute      Minute (0–59).
     * @param second      Second (0–59).
     * @param millisecond Millisecond (0–999).
     * @param era         Era (default CurrentEra).
     * @return The Gregorian DateTime corresponding to the given Persian date components.
     * @throws System::ArgumentOutOfRangeException if any component is out of range.
     */
    System::DateTime ToDateTime(intcs year, intcs month, intcs day, intcs hour, intcs minute,
                                intcs second, intcs millisecond, intcs era = CurrentEra) const override {
        int daysInMonth = GetDaysInMonth(year, month, era);
        if (day < 1 || day > daysInMonth)
            throw System::ArgumentOutOfRangeException("day", std::to_string(day),
                "Day must be between 1 and " + std::to_string(daysInMonth) + " for month " + std::to_string(month) + ".");
        long long lDate = GetAbsoluteDatePersian(year, month, day);
        if (lDate < 0)
            throw System::ArgumentOutOfRangeException("year",
                "Year, month, and day parameters describe an un-representable DateTime.");
        return System::DateTime(lDate * System::DateTime::TicksPerDay + TimeToTicks(hour, minute, second, millisecond));
    }

    /**
     * @brief Returns a DateTime that is the specified number of months from the given DateTime.
     *
     * C++ counterpart of .NET PersianCalendar.AddMonths(DateTime, int).
     * @param time   The starting DateTime.
     * @param months The number of months to add (may be negative).
     * @return A new DateTime offset by @p months Persian months.
     * @throws System::ArgumentOutOfRangeException if @p months is outside [-120000, 120000]
     *         or the result falls outside the supported date range.
     */
    System::DateTime AddMonths(const System::DateTime& time, intcs months) const override {
        if (months < -120000 || months > 120000)
            throw System::ArgumentOutOfRangeException("months", std::to_string(months),
                "Valid values are between -120000 and 120000, inclusive.");

        int y, m, d, doy;
        GetDate(time.getTicksProperty(), y, m, d, doy);
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

        long long ticks = GetAbsoluteDatePersian(y, m, d) * System::DateTime::TicksPerDay
                         + time.getTicksProperty() % System::DateTime::TicksPerDay;
        if (ticks < MinDate().getTicksProperty() || ticks > MaxDate().getTicksProperty())
            throw System::ArgumentOutOfRangeException("time",
                "The added or subtracted value results in an un-representable DateTime.");
        return System::DateTime(ticks);
    }

    /**
     * @brief Returns a DateTime that is the specified number of years from the given DateTime.
     *
     * C++ counterpart of .NET PersianCalendar.AddYears(DateTime, int). Delegates to
     * AddMonths(time, years * 12), matching real .NET exactly -- real .NET's own AddYears is
     * literally `return AddMonths(time, years * 12);` with no upfront check, since C#'s default
     * unchecked arithmetic is defined to wrap silently on overflow. C++ signed overflow is
     * undefined behavior for the same expression (confirmed via UBSan: "signed integer
     * overflow: 200000000 * 12 cannot be represented in type 'int'"), so this validates years
     * is small enough that the multiplication itself can never overflow before attempting it --
     * the exact bound AddMonths itself already enforces on its `months` parameter
     * (120000 / 12 = 10000), so this doesn't reject anything AddMonths would have accepted
     * anyway; it only moves the rejection earlier, before the overflow-prone multiply. Same
     * bug class and same fix as DateTime::AddYears (which this port's DateTimeOffset::AddYears
     * also had until it was fixed to delegate to DateTime::AddYears).
     * @param time  The starting DateTime.
     * @param years The number of years to add (may be negative).
     * @throws System::ArgumentOutOfRangeException if @p years is outside [-10000, 10000].
     * @return A new DateTime offset by @p years Persian years.
     */
    System::DateTime AddYears(const System::DateTime& time, intcs years) const override {
        if (years < -10000 || years > 10000)
            throw System::ArgumentOutOfRangeException("years", std::to_string(years),
                "Valid values are between -10000 and 10000, inclusive.");
        return AddMonths(time, years * 12);
    }

    /**
     * @brief Converts a two-digit year to a four-digit Persian year.
     *
     * C++ counterpart of .NET PersianCalendar.ToFourDigitYear(int).
     * @param year The two- or four-digit year.
     * @return A four-digit Persian year.
     * @throws System::ArgumentOutOfRangeException if @p year is negative or exceeds MaxCalendarYear.
     */
    [[nodiscard]] intcs ToFourDigitYear(intcs year) const override {
        System::ArgumentOutOfRangeException::ThrowIfNegative(year, "year");
        if (year < 100) return Calendar::ToFourDigitYear(year);
        if (year > MaxCalendarYear)
            throw System::ArgumentOutOfRangeException("year", std::to_string(year),
                "Valid values are between 1 and " + std::to_string(MaxCalendarYear) + ", inclusive.");
        return year;
    }

private:
    intcs twoDigitYearMax_{1410};

    static constexpr int MaxCalendarYear  = 9378;
    static constexpr int MaxCalendarMonth = 10;
    static constexpr int MaxCalendarDay   = 13;
    static constexpr int ApproximateHalfYear = 180;

    // DaysToMonth[m] = cumulative Persian days before the end of month m (0-based table,
    // index 0 = 0 days before month 1 starts). Verified against PersianCalendar.cs.
    static constexpr std::array<int, 13> DaysToMonth =
        {0, 31, 62, 93, 124, 155, 186, 216, 246, 276, 306, 336, 366};

    static const System::DateTime& MinDate() {
        static const System::DateTime d(622, 3, 22);
        return d;
    }
    static const System::DateTime& MaxDate() { return System::DateTime::MaxValue; }

    // s_persianEpoch = MinDate().Ticks / TicksPerDay (0-based day count; NOT the "absolute
    // date" +1 convention used by GetDate's numDays -- this mismatch is present in the real
    // .NET source too and is preserved here verbatim rather than "corrected").
    static long long PersianEpoch() {
        static const long long e = MinDate().getTicksProperty() / System::DateTime::TicksPerDay;
        return e;
    }

    static void CheckTicksRange(long long ticks) {
        if (ticks < MinDate().getTicksProperty() || ticks > MaxDate().getTicksProperty())
            throw System::ArgumentOutOfRangeException("time", std::to_string(ticks),
                "Specified time is not supported in this calendar. It should be between "
                + MinDate().ToString() + " and " + MaxDate().ToString() + " (Gregorian dates), inclusive.");
    }

    static void CheckEraRange(int era) {
        if (era != CurrentEra && era != PersianEra)
            throw System::ArgumentOutOfRangeException("era", std::to_string(era), "Era value was not valid.");
    }

    static void CheckYearRange(int year, int era) {
        CheckEraRange(era);
        if (year < 1 || year > MaxCalendarYear)
            throw System::ArgumentOutOfRangeException("year", std::to_string(year),
                "Valid values are between 1 and " + std::to_string(MaxCalendarYear) + ", inclusive.");
    }

    static void CheckYearMonthRange(int year, int month, int era) {
        CheckYearRange(year, era);
        if (year == MaxCalendarYear && month > MaxCalendarMonth)
            throw System::ArgumentOutOfRangeException("month", std::to_string(month),
                "Valid values are between 1 and " + std::to_string(MaxCalendarMonth) + ", inclusive.");
        if (month < 1 || month > 12)
            throw System::ArgumentOutOfRangeException("month", std::to_string(month),
                "Month must be between one and twelve.");
    }

    static int DaysInPreviousMonths(int month) {
        return DaysToMonth[static_cast<size_t>(month) - 1];
    }

    static int MonthFromOrdinalDay(int ordinalDay) {
        int index = 0;
        while (ordinalDay > DaysToMonth[static_cast<size_t>(index)]) ++index;
        return index;
    }

    static long long TimeToTicks(int hour, int minute, int second, int millisecond) {
        if (hour < 0 || hour >= 24)
            throw System::ArgumentOutOfRangeException("hour", std::to_string(hour), "Hour must be between 0 and 23.");
        if (minute < 0 || minute >= 60)
            throw System::ArgumentOutOfRangeException("minute", std::to_string(minute), "Minute must be between 0 and 59.");
        if (second < 0 || second >= 60)
            throw System::ArgumentOutOfRangeException("second", std::to_string(second), "Second must be between 0 and 59.");
        if (millisecond < 0 || millisecond >= 1000)
            throw System::ArgumentOutOfRangeException("millisecond", std::to_string(millisecond), "Millisecond must be between 0 and 999.");
        return static_cast<long long>(hour) * System::TimeSpan::TicksPerHour
             + static_cast<long long>(minute) * System::TimeSpan::TicksPerMinute
             + static_cast<long long>(second) * System::TimeSpan::TicksPerSecond
             + static_cast<long long>(millisecond) * System::TimeSpan::TicksPerMillisecond;
    }

    /**
     * @brief Faithful C++ port of .NET's internal CalendricalCalculationsHelper
     * (CalendricalCalculationsHelper.cs) -- the astronomical vernal-equinox calculation
     * PersianCalendar relies on. Every constant, coefficient table, and formula below is
     * copied verbatim from that file; only the DateTime/TimeSpan API calls are adapted to
     * this port's equivalents.
     */
    struct AstroHelper {
        static constexpr double FullCircleOfArc = 360.0;
        static constexpr int HalfCircleOfArc = 180;
        static constexpr double TwelveHours = 0.5;
        static constexpr double Noon2000Jan01 = 730120.5;
        static constexpr double MeanTropicalYearInDays = 365.242189;
        static constexpr double MeanSpeedOfSun = MeanTropicalYearInDays / FullCircleOfArc;
        static constexpr double LongitudeSpring = 0.0;
        static constexpr double TwoDegreesAfterSpring = 2.0;
        static constexpr int DaysInUniformLengthCentury = 36525;
        static constexpr double SecondsPerDay = 86400.0;

        static long long GetNumberOfDays(const System::DateTime& date) {
            return date.getTicksProperty() / System::DateTime::TicksPerDay;
        }

        static long long StartOf1810() {
            static const long long v = GetNumberOfDays(System::DateTime(1810, 1, 1));
            return v;
        }
        static long long StartOf1900Century() {
            static const long long v = GetNumberOfDays(System::DateTime(1900, 1, 1));
            return v;
        }

        static double RadiansFromDegrees(double degree) { return degree * System::Math::PI / 180.0; }
        static double SinOfDegree(double degree) { return std::sin(RadiansFromDegrees(degree)); }
        static double CosOfDegree(double degree) { return std::cos(RadiansFromDegrees(degree)); }
        static double TanOfDegree(double degree) { return std::tan(RadiansFromDegrees(degree)); }

        static double PolynomialSum(const std::vector<double>& coefficients, double indeterminate) {
            double sum = coefficients[0];
            double indeterminateRaised = 1;
            for (size_t i = 1; i < coefficients.size(); ++i) {
                indeterminateRaised *= indeterminate;
                sum += coefficients[i] * indeterminateRaised;
            }
            return sum;
        }

        static double Obliquity(double julianCenturies) {
            static const std::vector<double> Coefficients =
                {23.43929111111111, -0.013004166666666667, -1.638888888888889E-07, 5.03611111111111E-07};
            return PolynomialSum(Coefficients, julianCenturies);
        }

        static int GetGregorianYear(double numberOfDays) {
            long long ticks = std::min(static_cast<long long>(std::floor(numberOfDays)) * System::DateTime::TicksPerDay,
                                        static_cast<long long>(System::DateTime::MaxValue.getTicksProperty()));
            return System::DateTime(ticks).getYearProperty();
        }

        static double CenturiesFrom1900(int gregorianYear) {
            long long july1stOfYear = GetNumberOfDays(System::DateTime(gregorianYear, 7, 1));
            return static_cast<double>(july1stOfYear - StartOf1900Century()) / DaysInUniformLengthCentury;
        }

        static double DefaultEphemerisCorrection(int gregorianYear) {
            long long january1stOfYear = GetNumberOfDays(System::DateTime(gregorianYear, 1, 1));
            double daysSinceStartOf1810 = static_cast<double>(january1stOfYear - StartOf1810());
            double x = TwelveHours + daysSinceStartOf1810;
            return ((x * x / 41048480.0) - 15.0) / SecondsPerDay;
        }

        static double EphemerisCorrection1988to2019(int gregorianYear) {
            return static_cast<double>(gregorianYear - 1933) / SecondsPerDay;
        }

        static double EphemerisCorrection1900to1987(int gregorianYear) {
            static const std::vector<double> Coefficients1900to1987 =
                {-0.00002, 0.000297, 0.025184, -0.181133, 0.553040, -0.861938, 0.677066, -0.212591};
            return PolynomialSum(Coefficients1900to1987, CenturiesFrom1900(gregorianYear));
        }

        static double EphemerisCorrection1800to1899(int gregorianYear) {
            static const std::vector<double> Coefficients1800to1899 =
                {-0.000009, 0.003844, 0.083563, 0.865736, 4.867575, 15.845535, 31.332267,
                 38.291999, 28.316289, 11.636204, 2.043794};
            return PolynomialSum(Coefficients1800to1899, CenturiesFrom1900(gregorianYear));
        }

        static double EphemerisCorrection1700to1799(int gregorianYear) {
            static const std::vector<double> Coefficients1700to1799 =
                {8.118780842, -0.005092142, 0.003336121, -0.0000266484};
            return PolynomialSum(Coefficients1700to1799, gregorianYear - 1700) / SecondsPerDay;
        }

        static double EphemerisCorrection1620to1699(int gregorianYear) {
            static const std::vector<double> Coefficients1620to1699 = {196.58333, -4.0675, 0.0219167};
            return PolynomialSum(Coefficients1620to1699, gregorianYear - 1600) / SecondsPerDay;
        }

        // ephemeris-correction: correction to account for the slowing down of the rotation of the earth
        static double EphemerisCorrection(double time) {
            int year = GetGregorianYear(time);
            if (year >= 2020) return DefaultEphemerisCorrection(year);
            if (year >= 1988) return EphemerisCorrection1988to2019(year);
            if (year >= 1900) return EphemerisCorrection1900to1987(year);
            if (year >= 1800) return EphemerisCorrection1800to1899(year);
            if (year >= 1700) return EphemerisCorrection1700to1799(year);
            if (year >= 1620) return EphemerisCorrection1620to1699(year);
            return DefaultEphemerisCorrection(year);
        }

        static double JulianCenturies(double moment) {
            double dynamicalMoment = moment + EphemerisCorrection(moment);
            return (dynamicalMoment - Noon2000Jan01) / DaysInUniformLengthCentury;
        }

        // equation-of-time; approximate the difference between apparent solar time and mean solar time
        static double EquationOfTime(double time) {
            double julianCenturies = JulianCenturies(time);
            static const std::vector<double> LambdaCoefficients = {280.46645, 36000.76983, 0.0003032};
            static const std::vector<double> AnomalyCoefficients = {357.52910, 35999.05030, -0.0001559, -0.00000048};
            static const std::vector<double> EccentricityCoefficients = {0.016708617, -0.000042037, -0.0000001236};
            double lambda = PolynomialSum(LambdaCoefficients, julianCenturies);
            double anomaly = PolynomialSum(AnomalyCoefficients, julianCenturies);
            double eccentricity = PolynomialSum(EccentricityCoefficients, julianCenturies);

            double epsilon = Obliquity(julianCenturies);
            double tanHalfEpsilon = TanOfDegree(epsilon / 2);
            double y = tanHalfEpsilon * tanHalfEpsilon;

            double dividend = (y * SinOfDegree(2 * lambda))
                - (2 * eccentricity * SinOfDegree(anomaly))
                + (4 * eccentricity * y * SinOfDegree(anomaly) * CosOfDegree(2 * lambda))
                - (0.5 * y * y * SinOfDegree(4 * lambda))
                - (1.25 * eccentricity * eccentricity * SinOfDegree(2 * anomaly));
            const double Divisor = 2 * System::Math::PI;
            double equation = dividend / Divisor;

            double absEq = std::min(std::fabs(equation), TwelveHours);
            return std::copysign(absEq, equation);
        }

        static double AsDayFraction(double longitude) { return longitude / FullCircleOfArc; }

        static double AsLocalTime(double apparentMidday, double longitude) {
            double universalTime = apparentMidday - AsDayFraction(longitude);
            return apparentMidday - EquationOfTime(universalTime);
        }

        // midday
        static double Midday(double date, double longitude) {
            return AsLocalTime(date + TwelveHours, longitude) - AsDayFraction(longitude);
        }

        static double Reminder(double divisor, double dividend) {
            double whole = std::floor(divisor / dividend);
            return divisor - (dividend * whole);
        }

        static double NormalizeLongitude(double longitude) {
            longitude = Reminder(longitude, FullCircleOfArc);
            if (longitude < 0) longitude += FullCircleOfArc;
            return longitude;
        }

        static double InitLongitude(double longitude) {
            return NormalizeLongitude(longitude + HalfCircleOfArc) - HalfCircleOfArc;
        }

        // midday-in-tehran
        static double MiddayAtPersianObservationSite(double date) {
            return Midday(date, InitLongitude(52.5)); // 52.5 degrees east - Iran Standard Time longitude
        }

        static double PeriodicTerm(double julianCenturies, int x, double y, double z) {
            return x * SinOfDegree(y + z * julianCenturies);
        }

        static double SumLongSequenceOfPeriodicTerms(double julianCenturies) {
            double sum = 0.0;
            sum += PeriodicTerm(julianCenturies, 403406, 270.54861, 0.9287892);
            sum += PeriodicTerm(julianCenturies, 195207, 340.19128, 35999.1376958);
            sum += PeriodicTerm(julianCenturies, 119433, 63.91854, 35999.4089666);
            sum += PeriodicTerm(julianCenturies, 112392, 331.2622, 35998.7287385);
            sum += PeriodicTerm(julianCenturies, 3891, 317.843, 71998.20261);
            sum += PeriodicTerm(julianCenturies, 2819, 86.631, 71998.4403);
            sum += PeriodicTerm(julianCenturies, 1721, 240.052, 36000.35726);
            sum += PeriodicTerm(julianCenturies, 660, 310.26, 71997.4812);
            sum += PeriodicTerm(julianCenturies, 350, 247.23, 32964.4678);
            sum += PeriodicTerm(julianCenturies, 334, 260.87, -19.441);
            sum += PeriodicTerm(julianCenturies, 314, 297.82, 445267.1117);
            sum += PeriodicTerm(julianCenturies, 268, 343.14, 45036.884);
            sum += PeriodicTerm(julianCenturies, 242, 166.79, 3.1008);
            sum += PeriodicTerm(julianCenturies, 234, 81.53, 22518.4434);
            sum += PeriodicTerm(julianCenturies, 158, 3.5, -19.9739);
            sum += PeriodicTerm(julianCenturies, 132, 132.75, 65928.9345);
            sum += PeriodicTerm(julianCenturies, 129, 182.95, 9038.0293);
            sum += PeriodicTerm(julianCenturies, 114, 162.03, 3034.7684);
            sum += PeriodicTerm(julianCenturies, 99, 29.8, 33718.148);
            sum += PeriodicTerm(julianCenturies, 93, 266.4, 3034.448);
            sum += PeriodicTerm(julianCenturies, 86, 249.2, -2280.773);
            sum += PeriodicTerm(julianCenturies, 78, 157.6, 29929.992);
            sum += PeriodicTerm(julianCenturies, 72, 257.8, 31556.493);
            sum += PeriodicTerm(julianCenturies, 68, 185.1, 149.588);
            sum += PeriodicTerm(julianCenturies, 64, 69.9, 9037.75);
            sum += PeriodicTerm(julianCenturies, 46, 8.0, 107997.405);
            sum += PeriodicTerm(julianCenturies, 38, 197.1, -4444.176);
            sum += PeriodicTerm(julianCenturies, 37, 250.4, 151.771);
            sum += PeriodicTerm(julianCenturies, 32, 65.3, 67555.316);
            sum += PeriodicTerm(julianCenturies, 29, 162.7, 31556.08);
            sum += PeriodicTerm(julianCenturies, 28, 341.5, -4561.54);
            sum += PeriodicTerm(julianCenturies, 27, 291.6, 107996.706);
            sum += PeriodicTerm(julianCenturies, 27, 98.5, 1221.655);
            sum += PeriodicTerm(julianCenturies, 25, 146.7, 62894.167);
            sum += PeriodicTerm(julianCenturies, 24, 110.0, 31437.369);
            sum += PeriodicTerm(julianCenturies, 21, 5.2, 14578.298);
            sum += PeriodicTerm(julianCenturies, 21, 342.6, -31931.757);
            sum += PeriodicTerm(julianCenturies, 20, 230.9, 34777.243);
            sum += PeriodicTerm(julianCenturies, 18, 256.1, 1221.999);
            sum += PeriodicTerm(julianCenturies, 17, 45.3, 62894.511);
            sum += PeriodicTerm(julianCenturies, 14, 242.9, -4442.039);
            sum += PeriodicTerm(julianCenturies, 13, 115.2, 107997.909);
            sum += PeriodicTerm(julianCenturies, 13, 151.8, 119.066);
            sum += PeriodicTerm(julianCenturies, 13, 285.3, 16859.071);
            sum += PeriodicTerm(julianCenturies, 12, 53.3, -4.578);
            sum += PeriodicTerm(julianCenturies, 10, 126.6, 26895.292);
            sum += PeriodicTerm(julianCenturies, 10, 205.7, -39.127);
            sum += PeriodicTerm(julianCenturies, 10, 85.9, 12297.536);
            sum += PeriodicTerm(julianCenturies, 10, 146.1, 90073.778);
            return sum;
        }

        static double Aberration(double julianCenturies) {
            return (0.0000974 * CosOfDegree(177.63 + (35999.01848 * julianCenturies))) - 0.005575;
        }

        static double Nutation(double julianCenturies) {
            static const std::vector<double> CoefficientsA = {124.90, -1934.134, 0.002063};
            static const std::vector<double> CoefficientsB = {201.11, 72001.5377, 0.00057};
            double a = PolynomialSum(CoefficientsA, julianCenturies);
            double b = PolynomialSum(CoefficientsB, julianCenturies);
            return (-0.004778 * SinOfDegree(a)) - (0.0003667 * SinOfDegree(b));
        }

        static double Compute(double time) {
            double julianCenturies = JulianCenturies(time);
            double lambda = 282.7771834
                + (36000.76953744 * julianCenturies)
                + (0.000005729577951308232 * SumLongSequenceOfPeriodicTerms(julianCenturies));

            double longitude = lambda + Aberration(julianCenturies) + Nutation(julianCenturies);
            return InitLongitude(longitude);
        }

        static double AsSeason(double longitude) { return (longitude < 0) ? (longitude + FullCircleOfArc) : longitude; }

        static double EstimatePrior(double longitude, double time) {
            double timeSunLastAtLongitude = time - (MeanSpeedOfSun * AsSeason(InitLongitude(Compute(time) - longitude)));
            double longitudeErrorDelta = InitLongitude(Compute(timeSunLastAtLongitude) - longitude);
            return std::min(time, timeSunLastAtLongitude - (MeanSpeedOfSun * longitudeErrorDelta));
        }

        // persian-new-year-on-or-before
        //  number of days is the absolute date. The absolute date is the number of days from January 1st, 1 A.D.
        //  1/1/0001 is absolute date 1.
        static long long PersianNewYearOnOrBefore(long long numberOfDays) {
            double date = static_cast<double>(numberOfDays);

            double approx = EstimatePrior(LongitudeSpring, MiddayAtPersianObservationSite(date));
            long long lowerBoundNewYearDay = static_cast<long long>(std::floor(approx)) - 1;
            long long upperBoundNewYearDay = lowerBoundNewYearDay + 3;
            long long day = lowerBoundNewYearDay;
            for (; day != upperBoundNewYearDay; ++day) {
                double midday = MiddayAtPersianObservationSite(static_cast<double>(day));
                double l = Compute(midday);
                if (LongitudeSpring <= l && l <= TwoDegreesAfterSpring) break;
            }
            return day - 1;
        }
    };

    static long long GetAbsoluteDatePersian(int year, int month, int day) {
        if (year < 1 || year > MaxCalendarYear || month < 1 || month > 12)
            throw System::ArgumentOutOfRangeException("year",
                "Year, month, and day parameters describe an un-representable DateTime.");

        int ordinalDay = DaysInPreviousMonths(month) + day - 1;
        int approximateDaysFromEpochForYearStart = static_cast<int>(AstroHelper::MeanTropicalYearInDays * (year - 1));
        long long yearStart = AstroHelper::PersianNewYearOnOrBefore(
            PersianEpoch() + approximateDaysFromEpochForYearStart + ApproximateHalfYear);
        yearStart += ordinalDay;
        return yearStart;
    }

    // Faithful port of PersianCalendar.cs's GetDate (computes year/month/day/ordinalDay
    // together instead of via four separate GetDatePart(ticks, part) calls -- purely a
    // C++-side simplification since all four share this exact computation; not a logic
    // change).
    void GetDate(long long ticks, int& year, int& month, int& day, int& ordinalDay) const {
        CheckTicksRange(ticks);

        long long numDays = ticks / System::DateTime::TicksPerDay + 1;

        long long yearStart = AstroHelper::PersianNewYearOnOrBefore(numDays);
        year = static_cast<int>(std::floor(
            (static_cast<double>(yearStart - PersianEpoch()) / AstroHelper::MeanTropicalYearInDays) + 0.5)) + 1;

        ordinalDay = static_cast<int>(numDays - AstroHelper::GetNumberOfDays(ToDateTime(year, 1, 1, 0, 0, 0, 0, PersianEra)));

        month = MonthFromOrdinalDay(ordinalDay);
        day = ordinalDay - DaysInPreviousMonths(month);
    }
};

} // namespace System::Globalization
