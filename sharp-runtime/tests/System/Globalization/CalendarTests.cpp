// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/DateTime.hpp"
#include "System/Globalization/KoreanCalendar.hpp"
#include "System/Globalization/JapaneseCalendar.hpp"
#include "System/Globalization/HijriCalendar.hpp"
#include "System/Globalization/HebrewCalendar.hpp"
#include "System/Globalization/UmAlQuraCalendar.hpp"

using namespace System;
using namespace System::Globalization;

// ---------------------------------------------------------------------------
// KoreanCalendar
// ---------------------------------------------------------------------------
TEST(KoreanCalendarTests, Era) {
    KoreanCalendar cal;
    EXPECT_EQ(cal.GetEra(DateTime(2000, 1, 1)), KoreanCalendar::KoreanEra);
}
TEST(KoreanCalendarTests, Year2000) {
    KoreanCalendar cal;
    EXPECT_EQ(cal.GetYear(DateTime(2000, 1, 1)), 4333); // 2000 + 2333
}
TEST(KoreanCalendarTests, Year1) {
    KoreanCalendar cal;
    EXPECT_EQ(cal.GetYear(DateTime(1, 1, 1)), 2334); // 1 + 2333
}
TEST(KoreanCalendarTests, Month) {
    KoreanCalendar cal;
    EXPECT_EQ(cal.GetMonth(DateTime(2024, 6, 15)), 6);
}
TEST(KoreanCalendarTests, Day) {
    KoreanCalendar cal;
    EXPECT_EQ(cal.GetDayOfMonth(DateTime(2024, 6, 15)), 15);
}

// ---------------------------------------------------------------------------
// JapaneseCalendar
// ---------------------------------------------------------------------------
TEST(JapaneseCalendarTests, ReiwaEra) {
    JapaneseCalendar cal;
    EXPECT_EQ(cal.GetEra(DateTime(2019, 5, 1)), JapaneseCalendar::ReiwaEra);
}
TEST(JapaneseCalendarTests, HeiseiEra) {
    JapaneseCalendar cal;
    EXPECT_EQ(cal.GetEra(DateTime(1989, 1, 8)), JapaneseCalendar::HeiseiEra);
}
TEST(JapaneseCalendarTests, ShowaEra) {
    JapaneseCalendar cal;
    EXPECT_EQ(cal.GetEra(DateTime(1926, 12, 25)), JapaneseCalendar::ShowaEra);
}
TEST(JapaneseCalendarTests, TaishoEra) {
    JapaneseCalendar cal;
    EXPECT_EQ(cal.GetEra(DateTime(1912, 7, 30)), JapaneseCalendar::TaishoEra);
}
TEST(JapaneseCalendarTests, MeijiEra) {
    JapaneseCalendar cal;
    EXPECT_EQ(cal.GetEra(DateTime(1868, 1, 1)), JapaneseCalendar::MeijiEra);
}
TEST(JapaneseCalendarTests, Reiwa1Year) {
    JapaneseCalendar cal;
    // 2019 is Reiwa 1
    EXPECT_EQ(cal.GetYear(DateTime(2019, 5, 1)), 1);
}
TEST(JapaneseCalendarTests, Reiwa6Year) {
    JapaneseCalendar cal;
    // 2024 is Reiwa 6
    EXPECT_EQ(cal.GetYear(DateTime(2024, 6, 1)), 6);
}
TEST(JapaneseCalendarTests, Heisei1) {
    JapaneseCalendar cal;
    EXPECT_EQ(cal.GetYear(DateTime(1989, 1, 8)), 1);
}
TEST(JapaneseCalendarTests, Showa64) {
    JapaneseCalendar cal;
    // Jan 7 1989 is still Showa (emperor died on Jan 7)
    EXPECT_EQ(cal.GetYear(DateTime(1989, 1, 7)), 64);
}
TEST(JapaneseCalendarTests, BeforeMeijiThrows) {
    JapaneseCalendar cal;
    EXPECT_THROW(cal.GetEra(DateTime(1867, 12, 31)), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// HijriCalendar
// ---------------------------------------------------------------------------
TEST(HijriCalendarTests, Era) {
    HijriCalendar cal;
    EXPECT_EQ(cal.GetEra(DateTime(2000, 1, 1)), HijriCalendar::HijriEra);
}
TEST(HijriCalendarTests, LeapYear1) {
    HijriCalendar cal;
    // Year 2 is a leap year: (2*11+14)%30 = 36%30 = 6 < 11
    EXPECT_TRUE(cal.IsLeapYear(2));
}
TEST(HijriCalendarTests, LeapYear3) {
    HijriCalendar cal;
    // Year 3: (3*11+14)%30 = 47%30 = 17 >= 11 → not leap
    EXPECT_FALSE(cal.IsLeapYear(3));
}
TEST(HijriCalendarTests, CommonYearDays) {
    HijriCalendar cal;
    EXPECT_EQ(cal.GetDaysInYear(1, 1), 354);
}
TEST(HijriCalendarTests, LeapYearDays) {
    HijriCalendar cal;
    EXPECT_EQ(cal.GetDaysInYear(2, 1), 355);
}
TEST(HijriCalendarTests, DaysInMonth1) {
    HijriCalendar cal;
    EXPECT_EQ(cal.GetDaysInMonth(1445, 1), 30); // odd month → 30 days
}
TEST(HijriCalendarTests, DaysInMonth2) {
    HijriCalendar cal;
    EXPECT_EQ(cal.GetDaysInMonth(1445, 2), 29); // even month → 29 days
}
TEST(HijriCalendarTests, GetYear2000AD) {
    HijriCalendar cal;
    // Gregorian 2000-01-01 ≈ Hijri 1420-09-25 (approximately)
    int y = cal.GetYear(DateTime(2000, 1, 1));
    EXPECT_EQ(y, 1420);
}
TEST(HijriCalendarTests, GetMonth2000AD) {
    HijriCalendar cal;
    int m = cal.GetMonth(DateTime(2000, 1, 1));
    EXPECT_EQ(m, 9);
}
TEST(HijriCalendarTests, AddYears_LargeValue_ThrowsInsteadOfOverflowing) {
    // `years * 12` computed directly with no upfront bounds check is real signed-integer-
    // overflow UB in C++ for a merely large years argument.
    HijriCalendar cal;
    DateTime start(2000, 1, 1);
    EXPECT_THROW(cal.AddYears(start, 200000000), System::ArgumentOutOfRangeException);
    EXPECT_THROW(cal.AddYears(start, -200000000), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// HebrewCalendar
// ---------------------------------------------------------------------------
TEST(HebrewCalendarTests, Era) {
    HebrewCalendar cal;
    EXPECT_EQ(cal.GetEra(DateTime(2000, 1, 1)), HebrewCalendar::HebrewEra);
}
TEST(HebrewCalendarTests, LeapYear5784) {
    HebrewCalendar cal;
    // 5784 is a leap year: (7*5784+1)%19 = 40489%19 = 0 < 7 → leap
    EXPECT_TRUE(cal.IsLeapYear(5784));
}
TEST(HebrewCalendarTests, CommonYear5785) {
    HebrewCalendar cal;
    EXPECT_FALSE(cal.IsLeapYear(5785));
}
TEST(HebrewCalendarTests, GetYear2000) {
    HebrewCalendar cal;
    // Gregorian 2000-01-01 = Hebrew 5760 (from .NET reference table: 2000 → type 6, and lunarDate.year = 2000+3760 = 5760)
    EXPECT_EQ(cal.GetYear(DateTime(2000, 1, 1)), 5760);
}
TEST(HebrewCalendarTests, GetYear2024) {
    HebrewCalendar cal;
    // Gregorian 2024-01-01 = Hebrew 5784
    EXPECT_EQ(cal.GetYear(DateTime(2024, 1, 1)), 5784);
}
TEST(HebrewCalendarTests, SupportedRange) {
    HebrewCalendar cal;
    EXPECT_NO_THROW(cal.GetYear(DateTime(1583, 1, 1)));
    EXPECT_NO_THROW(cal.GetYear(DateTime(2239, 9, 1)));
}
TEST(HebrewCalendarTests, MonthsInLeapYear) {
    HebrewCalendar cal;
    EXPECT_EQ(cal.GetMonthsInYear(5784), 13); // 5784 is leap
}
TEST(HebrewCalendarTests, MonthsInCommonYear) {
    HebrewCalendar cal;
    EXPECT_EQ(cal.GetMonthsInYear(5785), 12); // 5785 is common
}
TEST(HebrewCalendarTests, AddMonths_LargeValue_ThrowsInsteadOfOverflowing) {
    // `int i = m + months;` computed directly with no upfront bounds check is real
    // signed-integer-overflow UB in C++ for a months argument as simple as INT_MAX/INT_MIN.
    HebrewCalendar cal;
    DateTime start(2024, 1, 1);
    EXPECT_THROW(cal.AddMonths(start, 1000000000), System::ArgumentOutOfRangeException);
    EXPECT_THROW(cal.AddMonths(start, -1000000000), System::ArgumentOutOfRangeException);
}
TEST(HebrewCalendarTests, AddYears_LargeValue_ThrowsInsteadOfOverflowing) {
    // `GetYear(time) + years` computed directly is real signed-integer-overflow UB in C++
    // for a years argument near INT_MAX/INT_MIN.
    HebrewCalendar cal;
    DateTime start(2024, 1, 1);
    EXPECT_THROW(cal.AddYears(start, 1000000000), System::ArgumentOutOfRangeException);
    EXPECT_THROW(cal.AddYears(start, -1000000000), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// UmAlQuraCalendar
// ---------------------------------------------------------------------------
TEST(UmAlQuraCalendarTests, Era) {
    UmAlQuraCalendar cal;
    EXPECT_EQ(cal.GetEra(DateTime(2000, 4, 6)), UmAlQuraCalendar::UmAlQuraEra);
}
TEST(UmAlQuraCalendarTests, Eras_ContainsUmAlQuraEra) {
    UmAlQuraCalendar cal;
    auto eras = cal.getErasProperty();
    ASSERT_EQ(eras.size(), 1u);
    EXPECT_EQ(eras[0], UmAlQuraCalendar::UmAlQuraEra);
}
TEST(UmAlQuraCalendarTests, GetYear1421) {
    UmAlQuraCalendar cal;
    // Gregorian 2000-04-06 = start of Hijri 1421 per table
    EXPECT_EQ(cal.GetYear(DateTime(2000, 4, 6)), 1421);
}
TEST(UmAlQuraCalendarTests, GetYear1445) {
    UmAlQuraCalendar cal;
    // Gregorian 2023-07-19 = start of Hijri 1445 per table
    EXPECT_EQ(cal.GetYear(DateTime(2023, 7, 19)), 1445);
}
TEST(UmAlQuraCalendarTests, GetMonth1) {
    UmAlQuraCalendar cal;
    // Start of year → month 1
    EXPECT_EQ(cal.GetMonth(DateTime(2000, 4, 6)), 1);
}
TEST(UmAlQuraCalendarTests, DaysInYear1421) {
    UmAlQuraCalendar cal;
    // flags for 1421 = 0x0BC4; popcount gives days
    int days = cal.GetDaysInYear(1421);
    EXPECT_TRUE(days == 354 || days == 355);
}
TEST(UmAlQuraCalendarTests, IsLeapYear) {
    UmAlQuraCalendar cal;
    // Just verify it doesn't throw for a valid year
    EXPECT_NO_THROW(cal.IsLeapYear(1445));
}
TEST(UmAlQuraCalendarTests, OutOfRangeThrows) {
    UmAlQuraCalendar cal;
    EXPECT_THROW(cal.IsLeapYear(1317), System::ArgumentOutOfRangeException);
    EXPECT_THROW(cal.IsLeapYear(1501), System::ArgumentOutOfRangeException);
}

// ToDateTime was previously missing entirely, so it fell back to Calendar's Gregorian-only
// base implementation -- UmAlQura year/month/day components were misinterpreted as literal
// Gregorian ones instead of being converted. Verifies the real conversion round-trips.
TEST(UmAlQuraCalendarTests, ToDateTime_RoundTripsThroughGetYearMonthDay) {
    UmAlQuraCalendar cal;
    DateTime greg(2024, 6, 15);
    int y = cal.GetYear(greg), m = cal.GetMonth(greg), d = cal.GetDayOfMonth(greg);
    DateTime back = cal.ToDateTime(y, m, d, 0, 0, 0, 0);
    EXPECT_EQ(back.getYearProperty(), 2024);
    EXPECT_EQ(back.getMonthProperty(), 6);
    EXPECT_EQ(back.getDayProperty(), 15);
}

TEST(UmAlQuraCalendarTests, ToDateTime_InvalidDay_Throws) {
    UmAlQuraCalendar cal;
    EXPECT_THROW(cal.ToDateTime(1445, 1, 31, 0, 0, 0, 0), System::ArgumentOutOfRangeException);
}

TEST(UmAlQuraCalendarTests, AddYears_LargeValue_ThrowsInsteadOfOverflowing) {
    // `years * 12` computed directly with no upfront bounds check is real signed-integer-
    // overflow UB in C++ for a merely large years argument.
    UmAlQuraCalendar cal;
    DateTime start(2024, 6, 15);
    EXPECT_THROW(cal.AddYears(start, 200000000), System::ArgumentOutOfRangeException);
    EXPECT_THROW(cal.AddYears(start, -200000000), System::ArgumentOutOfRangeException);
}
