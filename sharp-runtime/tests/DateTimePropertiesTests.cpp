// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for new DateTime calendar-component properties and constructors.
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/DateTime.hpp"
#include "System/DayOfWeek.hpp"

// ===========================================================================
// DateTime(year, month, day) constructor
// ===========================================================================

TEST(DateTimeConstructorTests, YMD_StoresCorrectTicks) {
    // .NET: new DateTime(2000, 1, 1).Ticks == 630822816000000000
    System::DateTime d(2000, 1, 1);
    EXPECT_EQ(d.getTicksProperty(), 630822816000000000LL);
}

TEST(DateTimeConstructorTests, YMD_EpochJan1_0001) {
    System::DateTime d(1, 1, 1);
    EXPECT_EQ(d.getTicksProperty(), 0LL);
}

TEST(DateTimeConstructorTests, YMDHMS_Stored) {
    System::DateTime d(2025, 6, 10, 14, 30, 55);
    EXPECT_EQ(d.getYearProperty(),   2025);
    EXPECT_EQ(d.getMonthProperty(),  6);
    EXPECT_EQ(d.getDayProperty(),    10);
    EXPECT_EQ(d.getHourProperty(),   14);
    EXPECT_EQ(d.getMinuteProperty(), 30);
    EXPECT_EQ(d.getSecondProperty(), 55);
}

TEST(DateTimeConstructorTests, YMDHMSMs_Stored) {
    System::DateTime d(2020, 3, 15, 8, 5, 3, 750);
    EXPECT_EQ(d.getMillisecondProperty(), 750);
    EXPECT_EQ(d.getSecondProperty(),      3);
    EXPECT_EQ(d.getMinuteProperty(),      5);
    EXPECT_EQ(d.getHourProperty(),        8);
}

TEST(DateTimeConstructorTests, InvalidMonth_Throws) {
    EXPECT_THROW(System::DateTime(2000, 13, 1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(System::DateTime(2000, 0, 1),  System::ArgumentOutOfRangeException);
}

TEST(DateTimeConstructorTests, InvalidDay_Throws) {
    EXPECT_THROW(System::DateTime(2000, 2, 30), System::ArgumentOutOfRangeException); // Feb has 29 in 2000
    EXPECT_THROW(System::DateTime(2001, 2, 29), System::ArgumentOutOfRangeException); // 2001 is not leap
}

TEST(DateTimeConstructorTests, LeapDay_Valid) {
    EXPECT_NO_THROW(System::DateTime(2000, 2, 29)); // 2000 is leap
    EXPECT_NO_THROW(System::DateTime(2024, 2, 29)); // 2024 is leap
}

// ===========================================================================
// Year / Month / Day properties
// ===========================================================================

TEST(DateTimeYearTests, Year_2000) {
    System::DateTime d(2000, 6, 15);
    EXPECT_EQ(d.getYearProperty(), 2000);
}

TEST(DateTimeYearTests, Year_1970) {
    // Unix epoch in .NET ticks = 621355968000000000
    System::DateTime d(621355968000000000LL);
    EXPECT_EQ(d.getYearProperty(),  1970);
    EXPECT_EQ(d.getMonthProperty(), 1);
    EXPECT_EQ(d.getDayProperty(),   1);
}

TEST(DateTimeYearTests, Year_2025) {
    System::DateTime d(2025, 1, 1);
    EXPECT_EQ(d.getYearProperty(), 2025);
}

TEST(DateTimeMonthTests, Month_January) {
    System::DateTime d(2020, 1, 31);
    EXPECT_EQ(d.getMonthProperty(), 1);
}

TEST(DateTimeMonthTests, Month_December) {
    System::DateTime d(2020, 12, 1);
    EXPECT_EQ(d.getMonthProperty(), 12);
}

TEST(DateTimeDayTests, Day_First) {
    System::DateTime d(2020, 5, 1);
    EXPECT_EQ(d.getDayProperty(), 1);
}

TEST(DateTimeDayTests, Day_Last_31) {
    System::DateTime d(2020, 1, 31);
    EXPECT_EQ(d.getDayProperty(), 31);
}

// ===========================================================================
// Hour / Minute / Second / Millisecond
// ===========================================================================

TEST(DateTimeTimeTests, Midnight_AllZero) {
    System::DateTime d(2021, 7, 4, 0, 0, 0);
    EXPECT_EQ(d.getHourProperty(),        0);
    EXPECT_EQ(d.getMinuteProperty(),      0);
    EXPECT_EQ(d.getSecondProperty(),      0);
    EXPECT_EQ(d.getMillisecondProperty(), 0);
}

TEST(DateTimeTimeTests, MaxTimeComponents) {
    System::DateTime d(2021, 12, 31, 23, 59, 59, 999);
    EXPECT_EQ(d.getHourProperty(),        23);
    EXPECT_EQ(d.getMinuteProperty(),      59);
    EXPECT_EQ(d.getSecondProperty(),      59);
    EXPECT_EQ(d.getMillisecondProperty(), 999);
}

TEST(DateTimeTimeTests, MillisecondRoundTrip) {
    for (int ms : {0, 1, 499, 500, 998, 999}) {
        System::DateTime d(2000, 1, 1, 0, 0, 0, ms);
        EXPECT_EQ(d.getMillisecondProperty(), ms) << "ms=" << ms;
    }
}

// ===========================================================================
// DayOfWeek
// ===========================================================================

TEST(DateTimeDayOfWeekTests, Monday_2025_06_09) {
    // 2025-06-09 is a Monday
    System::DateTime d(2025, 6, 9);
    EXPECT_EQ(d.getDayOfWeekProperty(), System::DayOfWeek::Monday);
}

TEST(DateTimeDayOfWeekTests, Sunday_2025_06_08) {
    // 2025-06-08 is a Sunday
    System::DateTime d(2025, 6, 8);
    EXPECT_EQ(d.getDayOfWeekProperty(), System::DayOfWeek::Sunday);
}

TEST(DateTimeDayOfWeekTests, Saturday_2025_06_07) {
    System::DateTime d(2025, 6, 7);
    EXPECT_EQ(d.getDayOfWeekProperty(), System::DayOfWeek::Saturday);
}

TEST(DateTimeDayOfWeekTests, Wednesday_2000_01_05) {
    // 2000-01-05 is a Wednesday
    System::DateTime d(2000, 1, 5);
    EXPECT_EQ(d.getDayOfWeekProperty(), System::DayOfWeek::Wednesday);
}

// ===========================================================================
// DayOfYear
// ===========================================================================

TEST(DateTimeDayOfYearTests, Jan1_Is1) {
    System::DateTime d(2020, 1, 1);
    EXPECT_EQ(d.getDayOfYearProperty(), 1);
}

TEST(DateTimeDayOfYearTests, Dec31_LeapYear_Is366) {
    System::DateTime d(2020, 12, 31); // 2020 is leap
    EXPECT_EQ(d.getDayOfYearProperty(), 366);
}

TEST(DateTimeDayOfYearTests, Dec31_NonLeap_Is365) {
    System::DateTime d(2021, 12, 31);
    EXPECT_EQ(d.getDayOfYearProperty(), 365);
}

TEST(DateTimeDayOfYearTests, Mar1_NonLeap_Is60) {
    System::DateTime d(2021, 3, 1); // 31 (Jan) + 28 (Feb) + 1 = day 60
    EXPECT_EQ(d.getDayOfYearProperty(), 60);
}

TEST(DateTimeDayOfYearTests, Mar1_LeapYear_Is61) {
    System::DateTime d(2020, 3, 1); // 31 + 29 + 1 = day 61
    EXPECT_EQ(d.getDayOfYearProperty(), 61);
}

// ===========================================================================
// getTodayProperty
// ===========================================================================

TEST(DateTimeTodayTests, Today_TimeIsZero) {
    System::DateTime today = System::DateTime::getTodayProperty();
    EXPECT_EQ(today.getHourProperty(),        0);
    EXPECT_EQ(today.getMinuteProperty(),      0);
    EXPECT_EQ(today.getSecondProperty(),      0);
    EXPECT_EQ(today.getMillisecondProperty(), 0);
}

TEST(DateTimeTodayTests, Today_YearIsReasonable) {
    System::DateTime today = System::DateTime::getTodayProperty();
    EXPECT_GE(today.getYearProperty(), 2025);
    EXPECT_LE(today.getYearProperty(), 2100);
}

// ===========================================================================
// ToString — now returns ISO-8601 style string
// ===========================================================================

TEST(DateTimeToStringTests, Format_2000_01_01) {
    System::DateTime d(2000, 1, 1, 0, 0, 0);
    EXPECT_EQ(d.ToString(), "2000-01-01 00:00:00");
}

TEST(DateTimeToStringTests, Format_WithTime) {
    System::DateTime d(2025, 6, 10, 14, 5, 3);
    EXPECT_EQ(d.ToString(), "2025-06-10 14:05:03");
}

// ===========================================================================
// Arithmetic still works after refactor
// ===========================================================================

TEST(DateTimeArithmeticTests, AddDay) {
    System::DateTime d(2020, 1, 31);
    System::DateTime next = d.Add(System::TimeSpan(864000000000LL)); // +1 day
    EXPECT_EQ(next.getMonthProperty(), 2);
    EXPECT_EQ(next.getDayProperty(),   1);
    EXPECT_EQ(next.getYearProperty(),  2020);
}

TEST(DateTimeArithmeticTests, SubtractDates_OneDayApart) {
    System::DateTime a(2020, 6, 2);
    System::DateTime b(2020, 6, 1);
    System::TimeSpan diff = a.Subtract(b);
    EXPECT_EQ(diff.getTicksProperty(), 864000000000LL);
}
