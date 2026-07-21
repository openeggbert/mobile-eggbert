// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include "System/ArgumentOutOfRangeException.hpp"
#include "System/DateOnly.hpp"
#include "System/DateTime.hpp"
#include "System/FormatException.hpp"
#include "System/TimeOnly.hpp"
#include "System/TimeSpan.hpp"

using System::DateOnly;
using System::DateTime;
using System::TimeOnly;
using System::TimeSpan;

// ===========================================================================
// DateOnly — AddDays / AddMonths / AddYears
// ===========================================================================

TEST(DateOnlyTests, Constructor_InvalidDayForMonth_Throws) {
    EXPECT_THROW(DateOnly(2023, 2, 30), System::ArgumentOutOfRangeException);
}

TEST(DateOnlyTests, Constructor_Feb29_NonLeapYear_Throws) {
    EXPECT_THROW(DateOnly(2023, 2, 29), System::ArgumentOutOfRangeException);
}

TEST(DateOnlyTests, Constructor_Feb29_LeapYear_Succeeds) {
    EXPECT_NO_THROW(DateOnly(2024, 2, 29));
}

TEST(DateOnlyTests, Constructor_MonthOutOfRange_Throws) {
    EXPECT_THROW(DateOnly(2023, 13, 1), System::ArgumentOutOfRangeException);
}

TEST(DateOnlyTests, TryParse_InvalidDayForMonth_ReturnsFalse) {
    DateOnly d;
    EXPECT_FALSE(DateOnly::TryParse("2023-02-30", d));
    EXPECT_FALSE(DateOnly::TryParse("2023-02-29", d)); // non-leap year
}

TEST(DateOnlyTests, AddDays_Positive) {
    DateOnly d(2024, 1, 30);
    DateOnly r = d.AddDays(3);
    EXPECT_EQ(r.getYearProperty(),  2024);
    EXPECT_EQ(r.getMonthProperty(), 2);
    EXPECT_EQ(r.getDayProperty(),   2);
}

TEST(DateOnlyTests, AddDays_Negative) {
    DateOnly d(2024, 3, 1);
    DateOnly r = d.AddDays(-1);
    EXPECT_EQ(r.getMonthProperty(), 2);
    EXPECT_EQ(r.getDayProperty(),   29); // 2024 is leap year
}

TEST(DateOnlyTests, AddDays_CrossYear) {
    DateOnly d(2023, 12, 31);
    DateOnly r = d.AddDays(1);
    EXPECT_EQ(r.getYearProperty(),  2024);
    EXPECT_EQ(r.getMonthProperty(), 1);
    EXPECT_EQ(r.getDayProperty(),   1);
}

TEST(DateOnlyTests, AddMonths_Positive) {
    DateOnly d(2024, 11, 15);
    DateOnly r = d.AddMonths(3);
    EXPECT_EQ(r.getYearProperty(),  2025);
    EXPECT_EQ(r.getMonthProperty(), 2);
    EXPECT_EQ(r.getDayProperty(),   15);
}

TEST(DateOnlyTests, AddMonths_ClampDay) {
    DateOnly d(2024, 1, 31);
    DateOnly r = d.AddMonths(1); // Feb has 29 days in 2024
    EXPECT_EQ(r.getMonthProperty(), 2);
    EXPECT_EQ(r.getDayProperty(),   29);
}

TEST(DateOnlyTests, AddMonths_Negative) {
    DateOnly d(2024, 3, 15);
    DateOnly r = d.AddMonths(-2);
    EXPECT_EQ(r.getMonthProperty(), 1);
}

TEST(DateOnlyTests, AddYears_Positive) {
    DateOnly d(2020, 2, 29); // leap day
    DateOnly r = d.AddYears(1);
    EXPECT_EQ(r.getYearProperty(),  2021);
    EXPECT_EQ(r.getMonthProperty(), 2);
    EXPECT_EQ(r.getDayProperty(),   28); // clamped
}

// ===========================================================================
// DateOnly — FromDateTime
// ===========================================================================

TEST(DateOnlyTests, FromDateTime_ExtractsDate) {
    DateTime dt(2025, 6, 13, 14, 30, 0);
    DateOnly d = DateOnly::FromDateTime(dt);
    EXPECT_EQ(d.getYearProperty(),  2025);
    EXPECT_EQ(d.getMonthProperty(), 6);
    EXPECT_EQ(d.getDayProperty(),   13);
}

// ===========================================================================
// DateOnly — Parse / TryParse
// ===========================================================================

TEST(DateOnlyTests, Parse_ValidIso) {
    DateOnly d = DateOnly::Parse("2025-06-13");
    EXPECT_EQ(d.getYearProperty(),  2025);
    EXPECT_EQ(d.getMonthProperty(), 6);
    EXPECT_EQ(d.getDayProperty(),   13);
}

TEST(DateOnlyTests, TryParse_Valid) {
    DateOnly d;
    EXPECT_TRUE(DateOnly::TryParse("2000-01-01", d));
    EXPECT_EQ(d.getYearProperty(), 2000);
}

TEST(DateOnlyTests, TryParse_Invalid_ReturnsFalse) {
    DateOnly d;
    EXPECT_FALSE(DateOnly::TryParse("not-a-date", d));
    EXPECT_FALSE(DateOnly::TryParse("20250613", d)); // missing dashes
}

TEST(DateOnlyTests, Parse_InvalidThrows) {
    EXPECT_THROW(DateOnly::Parse("bad"), System::FormatException);
}

// ===========================================================================
// DateOnly — ToString(format)
// ===========================================================================

TEST(DateOnlyTests, ToStringFormat_Iso) {
    DateOnly d(2025, 6, 3);
    EXPECT_EQ(d.ToString("yyyy-MM-dd"), "2025-06-03");
}

TEST(DateOnlyTests, ToStringFormat_ShortYear) {
    DateOnly d(2025, 1, 1);
    EXPECT_EQ(d.ToString("yy/M/d"), "25/1/1");
}

TEST(DateOnlyTests, ToStringFormat_Literal) {
    DateOnly d(2025, 12, 25);
    EXPECT_EQ(d.ToString("yyyy'.'MM'.'dd"), "2025.12.25");
}

TEST(DateOnlyTests, MinValue_Is_0001_01_01) {
    EXPECT_EQ(System::DateOnly::MinValue.getYearProperty(),  1);
    EXPECT_EQ(System::DateOnly::MinValue.getMonthProperty(), 1);
    EXPECT_EQ(System::DateOnly::MinValue.getDayProperty(),   1);
}

TEST(DateOnlyTests, MaxValue_Is_9999_12_31) {
    EXPECT_EQ(System::DateOnly::MaxValue.getYearProperty(),  9999);
    EXPECT_EQ(System::DateOnly::MaxValue.getMonthProperty(), 12);
    EXPECT_EQ(System::DateOnly::MaxValue.getDayProperty(),   31);
}

TEST(DateOnlyTests, DayNumber_MinValue_IsZero) {
    EXPECT_EQ(System::DateOnly::MinValue.getDayNumberProperty(), 0);
}

TEST(DateOnlyTests, FromDayNumber_RoundTrip) {
    System::DateOnly d(2025, 6, 14);
    int dn = d.getDayNumberProperty();
    EXPECT_EQ(System::DateOnly::FromDayNumber(dn), d);
}

TEST(DateOnlyTests, DayOfWeek_Known) {
    // 2025-06-14 is a Saturday
    System::DateOnly d(2025, 6, 14);
    EXPECT_EQ(d.getDayOfWeekProperty(), System::DayOfWeek::Saturday);
}

TEST(DateOnlyTests, DayOfYear_Jan1_IsOne) {
    EXPECT_EQ(System::DateOnly(2025, 1, 1).getDayOfYearProperty(), 1);
}

TEST(DateOnlyTests, DayOfYear_Dec31_NonLeap) {
    EXPECT_EQ(System::DateOnly(2025, 12, 31).getDayOfYearProperty(), 365);
}

TEST(DateOnlyTests, DayOfYear_Dec31_Leap) {
    EXPECT_EQ(System::DateOnly(2024, 12, 31).getDayOfYearProperty(), 366);
}

TEST(DateOnlyTests, CompareTo_Less) {
    System::DateOnly a(2020, 1, 1), b(2021, 1, 1);
    EXPECT_LT(a.CompareTo(b), 0);
}

TEST(DateOnlyTests, CompareTo_Equal) {
    System::DateOnly a(2020, 6, 15), b(2020, 6, 15);
    EXPECT_EQ(a.CompareTo(b), 0);
}

TEST(DateOnlyTests, CompareTo_Greater) {
    System::DateOnly a(2022, 1, 1), b(2021, 1, 1);
    EXPECT_GT(a.CompareTo(b), 0);
}

TEST(DateOnlyTests, Equals_SameDate) {
    System::DateOnly a(2025, 3, 1), b(2025, 3, 1);
    EXPECT_TRUE(a.Equals(b));
}

TEST(DateOnlyTests, Equals_DifferentDate) {
    System::DateOnly a(2025, 3, 1), b(2025, 3, 2);
    EXPECT_FALSE(a.Equals(b));
}

TEST(DateOnlyTests, Deconstruct) {
    System::DateOnly d(2025, 11, 7);
    System::intcs y, m, day;
    d.Deconstruct(y, m, day);
    EXPECT_EQ(y, 2025); EXPECT_EQ(m, 11); EXPECT_EQ(day, 7);
}

TEST(DateOnlyTests, GetHashCode_MatchesDayNumber) {
    System::DateOnly d(2025, 6, 14);
    EXPECT_EQ(d.GetHashCode(), d.getDayNumberProperty());
}

TEST(DateOnlyTests, GetHashCode_DifferentDatesDiffer) {
    System::DateOnly a(2025, 6, 14), b(2025, 6, 15);
    EXPECT_NE(a.GetHashCode(), b.GetHashCode());
}

TEST(DateOnlyTests, ToDateTime_CombinesDateAndTime) {
    DateOnly d(2025, 6, 14);
    TimeOnly t(14, 30, 45, 500);
    DateTime dt = d.ToDateTime(t);
    EXPECT_EQ(dt.getYearProperty(),        2025);
    EXPECT_EQ(dt.getMonthProperty(),       6);
    EXPECT_EQ(dt.getDayProperty(),         14);
    EXPECT_EQ(dt.getHourProperty(),        14);
    EXPECT_EQ(dt.getMinuteProperty(),      30);
    EXPECT_EQ(dt.getSecondProperty(),      45);
    EXPECT_EQ(dt.getMillisecondProperty(), 500);
}

// ===========================================================================
// TimeOnly — AddHours / AddMinutes
// ===========================================================================

TEST(TimeOnlyTests, AddHours_Simple) {
    TimeOnly t(10, 0, 0);
    TimeOnly r = t.AddHours(3);
    EXPECT_EQ(r.getHourProperty(), 13);
}

TEST(TimeOnlyTests, AddHours_WrapAround) {
    TimeOnly t(23, 0, 0);
    TimeOnly r = t.AddHours(2);
    EXPECT_EQ(r.getHourProperty(), 1);
}

TEST(TimeOnlyTests, AddHours_Negative) {
    TimeOnly t(1, 0, 0);
    TimeOnly r = t.AddHours(-3);
    EXPECT_EQ(r.getHourProperty(), 22);
}

TEST(TimeOnlyTests, AddMinutes_Simple) {
    TimeOnly t(12, 45, 0);
    TimeOnly r = t.AddMinutes(20);
    EXPECT_EQ(r.getHourProperty(),   13);
    EXPECT_EQ(r.getMinuteProperty(),  5);
}

// ===========================================================================
// TimeOnly — FromTimeSpan / ToTimeSpan
// ===========================================================================

TEST(TimeOnlyTests, FromTimeSpan_TwoHours) {
    TimeOnly t = TimeOnly::FromTimeSpan(TimeSpan::FromHours(2));
    EXPECT_EQ(t.getHourProperty(),   2);
    EXPECT_EQ(t.getMinuteProperty(), 0);
}

TEST(TimeOnlyTests, ToTimeSpan_RoundTrip) {
    TimeOnly t(8, 30, 15, 0);
    TimeSpan ts = t.ToTimeSpan();
    EXPECT_EQ(static_cast<int>(ts.getTotalMinutesProperty()), 510);
}

// ===========================================================================
// TimeOnly — FromDateTime
// ===========================================================================

TEST(TimeOnlyTests, FromDateTime_ExtractsTime) {
    DateTime dt(2025, 6, 13, 14, 30, 45);
    TimeOnly t = TimeOnly::FromDateTime(dt);
    EXPECT_EQ(t.getHourProperty(),   14);
    EXPECT_EQ(t.getMinuteProperty(), 30);
    EXPECT_EQ(t.getSecondProperty(), 45);
}

// ===========================================================================
// TimeOnly — Parse / TryParse
// ===========================================================================

TEST(TimeOnlyTests, Parse_Valid) {
    TimeOnly t = TimeOnly::Parse("14:30:00");
    EXPECT_EQ(t.getHourProperty(),   14);
    EXPECT_EQ(t.getMinuteProperty(), 30);
    EXPECT_EQ(t.getSecondProperty(),  0);
}

TEST(TimeOnlyTests, Parse_WithMilliseconds) {
    TimeOnly t = TimeOnly::Parse("08:05:03.750");
    EXPECT_EQ(t.getMillisecondProperty(), 750);
}

TEST(TimeOnlyTests, TryParse_Valid) {
    TimeOnly t;
    EXPECT_TRUE(TimeOnly::TryParse("00:00:00", t));
}

TEST(TimeOnlyTests, TryParse_Invalid_ReturnsFalse) {
    TimeOnly t;
    EXPECT_FALSE(TimeOnly::TryParse("not-a-time", t));
}

TEST(TimeOnlyTests, Parse_InvalidThrows) {
    EXPECT_THROW(TimeOnly::Parse("bad"), System::FormatException);
}

// ===========================================================================
// TimeOnly — ToString(format)
// ===========================================================================

TEST(TimeOnlyTests, ToStringFormat_HHmmss) {
    TimeOnly t(8, 5, 3);
    EXPECT_EQ(t.ToString("HH:mm:ss"), "08:05:03");
}

TEST(TimeOnlyTests, ToStringFormat_12Hour) {
    TimeOnly t(14, 30, 0);
    EXPECT_EQ(t.ToString("hh':'mm"), "02:30");
}

TEST(TimeOnlyTests, ToStringFormat_WithMilliseconds) {
    TimeOnly t(12, 0, 0, 123);
    EXPECT_EQ(t.ToString("HH:mm:ss'.'fff"), "12:00:00.123");
}
