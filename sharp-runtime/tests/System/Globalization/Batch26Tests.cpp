// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 26:
//   Calendar:              new methods (AddWeeks, AddMilliseconds, GetMonthsInYear, GetLeapMonth,
//                          IsLeapMonth, IsLeapDay, ToDateTime, ToFourDigitYear, properties)
//   CalendarAlgorithmType: enum values
//   CalendarWeekRule:      enum values
//   CharUnicodeInfo:       all methods
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Globalization/Calendar.hpp"
#include "System/Globalization/CalendarAlgorithmType.hpp"
#include "System/Globalization/CalendarWeekRule.hpp"
#include "System/Globalization/CharUnicodeInfo.hpp"

using System::DateTime;
using System::Globalization::Calendar;
using System::Globalization::CalendarAlgorithmType;
using System::Globalization::CalendarWeekRule;
using System::Globalization::CharUnicodeInfo;
using System::Globalization::UnicodeCategory;

// Concrete minimal Calendar subclass for testing
struct GregorianLikeCalendar : Calendar {};

// ===========================================================================
// Calendar — new properties
// ===========================================================================

TEST(CalendarBatch26Test, AlgorithmType) {
    // .NET's Calendar base class returns Unknown; concrete calendars (GregorianCalendar, etc.)
    // override it with their actual algorithm basis.
    GregorianLikeCalendar cal;
    EXPECT_EQ(cal.getAlgorithmTypeProperty(), CalendarAlgorithmType::Unknown);
}

TEST(CalendarBatch26Test, IsReadOnly_False) {
    GregorianLikeCalendar cal;
    EXPECT_FALSE(cal.getIsReadOnlyProperty());
}

TEST(CalendarBatch26Test, Eras_ContainsCurrentEra) {
    GregorianLikeCalendar cal;
    auto eras = cal.getErasProperty();
    ASSERT_EQ(eras.size(), 1u);
    EXPECT_EQ(eras[0], Calendar::CurrentEra);
}

TEST(CalendarBatch26Test, MinMax_SupportedDateTime) {
    GregorianLikeCalendar cal;
    EXPECT_EQ(cal.getMinSupportedDateTimeProperty().getYearProperty(), 1);
    EXPECT_EQ(cal.getMaxSupportedDateTimeProperty().getYearProperty(), 9999);
}

// ===========================================================================
// Calendar — new methods
// ===========================================================================

TEST(CalendarBatch26Test, AddWeeks) {
    GregorianLikeCalendar cal;
    DateTime dt(2024, 1, 1);
    auto result = cal.AddWeeks(dt, 2);
    EXPECT_EQ(result.getDayProperty(), 15); // 1 + 14 = 15
    EXPECT_EQ(result.getMonthProperty(), 1);
}

TEST(CalendarBatch26Test, AddMilliseconds) {
    GregorianLikeCalendar cal;
    DateTime dt(2024, 1, 1, 0, 0, 0, 0);
    auto result = cal.AddMilliseconds(dt, 1500.0);
    EXPECT_EQ(result.getSecondProperty(), 1);
    EXPECT_EQ(result.getMillisecondProperty(), 500);
}

TEST(CalendarBatch26Test, GetMonthsInYear) {
    GregorianLikeCalendar cal;
    EXPECT_EQ(cal.GetMonthsInYear(2024), 12);
    EXPECT_EQ(cal.GetMonthsInYear(2000, Calendar::CurrentEra), 12);
}

TEST(CalendarBatch26Test, GetLeapMonth_ReturnsZero) {
    GregorianLikeCalendar cal;
    EXPECT_EQ(cal.GetLeapMonth(2024), 0);
}

TEST(CalendarBatch26Test, IsLeapMonth_AlwaysFalse) {
    GregorianLikeCalendar cal;
    EXPECT_FALSE(cal.IsLeapMonth(2024, 2));
    EXPECT_FALSE(cal.IsLeapMonth(2024, 2, Calendar::CurrentEra));
}

TEST(CalendarBatch26Test, IsLeapDay) {
    GregorianLikeCalendar cal;
    EXPECT_TRUE(cal.IsLeapDay(2024, 2, 29));
    EXPECT_FALSE(cal.IsLeapDay(2023, 2, 28));
    EXPECT_FALSE(cal.IsLeapDay(2024, 3, 1));
}

TEST(CalendarBatch26Test, ToDateTime) {
    GregorianLikeCalendar cal;
    auto dt = cal.ToDateTime(2024, 6, 15, 10, 30, 45, 100);
    EXPECT_EQ(dt.getYearProperty(), 2024);
    EXPECT_EQ(dt.getMonthProperty(), 6);
    EXPECT_EQ(dt.getDayProperty(), 15);
    EXPECT_EQ(dt.getHourProperty(), 10);
    EXPECT_EQ(dt.getMinuteProperty(), 30);
    EXPECT_EQ(dt.getSecondProperty(), 45);
    EXPECT_EQ(dt.getMillisecondProperty(), 100);
}

TEST(CalendarBatch26Test, ToFourDigitYear_SmallYear) {
    GregorianLikeCalendar cal;
    int y = cal.ToFourDigitYear(30);
    EXPECT_GE(y, 1000);
    EXPECT_LE(y, 9999);
}

TEST(CalendarBatch26Test, ToFourDigitYear_LargeYear) {
    GregorianLikeCalendar cal;
    EXPECT_EQ(cal.ToFourDigitYear(2024), 2024);
}

TEST(CalendarBatch26Test, ExistingMethods_GetDaysInMonth) {
    GregorianLikeCalendar cal;
    EXPECT_EQ(cal.GetDaysInMonth(2024, 2), 29); // leap
    EXPECT_EQ(cal.GetDaysInMonth(2023, 2), 28);
    EXPECT_EQ(cal.GetDaysInMonth(2024, 1), 31);
}

TEST(CalendarBatch26Test, ExistingMethods_IsLeapYear) {
    GregorianLikeCalendar cal;
    EXPECT_TRUE(cal.IsLeapYear(2024));
    EXPECT_FALSE(cal.IsLeapYear(2023));
    EXPECT_TRUE(cal.IsLeapYear(2000));
    EXPECT_FALSE(cal.IsLeapYear(1900));
}

// ===========================================================================
// CalendarAlgorithmType
// ===========================================================================

TEST(CalendarAlgorithmTypeBatch26Test, Values) {
    EXPECT_EQ(static_cast<int>(CalendarAlgorithmType::Unknown),          0);
    EXPECT_EQ(static_cast<int>(CalendarAlgorithmType::SolarCalendar),    1);
    EXPECT_EQ(static_cast<int>(CalendarAlgorithmType::LunarCalendar),    2);
    EXPECT_EQ(static_cast<int>(CalendarAlgorithmType::LunisolarCalendar), 3);
}

// ===========================================================================
// CalendarWeekRule
// ===========================================================================

TEST(CalendarWeekRuleBatch26Test, Values) {
    EXPECT_EQ(static_cast<int>(CalendarWeekRule::FirstDay),          0);
    EXPECT_EQ(static_cast<int>(CalendarWeekRule::FirstFullWeek),     1);
    EXPECT_EQ(static_cast<int>(CalendarWeekRule::FirstFourDayWeek),  2);
}

// ===========================================================================
// CharUnicodeInfo
// ===========================================================================

TEST(CharUnicodeInfoBatch26Test, GetDecimalDigitValue_Digits) {
    EXPECT_EQ(CharUnicodeInfo::GetDecimalDigitValue(u'0'), 0);
    EXPECT_EQ(CharUnicodeInfo::GetDecimalDigitValue(u'5'), 5);
    EXPECT_EQ(CharUnicodeInfo::GetDecimalDigitValue(u'9'), 9);
}

TEST(CharUnicodeInfoBatch26Test, GetDecimalDigitValue_NonDigit) {
    EXPECT_EQ(CharUnicodeInfo::GetDecimalDigitValue(u'A'), -1);
    EXPECT_EQ(CharUnicodeInfo::GetDecimalDigitValue(u' '), -1);
}

TEST(CharUnicodeInfoBatch26Test, GetDecimalDigitValue_StringOverload) {
    std::u16string s = u"abc3def";
    EXPECT_EQ(CharUnicodeInfo::GetDecimalDigitValue(s, 3), 3);
    EXPECT_EQ(CharUnicodeInfo::GetDecimalDigitValue(s, 0), -1);
}

TEST(CharUnicodeInfoBatch26Test, GetDigitValue) {
    EXPECT_EQ(CharUnicodeInfo::GetDigitValue(u'7'), 7);
    EXPECT_EQ(CharUnicodeInfo::GetDigitValue(u'z'), -1);
}

TEST(CharUnicodeInfoBatch26Test, GetDigitValue_SuperscriptsHaveDigitValueButNotDecimal) {
    // .NET distinguishes GetDigitValue (Numeric_Type Decimal or Digit) from
    // GetDecimalDigitValue (Numeric_Type Decimal only) — superscript digits are Digit, not Decimal.
    EXPECT_EQ(CharUnicodeInfo::GetDigitValue(static_cast<char16_t>(0x00B9)), 1);
    EXPECT_EQ(CharUnicodeInfo::GetDigitValue(static_cast<char16_t>(0x00B2)), 2);
    EXPECT_EQ(CharUnicodeInfo::GetDigitValue(static_cast<char16_t>(0x00B3)), 3);
    EXPECT_EQ(CharUnicodeInfo::GetDecimalDigitValue(static_cast<char16_t>(0x00B2)), -1);
}

TEST(CharUnicodeInfoBatch26Test, StringOverloads_IndexOutOfRange_Throws) {
    std::u16string s = u"abc";
    EXPECT_THROW(CharUnicodeInfo::GetDecimalDigitValue(s, 3), System::ArgumentOutOfRangeException);
    EXPECT_THROW(CharUnicodeInfo::GetDigitValue(s, -1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(CharUnicodeInfo::GetNumericValue(s, 3), System::ArgumentOutOfRangeException);
    EXPECT_THROW(CharUnicodeInfo::GetUnicodeCategory(s, 3), System::ArgumentOutOfRangeException);
}

TEST(CharUnicodeInfoBatch26Test, GetUnicodeCategory_InvalidCodePoint_Throws) {
    EXPECT_THROW(CharUnicodeInfo::GetUnicodeCategory(-1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(CharUnicodeInfo::GetUnicodeCategory(0x110000), System::ArgumentOutOfRangeException);
}

TEST(CharUnicodeInfoBatch26Test, GetNumericValue_Ascii) {
    EXPECT_DOUBLE_EQ(CharUnicodeInfo::GetNumericValue(u'3'), 3.0);
    EXPECT_DOUBLE_EQ(CharUnicodeInfo::GetNumericValue(u'X'), -1.0);
}

TEST(CharUnicodeInfoBatch26Test, GetNumericValue_Fractions) {
    EXPECT_DOUBLE_EQ(CharUnicodeInfo::GetNumericValue(static_cast<char16_t>(0x00BC)), 0.25);
    EXPECT_DOUBLE_EQ(CharUnicodeInfo::GetNumericValue(static_cast<char16_t>(0x00BD)), 0.5);
    EXPECT_DOUBLE_EQ(CharUnicodeInfo::GetNumericValue(static_cast<char16_t>(0x00BE)), 0.75);
}

TEST(CharUnicodeInfoBatch26Test, GetNumericValue_StringOverload) {
    std::u16string s = u"x5y";
    EXPECT_DOUBLE_EQ(CharUnicodeInfo::GetNumericValue(s, 1), 5.0);
}

TEST(CharUnicodeInfoBatch26Test, GetUnicodeCategory_UpperLower) {
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(u'A'), UnicodeCategory::UppercaseLetter);
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(u'z'), UnicodeCategory::LowercaseLetter);
}

TEST(CharUnicodeInfoBatch26Test, GetUnicodeCategory_Digit) {
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(u'5'), UnicodeCategory::DecimalDigitNumber);
}

// TAB/LF/VT/FF/CR (U+0009-U+000D) all satisfy iswspace() in the C locale, so checking
// iswspace() before the C0-control check previously misclassified them as SpaceSeparator
// instead of Control. Real Unicode category for all of U+0000-U+001F is Cc (Control); only
// U+0020 (the actual space character) is Zs (SpaceSeparator).
TEST(CharUnicodeInfoBatch26Test, GetUnicodeCategory_WhitespaceControlChars_AreControl) {
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(u'\t'), UnicodeCategory::Control);
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(u'\n'), UnicodeCategory::Control);
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(u'\v'), UnicodeCategory::Control);
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(u'\f'), UnicodeCategory::Control);
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(u'\r'), UnicodeCategory::Control);
}

TEST(CharUnicodeInfoBatch26Test, GetUnicodeCategory_ActualSpace_IsSpaceSeparator) {
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(u' '), UnicodeCategory::SpaceSeparator);
}

TEST(CharUnicodeInfoBatch26Test, GetUnicodeCategory_StringOverload) {
    std::u16string s = u"Hello";
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(s, 0), UnicodeCategory::UppercaseLetter);
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(s, 1), UnicodeCategory::LowercaseLetter);
}
