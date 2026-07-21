// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <stdexcept>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Globalization/Calendar.hpp"

namespace System::Globalization {

using SharpRuntime::intcs;

/**
 * <summary>
 * Represents the Um Al Qura calendar used in Saudi Arabia.
 * Uses a pre-computed table of month lengths for Hijri years 1318–1500.
 * Support range: Gregorian 1900-04-30 – 2077-11-16.
 * </summary>
 */
class UmAlQuraCalendar : public Calendar {
public:
    static constexpr intcs UmAlQuraEra     = 1;    ///< The only era value for this calendar.
    static constexpr intcs MinCalendarYear = 1318; ///< Minimum supported Um Al Qura year.
    static constexpr intcs MaxCalendarYear = 1500; ///< Maximum supported Um Al Qura year.

    /** @return Always UmAlQuraEra (1). */
    [[nodiscard]] intcs GetEra(const System::DateTime& time) const override;
    /** @return Always 1 — the Um Al Qura calendar has a single era. */
    [[nodiscard]] intcs GetErasCount() const override { return 1; }
    /** @return A vector containing {UmAlQuraEra}. C++ counterpart of .NET UmAlQuraCalendar.Eras. */
    [[nodiscard]] std::vector<intcs> getErasProperty() const override { return {UmAlQuraEra}; }

    /** @return Always CalendarAlgorithmType::LunarCalendar. C++ counterpart of .NET UmAlQuraCalendar.AlgorithmType. */
    [[nodiscard]] CalendarAlgorithmType getAlgorithmTypeProperty() const override {
        return CalendarAlgorithmType::LunarCalendar;
    }

    /** @return The Um Al Qura year corresponding to @p time. */
    [[nodiscard]] intcs  GetYear      (const System::DateTime& time) const override;
    /** @return The Um Al Qura month (1–12) corresponding to @p time. */
    [[nodiscard]] intcs  GetMonth     (const System::DateTime& time) const override;
    /** @return The day of the Um Al Qura month corresponding to @p time. */
    [[nodiscard]] intcs  GetDayOfMonth(const System::DateTime& time) const override;
    /** @return The day of the Um Al Qura year corresponding to @p time. */
    [[nodiscard]] intcs  GetDayOfYear (const System::DateTime& time) const override;

    /** @return True if @p year is an Um Al Qura leap year (355 days). */
    [[nodiscard]] bool IsLeapYear   (intcs year, intcs era = CurrentEra) const override;
    /** @return Number of days in the given Um Al Qura month (29 or 30). */
    [[nodiscard]] intcs  GetDaysInMonth (intcs year, intcs month, intcs era = CurrentEra) const override;
    /** @return Number of days in the given Um Al Qura year (354 or 355). */
    [[nodiscard]] intcs  GetDaysInYear  (intcs year, intcs era = CurrentEra) const override;

    /** @return A new DateTime offset by @p months Um Al Qura months. */
    [[nodiscard]] System::DateTime AddMonths(const System::DateTime& time, intcs months) const override;
    /** @return A new DateTime offset by @p years Um Al Qura years. */
    [[nodiscard]] System::DateTime AddYears (const System::DateTime& time, intcs years)  const override;

    /**
     * @brief Returns a DateTime from the Um Al Qura date and time components.
     *
     * C++ counterpart of .NET UmAlQuraCalendar.ToDateTime(int, int, int, int, int, int, int, int).
     * @throws System::ArgumentOutOfRangeException if @p day is outside the valid range for
     *         the given Um Al Qura @p year and @p month.
     */
    [[nodiscard]] System::DateTime ToDateTime(intcs year, intcs month, intcs day, intcs hour, intcs minute,
                                              intcs second, intcs millisecond, intcs era = CurrentEra) const override;

private:
    struct DateMapping {
        int  flags;       // bit i set → month (i+1) has 30 days; else 29 days
        int  gYear, gMonth, gDay; // Gregorian start date of this Hijri year
    };

    static const DateMapping s_yearInfo[184]; // indices 0..183 → Hijri 1318..1501

    static void checkYear(int year, int era);
    static void checkYearMonth(int year, int month, int era);
    static long long absoluteDate(int y, int m, int d);
    static int  daysInYear(int year);

    static void gregorianToHijri(const System::DateTime& time,
                                  int& hy, int& hm, int& hd);
    static void hijriToGregorian(int hy, int hm, int hd,
                                  int& gy, int& gm, int& gd);
    static long long absoluteDateUAQ(int year, int month, int day);

    static constexpr int PartYear      = 0;
    static constexpr int PartDayOfYear = 1;
    static constexpr int PartMonth     = 2;
    static constexpr int PartDay       = 3;

    int getDatePart(const System::DateTime& time, int part) const;
};

} // namespace System::Globalization
