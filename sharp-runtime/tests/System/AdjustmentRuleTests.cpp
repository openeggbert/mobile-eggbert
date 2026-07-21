// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/TimeZoneInfo.hpp"

using System::TimeZoneInfo;
using System::DateTime;
using System::TimeSpan;
using System::DayOfWeek;

// Helpers
static TimeZoneInfo::TransitionTime fixedTT(int month = 3, int day = 14) {
    return TimeZoneInfo::TransitionTime::CreateFixedDateRule(DateTime{}, month, day);
}
static TimeZoneInfo::TransitionTime floatTT(int month = 10, int week = 4) {
    return TimeZoneInfo::TransitionTime::CreateFloatingDateRule(
        DateTime{}, month, week, DayOfWeek::Sunday);
}

// ---------------------------------------------------------------------------
// 5-param CreateAdjustmentRule — properties
// ---------------------------------------------------------------------------

TEST(AdjustmentRuleTests, Create5_DateStartStored) {
    DateTime start(2020, 1, 1), end(2020, 12, 31);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), floatTT());
    EXPECT_EQ(r->getDateStartProperty().getYearProperty(), 2020);
    EXPECT_EQ(r->getDateStartProperty().getMonthProperty(), 1);
}

TEST(AdjustmentRuleTests, Create5_DateEndStored) {
    DateTime start(2020, 1, 1), end(2025, 6, 30);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), floatTT());
    EXPECT_EQ(r->getDateEndProperty().getYearProperty(), 2025);
}

TEST(AdjustmentRuleTests, Create5_DaylightDeltaStored) {
    DateTime start(2020, 1, 1), end(2020, 12, 31);
    TimeSpan delta = TimeSpan::FromHours(1);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, delta, fixedTT(), floatTT());
    EXPECT_EQ(r->getDaylightDeltaProperty().getTotalHoursProperty(), 1.0);
}

TEST(AdjustmentRuleTests, Create5_TransitionStartStored) {
    auto tt = fixedTT(4, 7);
    DateTime start(2020, 1, 1), end(2020, 12, 31);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, tt, floatTT());
    EXPECT_EQ(r->getDaylightTransitionStartProperty().getMonthProperty(), 4);
    EXPECT_EQ(r->getDaylightTransitionStartProperty().getDayProperty(), 7);
}

TEST(AdjustmentRuleTests, Create5_TransitionEndStored) {
    auto tt = floatTT(11, 3);
    DateTime start(2020, 1, 1), end(2020, 12, 31);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), tt);
    EXPECT_EQ(r->getDaylightTransitionEndProperty().getMonthProperty(), 11);
    EXPECT_EQ(r->getDaylightTransitionEndProperty().getWeekProperty(), 3);
}

TEST(AdjustmentRuleTests, Create5_BaseUtcOffsetDelta_IsZero) {
    DateTime start(2020, 1, 1), end(2020, 12, 31);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), floatTT());
    EXPECT_EQ(r->getBaseUtcOffsetDeltaProperty().getTotalSecondsProperty(), 0.0);
}

TEST(AdjustmentRuleTests, Create5_NoDaylightTransitions_IsFalse) {
    DateTime start(2020, 1, 1), end(2020, 12, 31);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), floatTT());
    EXPECT_FALSE(r->getNoDaylightTransitionsProperty());
}

// ---------------------------------------------------------------------------
// 6-param CreateAdjustmentRule
// ---------------------------------------------------------------------------

TEST(AdjustmentRuleTests, Create6_BaseUtcOffsetDeltaStored) {
    DateTime start(2021, 1, 1), end(2021, 12, 31);
    TimeSpan baseDelta = TimeSpan::FromHours(2);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), floatTT(), baseDelta);
    EXPECT_EQ(r->getBaseUtcOffsetDeltaProperty().getTotalHoursProperty(), 2.0);
}

TEST(AdjustmentRuleTests, Create6_OtherFieldsPreserved) {
    DateTime start(2021, 3, 1), end(2021, 11, 1);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::FromMinutes(30), fixedTT(5, 1), floatTT(9, 2),
        TimeSpan::FromHours(1));
    EXPECT_EQ(r->getDateStartProperty().getMonthProperty(), 3);
    EXPECT_EQ(r->getDateEndProperty().getMonthProperty(), 11);
    EXPECT_EQ(r->getDaylightDeltaProperty().getTotalMinutesProperty(), 30.0);
    EXPECT_EQ(r->getDaylightTransitionStartProperty().getMonthProperty(), 5);
    EXPECT_EQ(r->getDaylightTransitionEndProperty().getMonthProperty(), 9);
}

// ---------------------------------------------------------------------------
// getNoDaylightTransitionsProperty
// ---------------------------------------------------------------------------

TEST(AdjustmentRuleTests, NoDaylightTransitions_DefaultIsFalse) {
    DateTime start(2020, 1, 1), end(2020, 12, 31);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), floatTT());
    EXPECT_FALSE(r->getNoDaylightTransitionsProperty());
}

// ---------------------------------------------------------------------------
// getHasDaylightSavingProperty
// ---------------------------------------------------------------------------

TEST(AdjustmentRuleTests, HasDaylightSaving_ZeroDelta_DefaultTransitions_False) {
    DateTime start(2020, 1, 1), end(2020, 12, 31);
    // Zero delta, default transitions → no DST
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero,
        TimeZoneInfo::TransitionTime{}, TimeZoneInfo::TransitionTime{});
    EXPECT_FALSE(r->getHasDaylightSavingProperty());
}

TEST(AdjustmentRuleTests, HasDaylightSaving_NonZeroDelta_True) {
    DateTime start(2020, 1, 1), end(2020, 12, 31);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::FromHours(1), fixedTT(), floatTT());
    EXPECT_TRUE(r->getHasDaylightSavingProperty());
}

TEST(AdjustmentRuleTests, HasDaylightSaving_NonDefaultTransitionStart_True) {
    DateTime start(2020, 1, 1), end(2020, 12, 31);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), TimeZoneInfo::TransitionTime{});
    EXPECT_TRUE(r->getHasDaylightSavingProperty());
}

// ---------------------------------------------------------------------------
// Equals — now includes BaseUtcOffsetDelta
// ---------------------------------------------------------------------------

TEST(AdjustmentRuleTests, Equals_SameBaseUtcOffsetDelta_True) {
    DateTime start(2022, 1, 1), end(2022, 12, 31);
    TimeSpan baseDelta = TimeSpan::FromHours(1);
    auto a = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), floatTT(), baseDelta);
    auto b = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), floatTT(), baseDelta);
    EXPECT_TRUE(a->Equals(*b));
    EXPECT_TRUE(*a == *b);
}

TEST(AdjustmentRuleTests, Equals_DiffBaseUtcOffsetDelta_False) {
    DateTime start(2022, 1, 1), end(2022, 12, 31);
    auto a = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), floatTT(), TimeSpan::FromHours(1));
    auto b = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), floatTT(), TimeSpan::FromHours(2));
    EXPECT_FALSE(a->Equals(*b));
    EXPECT_TRUE(*a != *b);
}

TEST(AdjustmentRuleTests, Equals_DiffDaylightDelta_False) {
    DateTime start(2022, 1, 1), end(2022, 12, 31);
    auto a = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::FromHours(1), fixedTT(), floatTT());
    auto b = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::FromHours(2), fixedTT(), floatTT());
    EXPECT_FALSE(a->Equals(*b));
}

// ---------------------------------------------------------------------------
// GetHashCode
// ---------------------------------------------------------------------------

TEST(AdjustmentRuleTests, GetHashCode_SameStart_SameHash) {
    DateTime start(2023, 3, 1), end(2023, 11, 1);
    auto a = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), floatTT());
    auto b = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::FromHours(1), fixedTT(), floatTT());
    // Hash is based only on DateStart ticks
    EXPECT_EQ(a->GetHashCode(), b->GetHashCode());
}

TEST(AdjustmentRuleTests, GetHashCode_DiffStart_LikelyDiffHash) {
    DateTime start1(2020, 1, 1), start2(2021, 1, 1);
    DateTime end(2025, 12, 31);
    auto a = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start1, end, TimeSpan::Zero, fixedTT(), floatTT());
    auto b = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start2, end, TimeSpan::Zero, fixedTT(), floatTT());
    EXPECT_NE(a->GetHashCode(), b->GetHashCode());
}

TEST(AdjustmentRuleTests, GetHashCode_IsConsistent) {
    DateTime start(2020, 1, 1), end(2020, 12, 31);
    auto r = TimeZoneInfo::AdjustmentRule::CreateAdjustmentRule(
        start, end, TimeSpan::Zero, fixedTT(), floatTT());
    EXPECT_EQ(r->GetHashCode(), r->GetHashCode());
}
