// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Calendar, GregorianCalendar, ISOWeek and the new DateTime
// Add* convenience methods (AddDays, AddHours, AddMinutes, AddSeconds, AddMilliseconds).
#include <climits>
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/DateTime.hpp"
#include "System/DayOfWeek.hpp"
#include "System/Globalization/Calendar.hpp"
#include "System/Globalization/CalendarWeekRule.hpp"
#include "System/Globalization/GregorianCalendar.hpp"
#include "System/Globalization/GregorianCalendarTypes.hpp"
#include "System/Globalization/ISOWeek.hpp"

using System::DateTime;
using System::DayOfWeek;
using namespace System::Globalization;

// ===========================================================================
// DateTime::AddDays / AddHours / AddMinutes / AddSeconds / AddMilliseconds
// ===========================================================================

TEST(DateTimeAddTests, AddDays_Positive) {
    DateTime d(2020, 1, 31);
    DateTime r = d.AddDays(1);
    EXPECT_EQ(r.getYearProperty(),  2020);
    EXPECT_EQ(r.getMonthProperty(), 2);
    EXPECT_EQ(r.getDayProperty(),   1);
}

TEST(DateTimeAddTests, AddDays_Negative) {
    DateTime d(2020, 3, 1);
    DateTime r = d.AddDays(-1);
    // 2020 is a leap year, so Feb has 29 days
    EXPECT_EQ(r.getMonthProperty(), 2);
    EXPECT_EQ(r.getDayProperty(),   29);
}

TEST(DateTimeAddTests, AddDays_Zero) {
    DateTime d(2021, 6, 15);
    EXPECT_EQ(d.AddDays(0), d);
}

TEST(DateTimeAddTests, AddHours_CrossesMidnight) {
    DateTime d(2021, 12, 31, 23, 0, 0);
    DateTime r = d.AddHours(2);
    EXPECT_EQ(r.getYearProperty(),  2022);
    EXPECT_EQ(r.getMonthProperty(), 1);
    EXPECT_EQ(r.getDayProperty(),   1);
    EXPECT_EQ(r.getHourProperty(),  1);
}

TEST(DateTimeAddTests, AddHours_Negative) {
    DateTime d(2021, 1, 1, 1, 0, 0);
    DateTime r = d.AddHours(-2);
    EXPECT_EQ(r.getYearProperty(),  2020);
    EXPECT_EQ(r.getDayProperty(),   31);
    EXPECT_EQ(r.getHourProperty(),  23);
}

TEST(DateTimeAddTests, AddMinutes_Wraps) {
    DateTime d(2021, 6, 15, 23, 50, 0);
    DateTime r = d.AddMinutes(15);
    EXPECT_EQ(r.getDayProperty(),    16);
    EXPECT_EQ(r.getHourProperty(),   0);
    EXPECT_EQ(r.getMinuteProperty(), 5);
}

TEST(DateTimeAddTests, AddSeconds_Wraps) {
    DateTime d(2021, 1, 1, 0, 0, 55);
    DateTime r = d.AddSeconds(10);
    EXPECT_EQ(r.getMinuteProperty(), 1);
    EXPECT_EQ(r.getSecondProperty(), 5);
}

TEST(DateTimeAddTests, AddMilliseconds_Wraps) {
    DateTime d(2021, 1, 1, 0, 0, 0, 990);
    DateTime r = d.AddMilliseconds(20);
    EXPECT_EQ(r.getSecondProperty(),      1);
    EXPECT_EQ(r.getMillisecondProperty(), 10);
}

TEST(DateTimeAddTests, AddDays_365_AddsOneYear_NonLeap) {
    DateTime d(2021, 1, 1);
    DateTime r = d.AddDays(365);
    EXPECT_EQ(r.getYearProperty(),  2022);
    EXPECT_EQ(r.getMonthProperty(), 1);
    EXPECT_EQ(r.getDayProperty(),   1);
}

// ===========================================================================
// Calendar (base class)
// ===========================================================================

TEST(CalendarTests, CurrentEra_IsZero) {
    EXPECT_EQ(Calendar::CurrentEra, 0);
}

TEST(CalendarTests, GetYear) {
    Calendar cal;
    DateTime d(2025, 6, 10);
    EXPECT_EQ(cal.GetYear(d), 2025);
}

TEST(CalendarTests, GetMonth) {
    Calendar cal;
    DateTime d(2025, 6, 10);
    EXPECT_EQ(cal.GetMonth(d), 6);
}

TEST(CalendarTests, GetDayOfMonth) {
    Calendar cal;
    DateTime d(2025, 6, 10);
    EXPECT_EQ(cal.GetDayOfMonth(d), 10);
}

TEST(CalendarTests, GetDayOfWeek_Monday) {
    Calendar cal;
    DateTime d(2025, 6, 9); // known Monday
    EXPECT_EQ(cal.GetDayOfWeek(d), DayOfWeek::Monday);
}

TEST(CalendarTests, GetDayOfYear_Jan1_Is1) {
    Calendar cal;
    DateTime d(2021, 1, 1);
    EXPECT_EQ(cal.GetDayOfYear(d), 1);
}

TEST(CalendarTests, GetHour) {
    Calendar cal;
    DateTime d(2021, 1, 1, 14, 30, 0);
    EXPECT_EQ(cal.GetHour(d), 14);
}

TEST(CalendarTests, GetMinute) {
    Calendar cal;
    DateTime d(2021, 1, 1, 0, 45, 0);
    EXPECT_EQ(cal.GetMinute(d), 45);
}

TEST(CalendarTests, GetSecond) {
    Calendar cal;
    DateTime d(2021, 1, 1, 0, 0, 59);
    EXPECT_EQ(cal.GetSecond(d), 59);
}

TEST(CalendarTests, GetMilliseconds) {
    Calendar cal;
    DateTime d(2021, 1, 1, 0, 0, 0, 123);
    EXPECT_EQ(cal.GetMilliseconds(d), 123);
}

TEST(CalendarTests, GetEra_IsCurrentEra) {
    Calendar cal;
    EXPECT_EQ(cal.GetEra(DateTime()), Calendar::CurrentEra);
}

TEST(CalendarTests, GetErasCount_Is1) {
    Calendar cal;
    EXPECT_EQ(cal.GetErasCount(), 1);
}

TEST(CalendarTests, IsLeapYear_2000_True) {
    Calendar cal;
    EXPECT_TRUE(cal.IsLeapYear(2000));
}

TEST(CalendarTests, IsLeapYear_1900_False) {
    Calendar cal;
    EXPECT_FALSE(cal.IsLeapYear(1900));
}

TEST(CalendarTests, IsLeapYear_2024_True) {
    Calendar cal;
    EXPECT_TRUE(cal.IsLeapYear(2024));
}

TEST(CalendarTests, IsLeapYear_2023_False) {
    Calendar cal;
    EXPECT_FALSE(cal.IsLeapYear(2023));
}

TEST(CalendarTests, GetDaysInMonth_Feb_LeapYear) {
    Calendar cal;
    EXPECT_EQ(cal.GetDaysInMonth(2000, 2), 29);
    EXPECT_EQ(cal.GetDaysInMonth(2024, 2), 29);
}

TEST(CalendarTests, GetDaysInMonth_Feb_NonLeap) {
    Calendar cal;
    EXPECT_EQ(cal.GetDaysInMonth(2021, 2), 28);
    EXPECT_EQ(cal.GetDaysInMonth(1900, 2), 28);
}

TEST(CalendarTests, GetDaysInMonth_January_31) {
    Calendar cal;
    EXPECT_EQ(cal.GetDaysInMonth(2021, 1), 31);
}

TEST(CalendarTests, GetDaysInMonth_April_30) {
    Calendar cal;
    EXPECT_EQ(cal.GetDaysInMonth(2021, 4), 30);
}

// Indexing the internal days-per-month table with an out-of-range month was previously
// undefined behavior (no bounds check), not a thrown exception; real .NET's base
// implementation (via DateTime.DaysInMonth) validates month first.
TEST(CalendarTests, GetDaysInMonth_OutOfRangeMonth_Throws) {
    Calendar cal;
    EXPECT_THROW(cal.GetDaysInMonth(2021, 0), System::ArgumentOutOfRangeException);
    EXPECT_THROW(cal.GetDaysInMonth(2021, 13), System::ArgumentOutOfRangeException);
    EXPECT_THROW(cal.GetDaysInMonth(2021, -1), System::ArgumentOutOfRangeException);
}

TEST(CalendarTests, GetDaysInYear_Leap_366) {
    Calendar cal;
    EXPECT_EQ(cal.GetDaysInYear(2000), 366);
    EXPECT_EQ(cal.GetDaysInYear(2024), 366);
}

TEST(CalendarTests, GetDaysInYear_NonLeap_365) {
    Calendar cal;
    EXPECT_EQ(cal.GetDaysInYear(2021), 365);
    EXPECT_EQ(cal.GetDaysInYear(1900), 365);
}

TEST(CalendarTests, AddYears_Simple) {
    Calendar cal;
    DateTime d(2020, 6, 15);
    DateTime r = cal.AddYears(d, 3);
    EXPECT_EQ(r.getYearProperty(),  2023);
    EXPECT_EQ(r.getMonthProperty(), 6);
    EXPECT_EQ(r.getDayProperty(),   15);
}

// AddYears previously constructed the result year/month/day directly with no clamping, so a
// Feb 29 source date landing on a non-leap target year would throw (DateTime's constructor
// rejects Feb 29 in a non-leap year) instead of clamping to Feb 28 like real .NET
// (GregorianCalendar.cs: AddYears delegates to AddMonths, which clamps `if (d > days) d =
// days;`).
TEST(CalendarTests, AddYears_Feb29_ClampsToFeb28InNonLeapYear) {
    Calendar cal;
    DateTime d(2024, 2, 29); // 2024 is a leap year
    DateTime r = cal.AddYears(d, 1); // 2025 is not a leap year
    EXPECT_EQ(r.getYearProperty(),  2025);
    EXPECT_EQ(r.getMonthProperty(), 2);
    EXPECT_EQ(r.getDayProperty(),   28);
}

TEST(CalendarTests, AddMonths_CrossesYear) {
    Calendar cal;
    DateTime d(2021, 11, 30);
    DateTime r = cal.AddMonths(d, 3);
    EXPECT_EQ(r.getYearProperty(),  2022);
    EXPECT_EQ(r.getMonthProperty(), 2);
    // Feb 2022 has 28 days, so day is clamped
    EXPECT_EQ(r.getDayProperty(), 28);
}

TEST(CalendarTests, AddMonths_Negative) {
    Calendar cal;
    DateTime d(2021, 3, 15);
    DateTime r = cal.AddMonths(d, -2);
    EXPECT_EQ(r.getYearProperty(),  2021);
    EXPECT_EQ(r.getMonthProperty(), 1);
    EXPECT_EQ(r.getDayProperty(),   15);
}

// Regression tests for a code-audit finding (ticket 251): AddMonths had no bounds check on
// `months` at all, unlike real .NET's GregorianCalendar.AddMonths, which rejects |months| >
// 120000 before doing any arithmetic. Without that check, `(year-1)*12 + (month-1) + months`
// can overflow signed int for an extreme months value -- confirmed via a standalone UBSan
// repro that AddMonths(<year 9999 date>, INT_MAX) silently produced a nonsensical negative
// year instead of throwing.
TEST(CalendarTests, AddMonths_AboveMaximum_Throws) {
    Calendar cal;
    DateTime d(2021, 3, 15);
    EXPECT_THROW(cal.AddMonths(d, 120001), System::ArgumentOutOfRangeException);
}

TEST(CalendarTests, AddMonths_BelowMinimum_Throws) {
    Calendar cal;
    DateTime d(2021, 3, 15);
    EXPECT_THROW(cal.AddMonths(d, -120001), System::ArgumentOutOfRangeException);
}

TEST(CalendarTests, AddMonths_ExtremeValue_DoesNotOverflow) {
    Calendar cal;
    DateTime d(2021, 3, 15);
    EXPECT_THROW(cal.AddMonths(d, INT_MAX), System::ArgumentOutOfRangeException);
    EXPECT_THROW(cal.AddMonths(d, INT_MIN), System::ArgumentOutOfRangeException);
}

// 120000 months (10000 years) itself always overflows DateTime's own 1-9999 year range
// regardless of starting year, so it can't be used to prove the new check's own boundary is
// exclusive (> not >=) -- this instead confirms a large-but-comfortably-in-range value (100
// years) is not spuriously rejected by the new check.
TEST(CalendarTests, AddMonths_LargeButInRange_DoesNotThrow) {
    Calendar cal;
    DateTime d(2021, 3, 15);
    EXPECT_NO_THROW(cal.AddMonths(d, 1200));
    EXPECT_NO_THROW(cal.AddMonths(d, -1200));
}

TEST(CalendarTests, AddDays_Via_Calendar) {
    Calendar cal;
    DateTime d(2021, 1, 31);
    DateTime r = cal.AddDays(d, 1);
    EXPECT_EQ(r.getMonthProperty(), 2);
    EXPECT_EQ(r.getDayProperty(),   1);
}

TEST(CalendarTests, AddHours_Via_Calendar) {
    Calendar cal;
    DateTime d(2021, 6, 15, 22, 0, 0);
    DateTime r = cal.AddHours(d, 3);
    EXPECT_EQ(r.getDayProperty(),  16);
    EXPECT_EQ(r.getHourProperty(), 1);
}

TEST(CalendarTests, AddMinutes_Via_Calendar) {
    Calendar cal;
    DateTime d(2021, 6, 15, 23, 50, 0);
    DateTime r = cal.AddMinutes(d, 20);
    EXPECT_EQ(r.getDayProperty(),    16);
    EXPECT_EQ(r.getMinuteProperty(), 10);
}

TEST(CalendarTests, AddSeconds_Via_Calendar) {
    Calendar cal;
    DateTime d(2021, 6, 15, 0, 0, 58);
    DateTime r = cal.AddSeconds(d, 5);
    EXPECT_EQ(r.getMinuteProperty(), 1);
    EXPECT_EQ(r.getSecondProperty(), 3);
}

TEST(CalendarTests, GetWeekOfYear_Approximate) {
    // Just checks it returns a value in the valid range [1, 53]
    Calendar cal;
    DateTime d(2021, 3, 15);
    int w = cal.GetWeekOfYear(d, CalendarWeekRule::FirstDay, DayOfWeek::Monday);
    EXPECT_GE(w, 1);
    EXPECT_LE(w, 53);
}

// ===========================================================================
// Calendar::GetWeekOfYear — rule and firstDayOfWeek must actually be honored
// (previously this always returned dayOfYear/7+1 regardless of parameters).
// Jan 1, 2021 is a Friday.
// ===========================================================================

TEST(CalendarTests, GetWeekOfYear_FirstDay_SundayStart) {
    Calendar cal;
    DateTime d(2021, 1, 4); // Monday
    EXPECT_EQ(cal.GetWeekOfYear(d, CalendarWeekRule::FirstDay, DayOfWeek::Sunday), 2);
}

TEST(CalendarTests, GetWeekOfYear_FirstDay_RespectsFirstDayOfWeek) {
    // Same date, different firstDayOfWeek must change the result.
    Calendar cal;
    DateTime d(2021, 1, 4);
    EXPECT_EQ(cal.GetWeekOfYear(d, CalendarWeekRule::FirstDay, DayOfWeek::Wednesday), 1);
}

TEST(CalendarTests, GetWeekOfYear_FirstFullWeek_DiffersFromFirstDay) {
    Calendar cal;
    DateTime d(2021, 1, 4);
    EXPECT_EQ(cal.GetWeekOfYear(d, CalendarWeekRule::FirstFullWeek, DayOfWeek::Sunday), 1);
}

TEST(CalendarTests, GetWeekOfYear_FirstFourDayWeek) {
    Calendar cal;
    DateTime d(2021, 1, 4);
    EXPECT_EQ(cal.GetWeekOfYear(d, CalendarWeekRule::FirstFourDayWeek, DayOfWeek::Sunday), 1);
}

TEST(CalendarTests, GetWeekOfYear_InvalidFirstDayOfWeek_Throws) {
    Calendar cal;
    DateTime d(2021, 1, 4);
    EXPECT_THROW(cal.GetWeekOfYear(d, CalendarWeekRule::FirstDay, static_cast<DayOfWeek>(7)),
                 System::ArgumentOutOfRangeException);
}

// ===========================================================================
// Calendar::ToFourDigitYear / TwoDigitYearMax
// ===========================================================================

TEST(CalendarTests, TwoDigitYearMax_DefaultsTo2049) {
    Calendar cal;
    EXPECT_EQ(cal.getTwoDigitYearMaxProperty(), 2049);
}

TEST(CalendarTests, ToFourDigitYear_BelowMaxCentury_UsesCurrentCentury) {
    Calendar cal; // TwoDigitYearMax defaults to 2049
    EXPECT_EQ(cal.ToFourDigitYear(30), 2030);
}

TEST(CalendarTests, ToFourDigitYear_AboveMaxCentury_UsesPreviousCentury) {
    Calendar cal; // TwoDigitYearMax defaults to 2049
    EXPECT_EQ(cal.ToFourDigitYear(50), 1950);
}

TEST(CalendarTests, ToFourDigitYear_YearAtOrAbove100_ReturnedAsIs) {
    Calendar cal;
    EXPECT_EQ(cal.ToFourDigitYear(2024), 2024);
}

TEST(CalendarTests, ToFourDigitYear_NegativeYear_Throws) {
    Calendar cal;
    EXPECT_THROW(cal.ToFourDigitYear(-1), System::ArgumentOutOfRangeException);
}

TEST(CalendarTests, ToFourDigitYear_RespectsCustomTwoDigitYearMax) {
    Calendar cal;
    cal.setTwoDigitYearMaxProperty(2099);
    EXPECT_EQ(cal.ToFourDigitYear(99), 2099);
    EXPECT_EQ(cal.getTwoDigitYearMaxProperty(), 2099);
}

// ===========================================================================
// GregorianCalendar
// ===========================================================================

TEST(GregorianCalendarTests, DefaultType_Localized) {
    GregorianCalendar gc;
    EXPECT_EQ(gc.getCalendarTypeProperty(), GregorianCalendarTypes::Localized);
}

TEST(GregorianCalendarTests, ExplicitType) {
    GregorianCalendar gc(GregorianCalendarTypes::USEnglish);
    EXPECT_EQ(gc.getCalendarTypeProperty(), GregorianCalendarTypes::USEnglish);
}

TEST(GregorianCalendarTests, AlgorithmType_IsSolar) {
    GregorianCalendar gc;
    EXPECT_EQ(gc.getAlgorithmTypeProperty(), CalendarAlgorithmType::SolarCalendar);
}

TEST(GregorianCalendarTests, SetCalendarType) {
    GregorianCalendar gc;
    gc.setCalendarTypeProperty(GregorianCalendarTypes::Arabic);
    EXPECT_EQ(gc.getCalendarTypeProperty(), GregorianCalendarTypes::Arabic);
}

TEST(GregorianCalendarTests, ADEra_Is1) {
    EXPECT_EQ(GregorianCalendar::ADEra, 1);
}

TEST(GregorianCalendarTests, GetEra_ReturnsADEra) {
    GregorianCalendar gc;
    EXPECT_EQ(gc.GetEra(DateTime(2021, 1, 1)), GregorianCalendar::ADEra);
}

TEST(GregorianCalendarTests, MinMaxYear) {
    EXPECT_EQ(GregorianCalendar::MinYear, 1);
    EXPECT_EQ(GregorianCalendar::MaxYear, 9999);
}

TEST(GregorianCalendarTests, IsLeapYear_2000_True) {
    GregorianCalendar gc;
    EXPECT_TRUE(gc.IsLeapYear(2000));
}

TEST(GregorianCalendarTests, IsLeapYear_1900_False) {
    GregorianCalendar gc;
    EXPECT_FALSE(gc.IsLeapYear(1900));
}

TEST(GregorianCalendarTests, GetYear_ViaBase) {
    GregorianCalendar gc;
    EXPECT_EQ(gc.GetYear(DateTime(2025, 3, 7)), 2025);
}

TEST(GregorianCalendarTests, GetDaysInMonth_DelegatedToBase) {
    GregorianCalendar gc;
    EXPECT_EQ(gc.GetDaysInMonth(2024, 2), 29); // leap
    EXPECT_EQ(gc.GetDaysInMonth(2023, 2), 28); // non-leap
}

// ===========================================================================
// ISOWeek
// ===========================================================================

TEST(ISOWeekTests, WeekOfYear_2021_Jan4_Is1) {
    // 2021-01-04 is a Monday, should be ISO week 1
    DateTime d(2021, 1, 4);
    EXPECT_EQ(ISOWeek::GetWeekOfYear(d), 1);
}

TEST(ISOWeekTests, WeekOfYear_2021_Jan3_Is53_Of2020) {
    // 2021-01-03 is a Sunday → still week 53 of ISO year 2020
    DateTime d(2021, 1, 3);
    // ISO week number is ≥ 52 (either 52 or 53)
    int w = ISOWeek::GetWeekOfYear(d);
    EXPECT_GE(w, 52);
    EXPECT_LE(w, 53);
}

TEST(ISOWeekTests, WeekOfYear_2020_Dec28_Is53) {
    // 2020-12-28 is a Monday, ISO week 53 of 2020 (2020 is a long year)
    DateTime d(2020, 12, 28);
    EXPECT_EQ(ISOWeek::GetWeekOfYear(d), 53);
}

TEST(ISOWeekTests, WeekOfYear_AlwaysInRange) {
    // Spot-check several dates
    struct { int y, m, d; } dates[] = {
        {2021, 6, 15}, {2022, 1, 1}, {2022, 12, 31},
        {2020, 2, 29}, {2019, 12, 30}
    };
    for (auto& dt : dates) {
        DateTime date(dt.y, dt.m, dt.d);
        int w = ISOWeek::GetWeekOfYear(date);
        EXPECT_GE(w, 1)  << dt.y << "-" << dt.m << "-" << dt.d;
        EXPECT_LE(w, 53) << dt.y << "-" << dt.m << "-" << dt.d;
    }
}

TEST(ISOWeekTests, GetYear_MidYear_EqualsCalendarYear) {
    DateTime d(2021, 6, 15);
    EXPECT_EQ(ISOWeek::GetYear(d), 2021);
}

TEST(ISOWeekTests, GetYear_Dec31_2020_Is2020) {
    // 2020-12-31 is Thursday, ISO year 2020 (week 53)
    DateTime d(2020, 12, 31);
    EXPECT_EQ(ISOWeek::GetYear(d), 2020);
}

TEST(ISOWeekTests, GetWeeksInYear_2020_Is53) {
    // 2020 starts on Wednesday (Jan 1) → long year
    EXPECT_EQ(ISOWeek::GetWeeksInYear(2020), 53);
}

TEST(ISOWeekTests, GetWeeksInYear_2021_Is52) {
    EXPECT_EQ(ISOWeek::GetWeeksInYear(2021), 52);
}

TEST(ISOWeekTests, GetWeeksInYear_2032_Is53) {
    // 2032 is a leap year whose preceding year (2031) ends on Thursday —
    // a long ISO year per the P(y-1)==3 rule, not captured by checking P(y) alone.
    EXPECT_EQ(ISOWeek::GetWeeksInYear(2032), 53);
}

TEST(ISOWeekTests, GetWeeksInYear_OutOfRange_Throws) {
    EXPECT_THROW(ISOWeek::GetWeeksInYear(0), System::ArgumentOutOfRangeException);
    EXPECT_THROW(ISOWeek::GetWeeksInYear(10000), System::ArgumentOutOfRangeException);
}

TEST(ISOWeekTests, ToDateTime_RoundTripsWithGetWeekOfYear) {
    DateTime d(2021, 1, 4); // Monday, ISO week 1 of 2021
    DateTime start = ISOWeek::ToDateTime(2021, 1, DayOfWeek::Monday);
    EXPECT_EQ(start.getYearProperty(),  2021);
    EXPECT_EQ(start.getMonthProperty(), 1);
    EXPECT_EQ(start.getDayProperty(),   4);
    EXPECT_EQ(d, start);
}

TEST(ISOWeekTests, ToDateTime_InvalidWeek_Throws) {
    EXPECT_THROW(ISOWeek::ToDateTime(2021, 0, DayOfWeek::Monday), System::ArgumentOutOfRangeException);
    EXPECT_THROW(ISOWeek::ToDateTime(2021, 54, DayOfWeek::Monday), System::ArgumentOutOfRangeException);
}

TEST(ISOWeekTests, GetYearStart_2021_Is_Jan4) {
    // ISO year 2021 starts on Monday 2021-01-04
    DateTime start = ISOWeek::GetYearStart(2021);
    EXPECT_EQ(start.getYearProperty(),  2021);
    EXPECT_EQ(start.getMonthProperty(), 1);
    EXPECT_EQ(start.getDayProperty(),   4);
    EXPECT_EQ(start.getDayOfWeekProperty(), System::DayOfWeek::Monday);
}

TEST(ISOWeekTests, GetYearEnd_2020_Is_Jan3_2021) {
    // ISO year 2020 ends on Sunday 2021-01-03
    DateTime end = ISOWeek::GetYearEnd(2020);
    EXPECT_EQ(end.getDayOfWeekProperty(), System::DayOfWeek::Sunday);
    // Should be 2021-01-03
    EXPECT_EQ(end.getYearProperty(),  2021);
    EXPECT_EQ(end.getMonthProperty(), 1);
    EXPECT_EQ(end.getDayProperty(),   3);
}

TEST(ISOWeekTests, GetYearStart_IsAlwaysMonday) {
    for (int y = 2015; y <= 2030; ++y) {
        DateTime start = ISOWeek::GetYearStart(y);
        EXPECT_EQ(start.getDayOfWeekProperty(), System::DayOfWeek::Monday)
            << "Year " << y;
    }
}
