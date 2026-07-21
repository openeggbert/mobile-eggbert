// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <stdexcept>
#include <string>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Globalization/Calendar.hpp"

namespace System::Globalization {

/**
 * @brief Represents the Japanese imperial calendar.
 *
 * C++ counterpart of .NET System.Globalization.JapaneseCalendar.
 * Month and day values are identical to the Gregorian calendar; the year
 * is counted from the start of the current imperial era.
 *
 * Supported eras (most recent first):
 *  - 5 Reiwa   — from 2019-05-01 AD
 *  - 4 Heisei  — from 1989-01-08 AD
 *  - 3 Showa   — from 1926-12-25 AD
 *  - 2 Taisho  — from 1912-07-30 AD
 *  - 1 Meiji   — from 1868-09-08 AD
 */
class JapaneseCalendar : public Calendar {
public:
    static constexpr intcs MeijiEra  = 1; ///< Meiji era identifier (1868-09-08 – 1912-07-29).
    static constexpr intcs TaishoEra = 2; ///< Taisho era identifier (1912-07-30 – 1926-12-24).
    static constexpr intcs ShowaEra  = 3; ///< Showa era identifier (1926-12-25 – 1989-01-07).
    static constexpr intcs HeiseiEra = 4; ///< Heisei era identifier (1989-01-08 – 2019-04-30).
    static constexpr intcs ReiwaEra  = 5; ///< Reiwa era identifier (from 2019-05-01).

    /**
     * @brief Gets the algorithm type for this calendar.
     *
     * C++ counterpart of .NET JapaneseCalendar.AlgorithmType.
     * @return Always CalendarAlgorithmType::SolarCalendar.
     */
    [[nodiscard]] CalendarAlgorithmType getAlgorithmTypeProperty() const override {
        return CalendarAlgorithmType::SolarCalendar;
    }

    /**
     * @brief Gets the earliest date supported by JapaneseCalendar.
     *
     * C++ counterpart of .NET JapaneseCalendar.MinSupportedDateTime.
     * @return DateTime(1868, 10, 23), matching real .NET's `s_calendarMinValue`
     *         (JapaneseCalendar.cs) exactly — not September 8, which is off by 45 days.
     */
    [[nodiscard]] System::DateTime getMinSupportedDateTimeProperty() const override {
        return System::DateTime(1868, 10, 23);
    }

    /**
     * @brief Gets the latest date supported by JapaneseCalendar.
     *
     * C++ counterpart of .NET JapaneseCalendar.MaxSupportedDateTime.
     * @return DateTime(9999, 12, 31).
     */
    [[nodiscard]] System::DateTime getMaxSupportedDateTimeProperty() const override {
        return System::DateTime(9999, 12, 31);
    }

    /**
     * @brief Gets the last two-digit year that maps into the range of this calendar.
     *
     * C++ counterpart of .NET JapaneseCalendar.TwoDigitYearMax.
     * @return The maximum two-digit year in the current era (default 99).
     */
    [[nodiscard]] intcs getTwoDigitYearMaxProperty() const override { return twoDigitYearMax_; }

    /**
     * @brief Sets the last two-digit year that maps into the range of this calendar.
     *
     * C++ counterpart of .NET JapaneseCalendar.TwoDigitYearMax setter.
     * @param value The new maximum two-digit year.
     * @throws System::InvalidOperationException if this instance is read-only.
     */
    void setTwoDigitYearMaxProperty(intcs value) override {
        VerifyWritable();
        twoDigitYearMax_ = value;
    }

    /**
     * @brief Returns the number of Japanese imperial eras.
     *
     * @return Always 5 (Meiji through Reiwa).
     */
    [[nodiscard]] intcs GetErasCount() const override { return 5; }

    /**
     * @brief Gets the list of era identifiers supported by this calendar.
     *
     * C++ counterpart of .NET JapaneseCalendar.Eras.
     * @return A vector of era identifiers, most recent era first.
     */
    [[nodiscard]] std::vector<intcs> getErasProperty() const override {
        return {ReiwaEra, HeiseiEra, ShowaEra, TaishoEra, MeijiEra};
    }

    /**
     * @brief Returns the imperial era for the given DateTime.
     *
     * C++ counterpart of .NET JapaneseCalendar.GetEra(DateTime).
     * @param time The date to classify.
     * @return The era identifier (1–5).
     * @throws System::ArgumentOutOfRangeException if @p time precedes the Meiji era (1868-09-08).
     */
    [[nodiscard]] intcs GetEra(const System::DateTime& time) const override {
        int y = time.getYearProperty(), m = time.getMonthProperty(), d = time.getDayProperty();
        if (y > 2019 || (y == 2019 && (m > 5 || (m == 5 && d >= 1)))) return ReiwaEra;
        if (y > 1989 || (y == 1989 && (m > 1 || (m == 1 && d >= 8)))) return HeiseiEra;
        if (y > 1926 || (y == 1926 && (m > 12 || (m == 12 && d >= 25)))) return ShowaEra;
        if (y > 1912 || (y == 1912 && (m > 7 || (m == 7 && d >= 30)))) return TaishoEra;
        if (y >= 1868) return MeijiEra;
        throw System::ArgumentOutOfRangeException("time");
    }

    /**
     * @brief Returns the year within the current imperial era for the given DateTime.
     *
     * C++ counterpart of .NET JapaneseCalendar.GetYear(DateTime).
     * Year 1 of each era corresponds to the first Gregorian year of that era.
     * @param time The date to query.
     * @return The era-relative year (1-based).
     */
    [[nodiscard]] intcs GetYear(const System::DateTime& time) const override {
        int era = GetEra(time);
        int gy  = time.getYearProperty();
        switch (era) {
            case ReiwaEra:  return gy - 2018;
            case HeiseiEra: return gy - 1988;
            case ShowaEra:  return gy - 1925;
            case TaishoEra: return gy - 1911;
            case MeijiEra:  return gy - 1867;
            default:        return gy;
        }
    }

    /**
     * @brief Determines whether the specified era-relative year is a leap year.
     *
     * C++ counterpart of .NET JapaneseCalendar.IsLeapYear(int, int).
     * The era-relative year is converted to a Gregorian year before applying the Gregorian leap rule.
     * @param year Era-relative year.
     * @param era  Era identifier (1–5); CurrentEra (0) defaults to Reiwa.
     * @return true if the corresponding Gregorian year is a leap year.
     */
    [[nodiscard]] bool IsLeapYear(intcs year, intcs era = CurrentEra) const override {
        int gy = eraYearToGregorian(year, era == CurrentEra ? ReiwaEra : era);
        return (gy % 4 == 0 && gy % 100 != 0) || (gy % 400 == 0);
    }

    /**
     * @brief Returns false; the Japanese calendar has no leap months.
     *
     * C++ counterpart of .NET JapaneseCalendar.IsLeapMonth(int, int, int).
     */
    [[nodiscard]] bool IsLeapMonth(intcs /*year*/, intcs /*month*/, intcs /*era*/ = CurrentEra) const override {
        return false;
    }

    /**
     * @brief Determines whether the specified era-relative date is a leap day.
     *
     * C++ counterpart of .NET JapaneseCalendar.IsLeapDay(int, int, int, int).
     * @param year  Era-relative year.
     * @param month Month (1–12).
     * @param day   Day (1–31).
     * @param era   Era identifier; CurrentEra defaults to Reiwa.
     * @return true if the date is February 29 in a leap year.
     */
    [[nodiscard]] bool IsLeapDay(intcs year, intcs month, intcs day, intcs era = CurrentEra) const override {
        return month == 2 && day == 29 && IsLeapYear(year, era);
    }

    /**
     * @brief Returns the number of days in the specified era-relative month.
     *
     * C++ counterpart of .NET JapaneseCalendar.GetDaysInMonth(int, int, int).
     * @param year  Era-relative year.
     * @param month Month (1–12).
     * @param era   Era identifier; CurrentEra defaults to Reiwa.
     * @return Number of days in the specified month.
     */
    [[nodiscard]] intcs GetDaysInMonth(intcs year, intcs month, intcs era = CurrentEra) const override {
        static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        return (month == 2 && IsLeapYear(year, era)) ? 29 : days[month];
    }

    /**
     * @brief Returns the number of days in the specified era-relative year.
     *
     * C++ counterpart of .NET JapaneseCalendar.GetDaysInYear(int, int).
     * @param year Era-relative year.
     * @param era  Era identifier; CurrentEra defaults to Reiwa.
     * @return 366 for leap years, 365 otherwise.
     */
    [[nodiscard]] intcs GetDaysInYear(intcs year, intcs era = CurrentEra) const override {
        return IsLeapYear(year, era) ? 366 : 365;
    }

    /**
     * @brief Returns a DateTime from the era-relative date and time components.
     *
     * C++ counterpart of .NET JapaneseCalendar.ToDateTime(int, int, int, int, int, int, int, int).
     * The era-relative year is converted to Gregorian using the era start offset.
     * @param year        Era-relative year.
     * @param month       Month (1–12).
     * @param day         Day (1–31).
     * @param hour        Hour (0–23).
     * @param minute      Minute (0–59).
     * @param second      Second (0–59).
     * @param millisecond Millisecond (0–999).
     * @param era         Era identifier; CurrentEra defaults to Reiwa.
     * @return The Gregorian DateTime corresponding to the given Japanese date components.
     */
    System::DateTime ToDateTime(intcs year, intcs month, intcs day, intcs hour, intcs minute,
                                intcs second, intcs millisecond, intcs era = CurrentEra) const override {
        int gy = eraYearToGregorian(year, era == CurrentEra ? ReiwaEra : era);
        return System::DateTime(gy, month, day, hour, minute, second, millisecond);
    }

private:
    intcs twoDigitYearMax_{99};

    /**
     * @brief Converts an era-relative year to the corresponding Gregorian year.
     * @param year Era-relative year (1-based within era).
     * @param era  Era identifier (1–5).
     * @return The Gregorian year.
     * @throws System::ArgumentOutOfRangeException if @p year is negative, outside the valid
     *         [1, maxEraYear] range for @p era, or @p era is not one of the 5 known eras.
     */
    static int eraYearToGregorian(int year, int era) {
        // Verified against GregorianCalendarHelper.cs's GetYearOffset(throwOnError: true) --
        // the shared validation path real .NET's JapaneseCalendar.GetGregorianYear/IsLeapYear/
        // GetDaysInMonth/GetDaysInYear/ToDateTime all route through: year must be non-negative
        // AND within [1, maxEraYear] for the resolved era (maxEraYear taken directly from
        // JapaneseCalendar.cs's built-in EraInfo table: Reiwa 9999-2018, Heisei 2019-1988,
        // Showa 1989-1925, Taisho 1926-1911, Meiji 1912-1867), and era must be one of the 5
        // known values. This port previously performed none of that validation, silently
        // computing (and using) a nonsensical Gregorian year for any out-of-range era-relative
        // year or era value instead of throwing -- e.g. IsLeapYear(100, HeiseiEra) computed
        // Gregorian year 2088 instead of rejecting 100 as outside Heisei's actual [1,31] range.
        if (year < 0)
            throw System::ArgumentOutOfRangeException("year", "Non-negative number required.");
        int maxYear;
        switch (era) {
            case ReiwaEra:  maxYear = 9999 - 2018; break;
            case HeiseiEra: maxYear = 2019 - 1988; break;
            case ShowaEra:  maxYear = 1989 - 1925; break;
            case TaishoEra: maxYear = 1926 - 1911; break;
            case MeijiEra:  maxYear = 1912 - 1867; break;
            default:
                throw System::ArgumentOutOfRangeException("era", "Era value was not valid.");
        }
        if (year < 1 || year > maxYear)
            throw System::ArgumentOutOfRangeException("year",
                "Valid values are between 1 and " + std::to_string(maxYear) + ", inclusive.");
        switch (era) {
            case ReiwaEra:  return 2018 + year;
            case HeiseiEra: return 1988 + year;
            case ShowaEra:  return 1925 + year;
            case TaishoEra: return 1911 + year;
            default:        return 1867 + year; // MeijiEra
        }
    }
};

} // namespace System::Globalization
