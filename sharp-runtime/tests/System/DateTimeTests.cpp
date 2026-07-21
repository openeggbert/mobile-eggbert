// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include "System/ArgumentOutOfRangeException.hpp"
#include "System/DateTime.hpp"
#include "System/TimeSpan.hpp"

using System::DateTime;
using System::TimeSpan;

// .NET tick constants (100-ns units)
static constexpr long long kTicksPerSecond = 10'000'000LL;
static constexpr long long kTicksPerMinute = 600'000'000LL;
static constexpr long long kTicksPerHour   = 36'000'000'000LL;
static constexpr long long kTicksPerDay    = 864'000'000'000LL;
// Unix epoch expressed as .NET ticks (Jan 1 1970 relative to Jan 1 0001)
static constexpr long long kUnixEpochTicks = 621'355'968'000'000'000LL;

// ---------------------------------------------------------------------------
// Construction & Ticks
// ---------------------------------------------------------------------------

TEST(DateTimeTests, DefaultCtorHasZeroTicks) {
    DateTime dt;
    EXPECT_EQ(dt.getTicksProperty(), 0LL);
}

TEST(DateTimeTests, ConstructFromTicks) {
    DateTime dt(kUnixEpochTicks);
    EXPECT_EQ(dt.getTicksProperty(), kUnixEpochTicks);
}

TEST(DateTimeTests, ConstructFromKnownTicks) {
    // From .NET DateTimeTests.cs: new DateTime(999999999999999999) is valid
    DateTime dt(999'999'999'999'999'999LL);
    EXPECT_EQ(dt.getTicksProperty(), 999'999'999'999'999'999LL);
}

TEST(DateTimeTests, ConstructFromNegativeTicks_Throws) {
    EXPECT_THROW(DateTime(-1LL), System::ArgumentOutOfRangeException);
}

TEST(DateTimeTests, ConstructFromTicksBeyondMaxValue_Throws) {
    EXPECT_THROW(DateTime(DateTime::MaxTicks + 1), System::ArgumentOutOfRangeException);
}

TEST(DateTimeTests, AddDays_BeyondMaxValue_Throws) {
    DateTime maxDate(DateTime::MaxTicks);
    EXPECT_THROW(maxDate.AddDays(1), System::ArgumentOutOfRangeException);
}

TEST(DateTimeTests, AddDays_BeforeMinValue_Throws) {
    DateTime minDate(0);
    EXPECT_THROW(minDate.AddDays(-1), System::ArgumentOutOfRangeException);
}

TEST(DateTimeTests, AddDays_LargeValue_ThrowsInsteadOfOverflowing) {
    // static_cast<longcs>(days) * TicksPerDay for a merely large (not extreme) int day
    // count is real signed-overflow UB in C++ without an upfront bounds check -- must throw
    // ArgumentOutOfRangeException instead of invoking UB or wrapping to a bogus DateTime.
    DateTime dt(2000, 1, 1);
    EXPECT_THROW(dt.AddDays(1000000000), System::ArgumentOutOfRangeException);
    EXPECT_THROW(dt.AddDays(-1000000000), System::ArgumentOutOfRangeException);
}

TEST(DateTimeTests, AddHours_LargeValue_ThrowsInsteadOfOverflowing) {
    DateTime dt(2000, 1, 1);
    EXPECT_THROW(dt.AddHours(1000000000), System::ArgumentOutOfRangeException);
    EXPECT_THROW(dt.AddHours(-1000000000), System::ArgumentOutOfRangeException);
}

TEST(DateTimeTests, EqualityBasedOnTicks) {
    DateTime a(1000LL), b(1000LL), c(2000LL);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

// ---------------------------------------------------------------------------
// Add / Subtract with TimeSpan
// ---------------------------------------------------------------------------

TEST(DateTimeTests, AddTimeSpanOneDay) {
    DateTime dt(kUnixEpochTicks);
    TimeSpan oneDay = TimeSpan::FromDays(1.0);
    DateTime next = dt.Add(oneDay);
    EXPECT_EQ(next.getTicksProperty(), kUnixEpochTicks + kTicksPerDay);
}

TEST(DateTimeTests, AddTimeSpanOneHour) {
    DateTime dt(kUnixEpochTicks);
    TimeSpan oneHour = TimeSpan::FromHours(1.0);
    DateTime next = dt.Add(oneHour);
    EXPECT_EQ(next.getTicksProperty(), kUnixEpochTicks + kTicksPerHour);
}

TEST(DateTimeTests, AddTimeSpanOneSecond) {
    DateTime dt(1'000'000'000LL);
    TimeSpan oneSecond = TimeSpan::FromSeconds(1.0);
    DateTime next = dt.Add(oneSecond);
    EXPECT_EQ(next.getTicksProperty(), 1'000'000'000LL + kTicksPerSecond);
}

TEST(DateTimeTests, Add_TimeSpanNearMaxValue_ThrowsInsteadOfOverflowing) {
    // ticks_ (bounded to [0, MaxTicks]) + a TimeSpan near TimeSpan::MaxValue's ticks (up to
    // ~Int64::MaxValue) is real signed-overflow UB in C++ without the unsigned-arithmetic
    // range check AddTicks now uses -- must throw ArgumentOutOfRangeException.
    DateTime dt(2000, 1, 1);
    EXPECT_THROW(dt.Add(TimeSpan::MaxValue), System::ArgumentOutOfRangeException);
}

TEST(DateTimeTests, Subtract_TimeSpanNearMaxValue_ThrowsInsteadOfOverflowing) {
    DateTime dt(2000, 1, 1);
    EXPECT_THROW(dt.Subtract(TimeSpan::MaxValue), System::ArgumentOutOfRangeException);
}

TEST(DateTimeTests, SubtractTimeSpan) {
    DateTime dt(kUnixEpochTicks + kTicksPerDay);
    TimeSpan oneDay = TimeSpan::FromDays(1.0);
    DateTime prev = dt.Subtract(oneDay);
    EXPECT_EQ(prev.getTicksProperty(), kUnixEpochTicks);
}

TEST(DateTimeTests, AddSubtractRoundtrip) {
    DateTime original(kUnixEpochTicks);
    TimeSpan ts = TimeSpan::FromHours(5.0);
    DateTime modified = original.Add(ts);
    DateTime restored = modified.Subtract(ts);
    EXPECT_EQ(restored, original);
}

// ---------------------------------------------------------------------------
// Subtract DateTime → TimeSpan
// ---------------------------------------------------------------------------

TEST(DateTimeTests, SubtractDateTimesGivesTimeSpan) {
    DateTime a(kUnixEpochTicks + kTicksPerDay);
    DateTime b(kUnixEpochTicks);
    TimeSpan diff = a.Subtract(b);
    EXPECT_EQ(diff.getTicksProperty(), kTicksPerDay);
}

TEST(DateTimeTests, SubtractDateTimesSameGivesZero) {
    DateTime dt(kUnixEpochTicks);
    TimeSpan diff = dt.Subtract(dt);
    EXPECT_EQ(diff.getTicksProperty(), 0LL);
}

TEST(DateTimeTests, SubtractDateTimesNegative) {
    // earlier - later → negative TimeSpan
    DateTime earlier(kUnixEpochTicks);
    DateTime later(kUnixEpochTicks + kTicksPerHour);
    TimeSpan diff = earlier.Subtract(later);
    EXPECT_EQ(diff.getTicksProperty(), -kTicksPerHour);
}

// ---------------------------------------------------------------------------
// Comparison operators
// ---------------------------------------------------------------------------

TEST(DateTimeTests, CompareEqualTicks) {
    DateTime a(1000LL), b(1000LL);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_FALSE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_TRUE(a >= b);
}

TEST(DateTimeTests, CompareLessThan) {
    DateTime earlier(1000LL), later(2000LL);
    EXPECT_LT(earlier, later);
    EXPECT_GT(later, earlier);
    EXPECT_LE(earlier, later);
    EXPECT_GE(later, earlier);
}

// ---------------------------------------------------------------------------
// getTimeOfDayProperty
// ---------------------------------------------------------------------------

TEST(DateTimeTests, TimeOfDayAtStartOfDay) {
    // A DateTime at an exact day boundary → time of day is zero
    DateTime dt(2LL * kTicksPerDay);   // exactly 2 days from tick 0
    TimeSpan tod = dt.getTimeOfDayProperty();
    EXPECT_EQ(tod.getTicksProperty(), 0LL);
}

TEST(DateTimeTests, TimeOfDayHalfwayThroughDay) {
    // 1.5 days → time of day is 0.5 day = 12 hours
    long long halfDay = kTicksPerDay / 2;
    DateTime dt(kTicksPerDay + halfDay);
    TimeSpan tod = dt.getTimeOfDayProperty();
    EXPECT_EQ(tod.getTicksProperty(), halfDay);
}

TEST(DateTimeTests, TimeOfDayLessThanOneDay) {
    // Any time-of-day value must be < 1 day
    DateTime dt(kUnixEpochTicks + kTicksPerHour * 15);
    TimeSpan tod = dt.getTimeOfDayProperty();
    EXPECT_GE(tod.getTicksProperty(), 0LL);
    EXPECT_LT(tod.getTicksProperty(), kTicksPerDay);
}

// ---------------------------------------------------------------------------
// getNowProperty
// ---------------------------------------------------------------------------

TEST(DateTimeTests, NowIsAfterUnixEpoch) {
    // DateTime.Now ticks must be > Unix epoch (1970-01-01) ticks
    DateTime now = DateTime::getNowProperty();
    EXPECT_GT(now.getTicksProperty(), kUnixEpochTicks);
}

TEST(DateTimeTests, NowMonotonicallyIncreases) {
    // Two consecutive Now() calls: second >= first
    DateTime t1 = DateTime::getNowProperty();
    DateTime t2 = DateTime::getNowProperty();
    EXPECT_GE(t2.getTicksProperty(), t1.getTicksProperty());
}

// ---------------------------------------------------------------------------
// ToString
// ---------------------------------------------------------------------------

TEST(DateTimeTests, ToStringContainsTicks) {
    // ToString() now returns ISO-8601 style "YYYY-MM-DD HH:MM:SS"
    // 123456789 ticks = ~12 seconds from .NET epoch 0001-01-01
    DateTime dt(123456789LL);
    std::string s = dt.ToString();
    EXPECT_NE(s.find("0001"), std::string::npos);  // year
    EXPECT_NE(s.find("00:00:12"), std::string::npos); // ~12 seconds
}

TEST(DateTimeTests, ToStringZeroTicks) {
    // DateTime(0) == 0001-01-01 00:00:00
    std::string s = DateTime().ToString();
    EXPECT_EQ(s, "0001-01-01 00:00:00");
}

// ---------------------------------------------------------------------------
// Arithmetic stress
// ---------------------------------------------------------------------------

TEST(DateTimeTests, Add365Days) {
    // Adding 365 days via FromDays should increment ticks by 365 * TicksPerDay
    DateTime dt(kUnixEpochTicks);
    TimeSpan year = TimeSpan::FromDays(365.0);
    DateTime next = dt.Add(year);
    EXPECT_EQ(next.getTicksProperty(), kUnixEpochTicks + 365LL * kTicksPerDay);
}

TEST(DateTimeTests, ChainedAddSubtract) {
    DateTime dt(kUnixEpochTicks);
    // +1h -30min +15s → net +3600-1800+15 = +1815 seconds
    TimeSpan net = TimeSpan::FromSeconds(3600.0 - 1800.0 + 15.0);
    DateTime result = dt.Add(TimeSpan::FromHours(1.0))
                        .Subtract(TimeSpan::FromMinutes(30.0))
                        .Add(TimeSpan::FromSeconds(15.0));
    EXPECT_EQ(result.getTicksProperty(), kUnixEpochTicks + net.getTicksProperty());
}

// --- ToString(format) ---

TEST(DateTimeTests, ToString_Format_yyyyMMdd) {
    DateTime dt(2024, 3, 5);
    EXPECT_EQ(dt.ToString("yyyy-MM-dd"), "2024-03-05");
}

TEST(DateTimeTests, ToString_Format_HHmmss) {
    DateTime dt(2024, 1, 1, 9, 5, 7);
    EXPECT_EQ(dt.ToString("HH:mm:ss"), "09:05:07");
}

TEST(DateTimeTests, ToString_Format_Full) {
    DateTime dt(2024, 12, 31, 23, 59, 58);
    EXPECT_EQ(dt.ToString("yyyy-MM-dd HH:mm:ss"), "2024-12-31 23:59:58");
}

TEST(DateTimeTests, ToString_Format_ShortDate_US) {
    DateTime dt(2024, 6, 15);
    EXPECT_EQ(dt.ToString("MM/dd/yyyy"), "06/15/2024");
}

TEST(DateTimeTests, ToString_Format_SingleTokens) {
    DateTime dt(2024, 6, 5, 8, 7, 9);
    EXPECT_EQ(dt.ToString("M/d/yyyy H:m:s"), "6/5/2024 8:7:9");
}

TEST(DateTimeTests, ToString_Format_Milliseconds) {
    DateTime dt(2024, 1, 1, 0, 0, 0, 123);
    EXPECT_EQ(dt.ToString("HH:mm:ss.fff"), "00:00:00.123");
}

TEST(DateTimeTests, ToString_Format_Literal) {
    DateTime dt(2024, 6, 15);
    EXPECT_EQ(dt.ToString("'Date: 'yyyy-MM-dd"), "Date: 2024-06-15");
}

TEST(DateTimeTests, ToString_Format_12Hour) {
    DateTime dt(2024, 1, 1, 13, 0, 0);
    EXPECT_EQ(dt.ToString("hh:mm"), "01:00");
}

TEST(DateTimeTests, ToString_Format_ddd_AbbreviatedDayName) {
    DateTime dt(2015, 10, 21); // a Wednesday
    EXPECT_EQ(dt.ToString("ddd"), "Wed");
}

TEST(DateTimeTests, ToString_Format_dddd_FullDayName) {
    DateTime dt(2015, 10, 21); // a Wednesday
    EXPECT_EQ(dt.ToString("dddd"), "Wednesday");
}

TEST(DateTimeTests, ToString_Format_MMM_AbbreviatedMonthName) {
    DateTime dt(2015, 10, 21);
    EXPECT_EQ(dt.ToString("MMM"), "Oct");
}

TEST(DateTimeTests, ToString_Format_MMMM_FullMonthName) {
    DateTime dt(2015, 10, 21);
    EXPECT_EQ(dt.ToString("MMMM"), "October");
}

TEST(DateTimeTests, ToString_Format_Rfc1123Style) {
    DateTime dt(2015, 10, 21, 7, 28, 0);
    EXPECT_EQ(dt.ToString("ddd, dd MMM yyyy HH:mm:ss"), "Wed, 21 Oct 2015 07:28:00");
}

// --- Parse ---

TEST(DateTimeTests, Parse_DateOnly) {
    DateTime dt = DateTime::Parse("2024-06-15");
    EXPECT_EQ(dt.getYearProperty(),  2024);
    EXPECT_EQ(dt.getMonthProperty(), 6);
    EXPECT_EQ(dt.getDayProperty(),   15);
    EXPECT_EQ(dt.getHourProperty(),  0);
}

TEST(DateTimeTests, Parse_DateAndTime) {
    DateTime dt = DateTime::Parse("2024-06-15 10:30:45");
    EXPECT_EQ(dt.getHourProperty(),   10);
    EXPECT_EQ(dt.getMinuteProperty(), 30);
    EXPECT_EQ(dt.getSecondProperty(), 45);
}

TEST(DateTimeTests, Parse_ISO8601_T_Separator) {
    DateTime dt = DateTime::Parse("2024-06-15T10:30:45");
    EXPECT_EQ(dt.getHourProperty(), 10);
}

TEST(DateTimeTests, Parse_WithMilliseconds) {
    DateTime dt = DateTime::Parse("2024-06-15 10:30:45.123");
    EXPECT_EQ(dt.getMillisecondProperty(), 123);
}

TEST(DateTimeTests, Parse_Invalid_Throws) {
    EXPECT_THROW(DateTime::Parse("not-a-date"), std::exception);
}

// --- TryParse ---

TEST(DateTimeTests, TryParse_ValidDate_ReturnsTrue) {
    DateTime dt;
    EXPECT_TRUE(DateTime::TryParse("2024-06-15", dt));
    EXPECT_EQ(dt.getYearProperty(), 2024);
}

TEST(DateTimeTests, TryParse_ValidDateTime_ReturnsTrue) {
    DateTime dt;
    EXPECT_TRUE(DateTime::TryParse("2024-06-15 08:00:00", dt));
    EXPECT_EQ(dt.getHourProperty(), 8);
}

TEST(DateTimeTests, TryParse_Invalid_ReturnsFalse) {
    DateTime dt;
    EXPECT_FALSE(DateTime::TryParse("hello", dt));
}

TEST(DateTimeTests, TryParse_BadMonth_ReturnsFalse) {
    DateTime dt;
    EXPECT_FALSE(DateTime::TryParse("2024-13-01", dt));
}

// Regression tests for a wave-3 audit finding: TryParse computed the fractional-second
// digit count as `s.size() - 20`, which counted a trailing ISO-8601 'Z'/offset marker as if
// it were extra fraction digits, corrupting the millisecond normalisation (".123Z" was read
// as 4 digits and truncated 123ms down to 12ms instead of being recognised as 3 digits).

TEST(DateTimeTests, TryParse_MillisecondsWithTrailingZ_ParsesCorrectly) {
    DateTime dt;
    ASSERT_TRUE(DateTime::TryParse("2024-06-15 10:30:45.123Z", dt));
    EXPECT_EQ(dt.getMillisecondProperty(), 123);
}

TEST(DateTimeTests, TryParse_MillisecondsWithTrailingOffset_ParsesCorrectly) {
    DateTime dt;
    ASSERT_TRUE(DateTime::TryParse("2024-06-15 10:30:45.123+02:00", dt));
    EXPECT_EQ(dt.getMillisecondProperty(), 123);
}

TEST(DateTimeTests, TryParse_SevenFractionalDigitsWithTrailingZ_TruncatesToMilliseconds) {
    DateTime dt;
    ASSERT_TRUE(DateTime::TryParse("2024-06-15 10:30:45.1234567Z", dt));
    EXPECT_EQ(dt.getMillisecondProperty(), 123);
}

TEST(DateTimeTests, TryParse_MillisecondsNoTimezone_StillParsesCorrectly) {
    DateTime dt;
    ASSERT_TRUE(DateTime::TryParse("2024-06-15 10:30:45.123", dt));
    EXPECT_EQ(dt.getMillisecondProperty(), 123);
}

// Regression tests for a second, separate TryParse bug (found while verifying the fix above,
// but out of scope for that commit): the fractional-second branch was gated on
// `s.size() >= 23`, i.e. "at least 3 characters after the dot", so a 1- or 2-digit fraction
// (".5", ".56", ".5Z") was silently skipped entirely -- Millisecond stayed 0 -- instead of
// being scaled up to milliseconds like a 3+ digit fraction already was. Fixed by gating on
// "at least 1 character after the dot" and letting the digit-counting loop (added by the fix
// above) handle any length correctly.
TEST(DateTimeTests, TryParse_OneFractionalDigit_ScalesToMilliseconds) {
    DateTime dt;
    ASSERT_TRUE(DateTime::TryParse("2024-06-15 10:30:45.5", dt));
    EXPECT_EQ(dt.getMillisecondProperty(), 500);
}

TEST(DateTimeTests, TryParse_OneFractionalDigitWithTrailingZ_ScalesToMilliseconds) {
    DateTime dt;
    ASSERT_TRUE(DateTime::TryParse("2024-06-15 10:30:45.5Z", dt));
    EXPECT_EQ(dt.getMillisecondProperty(), 500);
}

TEST(DateTimeTests, TryParse_TwoFractionalDigits_ScalesToMilliseconds) {
    DateTime dt;
    ASSERT_TRUE(DateTime::TryParse("2024-06-15 10:30:45.56", dt));
    EXPECT_EQ(dt.getMillisecondProperty(), 560);
}

TEST(DateTimeTests, TryParse_TwoFractionalDigitsWithTrailingOffset_ScalesToMilliseconds) {
    DateTime dt;
    ASSERT_TRUE(DateTime::TryParse("2024-06-15 10:30:45.56+02:00", dt));
    EXPECT_EQ(dt.getMillisecondProperty(), 560);
}

// ---------------------------------------------------------------------------
// MinValue / MaxValue / UnixEpoch
// ---------------------------------------------------------------------------

TEST(DateTimeTests, MinValue_IsZeroTicks) {
    EXPECT_EQ(DateTime::MinValue.getTicksProperty(), 0LL);
}

TEST(DateTimeTests, MaxValue_Is_9999_12_31) {
    EXPECT_EQ(DateTime::MaxValue.getYearProperty(),  9999);
    EXPECT_EQ(DateTime::MaxValue.getMonthProperty(), 12);
    EXPECT_EQ(DateTime::MaxValue.getDayProperty(),   31);
}

TEST(DateTimeTests, UnixEpoch_Is_1970_01_01) {
    EXPECT_EQ(DateTime::UnixEpoch.getTicksProperty(), kUnixEpochTicks);
    EXPECT_EQ(DateTime::UnixEpoch.getYearProperty(),  1970);
    EXPECT_EQ(DateTime::UnixEpoch.getMonthProperty(), 1);
    EXPECT_EQ(DateTime::UnixEpoch.getDayProperty(),   1);
}

// ---------------------------------------------------------------------------
// Equals / CompareTo / GetHashCode
// ---------------------------------------------------------------------------

TEST(DateTimeTests, Equals_SameTicks) {
    DateTime a(1000LL), b(1000LL);
    EXPECT_TRUE(a.Equals(b));
}

TEST(DateTimeTests, Equals_DifferentTicks) {
    DateTime a(1000LL), b(2000LL);
    EXPECT_FALSE(a.Equals(b));
}

TEST(DateTimeTests, CompareTo_Less) {
    DateTime a(1000LL), b(2000LL);
    EXPECT_LT(a.CompareTo(b), 0);
}

TEST(DateTimeTests, CompareTo_Equal) {
    DateTime a(1000LL), b(1000LL);
    EXPECT_EQ(a.CompareTo(b), 0);
}

TEST(DateTimeTests, CompareTo_Greater) {
    DateTime a(2000LL), b(1000LL);
    EXPECT_GT(a.CompareTo(b), 0);
}

TEST(DateTimeTests, GetHashCode_SameTicksMatch) {
    DateTime a(123456789LL), b(123456789LL);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(DateTimeTests, GetHashCode_DifferentTicksDiffer) {
    DateTime a(1000LL), b(2000LL);
    EXPECT_NE(a.GetHashCode(), b.GetHashCode());
}

// ---------------------------------------------------------------------------
// IsLeapYear / DaysInMonth
// ---------------------------------------------------------------------------

TEST(DateTimeTests, IsLeapYear_DivisibleBy4) {
    EXPECT_TRUE(DateTime::IsLeapYear(2024));
}

TEST(DateTimeTests, IsLeapYear_DivisibleBy100NotLeap) {
    EXPECT_FALSE(DateTime::IsLeapYear(1900));
}

TEST(DateTimeTests, IsLeapYear_DivisibleBy400IsLeap) {
    EXPECT_TRUE(DateTime::IsLeapYear(2000));
}

TEST(DateTimeTests, IsLeapYear_OutOfRangeThrows) {
    EXPECT_THROW(DateTime::IsLeapYear(0), System::ArgumentOutOfRangeException);
    EXPECT_THROW(DateTime::IsLeapYear(10000), System::ArgumentOutOfRangeException);
}

TEST(DateTimeTests, DaysInMonth_February_LeapYear) {
    EXPECT_EQ(DateTime::DaysInMonth(2024, 2), 29);
}

TEST(DateTimeTests, DaysInMonth_February_NonLeapYear) {
    EXPECT_EQ(DateTime::DaysInMonth(2023, 2), 28);
}

TEST(DateTimeTests, DaysInMonth_January) {
    EXPECT_EQ(DateTime::DaysInMonth(2024, 1), 31);
}

TEST(DateTimeTests, DaysInMonth_OutOfRangeThrows) {
    EXPECT_THROW(DateTime::DaysInMonth(2024, 0), System::ArgumentOutOfRangeException);
    EXPECT_THROW(DateTime::DaysInMonth(2024, 13), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// AddMonths / AddYears / AddTicks
// ---------------------------------------------------------------------------

TEST(DateTimeTests, AddMonths_SimpleForward) {
    DateTime dt(2024, 1, 15);
    DateTime r = dt.AddMonths(2);
    EXPECT_EQ(r.getYearProperty(),  2024);
    EXPECT_EQ(r.getMonthProperty(), 3);
    EXPECT_EQ(r.getDayProperty(),   15);
}

TEST(DateTimeTests, AddMonths_ClampsDay) {
    DateTime dt(2024, 1, 31);
    DateTime r = dt.AddMonths(1); // Feb 2024 has 29 days
    EXPECT_EQ(r.getMonthProperty(), 2);
    EXPECT_EQ(r.getDayProperty(),   29);
}

TEST(DateTimeTests, AddMonths_CrossYear) {
    DateTime dt(2024, 11, 15);
    DateTime r = dt.AddMonths(3);
    EXPECT_EQ(r.getYearProperty(),  2025);
    EXPECT_EQ(r.getMonthProperty(), 2);
}

TEST(DateTimeTests, AddMonths_Negative) {
    DateTime dt(2024, 3, 15);
    DateTime r = dt.AddMonths(-2);
    EXPECT_EQ(r.getMonthProperty(), 1);
}

TEST(DateTimeTests, AddMonths_PreservesTimeOfDay) {
    DateTime dt(2024, 1, 15, 10, 30, 0);
    DateTime r = dt.AddMonths(1);
    EXPECT_EQ(r.getHourProperty(),   10);
    EXPECT_EQ(r.getMinuteProperty(), 30);
}

TEST(DateTimeTests, AddMonths_OutOfRangeThrows) {
    DateTime dt(2024, 1, 15);
    EXPECT_THROW(dt.AddMonths(-200000), System::ArgumentOutOfRangeException);
}

TEST(DateTimeTests, AddYears_LeapDayClamps) {
    DateTime dt(2020, 2, 29);
    DateTime r = dt.AddYears(1);
    EXPECT_EQ(r.getYearProperty(),  2021);
    EXPECT_EQ(r.getMonthProperty(), 2);
    EXPECT_EQ(r.getDayProperty(),   28);
}

TEST(DateTimeTests, AddYears_OutOfRangeThrows) {
    DateTime dt(2024, 1, 15);
    EXPECT_THROW(dt.AddYears(-20000), System::ArgumentOutOfRangeException);
}

TEST(DateTimeTests, AddTicks_Simple) {
    DateTime dt(1000LL);
    DateTime r = dt.AddTicks(500LL);
    EXPECT_EQ(r.getTicksProperty(), 1500LL);
}

TEST(DateTimeTests, AddTicks_BelowMinThrows) {
    DateTime dt(0LL);
    EXPECT_THROW(dt.AddTicks(-1LL), System::ArgumentOutOfRangeException);
}

TEST(DateTimeTests, AddTicks_AboveMaxThrows) {
    DateTime dt(DateTime::MaxValue);
    EXPECT_THROW(dt.AddTicks(1LL), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// Arithmetic operators
// ---------------------------------------------------------------------------

TEST(DateTimeTests, OperatorPlus_TimeSpan) {
    DateTime dt(kUnixEpochTicks);
    DateTime r = dt + TimeSpan::FromDays(1.0);
    EXPECT_EQ(r.getTicksProperty(), kUnixEpochTicks + kTicksPerDay);
}

TEST(DateTimeTests, OperatorMinus_TimeSpan) {
    DateTime dt(kUnixEpochTicks + kTicksPerDay);
    DateTime r = dt - TimeSpan::FromDays(1.0);
    EXPECT_EQ(r.getTicksProperty(), kUnixEpochTicks);
}

TEST(DateTimeTests, OperatorMinus_DateTime) {
    DateTime a(kUnixEpochTicks + kTicksPerDay);
    DateTime b(kUnixEpochTicks);
    TimeSpan diff = a - b;
    EXPECT_EQ(diff.getTicksProperty(), kTicksPerDay);
}
