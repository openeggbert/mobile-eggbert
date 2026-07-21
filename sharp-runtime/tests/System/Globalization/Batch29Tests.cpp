// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 29:
//   GregorianCalendarTypes: enum values
//   GregorianCalendar:      constructors, CalendarType, GetEra, IsLeapYear, AddMonths, AddYears
//   HebrewCalendar:         IsLeapYear, GetMonthsInYear, GetDaysInYear, GetYear/Month/Day
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Globalization/GregorianCalendarTypes.hpp"
#include "System/Globalization/GregorianCalendar.hpp"
#include "System/Globalization/HebrewCalendar.hpp"
#include "System/DateTime.hpp"

using System::Globalization::GregorianCalendarTypes;
using System::Globalization::GregorianCalendar;
using System::Globalization::HebrewCalendar;

// ===========================================================================
// GregorianCalendarTypes
// ===========================================================================

TEST(GregorianCalendarTypesBatch29Test, Values) {
    EXPECT_EQ(static_cast<int>(GregorianCalendarTypes::Localized),             1);
    EXPECT_EQ(static_cast<int>(GregorianCalendarTypes::USEnglish),             2);
    EXPECT_EQ(static_cast<int>(GregorianCalendarTypes::MiddleEastFrench),      9);
    EXPECT_EQ(static_cast<int>(GregorianCalendarTypes::Arabic),               10);
    EXPECT_EQ(static_cast<int>(GregorianCalendarTypes::TransliteratedEnglish), 11);
    EXPECT_EQ(static_cast<int>(GregorianCalendarTypes::TransliteratedFrench),  12);
}

// ===========================================================================
// GregorianCalendar
// ===========================================================================

TEST(GregorianCalendarBatch29Test, DefaultCtor_IsLocalized) {
    GregorianCalendar gc;
    EXPECT_EQ(gc.getCalendarTypeProperty(), GregorianCalendarTypes::Localized);
}

TEST(GregorianCalendarBatch29Test, TypedCtor) {
    GregorianCalendar gc(GregorianCalendarTypes::Arabic);
    EXPECT_EQ(gc.getCalendarTypeProperty(), GregorianCalendarTypes::Arabic);
}

TEST(GregorianCalendarBatch29Test, SetCalendarType) {
    GregorianCalendar gc;
    gc.setCalendarTypeProperty(GregorianCalendarTypes::USEnglish);
    EXPECT_EQ(gc.getCalendarTypeProperty(), GregorianCalendarTypes::USEnglish);
}

TEST(GregorianCalendarBatch29Test, Ctor_InvalidType_Throws) {
    EXPECT_THROW(GregorianCalendar(static_cast<GregorianCalendarTypes>(0)),
                 System::ArgumentOutOfRangeException);
    EXPECT_THROW(GregorianCalendar(static_cast<GregorianCalendarTypes>(13)),
                 System::ArgumentOutOfRangeException);
}

TEST(GregorianCalendarBatch29Test, Eras_ContainsADEra) {
    GregorianCalendar gc;
    auto eras = gc.getErasProperty();
    ASSERT_EQ(eras.size(), 1u);
    EXPECT_EQ(eras[0], GregorianCalendar::ADEra);
}

TEST(GregorianCalendarBatch29Test, GetEra) {
    GregorianCalendar gc;
    EXPECT_EQ(gc.GetEra(System::DateTime(2024, 6, 1)), GregorianCalendar::ADEra);
}

TEST(GregorianCalendarBatch29Test, GetErasCount) {
    GregorianCalendar gc;
    EXPECT_EQ(gc.GetErasCount(), 1);
}

TEST(GregorianCalendarBatch29Test, IsLeapYear_CommonYear) {
    GregorianCalendar gc;
    EXPECT_FALSE(gc.IsLeapYear(2023));
    EXPECT_FALSE(gc.IsLeapYear(1900)); // century, not div by 400
}

TEST(GregorianCalendarBatch29Test, IsLeapYear_LeapYear) {
    GregorianCalendar gc;
    EXPECT_TRUE(gc.IsLeapYear(2024));
    EXPECT_TRUE(gc.IsLeapYear(2000)); // div by 400
    EXPECT_TRUE(gc.IsLeapYear(1600));
}

TEST(GregorianCalendarBatch29Test, AddMonths_BasicForward) {
    GregorianCalendar gc;
    System::DateTime dt(2024, 1, 15);
    auto result = gc.AddMonths(dt, 3);
    EXPECT_EQ(result.getYearProperty(),  2024);
    EXPECT_EQ(result.getMonthProperty(), 4);
    EXPECT_EQ(result.getDayProperty(),   15);
}

TEST(GregorianCalendarBatch29Test, AddMonths_YearRollover) {
    GregorianCalendar gc;
    System::DateTime dt(2024, 11, 1);
    auto result = gc.AddMonths(dt, 3);
    EXPECT_EQ(result.getYearProperty(),  2025);
    EXPECT_EQ(result.getMonthProperty(), 2);
}

TEST(GregorianCalendarBatch29Test, AddYears) {
    GregorianCalendar gc;
    System::DateTime dt(2020, 2, 29); // leap day
    auto result = gc.AddYears(dt, 4);
    EXPECT_EQ(result.getYearProperty(),  2024);
}

TEST(GregorianCalendarBatch29Test, GetDaysInMonth_Feb_LeapYear) {
    GregorianCalendar gc;
    EXPECT_EQ(gc.GetDaysInMonth(2024, 2), 29);
    EXPECT_EQ(gc.GetDaysInMonth(2023, 2), 28);
}

TEST(GregorianCalendarBatch29Test, GetDaysInYear) {
    GregorianCalendar gc;
    EXPECT_EQ(gc.GetDaysInYear(2024), 366);
    EXPECT_EQ(gc.GetDaysInYear(2023), 365);
}

TEST(GregorianCalendarBatch29Test, Constants) {
    EXPECT_EQ(GregorianCalendar::ADEra,   1);
    EXPECT_EQ(GregorianCalendar::MinYear, 1);
    EXPECT_EQ(GregorianCalendar::MaxYear, 9999);
}

// ===========================================================================
// HebrewCalendar
// ===========================================================================

TEST(HebrewCalendarBatch29Test, HebrewEraConstant) {
    EXPECT_EQ(HebrewCalendar::HebrewEra, 1);
}

TEST(HebrewCalendarBatch29Test, GetEra) {
    HebrewCalendar hc;
    EXPECT_EQ(hc.GetEra(System::DateTime(2024, 1, 1)), HebrewCalendar::HebrewEra);
}

TEST(HebrewCalendarBatch29Test, Eras_ContainsHebrewEra) {
    HebrewCalendar hc;
    auto eras = hc.getErasProperty();
    ASSERT_EQ(eras.size(), 1u);
    EXPECT_EQ(eras[0], HebrewCalendar::HebrewEra);
}

TEST(HebrewCalendarBatch29Test, IsLeapYear_InvalidEra_ThrowsArgumentOutOfRange) {
    HebrewCalendar hc;
    EXPECT_THROW(hc.IsLeapYear(5784, 2), System::ArgumentOutOfRangeException);
}

TEST(HebrewCalendarBatch29Test, IsLeapYear_YearOutOfRange_ThrowsArgumentOutOfRange) {
    HebrewCalendar hc;
    EXPECT_THROW(hc.IsLeapYear(1), System::ArgumentOutOfRangeException);
}

TEST(HebrewCalendarBatch29Test, IsLeapYear_Known) {
    HebrewCalendar hc;
    // Hebrew year 5784 (2023-2024) is a leap year
    EXPECT_TRUE(hc.IsLeapYear(5784));
    // Hebrew year 5783 (2022-2023) is not
    EXPECT_FALSE(hc.IsLeapYear(5783));
}

TEST(HebrewCalendarBatch29Test, GetMonthsInYear) {
    HebrewCalendar hc;
    EXPECT_EQ(hc.GetMonthsInYear(5784), 13); // leap year
    EXPECT_EQ(hc.GetMonthsInYear(5783), 12); // common year
}

TEST(HebrewCalendarBatch29Test, GetDaysInYear_Ranges) {
    HebrewCalendar hc;
    int days = hc.GetDaysInYear(5784);
    EXPECT_GE(days, 383);
    EXPECT_LE(days, 385);
    int daysCommon = hc.GetDaysInYear(5783);
    EXPECT_GE(daysCommon, 353);
    EXPECT_LE(daysCommon, 355);
}

TEST(HebrewCalendarBatch29Test, GetYear_KnownDate) {
    HebrewCalendar hc;
    // Rosh Hashanah 5784 started 2023-09-15
    System::DateTime rh(2023, 9, 16);
    EXPECT_EQ(hc.GetYear(rh), 5784);
}

TEST(HebrewCalendarBatch29Test, GetDayOfMonth_NonNegative) {
    HebrewCalendar hc;
    System::DateTime dt(2024, 1, 15);
    int d = hc.GetDayOfMonth(dt);
    EXPECT_GE(d, 1);
    EXPECT_LE(d, 30);
}

TEST(HebrewCalendarBatch29Test, GetDayOfYear_Range) {
    HebrewCalendar hc;
    System::DateTime dt(2024, 3, 15);
    int doy = hc.GetDayOfYear(dt);
    EXPECT_GE(doy, 1);
    EXPECT_LE(doy, 385);
}

// ToDateTime was previously missing entirely, so it fell back to Calendar's Gregorian-only
// base implementation -- Hebrew year/month/day components were misinterpreted as literal
// Gregorian ones instead of being converted. Verifies the real conversion round-trips.
TEST(HebrewCalendarBatch29Test, ToDateTime_RoundTripsThroughGetYearMonthDay) {
    HebrewCalendar hc;
    System::DateTime greg(2024, 6, 15);
    int y = hc.GetYear(greg), m = hc.GetMonth(greg), d = hc.GetDayOfMonth(greg);
    System::DateTime back = hc.ToDateTime(y, m, d, 0, 0, 0, 0);
    EXPECT_EQ(back.getYearProperty(), 2024);
    EXPECT_EQ(back.getMonthProperty(), 6);
    EXPECT_EQ(back.getDayProperty(), 15);
}

TEST(HebrewCalendarBatch29Test, ToDateTime_InvalidDay_Throws) {
    HebrewCalendar hc;
    EXPECT_THROW(hc.ToDateTime(5784, 10, 40, 0, 0, 0, 0), System::ArgumentOutOfRangeException);
}

TEST(HebrewCalendarBatch29Test, ToDateTime_InvalidEra_Throws) {
    // The era parameter was previously discarded entirely -- GetDaysInMonth was called with
    // its *default* era argument instead of the caller's actual era, so an invalid era
    // silently passed validation instead of throwing like real .NET's ToDateTime does
    // (CheckHebrewYearValue -> CheckEraRange).
    HebrewCalendar hc;
    EXPECT_THROW(hc.ToDateTime(5784, 10, 1, 0, 0, 0, 0, 99), System::ArgumentOutOfRangeException);
}
