// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <stdexcept>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/TimeZoneInfo.hpp"
#include "System/TimeZoneNotFoundException.hpp"

using System::TimeZoneInfo;
using System::TimeSpan;
using System::DateTime;
using System::TimeZoneNotFoundException;

// ---------------------------------------------------------------------------
// Utc() static zone
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, Utc_Id_IsUTC) {
    EXPECT_EQ(TimeZoneInfo::Utc().getIdProperty(), "UTC");
}

TEST(TimeZoneInfoTests, Utc_DisplayName_NonEmpty) {
    EXPECT_FALSE(TimeZoneInfo::Utc().getDisplayNameProperty().empty());
}

TEST(TimeZoneInfoTests, Utc_StandardName_NonEmpty) {
    EXPECT_FALSE(TimeZoneInfo::Utc().getStandardNameProperty().empty());
}

TEST(TimeZoneInfoTests, Utc_DaylightName_NonEmpty) {
    EXPECT_FALSE(TimeZoneInfo::Utc().getDaylightNameProperty().empty());
}

TEST(TimeZoneInfoTests, Utc_BaseUtcOffset_IsZero) {
    EXPECT_TRUE(TimeZoneInfo::Utc().getBaseUtcOffsetProperty() == TimeSpan::Zero);
}

TEST(TimeZoneInfoTests, Utc_SupportsDst_IsFalse) {
    EXPECT_FALSE(TimeZoneInfo::Utc().getSupportsDaylightSavingTimeProperty());
}

// ---------------------------------------------------------------------------
// Local() static zone
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, Local_Id_IsLocal) {
    EXPECT_EQ(TimeZoneInfo::Local().getIdProperty(), "Local");
}

TEST(TimeZoneInfoTests, Local_DisplayName_NonEmpty) {
    EXPECT_FALSE(TimeZoneInfo::Local().getDisplayNameProperty().empty());
}

TEST(TimeZoneInfoTests, Local_BaseUtcOffset_InValidRange) {
    double hours = TimeZoneInfo::Local().getBaseUtcOffsetProperty().getTotalHoursProperty();
    EXPECT_GE(hours, -14.0);
    EXPECT_LE(hours,  14.0);
}

// ---------------------------------------------------------------------------
// Instance methods (UTC zone has zero offset and no DST)
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, Utc_IsDaylightSavingTime_ReturnsFalse) {
    DateTime dt;
    EXPECT_FALSE(TimeZoneInfo::Utc().IsDaylightSavingTime(dt));
}

TEST(TimeZoneInfoTests, Utc_GetUtcOffset_IsZero) {
    DateTime dt;
    EXPECT_TRUE(TimeZoneInfo::Utc().GetUtcOffset(dt) == TimeSpan::Zero);
}

TEST(TimeZoneInfoTests, Utc_IsAmbiguousTime_ReturnsFalse) {
    DateTime dt;
    EXPECT_FALSE(TimeZoneInfo::Utc().IsAmbiguousTime(dt));
}

TEST(TimeZoneInfoTests, Utc_IsInvalidTime_ReturnsFalse) {
    DateTime dt;
    EXPECT_FALSE(TimeZoneInfo::Utc().IsInvalidTime(dt));
}

TEST(TimeZoneInfoTests, Utc_ConvertTimeToUtc_ZeroOffsetPreservesRawValue) {
    // UTC zone has zero offset, so ConvertTimeToUtc == Add(Zero) == identity on ticks
    DateTime dt;
    DateTime result = TimeZoneInfo::Utc().ConvertTimeToUtc(dt);
    EXPECT_EQ(result.getTicksProperty(), dt.getTicksProperty());
}

// ---------------------------------------------------------------------------
// FindSystemTimeZoneById
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, FindSystemTimeZoneById_UTC_ReturnsNonNull) {
    auto tz = TimeZoneInfo::FindSystemTimeZoneById("UTC");
    EXPECT_NE(tz, nullptr);
}

TEST(TimeZoneInfoTests, FindSystemTimeZoneById_UTC_HasCorrectId) {
    auto tz = TimeZoneInfo::FindSystemTimeZoneById("UTC");
    EXPECT_EQ(tz->getIdProperty(), "UTC");
}

TEST(TimeZoneInfoTests, FindSystemTimeZoneById_Unknown_ThrowsInvalidArgument) {
    EXPECT_THROW(TimeZoneInfo::FindSystemTimeZoneById("America/Unknown"),
                 TimeZoneNotFoundException);
}

TEST(TimeZoneInfoTests, FindSystemTimeZoneById_PathTraversal_Throws) {
    EXPECT_THROW(TimeZoneInfo::FindSystemTimeZoneById("../../etc/passwd"),
                 TimeZoneNotFoundException);
}

TEST(TimeZoneInfoTests, FindSystemTimeZoneById_EuropePrague_OffsetInRange) {
    auto tz = TimeZoneInfo::FindSystemTimeZoneById("Europe/Prague");
    double hours = tz->getBaseUtcOffsetProperty().getTotalHoursProperty();
    // CET=+1, CEST=+2
    EXPECT_GE(hours, 1.0);
    EXPECT_LE(hours, 2.0);
}

TEST(TimeZoneInfoTests, FindSystemTimeZoneById_AmericaNewYork_NegativeOffset) {
    auto tz = TimeZoneInfo::FindSystemTimeZoneById("America/New_York");
    double hours = tz->getBaseUtcOffsetProperty().getTotalHoursProperty();
    EXPECT_LT(hours, 0.0);
}

// ---------------------------------------------------------------------------
// GetSystemTimeZones
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, GetSystemTimeZones_ReturnsAtLeastTwoZones) {
    auto zones = TimeZoneInfo::GetSystemTimeZones();
    EXPECT_GE(zones.size(), 2u);
}

TEST(TimeZoneInfoTests, GetSystemTimeZones_AllNonNull) {
    for (const auto& tz : TimeZoneInfo::GetSystemTimeZones()) {
        EXPECT_NE(tz, nullptr);
    }
}

// ---------------------------------------------------------------------------
// CreateCustomTimeZone
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, CreateCustomTimeZone_HasCorrectId) {
    auto tz = TimeZoneInfo::CreateCustomTimeZone("MyZone", TimeSpan::Zero, "My Zone", "My Standard");
    EXPECT_EQ(tz->getIdProperty(), "MyZone");
}

TEST(TimeZoneInfoTests, CreateCustomTimeZone_HasCorrectDisplayName) {
    auto tz = TimeZoneInfo::CreateCustomTimeZone("X", TimeSpan::Zero, "Display Name", "Standard");
    EXPECT_EQ(tz->getDisplayNameProperty(), "Display Name");
}

TEST(TimeZoneInfoTests, CreateCustomTimeZone_HasCorrectStandardName) {
    auto tz = TimeZoneInfo::CreateCustomTimeZone("X", TimeSpan::Zero, "D", "StdName");
    EXPECT_EQ(tz->getStandardNameProperty(), "StdName");
}

TEST(TimeZoneInfoTests, CreateCustomTimeZone_NoSupportsDst) {
    auto tz = TimeZoneInfo::CreateCustomTimeZone("X", TimeSpan::Zero, "D", "S");
    EXPECT_FALSE(tz->getSupportsDaylightSavingTimeProperty());
}

TEST(TimeZoneInfoTests, SupportsDaylightSavingTime_ReflectsWholeYearNotJustNow) {
    // America/New_York observes DST every year; this must be true regardless of
    // which calendar month the test happens to run in.
    auto ny = TimeZoneInfo::FindSystemTimeZoneById("America/New_York");
    EXPECT_TRUE(ny->getSupportsDaylightSavingTimeProperty());
    // America/Phoenix (Arizona) never observes DST, in any month.
    auto phoenix = TimeZoneInfo::FindSystemTimeZoneById("America/Phoenix");
    EXPECT_FALSE(phoenix->getSupportsDaylightSavingTimeProperty());
}

TEST(TimeZoneInfoTests, CreateCustomTimeZone_PositiveOffsetStoredCorrectly) {
    TimeSpan offset = TimeSpan::FromHours(2);
    auto tz = TimeZoneInfo::CreateCustomTimeZone("CET", offset, "Central European Time", "CET");
    EXPECT_TRUE(tz->getBaseUtcOffsetProperty() == offset);
}

TEST(TimeZoneInfoTests, CreateCustomTimeZone_NegativeOffsetStoredCorrectly) {
    TimeSpan offset = TimeSpan::FromHours(-5);
    auto tz = TimeZoneInfo::CreateCustomTimeZone("EST", offset, "Eastern Standard Time", "EST");
    EXPECT_TRUE(tz->getBaseUtcOffsetProperty() == offset);
}

// ---------------------------------------------------------------------------
// ConvertTimeBySystemTimeZoneId
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, ConvertTimeBySystemTimeZoneId_UTC_DoesNotThrow) {
    DateTime dt;
    EXPECT_NO_THROW(TimeZoneInfo::ConvertTimeBySystemTimeZoneId(dt, "UTC"));
}

TEST(TimeZoneInfoTests, ConvertTimeBySystemTimeZoneId_Unknown_Throws) {
    DateTime dt;
    EXPECT_THROW(TimeZoneInfo::ConvertTimeBySystemTimeZoneId(dt, "Mars/Olympus"),
                 TimeZoneNotFoundException);
}

// ---------------------------------------------------------------------------
// ConvertTime / ConvertTimeFromUtc
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, ConvertTime_DestZone_AddsOffset) {
    auto utcPlus2 = TimeZoneInfo::CreateCustomTimeZone("+02", TimeSpan::FromHours(2), "+2", "+2");
    DateTime utc(2025, 1, 1, 12, 0, 0);
    DateTime local = TimeZoneInfo::ConvertTime(utc, *utcPlus2);
    EXPECT_EQ(local.getHourProperty(), 14);
}

TEST(TimeZoneInfoTests, ConvertTime_SrcDst_Correct) {
    auto src = TimeZoneInfo::CreateCustomTimeZone("src", TimeSpan::FromHours(2), "s", "s");
    auto dst = TimeZoneInfo::CreateCustomTimeZone("dst", TimeSpan::FromHours(5), "d", "d");
    DateTime srcTime(2025, 1, 1, 10, 0, 0);
    DateTime dstTime = TimeZoneInfo::ConvertTime(srcTime, *src, *dst);
    EXPECT_EQ(dstTime.getHourProperty(), 13); // +3h difference
}

TEST(TimeZoneInfoTests, ConvertTimeFromUtc_AddsOffset) {
    auto utcPlus3 = TimeZoneInfo::CreateCustomTimeZone("+03", TimeSpan::FromHours(3), "+3", "+3");
    DateTime utc(2025, 6, 1, 9, 0, 0);
    DateTime local = TimeZoneInfo::ConvertTimeFromUtc(utc, *utcPlus3);
    EXPECT_EQ(local.getHourProperty(), 12);
}

// ---------------------------------------------------------------------------
// TryFindSystemTimeZoneById
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, TryFind_Utc_ReturnsTrue) {
    std::shared_ptr<TimeZoneInfo> tz;
    bool ok = TimeZoneInfo::TryFindSystemTimeZoneById("UTC", tz);
    EXPECT_TRUE(ok);
    ASSERT_NE(tz, nullptr);
    EXPECT_EQ(tz->getIdProperty(), "UTC");
}

TEST(TimeZoneInfoTests, TryFind_Unknown_ReturnsFalse) {
    std::shared_ptr<TimeZoneInfo> tz;
    bool ok = TimeZoneInfo::TryFindSystemTimeZoneById("Mars/Olympus", tz);
    EXPECT_FALSE(ok);
}

// ---------------------------------------------------------------------------
// Equals / HasSameRules / operators
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, Equals_SameId_True) {
    auto a = TimeZoneInfo::CreateCustomTimeZone("X", TimeSpan::FromHours(1), "X", "X");
    auto b = TimeZoneInfo::CreateCustomTimeZone("X", TimeSpan::FromHours(1), "X", "X");
    EXPECT_TRUE(a->Equals(*b));
}

TEST(TimeZoneInfoTests, Equals_DiffId_False) {
    auto a = TimeZoneInfo::CreateCustomTimeZone("A", TimeSpan::FromHours(1), "A", "A");
    auto b = TimeZoneInfo::CreateCustomTimeZone("B", TimeSpan::FromHours(1), "B", "B");
    EXPECT_FALSE(a->Equals(*b));
}

TEST(TimeZoneInfoTests, HasSameRules_SameOffset_True) {
    auto a = TimeZoneInfo::CreateCustomTimeZone("A", TimeSpan::FromHours(2), "A", "A");
    auto b = TimeZoneInfo::CreateCustomTimeZone("B", TimeSpan::FromHours(2), "B", "B");
    EXPECT_TRUE(a->HasSameRules(*b));
}

TEST(TimeZoneInfoTests, HasSameRules_DiffOffset_False) {
    auto a = TimeZoneInfo::CreateCustomTimeZone("A", TimeSpan::FromHours(1), "A", "A");
    auto b = TimeZoneInfo::CreateCustomTimeZone("B", TimeSpan::FromHours(2), "B", "B");
    EXPECT_FALSE(a->HasSameRules(*b));
}

TEST(TimeZoneInfoTests, OperatorEqual_SameId) {
    auto a = TimeZoneInfo::CreateCustomTimeZone("Z", TimeSpan::Zero, "Z", "Z");
    auto b = TimeZoneInfo::CreateCustomTimeZone("Z", TimeSpan::Zero, "Z", "Z");
    EXPECT_TRUE(*a == *b);
    EXPECT_FALSE(*a != *b);
}

// ---------------------------------------------------------------------------
// New API: HasIanaId, GetHashCode, ToString
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, HasIanaId_WithSlash_True) {
    auto tz = TimeZoneInfo::FindSystemTimeZoneById("Europe/Prague");
    EXPECT_TRUE(tz->getHasIanaIdProperty());
}

TEST(TimeZoneInfoTests, HasIanaId_UTC_False) {
    EXPECT_FALSE(TimeZoneInfo::Utc().getHasIanaIdProperty());
}

TEST(TimeZoneInfoTests, GetHashCode_SameId_SameHash) {
    auto a = TimeZoneInfo::CreateCustomTimeZone("TZ1", TimeSpan::Zero, "X", "X");
    auto b = TimeZoneInfo::CreateCustomTimeZone("TZ1", TimeSpan::Zero, "Y", "Y");
    EXPECT_EQ(a->GetHashCode(), b->GetHashCode());
}

TEST(TimeZoneInfoTests, ToString_ReturnsDisplayName) {
    auto tz = TimeZoneInfo::CreateCustomTimeZone("X", TimeSpan::Zero, "My Display", "X");
    EXPECT_EQ(tz->ToString(), "My Display");
}

// ---------------------------------------------------------------------------
// GetAmbiguousTimeOffsets / GetAdjustmentRules
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, GetAmbiguousTimeOffsets_DateTime_ReturnsEmpty) {
    DateTime dt;
    auto offsets = TimeZoneInfo::Utc().GetAmbiguousTimeOffsets(dt);
    EXPECT_TRUE(offsets.empty());
}

TEST(TimeZoneInfoTests, GetAdjustmentRules_ReturnsEmpty) {
    auto rules = TimeZoneInfo::Utc().GetAdjustmentRules();
    EXPECT_TRUE(rules.empty());
}

// ---------------------------------------------------------------------------
// GetSystemTimeZones(bool) overload
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, GetSystemTimeZones_SkipSorting_SameCount) {
    auto a = TimeZoneInfo::GetSystemTimeZones(false);
    auto b = TimeZoneInfo::GetSystemTimeZones(true);
    EXPECT_EQ(a.size(), b.size());
}

// ---------------------------------------------------------------------------
// Static ConvertTimeToUtc(DateTime, TimeZoneInfo)
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, ConvertTimeToUtc_Static_SubtractsOffset) {
    auto tz = TimeZoneInfo::CreateCustomTimeZone("+02", TimeSpan::FromHours(2), "+2", "+2");
    DateTime local(2025, 6, 1, 14, 0, 0);
    DateTime utc = TimeZoneInfo::ConvertTimeToUtc(local, *tz);
    EXPECT_EQ(utc.getHourProperty(), 12);
}

// ---------------------------------------------------------------------------
// ClearCachedData — no-op, must not throw
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, ClearCachedData_DoesNotThrow) {
    EXPECT_NO_THROW(TimeZoneInfo::ClearCachedData());
}

// ---------------------------------------------------------------------------
// TryConvertIanaIdToWindowsId / TryConvertWindowsIdToIanaId
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, TryConvertIanaToWindows_KnownId_ReturnsTrue) {
    std::string win;
    bool ok = TimeZoneInfo::TryConvertIanaIdToWindowsId("Europe/Prague", win);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(win.empty());
}

TEST(TimeZoneInfoTests, TryConvertIanaToWindows_KnownId_CorrectMapping) {
    std::string win;
    TimeZoneInfo::TryConvertIanaIdToWindowsId("America/New_York", win);
    EXPECT_EQ(win, "Eastern Standard Time");
}

TEST(TimeZoneInfoTests, TryConvertIanaToWindows_Unknown_ReturnsFalse) {
    std::string win;
    bool ok = TimeZoneInfo::TryConvertIanaIdToWindowsId("Mars/Olympus", win);
    EXPECT_FALSE(ok);
}

TEST(TimeZoneInfoTests, TryConvertWindowsToIana_KnownId_ReturnsTrue) {
    std::string iana;
    bool ok = TimeZoneInfo::TryConvertWindowsIdToIanaId("Eastern Standard Time", iana);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(iana.empty());
}

TEST(TimeZoneInfoTests, TryConvertWindowsToIana_Unknown_ReturnsFalse) {
    std::string iana;
    bool ok = TimeZoneInfo::TryConvertWindowsIdToIanaId("Fake Standard Time", iana);
    EXPECT_FALSE(ok);
}

TEST(TimeZoneInfoTests, TryConvertRoundtrip_IanaToWindowsToIana) {
    std::string win, iana2;
    ASSERT_TRUE(TimeZoneInfo::TryConvertIanaIdToWindowsId("Asia/Tokyo", win));
    ASSERT_TRUE(TimeZoneInfo::TryConvertWindowsIdToIanaId(win, iana2));
    EXPECT_FALSE(iana2.empty());
}

// ---------------------------------------------------------------------------
// ConvertTimeBySystemTimeZoneId (DateTimeOffset variant)
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, ConvertTimeBySystemTimeZoneId_DateTimeOffset_UTC_NoThrow) {
    System::DateTimeOffset dto;
    EXPECT_NO_THROW(TimeZoneInfo::ConvertTimeBySystemTimeZoneId(dto, "UTC"));
}

TEST(TimeZoneInfoTests, ConvertTimeBySystemTimeZoneId_DateTimeOffset_Unknown_Throws) {
    System::DateTimeOffset dto;
    EXPECT_THROW(TimeZoneInfo::ConvertTimeBySystemTimeZoneId(dto, "Mars/Olympus"),
                 TimeZoneNotFoundException);
}

// ---------------------------------------------------------------------------
// TransitionTime::GetHashCode
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, TransitionTime_GetHashCode_FixedRule) {
    System::DateTime tod;
    auto t = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 3, 14);
    // Fixed rules initialise week_ = 1, so hash = month ^ (week << 8) = 3 ^ (1 << 8)
    EXPECT_EQ(t.GetHashCode(), 3 ^ (1 << 8));
}

TEST(TimeZoneInfoTests, TransitionTime_GetHashCode_SameInputSameHash) {
    System::DateTime tod;
    auto a = TimeZoneInfo::TransitionTime::CreateFloatingDateRule(tod, 10, 4, System::DayOfWeek::Sunday);
    auto b = TimeZoneInfo::TransitionTime::CreateFloatingDateRule(tod, 10, 4, System::DayOfWeek::Sunday);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

// ---------------------------------------------------------------------------
// AdjustmentRule::GetHashCode / Equals / operators
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, AdjustmentRule_GetHashCode_Consistent) {
    System::DateTime start(2020, 1, 1), end(2020, 12, 31);
    System::DateTime tod;
    auto tt = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 3, 14);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, System::TimeSpan::Zero, tt, tt);
    EXPECT_EQ(r->GetHashCode(), r->GetHashCode());
}

TEST(TimeZoneInfoTests, AdjustmentRule_Equals_SameRules_True) {
    System::DateTime start(2020, 1, 1), end(2020, 12, 31);
    System::DateTime tod;
    auto tt = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 3, 14);
    auto a = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, System::TimeSpan::Zero, tt, tt);
    auto b = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, System::TimeSpan::Zero, tt, tt);
    EXPECT_TRUE(a->Equals(*b));
    EXPECT_TRUE(*a == *b);
    EXPECT_FALSE(*a != *b);
}

TEST(TimeZoneInfoTests, AdjustmentRule_Equals_DiffEnd_False) {
    System::DateTime start(2020, 1, 1);
    System::DateTime end1(2020, 12, 31), end2(2021, 12, 31);
    System::DateTime tod;
    auto tt = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 3, 14);
    auto a = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end1, System::TimeSpan::Zero, tt, tt);
    auto b = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end2, System::TimeSpan::Zero, tt, tt);
    EXPECT_FALSE(a->Equals(*b));
    EXPECT_TRUE(*a != *b);
}

// ---------------------------------------------------------------------------
// TransitionTime
// ---------------------------------------------------------------------------

TEST(TimeZoneInfoTests, TransitionTime_FixedDateRule) {
    DateTime tod;
    auto t = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 3, 14);
    EXPECT_TRUE(t.getIsFixedDateRuleProperty());
    EXPECT_EQ(t.getMonthProperty(), 3);
    EXPECT_EQ(t.getDayProperty(), 14);
}

TEST(TimeZoneInfoTests, TransitionTime_FloatingDateRule) {
    DateTime tod;
    auto t = TimeZoneInfo::TransitionTime::CreateFloatingDateRule(
        tod, 10, 4, System::DayOfWeek::Sunday);
    EXPECT_FALSE(t.getIsFixedDateRuleProperty());
    EXPECT_EQ(t.getMonthProperty(), 10);
    EXPECT_EQ(t.getWeekProperty(), 4);
    EXPECT_EQ(t.getDayOfWeekProperty(), System::DayOfWeek::Sunday);
}

TEST(TimeZoneInfoTests, TransitionTime_EqualityOperators) {
    DateTime tod;
    auto a = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 3, 25);
    auto b = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 3, 25);
    auto c = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 4, 1);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
}

TEST(TimeZoneInfoTests, TransitionTime_Equals_SameFixed_True) {
    DateTime tod;
    auto a = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 6, 15);
    auto b = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 6, 15);
    EXPECT_TRUE(a.Equals(b));
}

TEST(TimeZoneInfoTests, TransitionTime_Equals_DiffDay_False) {
    DateTime tod;
    auto a = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 6, 10);
    auto b = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 6, 20);
    EXPECT_FALSE(a.Equals(b));
}

TEST(TimeZoneInfoTests, TransitionTime_Equals_FixedVsFloating_False) {
    DateTime tod;
    auto fixed    = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 3, 14);
    auto floating = TimeZoneInfo::TransitionTime::CreateFloatingDateRule(
        tod, 3, 2, System::DayOfWeek::Sunday);
    EXPECT_FALSE(fixed.Equals(floating));
}

TEST(TimeZoneInfoTests, TransitionTime_Equals_FloatingSameFields_True) {
    DateTime tod;
    auto a = TimeZoneInfo::TransitionTime::CreateFloatingDateRule(
        tod, 10, 5, System::DayOfWeek::Saturday);
    auto b = TimeZoneInfo::TransitionTime::CreateFloatingDateRule(
        tod, 10, 5, System::DayOfWeek::Saturday);
    EXPECT_TRUE(a.Equals(b));
}

TEST(TimeZoneInfoTests, TransitionTime_CreateFixed_InvalidMonth_Throws) {
    DateTime tod;
    EXPECT_THROW(TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 0, 1),
                 System::ArgumentOutOfRangeException);
    EXPECT_THROW(TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 13, 1),
                 System::ArgumentOutOfRangeException);
}

TEST(TimeZoneInfoTests, TransitionTime_CreateFixed_InvalidDay_Throws) {
    DateTime tod;
    EXPECT_THROW(TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 1, 0),
                 System::ArgumentOutOfRangeException);
    EXPECT_THROW(TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 1, 32),
                 System::ArgumentOutOfRangeException);
}

TEST(TimeZoneInfoTests, TransitionTime_CreateFloating_InvalidMonth_Throws) {
    DateTime tod;
    EXPECT_THROW(TimeZoneInfo::TransitionTime::CreateFloatingDateRule(
        tod, 13, 1, System::DayOfWeek::Sunday), System::ArgumentOutOfRangeException);
}

TEST(TimeZoneInfoTests, TransitionTime_CreateFloating_InvalidWeek_Throws) {
    DateTime tod;
    EXPECT_THROW(TimeZoneInfo::TransitionTime::CreateFloatingDateRule(
        tod, 3, 6, System::DayOfWeek::Sunday), System::ArgumentOutOfRangeException);
    EXPECT_THROW(TimeZoneInfo::TransitionTime::CreateFloatingDateRule(
        tod, 3, 0, System::DayOfWeek::Sunday), System::ArgumentOutOfRangeException);
}

// Regression tests for a code-audit finding (ticket 243): CreateFixedDateRule/
// CreateFloatingDateRule previously did not validate timeOfDay at all. Verified against real
// .NET's TimeZoneInfo.TransitionTime.ValidateTransitionTime (TimeZoneInfo.TransitionTime.cs),
// which throws ArgumentException when timeOfDay.Ticks >= TimeSpan.TicksPerDay (i.e. it has a
// date component beyond DateTime.MinValue's implicit date) -- a full DateTime like year
// 2024/January/1 with a time-of-day fell straight through with no validation before this fix.
TEST(TimeZoneInfoTests, TransitionTime_CreateFixed_TimeOfDayWithDateComponent_Throws) {
    DateTime withDate(2024, 1, 1, 2, 0, 0);
    EXPECT_THROW(TimeZoneInfo::TransitionTime::CreateFixedDateRule(withDate, 3, 14),
                 System::ArgumentException);
}

TEST(TimeZoneInfoTests, TransitionTime_CreateFloating_TimeOfDayWithDateComponent_Throws) {
    DateTime withDate(2024, 1, 1, 2, 0, 0);
    EXPECT_THROW(TimeZoneInfo::TransitionTime::CreateFloatingDateRule(
        withDate, 10, 4, System::DayOfWeek::Sunday), System::ArgumentException);
}

TEST(TimeZoneInfoTests, TransitionTime_CreateFixed_ValidTimeOfDay_DoesNotThrow) {
    // A pure time-of-day (year/month/day fixed at DateTime.MinValue's 1/1/1) at
    // millisecond granularity must still be accepted.
    DateTime validTod(1, 1, 1, 2, 30, 0);
    EXPECT_NO_THROW(TimeZoneInfo::TransitionTime::CreateFixedDateRule(validTod, 3, 14));
}

TEST(TimeZoneInfoTests, TransitionTime_DayProperty_StoredByFixedRule) {
    DateTime tod;
    auto t = TimeZoneInfo::TransitionTime::CreateFixedDateRule(tod, 7, 4);
    EXPECT_EQ(t.getDayProperty(), 4);
    EXPECT_EQ(t.getMonthProperty(), 7);
    EXPECT_TRUE(t.getIsFixedDateRuleProperty());
}

TEST(TimeZoneInfoTests, TransitionTime_WeekProperty_StoredByFloatingRule) {
    DateTime tod;
    auto t = TimeZoneInfo::TransitionTime::CreateFloatingDateRule(
        tod, 11, 3, System::DayOfWeek::Thursday);
    EXPECT_EQ(t.getWeekProperty(), 3);
    EXPECT_EQ(t.getDayOfWeekProperty(), System::DayOfWeek::Thursday);
    EXPECT_FALSE(t.getIsFixedDateRuleProperty());
}

TEST(TimeZoneInfoTests, TransitionTime_GetHashCode_FloatingRule) {
    DateTime tod;
    auto t = TimeZoneInfo::TransitionTime::CreateFloatingDateRule(
        tod, 10, 4, System::DayOfWeek::Sunday);
    EXPECT_EQ(t.GetHashCode(), 10 ^ (4 << 8));
}
