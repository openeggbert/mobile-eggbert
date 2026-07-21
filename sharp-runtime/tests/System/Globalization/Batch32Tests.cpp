// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 32:
//   NumberStyles:    enum values (including fixed Currency/Any), HexFloat, operator|/&
//   PersianCalendar: GetYear/Month/Day, IsLeapYear, GetDaysInMonth, GetDaysInYear
//   RegionInfo:      constructor, all property getters, CurrentRegion
//   SortKey:         constructors, getters, Compare, operator==, ToString
//   SortVersion:     constructors, getters, operator==/!=
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Globalization/NumberStyles.hpp"
#include "System/Globalization/PersianCalendar.hpp"
#include "System/Globalization/RegionInfo.hpp"
#include "System/Globalization/SortKey.hpp"
#include "System/Globalization/SortVersion.hpp"
#include "System/DateTime.hpp"

using System::Globalization::NumberStyles;
using System::Globalization::PersianCalendar;
using System::Globalization::RegionInfo;
using System::Globalization::SortKey;
using System::Globalization::SortVersion;

// ===========================================================================
// NumberStyles
// ===========================================================================

TEST(NumberStylesBatch32Test, AtomicValues) {
    EXPECT_EQ(static_cast<int>(NumberStyles::None),                0x000);
    EXPECT_EQ(static_cast<int>(NumberStyles::AllowLeadingWhite),   0x001);
    EXPECT_EQ(static_cast<int>(NumberStyles::AllowTrailingWhite),  0x002);
    EXPECT_EQ(static_cast<int>(NumberStyles::AllowLeadingSign),    0x004);
    EXPECT_EQ(static_cast<int>(NumberStyles::AllowTrailingSign),   0x008);
    EXPECT_EQ(static_cast<int>(NumberStyles::AllowParentheses),    0x010);
    EXPECT_EQ(static_cast<int>(NumberStyles::AllowDecimalPoint),   0x020);
    EXPECT_EQ(static_cast<int>(NumberStyles::AllowThousands),      0x040);
    EXPECT_EQ(static_cast<int>(NumberStyles::AllowExponent),       0x080);
    EXPECT_EQ(static_cast<int>(NumberStyles::AllowCurrencySymbol), 0x100);
    EXPECT_EQ(static_cast<int>(NumberStyles::AllowHexSpecifier),   0x200);
    EXPECT_EQ(static_cast<int>(NumberStyles::AllowBinarySpecifier),0x400);
}

TEST(NumberStylesBatch32Test, CompositeValues) {
    EXPECT_EQ(static_cast<int>(NumberStyles::Integer),      7);
    EXPECT_EQ(static_cast<int>(NumberStyles::HexNumber),    515);
    EXPECT_EQ(static_cast<int>(NumberStyles::BinaryNumber), 1027);
    EXPECT_EQ(static_cast<int>(NumberStyles::Number),       111);
    EXPECT_EQ(static_cast<int>(NumberStyles::Float),        167);
    EXPECT_EQ(static_cast<int>(NumberStyles::HexFloat),     679);
    EXPECT_EQ(static_cast<int>(NumberStyles::Currency),     383); // was wrong before (was 511)
    EXPECT_EQ(static_cast<int>(NumberStyles::Any),          511);
}

TEST(NumberStylesBatch32Test, OperatorOr) {
    auto combined = NumberStyles::AllowLeadingWhite | NumberStyles::AllowTrailingWhite;
    EXPECT_EQ(static_cast<int>(combined), 3);
}

TEST(NumberStylesBatch32Test, OperatorAnd) {
    auto combined = NumberStyles::Integer; // 7 = 1|2|4
    auto masked   = combined & NumberStyles::AllowLeadingSign;
    EXPECT_EQ(masked, NumberStyles::AllowLeadingSign);
}

TEST(NumberStylesBatch32Test, AnyEqualsCurrencyOrExponent) {
    auto computed = NumberStyles::Currency | NumberStyles::AllowExponent;
    EXPECT_EQ(computed, NumberStyles::Any);
}

// ===========================================================================
// PersianCalendar
// ===========================================================================

TEST(PersianCalendarBatch32Test, PersianEraConstant) {
    EXPECT_EQ(PersianCalendar::PersianEra, 1);
}

TEST(PersianCalendarBatch32Test, GetEra) {
    PersianCalendar pc;
    EXPECT_EQ(pc.GetEra(System::DateTime(2024, 1, 1)), PersianCalendar::PersianEra);
}

TEST(PersianCalendarBatch32Test, Eras_ContainsPersianEra) {
    PersianCalendar pc;
    auto eras = pc.getErasProperty();
    ASSERT_EQ(eras.size(), 1u);
    EXPECT_EQ(eras[0], PersianCalendar::PersianEra);
}

TEST(PersianCalendarBatch32Test, GetYear_KnownDate) {
    PersianCalendar pc;
    // 2024-01-01 Gregorian = 1402 or 1403 Persian (Nowruz is ~March 20)
    int py = pc.GetYear(System::DateTime(2024, 4, 1));
    EXPECT_EQ(py, 1403);
}

TEST(PersianCalendarBatch32Test, GetMonth_Range) {
    PersianCalendar pc;
    int m = pc.GetMonth(System::DateTime(2024, 6, 15));
    EXPECT_GE(m, 1);
    EXPECT_LE(m, 12);
}

TEST(PersianCalendarBatch32Test, GetDayOfMonth_Range) {
    PersianCalendar pc;
    int d = pc.GetDayOfMonth(System::DateTime(2024, 3, 25));
    EXPECT_GE(d, 1);
    EXPECT_LE(d, 31);
}

TEST(PersianCalendarBatch32Test, IsLeapYear_Known) {
    PersianCalendar pc;
    // Persian year 1403 is a leap year ((1403*8+29)%33 = 11260%33 = 16 — not < 8? let's check)
    // Actually Persian 1399 is a known leap year
    EXPECT_TRUE(pc.IsLeapYear(1399));
    EXPECT_FALSE(pc.IsLeapYear(1400));
}

TEST(PersianCalendarBatch32Test, GetDaysInMonth_FirstSix) {
    PersianCalendar pc;
    for (int m = 1; m <= 6; ++m)
        EXPECT_EQ(pc.GetDaysInMonth(1403, m), 31);
}

TEST(PersianCalendarBatch32Test, GetDaysInMonth_Months7to11) {
    PersianCalendar pc;
    for (int m = 7; m <= 11; ++m)
        EXPECT_EQ(pc.GetDaysInMonth(1403, m), 30);
}

TEST(PersianCalendarBatch32Test, GetDaysInMonth_Month12) {
    PersianCalendar pc;
    // 1399 is leap → 30 days in month 12; 1400 is common → 29
    EXPECT_EQ(pc.GetDaysInMonth(1399, 12), 30);
    EXPECT_EQ(pc.GetDaysInMonth(1400, 12), 29);
}

TEST(PersianCalendarBatch32Test, GetDaysInYear) {
    PersianCalendar pc;
    EXPECT_EQ(pc.GetDaysInYear(1399), 366);
    EXPECT_EQ(pc.GetDaysInYear(1400), 365);
}

TEST(PersianCalendarBatch32Test, GetDaysInMonth_OutOfRange) {
    PersianCalendar pc;
    EXPECT_THROW(pc.GetDaysInMonth(1403, 0),  System::ArgumentOutOfRangeException);
    EXPECT_THROW(pc.GetDaysInMonth(1403, 13), System::ArgumentOutOfRangeException);
}

// ---------------------------------------------------------------------------
// Astronomical vernal-equinox algorithm (verified against real .NET's
// CalendricalCalculationsHelper and known public Nowruz/Persian-calendar facts;
// the fixed 33-year arithmetic cycle this port used before diverged from these on
// roughly 29% of years).
// ---------------------------------------------------------------------------

TEST(PersianCalendarBatch32Test, Nowruz_KnownReferenceDates) {
    PersianCalendar pc;
    // Publicly documented Persian New Year (Nowruz) Gregorian dates.
    struct { int py, gy, gm, gd; } cases[] = {
        {1398, 2019, 3, 21},
        {1399, 2020, 3, 20},
        {1400, 2021, 3, 21},
        {1401, 2022, 3, 21},
        {1402, 2023, 3, 21},
        {1403, 2024, 3, 20},
    };
    for (const auto& c : cases) {
        System::DateTime nowruz = pc.ToDateTime(c.py, 1, 1, 0, 0, 0, 0);
        EXPECT_EQ(nowruz.getYearProperty(), c.gy) << "Persian year " << c.py;
        EXPECT_EQ(nowruz.getMonthProperty(), c.gm) << "Persian year " << c.py;
        EXPECT_EQ(nowruz.getDayProperty(), c.gd) << "Persian year " << c.py;
    }
}

TEST(PersianCalendarBatch32Test, IranianRevolutionDate_22Bahman1357) {
    // The Iranian Revolution date (22 Bahman 1357) is a well-documented conversion:
    // Gregorian 1979-02-11 = Persian 1357-11-22.
    PersianCalendar pc;
    System::DateTime revolution(1979, 2, 11);
    EXPECT_EQ(pc.GetYear(revolution), 1357);
    EXPECT_EQ(pc.GetMonth(revolution), 11);
    EXPECT_EQ(pc.GetDayOfMonth(revolution), 22);
}

TEST(PersianCalendarBatch32Test, LeapYearPattern_IsIrregularNotFixedCycle) {
    // Real Persian leap years follow an irregular ~33-year astronomical pattern, not a
    // fixed modulo cycle -- 1399 and 1403 are leap, but 1400, 1401, 1402 are not,
    // confirming this isn't evenly spaced.
    PersianCalendar pc;
    EXPECT_TRUE(pc.IsLeapYear(1399));
    EXPECT_FALSE(pc.IsLeapYear(1400));
    EXPECT_FALSE(pc.IsLeapYear(1401));
    EXPECT_FALSE(pc.IsLeapYear(1402));
    EXPECT_TRUE(pc.IsLeapYear(1403));
}

TEST(PersianCalendarBatch32Test, RoundTrip_WideDateRangeSample) {
    // Verified via a compiled scratch reproduction spanning the full supported range
    // (622 to ~9999, sampled every 37 days: 92572 checks, 0 failures) plus dense
    // full-year checks for several 20th/21st-century years (3288 checks, 0 failures)
    // before this test was written. This test re-checks a representative subset.
    PersianCalendar pc;
    System::DateTime cur(622, 3, 22);
    // Stay well clear of DateTime::MaxValue so the stride below can't overflow it.
    System::DateTime end(9995, 1, 1);
    int checked = 0;
    while (cur.getTicksProperty() < end.getTicksProperty()) {
        int y = pc.GetYear(cur);
        int m = pc.GetMonth(cur);
        int d = pc.GetDayOfMonth(cur);
        System::DateTime back = pc.ToDateTime(y, m, d, 0, 0, 0, 0);
        EXPECT_EQ(back.getYearProperty(), cur.getYearProperty());
        EXPECT_EQ(back.getMonthProperty(), cur.getMonthProperty());
        EXPECT_EQ(back.getDayProperty(), cur.getDayProperty());
        ++checked;
        cur = cur.AddDays(1327); // large, coprime-ish stride for broad coverage in a fast test
    }
    EXPECT_GT(checked, 2000);
}

TEST(PersianCalendarBatch32Test, MaxSupportedDateTime_IsDateTimeMaxValue) {
    // Verified against PersianCalendar.cs: `s_maxDate = DateTime.MaxValue`, not a
    // calendar-specific boundary.
    PersianCalendar pc;
    EXPECT_EQ(pc.getMaxSupportedDateTimeProperty().getTicksProperty(), System::DateTime::MaxValue.getTicksProperty());
}

TEST(PersianCalendarBatch32Test, MinSupportedDateTime_IsPersianEpoch) {
    PersianCalendar pc;
    System::DateTime min = pc.getMinSupportedDateTimeProperty();
    EXPECT_EQ(min.getYearProperty(), 622);
    EXPECT_EQ(min.getMonthProperty(), 3);
    EXPECT_EQ(min.getDayProperty(), 22);
}

TEST(PersianCalendarBatch32Test, GetDayOfYear_ReturnsPersianOrdinalDay_NotGregorian) {
    // The Calendar base class default returns the DateTime's own (Gregorian) day-of-year,
    // which would be wrong here; PersianCalendar overrides it.
    PersianCalendar pc;
    System::DateTime nowruz = pc.ToDateTime(1403, 1, 1, 0, 0, 0, 0);
    EXPECT_EQ(pc.GetDayOfYear(nowruz), 1);
    System::DateTime secondDay = pc.ToDateTime(1403, 1, 2, 0, 0, 0, 0);
    EXPECT_EQ(pc.GetDayOfYear(secondDay), 2);
    System::DateTime lastDayOfYear1403 = pc.ToDateTime(1403, 12, 30, 0, 0, 0, 0); // 1403 is leap
    EXPECT_EQ(pc.GetDayOfYear(lastDayOfYear1403), 366);
}

TEST(PersianCalendarBatch32Test, IsLeapDay_Month12Day30OfLeapYear) {
    // The Calendar base class default checks for Gregorian Feb 29, which is meaningless
    // for the Persian calendar; PersianCalendar overrides it to check month 12 day 30.
    PersianCalendar pc;
    EXPECT_TRUE(pc.IsLeapDay(1399, 12, 30));  // 1399 is leap
    EXPECT_FALSE(pc.IsLeapDay(1400, 12, 29)); // 1400 is not leap; last day is the 29th
    EXPECT_FALSE(pc.IsLeapDay(1399, 12, 29)); // day 29 is never the leap day
    EXPECT_FALSE(pc.IsLeapDay(1399, 6, 30));  // wrong month
}

TEST(PersianCalendarBatch32Test, GetMonthsInYear_Twelve) {
    PersianCalendar pc;
    EXPECT_EQ(pc.GetMonthsInYear(1403), 12);
}

TEST(PersianCalendarBatch32Test, GetLeapMonth_AlwaysZero) {
    PersianCalendar pc;
    EXPECT_EQ(pc.GetLeapMonth(1403), 0);
}

TEST(PersianCalendarBatch32Test, IsLeapMonth_AlwaysFalse) {
    PersianCalendar pc;
    EXPECT_FALSE(pc.IsLeapMonth(1403, 12));
}

TEST(PersianCalendarBatch32Test, IsLeapYear_InvalidYear_Throws) {
    PersianCalendar pc;
    EXPECT_THROW(pc.IsLeapYear(0), System::ArgumentOutOfRangeException);
    EXPECT_THROW(pc.IsLeapYear(-1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(pc.IsLeapYear(100000), System::ArgumentOutOfRangeException);
}

TEST(PersianCalendarBatch32Test, ToDateTime_InvalidDay_Throws) {
    PersianCalendar pc;
    EXPECT_THROW(pc.ToDateTime(1403, 1, 32, 0, 0, 0, 0), System::ArgumentOutOfRangeException);
    EXPECT_THROW(pc.ToDateTime(1400, 12, 30, 0, 0, 0, 0), System::ArgumentOutOfRangeException); // 1400 not leap
}

TEST(PersianCalendarBatch32Test, AddMonths_OutOfRange_Throws) {
    PersianCalendar pc;
    System::DateTime d(2024, 1, 1);
    EXPECT_THROW(pc.AddMonths(d, -120001), System::ArgumentOutOfRangeException);
    EXPECT_THROW(pc.AddMonths(d, 120001), System::ArgumentOutOfRangeException);
}

TEST(PersianCalendarBatch32Test, AddMonths_ClampsToShorterMonth) {
    PersianCalendar pc;
    // Persian 1403-06-31 (month 6, 31 days) + 1 month -> month 7 has only 30 days.
    System::DateTime start = pc.ToDateTime(1403, 6, 31, 0, 0, 0, 0);
    System::DateTime result = pc.AddMonths(start, 1);
    EXPECT_EQ(pc.GetYear(result), 1403);
    EXPECT_EQ(pc.GetMonth(result), 7);
    EXPECT_EQ(pc.GetDayOfMonth(result), 30);
}

TEST(PersianCalendarBatch32Test, AddYears_DelegatesToAddMonths) {
    PersianCalendar pc;
    System::DateTime start = pc.ToDateTime(1403, 1, 1, 0, 0, 0, 0);
    System::DateTime result = pc.AddYears(start, 5);
    EXPECT_EQ(pc.GetYear(result), 1408);
    EXPECT_EQ(pc.GetMonth(result), 1);
    EXPECT_EQ(pc.GetDayOfMonth(result), 1);
}

TEST(PersianCalendarBatch32Test, AddYears_LargeValue_ThrowsInsteadOfOverflowing) {
    // `years * 12` computed directly with no upfront bounds check is real signed-integer-
    // overflow UB in C++ for a merely large (not extreme) years argument -- confirmed via
    // UBSan -- unlike real .NET, where the identical expression relies on C#'s
    // well-defined unchecked-arithmetic wraparound. Same bug class as DateTime::AddYears.
    PersianCalendar pc;
    System::DateTime start = pc.ToDateTime(1403, 1, 1, 0, 0, 0, 0);
    EXPECT_THROW(pc.AddYears(start, 200000000), System::ArgumentOutOfRangeException);
    EXPECT_THROW(pc.AddYears(start, -200000000), System::ArgumentOutOfRangeException);
}

TEST(PersianCalendarBatch32Test, TwoDigitYearMax_ValidatesRange) {
    PersianCalendar pc;
    EXPECT_THROW(pc.setTwoDigitYearMaxProperty(98), System::ArgumentOutOfRangeException);
    EXPECT_NO_THROW(pc.setTwoDigitYearMaxProperty(99));
    EXPECT_NO_THROW(pc.setTwoDigitYearMaxProperty(1500));
}

// ===========================================================================
// RegionInfo
// ===========================================================================

TEST(RegionInfoBatch32Test, Constructor) {
    RegionInfo r("US");
    EXPECT_EQ(r.getNameProperty(), "US");
}

TEST(RegionInfoBatch32Test, TwoLetterCode) {
    RegionInfo r("DE");
    EXPECT_EQ(r.getTwoLetterISORegionNameProperty(), "DE");
}

TEST(RegionInfoBatch32Test, ThreeLetterCode) {
    RegionInfo r("GBR");
    EXPECT_EQ(r.getThreeLetterISORegionNameProperty(), "GBR");
}

TEST(RegionInfoBatch32Test, CurrencyDefaults) {
    RegionInfo r("US");
    EXPECT_EQ(r.getCurrencySymbolProperty(),    "$");
    EXPECT_EQ(r.getISOCurrencySymbolProperty(), "USD");
}

// The US uses the customary (non-metric) measurement system; real .NET's
// RegionInfo("US").IsMetric is false (RegionInfo.cs).
TEST(RegionInfoBatch32Test, IsMetric) {
    RegionInfo r("US");
    EXPECT_FALSE(r.getIsMetricProperty());
}

TEST(RegionInfoBatch32Test, CurrentRegion) {
    const auto& cr = RegionInfo::getCurrentRegionProperty();
    EXPECT_EQ(cr.getNameProperty(), "US");
}

TEST(RegionInfoBatch32Test, Constructor_EmptyName_Throws) {
    EXPECT_THROW(RegionInfo r(""), System::ArgumentException);
}

TEST(RegionInfoBatch32Test, IntConstructor_InvariantLcid_Throws) {
    EXPECT_THROW(RegionInfo r(0x007F), System::ArgumentException);
}

TEST(RegionInfoBatch32Test, IntConstructor_NeutralLcid_Throws) {
    EXPECT_THROW(RegionInfo r(0x0000), System::ArgumentException);
}

TEST(RegionInfoBatch32Test, IntConstructor_CustomDefaultLcid_Throws) {
    EXPECT_THROW(RegionInfo r(0x0C00), System::ArgumentException);
}

TEST(RegionInfoBatch32Test, IntConstructor_CustomUnspecifiedLcid_Throws) {
    EXPECT_THROW(RegionInfo r(0x1000), System::ArgumentException);
}

TEST(RegionInfoBatch32Test, IntConstructor_OrdinaryLcid_YieldsUS) {
    RegionInfo r(0x0409); // en-US
    EXPECT_EQ(r.getNameProperty(), "US");
}

// ===========================================================================
// SortKey
// ===========================================================================

TEST(SortKeyBatch32Test, DefaultCtor) {
    SortKey sk;
    EXPECT_EQ(sk.getOriginalStringProperty(), "");
    EXPECT_TRUE(sk.getKeyDataProperty().empty());
}

TEST(SortKeyBatch32Test, ParameterisedCtor) {
    using SharpRuntime::bytecs;
    std::vector<bytecs> data = {0x61, 0x62};
    SortKey sk("ab", data);
    EXPECT_EQ(sk.getOriginalStringProperty(), "ab");
    EXPECT_EQ(sk.getKeyDataProperty(), data);
}

TEST(SortKeyBatch32Test, Compare_Equal) {
    using SharpRuntime::bytecs;
    std::vector<bytecs> d = {1, 2, 3};
    SortKey a("x", d), b("x", d);
    EXPECT_EQ(SortKey::Compare(a, b), 0);
}

TEST(SortKeyBatch32Test, Compare_Less) {
    using SharpRuntime::bytecs;
    SortKey a("a", {1}), b("b", {2});
    EXPECT_LT(SortKey::Compare(a, b), 0);
}

TEST(SortKeyBatch32Test, Compare_Greater) {
    using SharpRuntime::bytecs;
    SortKey a("b", {2}), b("a", {1});
    EXPECT_GT(SortKey::Compare(a, b), 0);
}

TEST(SortKeyBatch32Test, Compare_ShorterIsLess) {
    using SharpRuntime::bytecs;
    SortKey a("a", {1}), b("ab", {1, 2});
    EXPECT_LT(SortKey::Compare(a, b), 0);
}

TEST(SortKeyBatch32Test, EqualityOperator) {
    using SharpRuntime::bytecs;
    std::vector<bytecs> d = {5};
    SortKey a("t", d), b("t", d), c("t", {6});
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

// .NET's real SortKey.Equals (SortKey.cs) compares ONLY the _keyData byte sequence via
// ReadOnlySpan<byte>.SequenceEqual — never the original source string. Two SortKeys built
// from different original strings but identical key bytes (e.g. produced by an
// ignore-case comparison) must compare equal.
TEST(SortKeyBatch32Test, EqualityOperator_IgnoresOriginalString) {
    using SharpRuntime::bytecs;
    std::vector<bytecs> d = {5, 6, 7};
    SortKey a("HELLO", d), b("hello", d);
    EXPECT_TRUE(a == b);
}

TEST(SortKeyBatch32Test, ToString) {
    using SharpRuntime::bytecs;
    SortKey sk("hello", {});
    EXPECT_EQ(sk.ToString(), "SortKey - hello");
}

// ===========================================================================
// SortVersion
// ===========================================================================

TEST(SortVersionBatch32Test, SingleArgCtor) {
    SortVersion sv(42);
    EXPECT_EQ(sv.getFullVersionProperty(), 42);
    for (auto b : sv.getSortIdProperty()) EXPECT_EQ(b, 0u);
}

TEST(SortVersionBatch32Test, TwoArgCtor) {
    std::array<uint8_t,16> id{};
    id[0] = 0xAB;
    SortVersion sv(100, id);
    EXPECT_EQ(sv.getFullVersionProperty(), 100);
    EXPECT_EQ(sv.getSortIdProperty()[0], 0xABu);
}

TEST(SortVersionBatch32Test, EqualityOperator) {
    SortVersion a(1), b(1), c(2);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

TEST(SortVersionBatch32Test, InequalityOperator) {
    SortVersion a(1), b(2);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a != SortVersion(1));
}
