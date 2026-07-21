// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Globalization/Calendar.hpp"

namespace System::Globalization {

/**
 * @brief Represents the Korean calendar.
 *
 * C++ counterpart of .NET System.Globalization.KoreanCalendar.
 * The Korean calendar uses the same month/day/time structure as the Gregorian calendar,
 * but adds 2333 to the Gregorian year. For example, Gregorian 2000-01-01 = Korean 4333-01-01.
 * The offset 2333 derives from the traditional year of the legendary founding of Gojoseon (2333 BCE).
 */
class KoreanCalendar : public Calendar {
public:
    static constexpr intcs KoreanEra      = 1;    ///< The only era value for this calendar.
    static constexpr intcs GregorianOffset = 2333; ///< Years added to the Gregorian year to obtain the Korean year.

    /**
     * @brief Gets the algorithm type for this calendar.
     *
     * C++ counterpart of .NET KoreanCalendar.AlgorithmType.
     * @return Always CalendarAlgorithmType::SolarCalendar.
     */
    [[nodiscard]] CalendarAlgorithmType getAlgorithmTypeProperty() const override {
        return CalendarAlgorithmType::SolarCalendar;
    }

    /**
     * @brief Gets the earliest date supported by KoreanCalendar.
     *
     * C++ counterpart of .NET KoreanCalendar.MinSupportedDateTime.
     * @return DateTime(1, 1, 1) — Korean year 2334.
     */
    [[nodiscard]] System::DateTime getMinSupportedDateTimeProperty() const override {
        return System::DateTime(1, 1, 1);
    }

    /**
     * @brief Gets the latest date supported by KoreanCalendar.
     *
     * C++ counterpart of .NET KoreanCalendar.MaxSupportedDateTime.
     * @return DateTime(9999, 12, 31) — Korean year 12332.
     */
    [[nodiscard]] System::DateTime getMaxSupportedDateTimeProperty() const override {
        return System::DateTime(9999, 12, 31);
    }

    /**
     * @brief Gets the last two-digit year that maps into the range of this calendar.
     *
     * C++ counterpart of .NET KoreanCalendar.TwoDigitYearMax.
     * @return The maximum two-digit year (default 4362 = Gregorian 2029 + 2333).
     */
    [[nodiscard]] intcs getTwoDigitYearMaxProperty() const override { return twoDigitYearMax_; }

    /**
     * @brief Sets the last two-digit year that maps into the range of this calendar.
     *
     * C++ counterpart of .NET KoreanCalendar.TwoDigitYearMax setter.
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
     * C++ counterpart of .NET KoreanCalendar.GetEra(DateTime).
     * @return Always KoreanEra (1).
     */
    [[nodiscard]] intcs GetEra(const System::DateTime& /*time*/) const override { return KoreanEra; }

    /**
     * @brief Returns the number of eras in this calendar.
     *
     * @return Always 1; the Korean calendar has a single era.
     */
    [[nodiscard]] intcs GetErasCount() const override { return 1; }

    /**
     * @brief Gets the list of era identifiers supported by this calendar.
     *
     * C++ counterpart of .NET KoreanCalendar.Eras.
     * @return A vector containing {KoreanEra}.
     */
    [[nodiscard]] std::vector<intcs> getErasProperty() const override { return {KoreanEra}; }

    /**
     * @brief Returns the Korean year corresponding to the given DateTime.
     *
     * C++ counterpart of .NET KoreanCalendar.GetYear(DateTime).
     * The Korean year equals the Gregorian year plus 2333.
     * @param time The DateTime to convert.
     * @return The Korean year (Gregorian year + 2333).
     */
    [[nodiscard]] intcs GetYear(const System::DateTime& time) const override {
        return time.getYearProperty() + GregorianOffset;
    }

    /**
     * @brief Determines whether the specified Korean year is a leap year.
     *
     * C++ counterpart of .NET KoreanCalendar.IsLeapYear(int, int).
     * The Korean year is converted to the Gregorian year before applying the Gregorian leap rule.
     * @param year Korean year.
     * @param era  Era (unused; always KoreanEra).
     * @return true if the corresponding Gregorian year is a leap year.
     */
    [[nodiscard]] bool IsLeapYear(intcs year, intcs /*era*/ = CurrentEra) const override {
        int gy = year - GregorianOffset;
        return (gy % 4 == 0 && gy % 100 != 0) || (gy % 400 == 0);
    }

    /**
     * @brief Returns false; the Korean calendar has no leap months.
     *
     * C++ counterpart of .NET KoreanCalendar.IsLeapMonth(int, int, int).
     */
    [[nodiscard]] bool IsLeapMonth(intcs /*year*/, intcs /*month*/, intcs /*era*/ = CurrentEra) const override {
        return false;
    }

    /**
     * @brief Determines whether the specified Korean date is a leap day.
     *
     * C++ counterpart of .NET KoreanCalendar.IsLeapDay(int, int, int, int).
     * @param year  Korean year.
     * @param month Month (1–12).
     * @param day   Day (1–31).
     * @param era   Era (unused).
     * @return true if the date is February 29 in a leap year.
     */
    [[nodiscard]] bool IsLeapDay(intcs year, intcs month, intcs day, intcs /*era*/ = CurrentEra) const override {
        return month == 2 && day == 29 && IsLeapYear(year);
    }

    /**
     * @brief Returns the number of days in the specified Korean month.
     *
     * C++ counterpart of .NET KoreanCalendar.GetDaysInMonth(int, int, int).
     * The Korean year is converted to Gregorian before computing the day count.
     * @param year  Korean year.
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
     * @brief Returns the number of days in the specified Korean year.
     *
     * C++ counterpart of .NET KoreanCalendar.GetDaysInYear(int, int).
     * @param year Korean year.
     * @param era  Era (unused).
     * @return 366 for leap years, 365 otherwise.
     */
    [[nodiscard]] intcs GetDaysInYear(intcs year, intcs /*era*/ = CurrentEra) const override {
        return IsLeapYear(year) ? 366 : 365;
    }

    /**
     * @brief Returns a DateTime from the Korean date and time components.
     *
     * C++ counterpart of .NET KoreanCalendar.ToDateTime(int, int, int, int, int, int, int, int).
     * The Korean year is converted to Gregorian by subtracting 2333.
     * @param year        Korean year.
     * @param month       Month (1–12).
     * @param day         Day (1–31).
     * @param hour        Hour (0–23).
     * @param minute      Minute (0–59).
     * @param second      Second (0–59).
     * @param millisecond Millisecond (0–999).
     * @param era         Era (unused).
     * @return The Gregorian DateTime corresponding to the given Korean date components.
     */
    System::DateTime ToDateTime(intcs year, intcs month, intcs day, intcs hour, intcs minute,
                                intcs second, intcs millisecond, intcs /*era*/ = CurrentEra) const override {
        return System::DateTime(year - GregorianOffset, month, day, hour, minute, second, millisecond);
    }

private:
    intcs twoDigitYearMax_{4362};
};

} // namespace System::Globalization
