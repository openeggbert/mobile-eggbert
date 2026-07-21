// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 30:
//   HijriCalendar:   IsLeapYear, GetDaysInYear/Month, GetYear/Month/Day, AddYears
//   ISOWeek:         GetWeekOfYear, GetYear, GetWeeksInYear, GetYearStart, GetYearEnd
//   IdnMapping:      default ctor, property getters/setters, GetAscii, operator==
//   JapaneseCalendar: era constants, GetEra, GetYear, GetErasCount
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Globalization/HijriCalendar.hpp"
#include "System/Globalization/ISOWeek.hpp"
#include "System/Globalization/IdnMapping.hpp"
#include "System/Globalization/JapaneseCalendar.hpp"
#include "System/DateTime.hpp"

using System::Globalization::HijriCalendar;
using System::Globalization::ISOWeek;
using System::Globalization::IdnMapping;
using System::Globalization::JapaneseCalendar;

// ===========================================================================
// HijriCalendar
// ===========================================================================

TEST(HijriCalendarBatch30Test, HijriEraConstant) {
    EXPECT_EQ(HijriCalendar::HijriEra, 1);
}

TEST(HijriCalendarBatch30Test, GetErasCount) {
    HijriCalendar hc;
    EXPECT_EQ(hc.GetErasCount(), 1);
}

TEST(HijriCalendarBatch30Test, Eras_ContainsHijriEra) {
    HijriCalendar hc;
    auto eras = hc.getErasProperty();
    ASSERT_EQ(eras.size(), 1u);
    EXPECT_EQ(eras[0], HijriCalendar::HijriEra);
}

TEST(HijriCalendarBatch30Test, IsLeapYear_OutOfRange_ThrowsArgumentOutOfRange) {
    HijriCalendar hc;
    EXPECT_THROW(hc.IsLeapYear(0), System::ArgumentOutOfRangeException);
    EXPECT_THROW(hc.IsLeapYear(HijriCalendar::MaxCalendarYear + 1), System::ArgumentOutOfRangeException);
}

TEST(HijriCalendarBatch30Test, GetDaysInMonth_InvalidMonth_ThrowsArgumentOutOfRange) {
    HijriCalendar hc;
    EXPECT_THROW(hc.GetDaysInMonth(1, 0), System::ArgumentOutOfRangeException);
    EXPECT_THROW(hc.GetDaysInMonth(1, 13), System::ArgumentOutOfRangeException);
}

TEST(HijriCalendarBatch30Test, IsLeapYear_Known) {
    HijriCalendar hc;
    // Hijri year 2 is a leap year ((2*11+14)%30 = 36%30 = 6 < 11)
    EXPECT_TRUE(hc.IsLeapYear(2));
    // Hijri year 1 is not
    EXPECT_FALSE(hc.IsLeapYear(1));
}

TEST(HijriCalendarBatch30Test, GetDaysInYear) {
    HijriCalendar hc;
    EXPECT_EQ(hc.GetDaysInYear(2), 355); // leap
    EXPECT_EQ(hc.GetDaysInYear(1), 354); // common
}

TEST(HijriCalendarBatch30Test, GetDaysInMonth_OddEven) {
    HijriCalendar hc;
    // Month 1 always 30, month 2 always 29
    EXPECT_EQ(hc.GetDaysInMonth(1445, 1), 30);
    EXPECT_EQ(hc.GetDaysInMonth(1445, 2), 29);
}

TEST(HijriCalendarBatch30Test, GetYear_KnownDate) {
    HijriCalendar hc;
    // 2024-01-01 Gregorian is in Hijri year 1445
    System::DateTime dt(2024, 1, 1);
    EXPECT_EQ(hc.GetYear(dt), 1445);
}

TEST(HijriCalendarBatch30Test, GetMonth_Range) {
    HijriCalendar hc;
    System::DateTime dt(2024, 6, 15);
    int m = hc.GetMonth(dt);
    EXPECT_GE(m, 1);
    EXPECT_LE(m, 12);
}

TEST(HijriCalendarBatch30Test, GetDayOfMonth_Range) {
    HijriCalendar hc;
    System::DateTime dt(2024, 3, 10);
    int d = hc.GetDayOfMonth(dt);
    EXPECT_GE(d, 1);
    EXPECT_LE(d, 30);
}

TEST(HijriCalendarBatch30Test, GetDayOfYear_Range) {
    HijriCalendar hc;
    System::DateTime dt(2024, 6, 1);
    int doy = hc.GetDayOfYear(dt);
    EXPECT_GE(doy, 1);
    EXPECT_LE(doy, 355);
}

TEST(HijriCalendarBatch30Test, AddYears_Roundtrip) {
    HijriCalendar hc;
    System::DateTime dt(2024, 1, 15);
    auto dt2 = hc.AddYears(dt, 1);
    EXPECT_EQ(hc.GetYear(dt2), hc.GetYear(dt) + 1);
}

// ToDateTime was previously missing entirely, so it fell back to Calendar's Gregorian-only
// base implementation -- Hijri year/month/day components were misinterpreted as literal
// Gregorian ones instead of being converted. Verifies the real conversion round-trips.
TEST(HijriCalendarBatch30Test, ToDateTime_RoundTripsThroughGetYearMonthDay) {
    HijriCalendar hc;
    System::DateTime greg(2024, 6, 15);
    int y = hc.GetYear(greg), m = hc.GetMonth(greg), d = hc.GetDayOfMonth(greg);
    System::DateTime back = hc.ToDateTime(y, m, d, 0, 0, 0, 0);
    EXPECT_EQ(back.getYearProperty(), 2024);
    EXPECT_EQ(back.getMonthProperty(), 6);
    EXPECT_EQ(back.getDayProperty(), 15);
}

TEST(HijriCalendarBatch30Test, ToDateTime_InvalidDay_Throws) {
    HijriCalendar hc;
    EXPECT_THROW(hc.ToDateTime(1445, 1, 31, 0, 0, 0, 0), System::ArgumentOutOfRangeException);
}

// ===========================================================================
// ISOWeek
// ===========================================================================

TEST(ISOWeekBatch30Test, GetWeekOfYear_Jan1_2024) {
    // 2024-01-01 is Monday — ISO week 1
    EXPECT_EQ(ISOWeek::GetWeekOfYear(System::DateTime(2024, 1, 1)), 1);
}

TEST(ISOWeekBatch30Test, GetWeekOfYear_Dec31_2023) {
    // 2023-12-31 is Sunday — belongs to week 52 of 2023
    EXPECT_EQ(ISOWeek::GetWeekOfYear(System::DateTime(2023, 12, 31)), 52);
}

TEST(ISOWeekBatch30Test, GetWeekOfYear_MidYear) {
    // 2024-07-01 should be in week 27
    EXPECT_EQ(ISOWeek::GetWeekOfYear(System::DateTime(2024, 7, 1)), 27);
}

TEST(ISOWeekBatch30Test, GetYear_SameAsCalendar) {
    // 2024-06-15 — ISO year is same as calendar year
    EXPECT_EQ(ISOWeek::GetYear(System::DateTime(2024, 6, 15)), 2024);
}

TEST(ISOWeekBatch30Test, GetYear_Dec30_2019_IsWeek1_2020) {
    // 2019-12-30 is Monday of ISO week 1, 2020
    EXPECT_EQ(ISOWeek::GetYear(System::DateTime(2019, 12, 30)), 2020);
}

TEST(ISOWeekBatch30Test, GetWeeksInYear_52or53) {
    int w2020 = ISOWeek::GetWeeksInYear(2020);
    int w2021 = ISOWeek::GetWeeksInYear(2021);
    EXPECT_TRUE(w2020 == 52 || w2020 == 53);
    EXPECT_TRUE(w2021 == 52 || w2021 == 53);
}

TEST(ISOWeekBatch30Test, GetYearStart_IsMonday) {
    auto start = ISOWeek::GetYearStart(2024);
    // 2024 ISO week 1 starts 2024-01-01 (Monday)
    EXPECT_EQ(start.getDayOfWeekProperty(), System::DayOfWeek::Monday);
    EXPECT_EQ(start.getYearProperty(), 2024);
    EXPECT_EQ(start.getMonthProperty(), 1);
    EXPECT_EQ(start.getDayProperty(), 1);
}

TEST(ISOWeekBatch30Test, GetYearEnd_IsSunday) {
    auto end = ISOWeek::GetYearEnd(2024);
    EXPECT_EQ(end.getDayOfWeekProperty(), System::DayOfWeek::Sunday);
}

// ===========================================================================
// IdnMapping
// ===========================================================================

TEST(IdnMappingBatch30Test, DefaultProperties) {
    IdnMapping idn;
    EXPECT_FALSE(idn.getAllowUnassignedProperty());
    EXPECT_FALSE(idn.getUseStd3AsciiRulesProperty());
}

TEST(IdnMappingBatch30Test, SetProperties) {
    IdnMapping idn;
    idn.setAllowUnassignedProperty(true);
    idn.setUseStd3AsciiRulesProperty(true);
    EXPECT_TRUE(idn.getAllowUnassignedProperty());
    EXPECT_TRUE(idn.getUseStd3AsciiRulesProperty());
}

TEST(IdnMappingBatch30Test, GetAscii_AsciiOnly_Passthrough) {
    IdnMapping idn;
    EXPECT_EQ(idn.GetAscii("example.com"), "example.com");
}

TEST(IdnMappingBatch30Test, GetAscii_Unicode_EncodesLabel) {
    IdnMapping idn;
    // "bücher.de" → "xn--bcher-kva.de"
    std::string result = idn.GetAscii("bücher.de");
    EXPECT_NE(result.find("xn--"), std::string::npos);
}

TEST(IdnMappingBatch30Test, GetUnicode_Roundtrip) {
    IdnMapping idn;
    std::string ascii   = idn.GetAscii("bücher.de");
    std::string unicode = idn.GetUnicode(ascii);
    EXPECT_EQ(unicode, "bücher.de");
}

TEST(IdnMappingBatch30Test, Equality) {
    IdnMapping a, b;
    EXPECT_TRUE(a == b);
    b.setAllowUnassignedProperty(true);
    EXPECT_FALSE(a == b);
}

// ===========================================================================
// JapaneseCalendar
// ===========================================================================

TEST(JapaneseCalendarBatch30Test, EraConstants) {
    EXPECT_EQ(JapaneseCalendar::MeijiEra,  1);
    EXPECT_EQ(JapaneseCalendar::TaishoEra, 2);
    EXPECT_EQ(JapaneseCalendar::ShowaEra,  3);
    EXPECT_EQ(JapaneseCalendar::HeiseiEra, 4);
    EXPECT_EQ(JapaneseCalendar::ReiwaEra,  5);
}

TEST(JapaneseCalendarBatch30Test, GetErasCount) {
    JapaneseCalendar jc;
    EXPECT_EQ(jc.GetErasCount(), 5);
}

TEST(JapaneseCalendarBatch30Test, GetEra_Reiwa) {
    JapaneseCalendar jc;
    EXPECT_EQ(jc.GetEra(System::DateTime(2024, 6, 1)), JapaneseCalendar::ReiwaEra);
    EXPECT_EQ(jc.GetEra(System::DateTime(2019, 5, 1)), JapaneseCalendar::ReiwaEra);
}

TEST(JapaneseCalendarBatch30Test, GetEra_Heisei) {
    JapaneseCalendar jc;
    EXPECT_EQ(jc.GetEra(System::DateTime(2019, 4, 30)), JapaneseCalendar::HeiseiEra);
    EXPECT_EQ(jc.GetEra(System::DateTime(1989, 1, 8)),  JapaneseCalendar::HeiseiEra);
}

TEST(JapaneseCalendarBatch30Test, GetEra_Showa) {
    JapaneseCalendar jc;
    EXPECT_EQ(jc.GetEra(System::DateTime(1989, 1, 7)),   JapaneseCalendar::ShowaEra);
    EXPECT_EQ(jc.GetEra(System::DateTime(1926, 12, 25)), JapaneseCalendar::ShowaEra);
}

TEST(JapaneseCalendarBatch30Test, GetEra_Meiji) {
    JapaneseCalendar jc;
    EXPECT_EQ(jc.GetEra(System::DateTime(1868, 1, 1)), JapaneseCalendar::MeijiEra);
}

TEST(JapaneseCalendarBatch30Test, GetEra_BeforeMeiji_Throws) {
    JapaneseCalendar jc;
    EXPECT_THROW(jc.GetEra(System::DateTime(1867, 12, 31)), System::ArgumentOutOfRangeException);
}

TEST(JapaneseCalendarBatch30Test, Eras_ContainsAllFiveEras) {
    JapaneseCalendar jc;
    auto eras = jc.getErasProperty();
    ASSERT_EQ(eras.size(), 5u);
    EXPECT_EQ(eras[0], JapaneseCalendar::ReiwaEra);
    EXPECT_EQ(eras[4], JapaneseCalendar::MeijiEra);
}

TEST(JapaneseCalendarBatch30Test, GetYear_Reiwa) {
    JapaneseCalendar jc;
    // Reiwa 6 = 2024
    EXPECT_EQ(jc.GetYear(System::DateTime(2024, 1, 1)), 6);
}

TEST(JapaneseCalendarBatch30Test, GetYear_Heisei1) {
    JapaneseCalendar jc;
    EXPECT_EQ(jc.GetYear(System::DateTime(1989, 6, 1)), 1);
}

// Regression for ticket 348: eraYearToGregorian previously performed no validation at all,
// silently computing a nonsensical Gregorian year for an era-relative year outside that era's
// actual span. Verified against GregorianCalendarHelper.cs's GetYearOffset(throwOnError: true) --
// Heisei only spans era-relative years [1,31] (1989-2019); year 100 is invalid for that era.
TEST(JapaneseCalendarBatch30Test, IsLeapYear_YearExceedsEraSpan_Throws) {
    JapaneseCalendar jc;
    EXPECT_THROW(jc.IsLeapYear(100, JapaneseCalendar::HeiseiEra), System::ArgumentOutOfRangeException);
}

TEST(JapaneseCalendarBatch30Test, IsLeapYear_YearAtEraSpanBoundary_Succeeds) {
    JapaneseCalendar jc;
    // Heisei's maxEraYear is 31 (2019-1988); this must not throw.
    EXPECT_NO_THROW(jc.IsLeapYear(31, JapaneseCalendar::HeiseiEra));
}

TEST(JapaneseCalendarBatch30Test, ToDateTime_YearExceedsEraSpan_Throws) {
    JapaneseCalendar jc;
    EXPECT_THROW(jc.ToDateTime(100, 1, 1, 0, 0, 0, 0, JapaneseCalendar::HeiseiEra),
                 System::ArgumentOutOfRangeException);
}

TEST(JapaneseCalendarBatch30Test, ToDateTime_ZeroYear_Throws) {
    JapaneseCalendar jc;
    EXPECT_THROW(jc.ToDateTime(0, 1, 1, 0, 0, 0, 0, JapaneseCalendar::ReiwaEra),
                 System::ArgumentOutOfRangeException);
}

TEST(JapaneseCalendarBatch30Test, ToDateTime_NegativeYear_Throws) {
    JapaneseCalendar jc;
    EXPECT_THROW(jc.ToDateTime(-1, 1, 1, 0, 0, 0, 0, JapaneseCalendar::ReiwaEra),
                 System::ArgumentOutOfRangeException);
}

TEST(JapaneseCalendarBatch30Test, ToDateTime_InvalidEra_Throws) {
    JapaneseCalendar jc;
    EXPECT_THROW(jc.ToDateTime(1, 1, 1, 0, 0, 0, 0, 99), System::ArgumentOutOfRangeException);
}

// Real .NET's s_calendarMinValue is 1868-10-23 (JapaneseCalendar.cs); this port previously
// used 1868-09-08, off by 45 days.
TEST(JapaneseCalendarBatch30Test, MinSupportedDateTime_MatchesRealDotNet) {
    JapaneseCalendar jc;
    System::DateTime minDate = jc.getMinSupportedDateTimeProperty();
    EXPECT_EQ(minDate.getYearProperty(), 1868);
    EXPECT_EQ(minDate.getMonthProperty(), 10);
    EXPECT_EQ(minDate.getDayProperty(), 23);
}
