// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Globalization/Calendar.hpp"

namespace System::Globalization {

using SharpRuntime::intcs;

/**
 * @brief Represents the Taiwan calendar (Republic of China calendar).
 *
 * C++ counterpart of .NET System.Globalization.TaiwanCalendar.
 * The Taiwan calendar uses the same month, day, and time structure as the Gregorian
 * calendar but counts years from the founding of the Republic of China in 1912.
 * The ROC year equals the Gregorian year minus 1911.
 */
class TaiwanCalendar : public Calendar {
public:
    static constexpr intcs TaiwanEra       = 1;    ///< The only era value for this calendar.
    static constexpr intcs GregorianOffset = 1911;  ///< Offset subtracted from Gregorian year to get ROC year.

    /**
     * @brief Gets the algorithm type for this calendar.
     *
     * C++ counterpart of .NET TaiwanCalendar.AlgorithmType.
     * @return Always CalendarAlgorithmType::SolarCalendar.
     */
    [[nodiscard]] CalendarAlgorithmType getAlgorithmTypeProperty() const override {
        return CalendarAlgorithmType::SolarCalendar;
    }

    /**
     * @brief Gets the earliest date supported by TaiwanCalendar.
     *
     * C++ counterpart of .NET TaiwanCalendar.MinSupportedDateTime.
     * @return DateTime(1912, 1, 1) — the first day of the Republic of China calendar.
     */
    [[nodiscard]] System::DateTime getMinSupportedDateTimeProperty() const override {
        return System::DateTime(1912, 1, 1);
    }

    /**
     * @brief Gets the latest date supported by TaiwanCalendar.
     *
     * C++ counterpart of .NET TaiwanCalendar.MaxSupportedDateTime.
     * @return DateTime(9999, 12, 31).
     */
    [[nodiscard]] System::DateTime getMaxSupportedDateTimeProperty() const override {
        return System::DateTime(9999, 12, 31);
    }

    /**
     * @brief Gets the last two-digit year that maps into the range of this calendar.
     *
     * C++ counterpart of .NET TaiwanCalendar.TwoDigitYearMax.
     * @return The maximum two-digit year (default 99).
     */
    [[nodiscard]] intcs getTwoDigitYearMaxProperty() const override { return twoDigitYearMax_; }

    /**
     * @brief Sets the last two-digit year that maps into the range of this calendar.
     *
     * C++ counterpart of .NET TaiwanCalendar.TwoDigitYearMax setter.
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
     * C++ counterpart of .NET TaiwanCalendar.GetEra(DateTime).
     * @return Always TaiwanEra (1).
     */
    [[nodiscard]] intcs GetEra(const System::DateTime& /*time*/) const override { return TaiwanEra; }

    /**
     * @brief Returns the number of eras in this calendar.
     *
     * @return Always 1; the Taiwan calendar has a single era.
     */
    [[nodiscard]] intcs GetErasCount() const override { return 1; }

    /**
     * @brief Gets the list of era identifiers supported by this calendar.
     *
     * C++ counterpart of .NET TaiwanCalendar.Eras.
     * @return A vector containing {TaiwanEra}.
     */
    [[nodiscard]] std::vector<intcs> getErasProperty() const override { return {TaiwanEra}; }

    /**
     * @brief Returns the Republic of China year corresponding to the given DateTime.
     *
     * C++ counterpart of .NET TaiwanCalendar.GetYear(DateTime).
     * The ROC year equals the Gregorian year minus 1911.
     * @param time The Gregorian DateTime to convert.
     * @return The ROC year (Gregorian year - 1911).
     */
    [[nodiscard]] intcs GetYear(const System::DateTime& time) const override {
        return time.getYearProperty() - GregorianOffset;
    }

    /**
     * @brief Determines whether the specified Taiwan year is a leap year.
     *
     * C++ counterpart of .NET TaiwanCalendar.IsLeapYear(int, int).
     * The Taiwan year is converted to the Gregorian year before applying the Gregorian leap rule.
     * @param year Taiwan (ROC) year.
     * @param era  Era (unused; always TaiwanEra).
     * @return true if the corresponding Gregorian year is a leap year.
     */
    [[nodiscard]] bool IsLeapYear(intcs year, intcs /*era*/ = CurrentEra) const override {
        intcs gy = year + GregorianOffset;
        return (gy % 4 == 0 && gy % 100 != 0) || (gy % 400 == 0);
    }

    /**
     * @brief Returns false; the Taiwan calendar has no leap months.
     *
     * C++ counterpart of .NET TaiwanCalendar.IsLeapMonth(int, int, int).
     */
    [[nodiscard]] bool IsLeapMonth(intcs /*year*/, intcs /*month*/, intcs /*era*/ = CurrentEra) const override {
        return false;
    }

    /**
     * @brief Determines whether the specified Taiwan date is a leap day.
     *
     * C++ counterpart of .NET TaiwanCalendar.IsLeapDay(int, int, int, int).
     * @param year  Taiwan (ROC) year.
     * @param month Month (1–12).
     * @param day   Day (1–31).
     * @param era   Era (unused).
     * @return true if the date is February 29 in a leap year.
     */
    [[nodiscard]] bool IsLeapDay(intcs year, intcs month, intcs day, intcs /*era*/ = CurrentEra) const override {
        return month == 2 && day == 29 && IsLeapYear(year);
    }

    /**
     * @brief Returns the number of days in the specified Taiwan month.
     *
     * C++ counterpart of .NET TaiwanCalendar.GetDaysInMonth(int, int, int).
     * The Taiwan year is converted to Gregorian before computing the day count.
     * @param year  Taiwan (ROC) year.
     * @param month Month (1–12).
     * @param era   Era (unused).
     * @return Number of days in the specified month.
     * @throws System::ArgumentOutOfRangeException if @p month is not in [1, 12].
     */
    [[nodiscard]] intcs GetDaysInMonth(intcs year, intcs month, intcs /*era*/ = CurrentEra) const override {
        static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (month < 1 || month > 12) throw System::ArgumentOutOfRangeException("month");
        return (month == 2 && IsLeapYear(year)) ? 29 : days[month];
    }

    /**
     * @brief Returns the number of days in the specified Taiwan year.
     *
     * C++ counterpart of .NET TaiwanCalendar.GetDaysInYear(int, int).
     * @param year Taiwan (ROC) year.
     * @param era  Era (unused).
     * @return 366 for leap years, 365 otherwise.
     */
    [[nodiscard]] intcs GetDaysInYear(intcs year, intcs /*era*/ = CurrentEra) const override {
        return IsLeapYear(year) ? 366 : 365;
    }

    /**
     * @brief Returns a DateTime from the Taiwan date and time components.
     *
     * C++ counterpart of .NET TaiwanCalendar.ToDateTime(int, int, int, int, int, int, int, int).
     * The Taiwan year is converted to Gregorian by adding 1911.
     * @param year        Taiwan (ROC) year.
     * @param month       Month (1–12).
     * @param day         Day (1–31).
     * @param hour        Hour (0–23).
     * @param minute      Minute (0–59).
     * @param second      Second (0–59).
     * @param millisecond Millisecond (0–999).
     * @param era         Era (unused).
     * @return The Gregorian DateTime corresponding to the given Taiwan date components.
     */
    System::DateTime ToDateTime(intcs year, intcs month, intcs day, intcs hour, intcs minute,
                                intcs second, intcs millisecond, intcs /*era*/ = CurrentEra) const override {
        return System::DateTime(year + GregorianOffset, month, day, hour, minute, second, millisecond);
    }

private:
    intcs twoDigitYearMax_{99};
};

} // namespace System::Globalization
