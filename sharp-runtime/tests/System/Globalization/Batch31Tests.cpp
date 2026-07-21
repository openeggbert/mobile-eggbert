// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Tests for Batch 31:
//   JulianCalendar:    IsLeapYear, GetDaysInMonth, GetEra, GetErasCount
//   KoreanCalendar:    GetEra, GetErasCount, GetYear
//   NumberFormatInfo:  default values, IsReadOnly, InvariantInfo, ReadOnly, Clone,
//                      missing fields (CurrencyDecimalDigits, NativeDigits, PerMilleSymbol, etc.)
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Globalization/JulianCalendar.hpp"
#include "System/Globalization/KoreanCalendar.hpp"
#include "System/Globalization/NumberFormatInfo.hpp"
#include "System/Globalization/DigitShapes.hpp"
#include "System/DateTime.hpp"

using System::Globalization::DigitShapes;
using System::Globalization::JulianCalendar;
using System::Globalization::KoreanCalendar;
using System::Globalization::NumberFormatInfo;

// ===========================================================================
// JulianCalendar
// ===========================================================================

TEST(JulianCalendarBatch31Test, EraConstant) {
    EXPECT_EQ(JulianCalendar::JulianEra, 1);
}

TEST(JulianCalendarBatch31Test, GetEra) {
    JulianCalendar jc;
    EXPECT_EQ(jc.GetEra(System::DateTime(2024, 6, 1)), JulianCalendar::JulianEra);
}

TEST(JulianCalendarBatch31Test, GetErasCount) {
    JulianCalendar jc;
    EXPECT_EQ(jc.GetErasCount(), 1);
}

TEST(JulianCalendarBatch31Test, Eras_ContainsJulianEra) {
    JulianCalendar jc;
    auto eras = jc.getErasProperty();
    ASSERT_EQ(eras.size(), 1u);
    EXPECT_EQ(eras[0], JulianCalendar::JulianEra);
}

TEST(JulianCalendarBatch31Test, IsLeapYear_DivisibleBy4) {
    JulianCalendar jc;
    EXPECT_TRUE(jc.IsLeapYear(100));  // leap in Julian (not in Gregorian)
    EXPECT_TRUE(jc.IsLeapYear(1900)); // leap in Julian (not in Gregorian)
    EXPECT_TRUE(jc.IsLeapYear(2000));
    EXPECT_TRUE(jc.IsLeapYear(2024));
}

TEST(JulianCalendarBatch31Test, IsLeapYear_NotDivisibleBy4) {
    JulianCalendar jc;
    EXPECT_FALSE(jc.IsLeapYear(2023));
    EXPECT_FALSE(jc.IsLeapYear(1999));
}

TEST(JulianCalendarBatch31Test, GetDaysInMonth_February_JulianLeap) {
    JulianCalendar jc;
    EXPECT_EQ(jc.GetDaysInMonth(1900, 2), 29); // leap in Julian
    EXPECT_EQ(jc.GetDaysInMonth(2024, 2), 29);
    EXPECT_EQ(jc.GetDaysInMonth(2023, 2), 28);
}

TEST(JulianCalendarBatch31Test, GetDaysInMonth_OtherMonths) {
    JulianCalendar jc;
    EXPECT_EQ(jc.GetDaysInMonth(2024, 1),  31);
    EXPECT_EQ(jc.GetDaysInMonth(2024, 4),  30);
    EXPECT_EQ(jc.GetDaysInMonth(2024, 12), 31);
}

// Real date-conversion regression tests: prior to this fix, GetYear/GetMonth/GetDayOfMonth/
// ToDateTime/AddMonths/AddYears/GetDaysInYear were all inherited unmodified from the
// Gregorian-only Calendar base class, so JulianCalendar never actually applied the
// Julian<->Gregorian day-number offset -- it just relabeled Gregorian dates. .NET's own doc
// comment on JulianCalendar.cs states the reference point verified here: "Gregorian 1/1/0001
// is Julian 1/3/0001".
TEST(JulianCalendarBatch31Test, GetYear_GetMonth_GetDayOfMonth_AppliesTwoDayOffset) {
    JulianCalendar jc;
    System::DateTime gregorian(1, 1, 1);
    EXPECT_EQ(jc.GetYear(gregorian), 1);
    EXPECT_EQ(jc.GetMonth(gregorian), 1);
    EXPECT_EQ(jc.GetDayOfMonth(gregorian), 3);
}

TEST(JulianCalendarBatch31Test, ToDateTime_RoundTripsThroughGregorianOffset) {
    JulianCalendar jc;
    System::DateTime back = jc.ToDateTime(1, 1, 3, 0, 0, 0, 0);
    EXPECT_EQ(back.getYearProperty(), 1);
    EXPECT_EQ(back.getMonthProperty(), 1);
    EXPECT_EQ(back.getDayProperty(), 1);
}

TEST(JulianCalendarBatch31Test, GetDayOfYear_JanuaryFirst_IsOne) {
    JulianCalendar jc;
    // Julian 1/1/3 == day-of-year 3 (the Julian calendar's own 3rd day).
    System::DateTime julianDay3 = jc.ToDateTime(1, 1, 3, 0, 0, 0, 0);
    EXPECT_EQ(jc.GetDayOfYear(julianDay3), 3);
}

TEST(JulianCalendarBatch31Test, AddYears_PreservesJulianMonthAndDay) {
    JulianCalendar jc;
    System::DateTime start = jc.ToDateTime(1, 1, 3, 0, 0, 0, 0);
    System::DateTime plusOneYear = jc.AddYears(start, 1);
    EXPECT_EQ(jc.GetYear(plusOneYear), 2);
    EXPECT_EQ(jc.GetMonth(plusOneYear), 1);
    EXPECT_EQ(jc.GetDayOfMonth(plusOneYear), 3);
}

TEST(JulianCalendarBatch31Test, AddMonths_RollsOverYearBoundary) {
    JulianCalendar jc;
    System::DateTime dec = jc.ToDateTime(1, 12, 1, 0, 0, 0, 0);
    System::DateTime plusTwoMonths = jc.AddMonths(dec, 2);
    EXPECT_EQ(jc.GetYear(plusTwoMonths), 2);
    EXPECT_EQ(jc.GetMonth(plusTwoMonths), 2);
    EXPECT_EQ(jc.GetDayOfMonth(plusTwoMonths), 1);
}

TEST(JulianCalendarBatch31Test, GetDaysInYear_MatchesLeapRule) {
    JulianCalendar jc;
    EXPECT_EQ(jc.GetDaysInYear(2000), 366);
    // Julian leap rule has no century exception, unlike Gregorian: 1900 is a Julian leap year.
    EXPECT_EQ(jc.GetDaysInYear(1900), 366);
    EXPECT_EQ(jc.GetDaysInYear(2023), 365);
}

TEST(JulianCalendarBatch31Test, TwoDigitYearMax_DefaultIs2049) {
    JulianCalendar jc;
    // .NET JulianCalendar's constructor sets _twoDigitYearMax = 2049 (not the 2029 this port
    // previously used), matching JulianCalendar.cs's `_twoDigitYearMax = 2049;`.
    EXPECT_EQ(jc.getTwoDigitYearMaxProperty(), 2049);
}

// ===========================================================================
// KoreanCalendar
// ===========================================================================

TEST(KoreanCalendarBatch31Test, EraConstant) {
    EXPECT_EQ(KoreanCalendar::KoreanEra, 1);
}

TEST(KoreanCalendarBatch31Test, GetEra) {
    KoreanCalendar kc;
    EXPECT_EQ(kc.GetEra(System::DateTime(2024, 1, 1)), KoreanCalendar::KoreanEra);
}

TEST(KoreanCalendarBatch31Test, GetErasCount) {
    KoreanCalendar kc;
    EXPECT_EQ(kc.GetErasCount(), 1);
}

TEST(KoreanCalendarBatch31Test, Eras_ContainsKoreanEra) {
    KoreanCalendar kc;
    auto eras = kc.getErasProperty();
    ASSERT_EQ(eras.size(), 1u);
    EXPECT_EQ(eras[0], KoreanCalendar::KoreanEra);
}

TEST(KoreanCalendarBatch31Test, GetYear_Offset2333) {
    KoreanCalendar kc;
    EXPECT_EQ(kc.GetYear(System::DateTime(2000, 1, 1)), 4333);
    EXPECT_EQ(kc.GetYear(System::DateTime(2024, 6, 1)), 4357);
    EXPECT_EQ(kc.GetYear(System::DateTime(1,    1, 1)), 2334);
}

// ===========================================================================
// NumberFormatInfo
// ===========================================================================

TEST(NumberFormatInfoBatch31Test, DefaultCtor_IsMutable) {
    NumberFormatInfo nfi;
    EXPECT_FALSE(nfi.getIsReadOnlyProperty());
}

TEST(NumberFormatInfoBatch31Test, InvariantInfo_IsReadOnly) {
    const auto& inv = NumberFormatInfo::getInvariantInfoProperty();
    EXPECT_TRUE(inv.getIsReadOnlyProperty());
}

TEST(NumberFormatInfoBatch31Test, CurrentInfo_IsInvariant) {
    const auto& cur = NumberFormatInfo::getCurrentInfoProperty();
    EXPECT_TRUE(cur.getIsReadOnlyProperty());
}

TEST(NumberFormatInfoBatch31Test, ReadOnly_MakesReadOnly) {
    NumberFormatInfo nfi;
    auto ro = NumberFormatInfo::ReadOnly(nfi);
    EXPECT_TRUE(ro.getIsReadOnlyProperty());
}

TEST(NumberFormatInfoBatch31Test, Clone_IsMutable) {
    auto clone = NumberFormatInfo::getInvariantInfoProperty().Clone();
    EXPECT_FALSE(clone.getIsReadOnlyProperty());
}

TEST(NumberFormatInfoBatch31Test, DefaultNumberFields) {
    NumberFormatInfo nfi;
    EXPECT_EQ(nfi.getNumberDecimalSeparatorProperty(), ".");
    EXPECT_EQ(nfi.getNumberGroupSeparatorProperty(),   ",");
    EXPECT_EQ(nfi.getNumberDecimalDigitsProperty(),    2);
    EXPECT_EQ(nfi.getNegativeSignProperty(),           "-");
    EXPECT_EQ(nfi.getPositiveSignProperty(),           "+");
}

TEST(NumberFormatInfoBatch31Test, DefaultCurrencyFields) {
    NumberFormatInfo nfi;
    EXPECT_EQ(nfi.getCurrencyDecimalSeparatorProperty(), ".");
    EXPECT_EQ(nfi.getCurrencyGroupSeparatorProperty(),   ",");
    EXPECT_EQ(nfi.getCurrencySymbolProperty(),           "¤");
    EXPECT_EQ(nfi.getCurrencyDecimalDigitsProperty(),    2);
}

TEST(NumberFormatInfoBatch31Test, DefaultSpecialSymbols) {
    NumberFormatInfo nfi;
    EXPECT_EQ(nfi.getNaNSymbolProperty(),              "NaN");
    EXPECT_EQ(nfi.getPositiveInfinitySymbolProperty(), "Infinity");
    EXPECT_EQ(nfi.getNegativeInfinitySymbolProperty(), "-Infinity");
    EXPECT_EQ(nfi.getPercentSymbolProperty(),          "%");
    EXPECT_FALSE(nfi.getPerMilleSymbolProperty().empty());
}

TEST(NumberFormatInfoBatch31Test, GroupSizes) {
    NumberFormatInfo nfi;
    EXPECT_EQ(nfi.getNumberGroupSizesProperty().size(),   1u);
    EXPECT_EQ(nfi.getNumberGroupSizesProperty()[0],       3);
    EXPECT_EQ(nfi.getCurrencyGroupSizesProperty().size(), 1u);
    EXPECT_EQ(nfi.getCurrencyGroupSizesProperty()[0],     3);
    EXPECT_EQ(nfi.getPercentGroupSizesProperty().size(),  1u);
    EXPECT_EQ(nfi.getPercentGroupSizesProperty()[0],      3);
}

TEST(NumberFormatInfoBatch31Test, NativeDigits) {
    NumberFormatInfo nfi;
    ASSERT_EQ(nfi.getNativeDigitsProperty().size(), 10u);
    EXPECT_EQ(nfi.getNativeDigitsProperty()[0], "0");
    EXPECT_EQ(nfi.getNativeDigitsProperty()[9], "9");
}

TEST(NumberFormatInfoBatch31Test, DigitSubstitution_Default) {
    NumberFormatInfo nfi;
    EXPECT_EQ(nfi.getDigitSubstitutionProperty(), System::Globalization::DigitShapes::None);
}

// ===========================================================================
// NumberFormatInfo -- setter validation (verified against NumberFormatInfo.cs)
// ===========================================================================

TEST(NumberFormatInfoBatch31Test, DecimalDigits_OutOfRange_Throws) {
    NumberFormatInfo nfi;
    EXPECT_THROW(nfi.setNumberDecimalDigitsProperty(-1), System::ArgumentOutOfRangeException);
    EXPECT_THROW(nfi.setNumberDecimalDigitsProperty(100), System::ArgumentOutOfRangeException);
    EXPECT_THROW(nfi.setCurrencyDecimalDigitsProperty(100), System::ArgumentOutOfRangeException);
    EXPECT_THROW(nfi.setPercentDecimalDigitsProperty(-1), System::ArgumentOutOfRangeException);
    EXPECT_NO_THROW(nfi.setNumberDecimalDigitsProperty(0));
    EXPECT_NO_THROW(nfi.setNumberDecimalDigitsProperty(99));
}

TEST(NumberFormatInfoBatch31Test, DecimalDigits_OutOfRangeOnReadOnlyInstance_ThrowsArgumentOutOfRange) {
    // Regression guard for a specific check-ordering rule verified against real .NET's
    // NumberDecimalDigits setter (NumberFormatInfo.cs): the out-of-range check runs BEFORE
    // VerifyWritable(), so an out-of-range value on a read-only instance throws
    // ArgumentOutOfRangeException, not InvalidOperationException -- unlike an in-range value on
    // a read-only instance (covered by NumberFormatInfoTests.SetOnReadOnlyInstance_Throws),
    // which does throw InvalidOperationException.
    NumberFormatInfo ro = NumberFormatInfo::ReadOnly(NumberFormatInfo());
    EXPECT_THROW(ro.setNumberDecimalDigitsProperty(100), System::ArgumentOutOfRangeException);
}

TEST(NumberFormatInfoBatch31Test, NegativePositivePatterns_OutOfRange_Throws) {
    NumberFormatInfo nfi;
    EXPECT_THROW(nfi.setNumberNegativePatternProperty(5), System::ArgumentOutOfRangeException);
    EXPECT_THROW(nfi.setCurrencyNegativePatternProperty(17), System::ArgumentOutOfRangeException);
    EXPECT_THROW(nfi.setCurrencyPositivePatternProperty(4), System::ArgumentOutOfRangeException);
    EXPECT_THROW(nfi.setPercentNegativePatternProperty(12), System::ArgumentOutOfRangeException);
    EXPECT_THROW(nfi.setPercentPositivePatternProperty(4), System::ArgumentOutOfRangeException);
    EXPECT_NO_THROW(nfi.setNumberNegativePatternProperty(4));
    EXPECT_NO_THROW(nfi.setCurrencyNegativePatternProperty(16));
}

TEST(NumberFormatInfoBatch31Test, GroupSizes_InvalidElement_Throws) {
    NumberFormatInfo nfi;
    EXPECT_THROW(nfi.setNumberGroupSizesProperty({10}), System::ArgumentException);
    EXPECT_THROW(nfi.setNumberGroupSizesProperty({3, 0, 2}), System::ArgumentException);
    EXPECT_NO_THROW(nfi.setNumberGroupSizesProperty({3, 2, 0}));
    EXPECT_NO_THROW(nfi.setCurrencyGroupSizesProperty({9}));
}

TEST(NumberFormatInfoBatch31Test, DecimalSeparators_Empty_Throws) {
    NumberFormatInfo nfi;
    EXPECT_THROW(nfi.setNumberDecimalSeparatorProperty(""), System::ArgumentException);
    EXPECT_THROW(nfi.setCurrencyDecimalSeparatorProperty(""), System::ArgumentException);
    EXPECT_THROW(nfi.setPercentDecimalSeparatorProperty(""), System::ArgumentException);
    EXPECT_NO_THROW(nfi.setNumberDecimalSeparatorProperty(","));
}

TEST(NumberFormatInfoBatch31Test, NativeDigits_WrongCount_Throws) {
    NumberFormatInfo nfi;
    EXPECT_THROW(nfi.setNativeDigitsProperty({"0","1","2"}), System::ArgumentException);
}

TEST(NumberFormatInfoBatch31Test, NativeDigits_MultiCodepointEntry_Throws) {
    NumberFormatInfo nfi;
    std::vector<std::string> bad = {"0","1","2","3","4","5","6","7","8","ab"};
    EXPECT_THROW(nfi.setNativeDigitsProperty(bad), System::ArgumentException);
}

TEST(NumberFormatInfoBatch31Test, NativeDigits_ValidReplacement_Succeeds) {
    NumberFormatInfo nfi;
    std::vector<std::string> arabicIndic = {"٠","١","٢","٣","٤",
                                             "٥","٦","٧","٨","٩"};
    EXPECT_NO_THROW(nfi.setNativeDigitsProperty(arabicIndic));
    EXPECT_EQ(nfi.getNativeDigitsProperty()[0], "٠");
}

// Regression for ticket 347: CheckNativeDigits previously only compared the total byte length
// of each entry against what its leading byte's value range implies, without validating that
// the leading byte is actually a valid UTF-8 lead byte or that the following bytes are valid
// continuation bytes (0x80-0xBF) -- letting malformed UTF-8 slip through despite the method's
// own stated intent of validating "exactly one well-formed codepoint" per entry.
TEST(NumberFormatInfoBatch31Test, NativeDigits_MalformedContinuationByte_Throws) {
    NumberFormatInfo nfi;
    // Leading byte 0xC2 correctly implies a 2-byte sequence, but the second byte (0x20, a plain
    // space) is not a valid UTF-8 continuation byte.
    std::vector<std::string> bad = {"0","1","2","3","4","5","6","7","8","\xC2\x20"};
    EXPECT_THROW(nfi.setNativeDigitsProperty(bad), System::ArgumentException);
}

TEST(NumberFormatInfoBatch31Test, NativeDigits_ContinuationByteAsLeadByte_Throws) {
    NumberFormatInfo nfi;
    // 0x80 is a continuation byte and can never validly start a UTF-8 sequence.
    std::vector<std::string> bad = {"0","1","2","3","4","5","6","7","8","\x80\x80"};
    EXPECT_THROW(nfi.setNativeDigitsProperty(bad), System::ArgumentException);
}

TEST(NumberFormatInfoBatch31Test, DigitSubstitution_InvalidValue_Throws) {
    NumberFormatInfo nfi;
    EXPECT_THROW(nfi.setDigitSubstitutionProperty(static_cast<DigitShapes>(99)), System::ArgumentException);
    EXPECT_NO_THROW(nfi.setDigitSubstitutionProperty(DigitShapes::NativeNational));
}
