// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/DateTime.hpp"
#include "System/DayOfWeek.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Globalization/CalendarAlgorithmType.hpp"
#include "System/Globalization/CalendarWeekRule.hpp"

namespace System::Globalization {

using SharpRuntime::intcs;

/**
 * @brief Represents time in divisions such as weeks, months, and years.
 *
 * C++ counterpart of .NET System.Globalization.Calendar.
 * Abstract base class for all calendar implementations. Provides virtual default
 * implementations that delegate to the Gregorian calendar rules; subclasses override
 * as needed for other calendar systems.
 */
class Calendar {
    mutable intcs twoDigitYearMax_ = -1;

protected:
    /**
     * @brief Throws InvalidOperationException if this calendar instance is read-only.
     *
     * C++ counterpart of .NET Calendar.VerifyWritable(). Called before mutating any
     * writable calendar state (currently just TwoDigitYearMax).
     */
    void VerifyWritable() const {
        if (getIsReadOnlyProperty()) {
            throw System::InvalidOperationException("Instance is read-only.");
        }
    }

public:
    /** @brief Constant representing the current era. */
    static constexpr intcs CurrentEra = 0;

    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~Calendar() = default;

    /**
     * @brief Gets the algorithm type for this calendar.
     *
     * C++ counterpart of .NET Calendar.AlgorithmType. This is the default value returned
     * by the Calendar base class; concrete calendar subclasses override it with their
     * actual algorithm basis (solar/lunar/lunisolar).
     * @return CalendarAlgorithmType::Unknown by default.
     */
    [[nodiscard]] virtual CalendarAlgorithmType getAlgorithmTypeProperty() const {
        return CalendarAlgorithmType::Unknown;
    }

    /**
     * @brief Gets a value indicating whether this calendar is read-only.
     *
     * C++ counterpart of .NET Calendar.IsReadOnly.
     * @return Always false in the base implementation.
     */
    [[nodiscard]] virtual bool getIsReadOnlyProperty() const { return false; }

    /**
     * @brief Gets the maximum year that can be represented by a two-digit year value.
     *
     * C++ counterpart of .NET Calendar.TwoDigitYearMax. Lazily defaults to 2049 (the
     * Gregorian calendar's default) on first access, matching this base class's Gregorian
     * fallback behaviour elsewhere (IsLeapYear, GetDaysInMonth). Calendar-specific
     * subclasses override this default to match their own .NET defaults.
     */
    [[nodiscard]] virtual intcs getTwoDigitYearMaxProperty() const {
        if (twoDigitYearMax_ == -1) twoDigitYearMax_ = 2049;
        return twoDigitYearMax_;
    }

    /**
     * @brief Sets the maximum year that can be represented by a two-digit year value.
     *
     * C++ counterpart of .NET Calendar.TwoDigitYearMax (setter).
     * @throws System::InvalidOperationException if this calendar instance is read-only.
     */
    virtual void setTwoDigitYearMaxProperty(intcs value) {
        VerifyWritable();
        twoDigitYearMax_ = value;
    }

    /**
     * @brief Gets the list of era identifiers supported by this calendar.
     *
     * C++ counterpart of .NET Calendar.Eras.
     * @return A vector containing {CurrentEra}.
     */
    [[nodiscard]] virtual std::vector<intcs> getErasProperty() const { return {CurrentEra}; }

    /**
     * @brief Gets the earliest date and time supported by this calendar.
     *
     * C++ counterpart of .NET Calendar.MinSupportedDateTime.
     * @return DateTime(1, 1, 1) by default.
     */
    [[nodiscard]] virtual System::DateTime getMinSupportedDateTimeProperty() const {
        return System::DateTime(1, 1, 1);
    }

    /**
     * @brief Gets the latest date and time supported by this calendar.
     *
     * C++ counterpart of .NET Calendar.MaxSupportedDateTime.
     * @return DateTime(9999, 12, 31) by default.
     */
    [[nodiscard]] virtual System::DateTime getMaxSupportedDateTimeProperty() const {
        return System::DateTime(9999, 12, 31);
    }

    /**
     * @brief Returns the year component of the given DateTime in this calendar.
     *
     * C++ counterpart of .NET Calendar.GetYear(DateTime).
     * @param time The DateTime to extract the year from.
     * @return The year component.
     */
    [[nodiscard]] virtual intcs GetYear(const System::DateTime& time) const {
        return time.getYearProperty();
    }

    /**
     * @brief Returns the month component of the given DateTime in this calendar.
     *
     * C++ counterpart of .NET Calendar.GetMonth(DateTime).
     * @param time The DateTime to extract the month from.
     * @return The month component (1–12).
     */
    [[nodiscard]] virtual intcs GetMonth(const System::DateTime& time) const {
        return time.getMonthProperty();
    }

    /**
     * @brief Returns the day-of-month for the given DateTime.
     *
     * C++ counterpart of .NET Calendar.GetDayOfMonth(DateTime).
     * @param time The DateTime to query.
     * @return The day-of-month (1–31).
     */
    [[nodiscard]] virtual intcs GetDayOfMonth(const System::DateTime& time) const {
        return time.getDayProperty();
    }

    /**
     * @brief Returns the day-of-week for the given DateTime.
     *
     * C++ counterpart of .NET Calendar.GetDayOfWeek(DateTime).
     * @param time The DateTime to query.
     * @return The DayOfWeek value.
     */
    [[nodiscard]] virtual System::DayOfWeek GetDayOfWeek(const System::DateTime& time) const {
        return time.getDayOfWeekProperty();
    }

    /**
     * @brief Returns the day-of-year for the given DateTime.
     *
     * C++ counterpart of .NET Calendar.GetDayOfYear(DateTime).
     * @param time The DateTime to query.
     * @return The day-of-year (1–366).
     */
    [[nodiscard]] virtual intcs GetDayOfYear(const System::DateTime& time) const {
        return time.getDayOfYearProperty();
    }

    /**
     * @brief Returns the hour component of the given DateTime.
     *
     * C++ counterpart of .NET Calendar.GetHour(DateTime).
     * @param time The DateTime to query.
     * @return The hour component (0–23).
     */
    [[nodiscard]] virtual intcs GetHour(const System::DateTime& time) const {
        return time.getHourProperty();
    }

    /**
     * @brief Returns the minute component of the given DateTime.
     *
     * C++ counterpart of .NET Calendar.GetMinute(DateTime).
     * @param time The DateTime to query.
     * @return The minute component (0–59).
     */
    [[nodiscard]] virtual intcs GetMinute(const System::DateTime& time) const {
        return time.getMinuteProperty();
    }

    /**
     * @brief Returns the second component of the given DateTime.
     *
     * C++ counterpart of .NET Calendar.GetSecond(DateTime).
     * @param time The DateTime to query.
     * @return The second component (0–59).
     */
    [[nodiscard]] virtual intcs GetSecond(const System::DateTime& time) const {
        return time.getSecondProperty();
    }

    /**
     * @brief Returns the millisecond component of the given DateTime.
     *
     * C++ counterpart of .NET Calendar.GetMilliseconds(DateTime).
     * @param time The DateTime to query.
     * @return The millisecond component (0–999) as a double.
     */
    [[nodiscard]] virtual double GetMilliseconds(const System::DateTime& time) const {
        return static_cast<double>(time.getMillisecondProperty());
    }

    /**
     * @brief Returns the era for the given DateTime.
     *
     * C++ counterpart of .NET Calendar.GetEra(DateTime).
     * @param time The DateTime to query.
     * @return Always CurrentEra (0) in the base implementation.
     */
    [[nodiscard]] virtual intcs GetEra(const System::DateTime& /*time*/) const { return CurrentEra; }

    /**
     * @brief Returns the number of eras in this calendar.
     *
     * Helper method not present in .NET API; use getErasProperty().size() for .NET compatibility.
     * @return Always 1 in the base implementation.
     */
    [[nodiscard]] virtual intcs GetErasCount() const { return 1; }

    /**
     * @brief Returns the number of months in the specified year and era.
     *
     * C++ counterpart of .NET Calendar.GetMonthsInYear(int, int).
     * @param year The year.
     * @param era  The era (default CurrentEra).
     * @return Always 12 in the base (Gregorian) implementation.
     */
    [[nodiscard]] virtual intcs GetMonthsInYear(intcs /*year*/, intcs /*era*/ = CurrentEra) const {
        return 12;
    }

    /**
     * @brief Returns the leap month for the specified year and era, or 0 if none.
     *
     * C++ counterpart of .NET Calendar.GetLeapMonth(int, int).
     * @param year The year.
     * @param era  The era (default CurrentEra).
     * @return Always 0 in the base implementation (no leap month in Gregorian).
     */
    [[nodiscard]] virtual intcs GetLeapMonth(intcs /*year*/, intcs /*era*/ = CurrentEra) const {
        return 0;
    }

    /**
     * @brief Determines whether the specified year is a leap year.
     *
     * C++ counterpart of .NET Calendar.IsLeapYear(int, int).
     * @param year The year to check.
     * @param era  The era (default CurrentEra).
     * @return true if @p year is a leap year; otherwise false.
     */
    [[nodiscard]] virtual bool IsLeapYear(intcs year, intcs /*era*/ = CurrentEra) const {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    /**
     * @brief Determines whether the specified month in the specified year is a leap month.
     *
     * C++ counterpart of .NET Calendar.IsLeapMonth(int, int, int).
     * @param year  The year.
     * @param month The month (1–12).
     * @param era   The era (default CurrentEra).
     * @return Always false in the base (Gregorian) implementation.
     */
    [[nodiscard]] virtual bool IsLeapMonth(intcs /*year*/, intcs /*month*/, intcs /*era*/ = CurrentEra) const {
        return false;
    }

    /**
     * @brief Determines whether the specified date is a leap day.
     *
     * C++ counterpart of .NET Calendar.IsLeapDay(int, int, int, int).
     * @param year  The year.
     * @param month The month (1–12).
     * @param day   The day (1–31).
     * @param era   The era (default CurrentEra).
     * @return true if the date is February 29 in a leap year; otherwise false.
     */
    [[nodiscard]] virtual bool IsLeapDay(intcs year, intcs month, intcs day, intcs era = CurrentEra) const {
        return month == 2 && day == 29 && IsLeapYear(year, era);
    }

    /**
     * @brief Returns the number of days in the specified month, year, and era.
     *
     * C++ counterpart of .NET Calendar.GetDaysInMonth(int, int, int), whose base
     * implementation delegates to DateTime.DaysInMonth(year, month), which validates month
     * before indexing (DateTime.cs: `if (month < 1 || month > 12) Throw...`).
     * @param year  The year.
     * @param month The month (1–12).
     * @param era   The era (default CurrentEra).
     * @return The number of days in the specified month.
     * @throws System::ArgumentOutOfRangeException if @p month is not in [1, 12]. Without this
     *         check, indexing the internal days-per-month table with an out-of-range month is
     *         undefined behavior, not a thrown exception.
     */
    [[nodiscard]] virtual intcs GetDaysInMonth(intcs year, intcs month, intcs /*era*/ = CurrentEra) const {
        static const intcs days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (month < 1 || month > 12) throw System::ArgumentOutOfRangeException("month");
        if (month == 2 && IsLeapYear(year)) return 29;
        return days[month];
    }

    /**
     * @brief Returns the number of days in the specified year and era.
     *
     * C++ counterpart of .NET Calendar.GetDaysInYear(int, int).
     * @param year The year.
     * @param era  The era (default CurrentEra).
     * @return 366 for leap years, 365 otherwise.
     */
    [[nodiscard]] virtual intcs GetDaysInYear(intcs year, intcs era = CurrentEra) const {
        return IsLeapYear(year, era) ? 366 : 365;
    }

private:
    /** @brief C++ counterpart of .NET Calendar.GetFirstDayWeekOfYear(DateTime, int) (internal helper for CalendarWeekRule::FirstDay). */
    [[nodiscard]] intcs GetFirstDayWeekOfYear(const System::DateTime& time, intcs firstDayOfWeek) const {
        intcs dayOfYear = GetDayOfYear(time) - 1;
        intcs dayForJan1 = static_cast<intcs>(GetDayOfWeek(time)) - (dayOfYear % 7);
        intcs offset = (dayForJan1 - firstDayOfWeek + 14) % 7;
        return (dayOfYear + offset) / 7 + 1;
    }

    /** @brief C++ counterpart of .NET Calendar.GetWeekOfYearOfMinSupportedDateTime(int, int). */
    [[nodiscard]] intcs GetWeekOfYearOfMinSupportedDateTime(intcs firstDayOfWeek, intcs minimumDaysInFirstWeek) const {
        const System::DateTime minSupported = getMinSupportedDateTimeProperty();
        intcs dayOfYear = GetDayOfYear(minSupported) - 1;
        intcs dayOfWeekOfFirstOfYear = static_cast<intcs>(GetDayOfWeek(minSupported)) - dayOfYear % 7;

        intcs offset = (firstDayOfWeek + 7 - dayOfWeekOfFirstOfYear) % 7;
        if (offset == 0 || offset >= minimumDaysInFirstWeek) {
            return 1;
        }

        intcs daysInYearBeforeMinSupportedYear = getDaysInYearBeforeMinSupportedYearProperty() - 1;
        intcs dayOfWeekOfFirstOfPreviousYear = dayOfWeekOfFirstOfYear - 1 - (daysInYearBeforeMinSupportedYear % 7);

        intcs daysInInitialPartialWeek = (firstDayOfWeek - dayOfWeekOfFirstOfPreviousYear + 14) % 7;
        intcs day = daysInYearBeforeMinSupportedYear - daysInInitialPartialWeek;
        if (daysInInitialPartialWeek >= minimumDaysInFirstWeek) {
            day += 7;
        }

        return day / 7 + 1;
    }

    /** @brief C++ counterpart of .NET Calendar.GetWeekOfYearFullDays(DateTime, int, int) (internal helper for FirstFullWeek/FirstFourDayWeek). */
    [[nodiscard]] intcs GetWeekOfYearFullDays(const System::DateTime& time, intcs firstDayOfWeek, intcs fullDays) const {
        intcs dayOfYear = GetDayOfYear(time) - 1;
        intcs dayForJan1 = static_cast<intcs>(GetDayOfWeek(time)) - (dayOfYear % 7);

        intcs offset = (firstDayOfWeek - dayForJan1 + 14) % 7;
        if (offset != 0 && offset >= fullDays) {
            offset -= 7;
        }

        intcs day = dayOfYear - offset;
        if (day >= 0) {
            return day / 7 + 1;
        }

        if (time <= AddDays(getMinSupportedDateTimeProperty(), dayOfYear)) {
            return GetWeekOfYearOfMinSupportedDateTime(firstDayOfWeek, fullDays);
        }

        return GetWeekOfYearFullDays(AddDays(time, -(dayOfYear + 1)), firstDayOfWeek, fullDays);
    }

protected:
    /**
     * @brief Number of days in the year immediately preceding MinSupportedDateTime's year.
     *
     * C++ counterpart of .NET Calendar.DaysInYearBeforeMinSupportedYear.
     * @return 365 by default; overridden by calendars whose minimum year is a leap year
     *         or otherwise irregular.
     */
    [[nodiscard]] virtual intcs getDaysInYearBeforeMinSupportedYearProperty() const { return 365; }

public:
    /**
     * @brief Returns the week-of-year for the given DateTime using the specified rule.
     *
     * C++ counterpart of .NET Calendar.GetWeekOfYear(DateTime, CalendarWeekRule, DayOfWeek).
     * @param time           The DateTime to query.
     * @param rule           The rule that defines the first week of the year.
     * @param firstDayOfWeek The first day of the week.
     * @return The week number (1-based).
     * @throws System::ArgumentOutOfRangeException if @p firstDayOfWeek or @p rule is out of range.
     */
    [[nodiscard]] virtual intcs GetWeekOfYear(const System::DateTime& time,
                                             CalendarWeekRule rule,
                                             System::DayOfWeek firstDayOfWeek) const {
        if (firstDayOfWeek < System::DayOfWeek::Sunday || firstDayOfWeek > System::DayOfWeek::Saturday) {
            throw System::ArgumentOutOfRangeException("firstDayOfWeek",
                std::to_string(static_cast<intcs>(firstDayOfWeek)),
                "Value must be within the range for the DayOfWeek enumeration.");
        }

        switch (rule) {
            case CalendarWeekRule::FirstDay:
                return GetFirstDayWeekOfYear(time, static_cast<intcs>(firstDayOfWeek));
            case CalendarWeekRule::FirstFullWeek:
                return GetWeekOfYearFullDays(time, static_cast<intcs>(firstDayOfWeek), 7);
            case CalendarWeekRule::FirstFourDayWeek:
                return GetWeekOfYearFullDays(time, static_cast<intcs>(firstDayOfWeek), 4);
            default:
                throw System::ArgumentOutOfRangeException("rule",
                    std::to_string(static_cast<intcs>(rule)),
                    "Value must be within the range for the CalendarWeekRule enumeration.");
        }
    }

    /**
     * @brief Returns a DateTime that is the specified number of years from the given DateTime.
     *
     * C++ counterpart of .NET Calendar.AddYears(DateTime, int). Real .NET's GregorianCalendar
     * implements this as `AddMonths(time, years * 12)` (GregorianCalendar.cs) specifically so
     * that a Feb 29 source date lands on Feb 28 in a non-leap target year, via AddMonths'
     * `if (d > days) d = days;` clamp -- constructing the result year/month/day directly, as
     * this method previously did, skips that clamp and throws instead (DateTime's constructor
     * rejects Feb 29 in a non-leap year) where .NET clamps silently.
     * @param time  The starting DateTime.
     * @param years The number of years to add.
     * @throws System::ArgumentOutOfRangeException if @p years is outside [-10000, 10000].
     * @return A new DateTime offset by @p years, with the day clamped if the target month is
     *         shorter than the source day (e.g. Feb 29 -> Feb 28 in a non-leap year).
     *
     * `years * 12` computed directly with no upfront bounds check is real signed-integer-
     * overflow UB in C++ for a merely large (not extreme) years argument -- unlike real .NET,
     * where the identical `AddMonths(time, years * 12)` expression relies on C#'s well-defined
     * unchecked-arithmetic wraparound (confirmed via UBSan). AddMonths (below) already rejects
     * |months| > 120000 before doing arithmetic, so validating years against the equivalent
     * bound (120000 / 12 = 10000) here, before the multiply, rejects nothing AddMonths would
     * have accepted anyway -- it only moves the rejection earlier, before the overflow-prone
     * multiplication. This exact bug class was already fixed once in this same file (see
     * AddMonths's own doc comment below) but missed here in the sibling method.
     */
    virtual System::DateTime AddYears(const System::DateTime& time, intcs years) const {
        if (years < -10000 || years > 10000)
            throw System::ArgumentOutOfRangeException("years", std::to_string(years),
                "Valid values are between -10000 and 10000, inclusive.");
        return AddMonths(time, years * 12);
    }

    /**
     * @brief Returns a DateTime that is the specified number of months from the given DateTime.
     *
     * C++ counterpart of .NET Calendar.AddMonths(DateTime, int). Verified against
     * GregorianCalendar.cs's AddMonths, which rejects |months| > 120000 before doing any
     * arithmetic: without that check, a caller-supplied @p months near INT_MAX/INT_MIN makes
     * `(year-1)*12 + (month-1) + months` overflow signed 32-bit int -- confirmed via a standalone
     * UBSan repro that AddMonths(<year 9999 date>, INT_MAX) silently produced a nonsensical
     * negative "year" instead of throwing, rather than the clean, documented exception .NET's
     * bounds check produces. 120000 months (10000 years) safely covers every representable
     * DateTime year range with no realistic legitimate use case beyond it.
     * @param time   The starting DateTime.
     * @param months The number of months to add.
     * @return A new DateTime offset by @p months.
     * @throws System::ArgumentOutOfRangeException if @p months is outside [-120000, 120000].
     */
    virtual System::DateTime AddMonths(const System::DateTime& time, intcs months) const {
        if (months < -120000 || months > 120000)
            throw System::ArgumentOutOfRangeException("months", std::to_string(months),
                "Valid values are between -120000 and 120000, inclusive.");
        intcs totalMonths = (time.getYearProperty() - 1) * 12 + (time.getMonthProperty() - 1) + months;
        intcs year  = totalMonths / 12 + 1;
        intcs month = totalMonths % 12 + 1;
        intcs day   = std::min(time.getDayProperty(), GetDaysInMonth(year, month));
        return System::DateTime(year, month, day,
                                time.getHourProperty(), time.getMinuteProperty(),
                                time.getSecondProperty());
    }

    /**
     * @brief Returns a DateTime that is the specified number of weeks from the given DateTime.
     *
     * C++ counterpart of .NET Calendar.AddWeeks(DateTime, int). Real .NET's implementation is
     * `AddDays(time, weeks * 7)` with no upfront check, relying on C#'s well-defined unchecked-
     * arithmetic wraparound for `weeks * 7`. That expression is real signed-integer-overflow UB
     * in C++ for a merely large (not extreme) weeks argument -- same bug class as AddYears
     * above. Computes the product in a wider type first so the multiplication itself can never
     * overflow, then lets DateTime::AddDays's own bounds check (already validated against
     * DateTime::MaxDays) reject anything outside DateTime's representable range, rather than
     * duplicating that bound here.
     * @param time  The starting DateTime.
     * @param weeks The number of weeks to add.
     * @throws System::ArgumentOutOfRangeException if `weeks * 7` doesn't fit in a 32-bit int.
     * @return A new DateTime offset by @p weeks * 7 days.
     */
    virtual System::DateTime AddWeeks(const System::DateTime& time, intcs weeks) const {
        long long totalDays = static_cast<long long>(weeks) * 7;
        if (totalDays < std::numeric_limits<intcs>::min() || totalDays > std::numeric_limits<intcs>::max())
            throw System::ArgumentOutOfRangeException("weeks", std::to_string(weeks),
                "Value was either too large or too small for an Int32.");
        return time.AddDays(static_cast<intcs>(totalDays));
    }

    /**
     * @brief Returns a DateTime that is the specified number of days from the given DateTime.
     *
     * C++ counterpart of .NET Calendar.AddDays(DateTime, int).
     * @param time The starting DateTime.
     * @param days The number of days to add.
     * @return A new DateTime offset by @p days.
     */
    virtual System::DateTime AddDays(const System::DateTime& time, intcs days) const {
        return time.AddDays(days);
    }

    /**
     * @brief Returns a DateTime that is the specified number of hours from the given DateTime.
     *
     * C++ counterpart of .NET Calendar.AddHours(DateTime, int).
     * @param time  The starting DateTime.
     * @param hours The number of hours to add.
     * @return A new DateTime offset by @p hours.
     */
    virtual System::DateTime AddHours(const System::DateTime& time, intcs hours) const {
        return time.AddHours(hours);
    }

    /**
     * @brief Returns a DateTime that is the specified number of minutes from the given DateTime.
     *
     * C++ counterpart of .NET Calendar.AddMinutes(DateTime, int).
     * @param time    The starting DateTime.
     * @param minutes The number of minutes to add.
     * @return A new DateTime offset by @p minutes.
     */
    virtual System::DateTime AddMinutes(const System::DateTime& time, intcs minutes) const {
        return time.AddMinutes(minutes);
    }

    /**
     * @brief Returns a DateTime that is the specified number of seconds from the given DateTime.
     *
     * C++ counterpart of .NET Calendar.AddSeconds(DateTime, int).
     * @param time    The starting DateTime.
     * @param seconds The number of seconds to add.
     * @return A new DateTime offset by @p seconds.
     */
    virtual System::DateTime AddSeconds(const System::DateTime& time, intcs seconds) const {
        return time.AddSeconds(seconds);
    }

    /**
     * @brief Returns a DateTime that is the specified number of milliseconds from the given DateTime.
     *
     * C++ counterpart of .NET Calendar.AddMilliseconds(DateTime, double).
     * @param time         The starting DateTime.
     * @param milliseconds The number of milliseconds to add.
     * @return A new DateTime offset by @p milliseconds.
     */
    virtual System::DateTime AddMilliseconds(const System::DateTime& time, double milliseconds) const {
        return time.AddMilliseconds(static_cast<long long>(milliseconds));
    }

    /**
     * @brief Returns a DateTime from the individual date and time components.
     *
     * C++ counterpart of .NET Calendar.ToDateTime(int, int, int, int, int, int, int, int).
     * @param year        The year.
     * @param month       The month (1–12).
     * @param day         The day (1–31).
     * @param hour        The hour (0–23).
     * @param minute      The minute (0–59).
     * @param second      The second (0–59).
     * @param millisecond The millisecond (0–999).
     * @param era         The era (default CurrentEra; ignored in base implementation).
     * @return The DateTime corresponding to the given components.
     */
    virtual System::DateTime ToDateTime(intcs year, intcs month, intcs day,
                                        intcs hour, intcs minute, intcs second,
                                        intcs millisecond, intcs /*era*/ = CurrentEra) const {
        return System::DateTime(year, month, day, hour, minute, second, millisecond);
    }

    /**
     * @brief Converts a two-digit year to a four-digit year.
     *
     * C++ counterpart of .NET Calendar.ToFourDigitYear(int).
     * Converts the year value to the appropriate century by using the TwoDigitYearMax
     * property. For example, if TwoDigitYearMax is 2049, a two-digit value of 50 maps
     * to 1950, while a two-digit value of 49 maps to 2049. Years >= 100 are returned as-is.
     * @param year The year to convert.
     * @return A four-digit year.
     * @throws System::ArgumentOutOfRangeException if @p year is negative.
     */
    virtual intcs ToFourDigitYear(intcs year) const {
        System::ArgumentOutOfRangeException::ThrowIfNegative(year, "year");
        if (year < 100) {
            intcs max = getTwoDigitYearMaxProperty();
            return (max / 100 - (year > max % 100 ? 1 : 0)) * 100 + year;
        }
        return year;
    }
};

} // namespace System::Globalization
