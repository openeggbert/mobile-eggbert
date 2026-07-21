// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include <gtest/gtest.h>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Globalization/TextInfo.hpp"
#include "System/Globalization/TextElementEnumerator.hpp"
#include "System/Globalization/SortKey.hpp"
#include "System/Globalization/CompareInfo.hpp"
#include "System/Globalization/CharUnicodeInfo.hpp"
#include "System/Globalization/DateTimeFormatInfo.hpp"
#include "System/Globalization/JulianCalendar.hpp"
#include "System/Globalization/ThaiBuddhistCalendar.hpp"
#include "System/Globalization/TaiwanCalendar.hpp"
#include "System/Globalization/PersianCalendar.hpp"

using namespace System::Globalization;

// --- TextInfo ---
TEST(TextInfoTests, ToUpper) {
    TextInfo ti;
    EXPECT_EQ(ti.ToUpper("hello"), "HELLO");
}
TEST(TextInfoTests, ToLower) {
    TextInfo ti;
    EXPECT_EQ(ti.ToLower("WORLD"), "world");
}
TEST(TextInfoTests, ToTitleCase) {
    TextInfo ti;
    EXPECT_EQ(ti.ToTitleCase("hello world"), "Hello World");
}

// --- TextElementEnumerator ---
TEST(TextElementEnumeratorTests, ASCIIIteration) {
    TextElementEnumerator e("abc");
    EXPECT_TRUE(e.MoveNext()); EXPECT_EQ(e.GetTextElement(), "a");
    EXPECT_TRUE(e.MoveNext()); EXPECT_EQ(e.GetTextElement(), "b");
    EXPECT_TRUE(e.MoveNext()); EXPECT_EQ(e.GetTextElement(), "c");
    EXPECT_FALSE(e.MoveNext());
}
TEST(TextElementEnumeratorTests, Reset) {
    TextElementEnumerator e("ab");
    e.MoveNext(); e.Reset();
    EXPECT_TRUE(e.MoveNext()); EXPECT_EQ(e.GetTextElement(), "a");
}

// --- SortKey ---
TEST(SortKeyTests, Compare) {
    SortKey k1{"a", {0x61}};
    SortKey k2{"b", {0x62}};
    EXPECT_LT(SortKey::Compare(k1,k2), 0);
    EXPECT_GT(SortKey::Compare(k2,k1), 0);
    EXPECT_EQ(SortKey::Compare(k1,k1), 0);
}

// --- CompareInfo ---
TEST(CompareInfoTests, Compare) {
    CompareInfo ci;
    EXPECT_EQ(ci.Compare("abc","abc"), 0);
    EXPECT_LT(ci.Compare("abc","abd"), 0);
}
TEST(CompareInfoTests, IgnoreCase) {
    CompareInfo ci;
    EXPECT_EQ(ci.Compare("Hello","hello", CompareOptions::IgnoreCase), 0);
}
TEST(CompareInfoTests, IsPrefix) {
    CompareInfo ci;
    EXPECT_TRUE(ci.IsPrefix("Hello World", "Hello"));
    EXPECT_FALSE(ci.IsPrefix("Hello World", "World"));
}
TEST(CompareInfoTests, IsSuffix) {
    CompareInfo ci;
    EXPECT_TRUE(ci.IsSuffix("Hello World", "World"));
    EXPECT_FALSE(ci.IsSuffix("Hello World", "Hello"));
}
TEST(CompareInfoTests, IndexOf) {
    CompareInfo ci;
    EXPECT_EQ(ci.IndexOf("Hello World", "World"), 6);
    EXPECT_EQ(ci.IndexOf("Hello World", "xyz"), -1);
}

// --- CharUnicodeInfo ---
TEST(CharUnicodeInfoTests, DecimalDigit) {
    EXPECT_EQ(CharUnicodeInfo::GetDecimalDigitValue(u'5'), 5);
    EXPECT_EQ(CharUnicodeInfo::GetDecimalDigitValue(u'a'), -1);
}
TEST(CharUnicodeInfoTests, NumericValue) {
    EXPECT_DOUBLE_EQ(CharUnicodeInfo::GetNumericValue(u'7'), 7.0);
    EXPECT_DOUBLE_EQ(CharUnicodeInfo::GetNumericValue(u'x'), -1.0);
}
TEST(CharUnicodeInfoTests, Category) {
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(u'A'), UnicodeCategory::UppercaseLetter);
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(u'a'), UnicodeCategory::LowercaseLetter);
    EXPECT_EQ(CharUnicodeInfo::GetUnicodeCategory(u'5'), UnicodeCategory::DecimalDigitNumber);
}

// --- DateTimeFormatInfo ---
TEST(DateTimeFormatInfoTests, MonthNames) {
    DateTimeFormatInfo dtfi;
    EXPECT_EQ(dtfi.GetMonthName(1), "January");
    EXPECT_EQ(dtfi.GetMonthName(12), "December");
}
TEST(DateTimeFormatInfoTests, DayNames) {
    DateTimeFormatInfo dtfi;
    EXPECT_EQ(dtfi.GetDayName(System::DayOfWeek::Monday), "Monday");
}

// Indexing the internal day-names array with an invalid DayOfWeek was previously undefined
// behavior (std::array::operator[] does not bounds-check), not a thrown exception; real
// .NET's base implementation validates first (DateTimeFormatInfo.cs's
// `(uint)dow >= (uint)names.Length` check).
TEST(DateTimeFormatInfoTests, GetDayName_InvalidValue_Throws) {
    DateTimeFormatInfo dtfi;
    auto invalid = static_cast<System::DayOfWeek>(7);
    EXPECT_THROW(dtfi.GetDayName(invalid), System::ArgumentOutOfRangeException);
    EXPECT_THROW(dtfi.GetAbbreviatedDayName(invalid), System::ArgumentOutOfRangeException);
    EXPECT_THROW(dtfi.GetShortestDayName(invalid), System::ArgumentOutOfRangeException);
}
TEST(DateTimeFormatInfoTests, AbbreviatedMonthNames) {
    DateTimeFormatInfo dtfi;
    EXPECT_EQ(dtfi.GetAbbreviatedMonthName(1), "Jan");
}

// --- Calendars ---
TEST(JulianCalendarTests, LeapYear) {
    JulianCalendar cal;
    EXPECT_TRUE(cal.IsLeapYear(100));  // Julian: every 4th year
    EXPECT_TRUE(cal.IsLeapYear(400));
}
TEST(JulianCalendarTests, AddMonths_LargeValue_ThrowsInsteadOfOverflowing) {
    // JulianCalendar::AddMonths is a virtual override that replaces (not augments)
    // Calendar::AddMonths's own |months|>120000 check, and previously had none of its own --
    // `i = m - 1 + months` is real signed-integer-overflow UB in C++ for a months argument
    // as simple as INT_MAX.
    JulianCalendar cal;
    System::DateTime dt(2024, 1, 1);
    EXPECT_THROW(cal.AddMonths(dt, 1000000000), System::ArgumentOutOfRangeException);
    EXPECT_THROW(cal.AddMonths(dt, -1000000000), System::ArgumentOutOfRangeException);
}
TEST(JulianCalendarTests, AddYears_LargeValue_ThrowsInsteadOfOverflowing) {
    // `years * 12` computed directly with no upfront bounds check is real signed-integer-
    // overflow UB in C++ for a merely large years argument.
    JulianCalendar cal;
    System::DateTime dt(2024, 1, 1);
    EXPECT_THROW(cal.AddYears(dt, 200000000), System::ArgumentOutOfRangeException);
    EXPECT_THROW(cal.AddYears(dt, -200000000), System::ArgumentOutOfRangeException);
}
TEST(ThaiBuddhistCalendarTests, Year) {
    ThaiBuddhistCalendar cal;
    System::DateTime dt(2024,1,1);
    EXPECT_EQ(cal.GetYear(dt), 2567);
}
TEST(ThaiBuddhistCalendarTests, AddYears_LargeValue_ThrowsInsteadOfOverflowing) {
    // ThaiBuddhistCalendar doesn't override AddYears -- inherits Calendar::AddYears (the
    // base-class default). `years * 12` computed directly with no upfront bounds check was
    // real signed-integer-overflow UB in C++ for a merely large years argument; this exercises
    // the fix at the base-class level, which every non-overriding calendar subclass inherits.
    ThaiBuddhistCalendar cal;
    System::DateTime dt(2024, 1, 1);
    EXPECT_THROW(cal.AddYears(dt, 200000000), System::ArgumentOutOfRangeException);
    EXPECT_THROW(cal.AddYears(dt, -200000000), System::ArgumentOutOfRangeException);
}
TEST(ThaiBuddhistCalendarTests, AddWeeks_LargeValue_ThrowsInsteadOfOverflowing) {
    // `weeks * 7` computed directly with no upfront bounds check was real signed-integer-
    // overflow UB in C++ for a merely large weeks argument (Calendar::AddWeeks, base class).
    ThaiBuddhistCalendar cal;
    System::DateTime dt(2024, 1, 1);
    EXPECT_THROW(cal.AddWeeks(dt, 400000000), System::ArgumentOutOfRangeException);
    EXPECT_THROW(cal.AddWeeks(dt, -400000000), System::ArgumentOutOfRangeException);
}
TEST(ThaiBuddhistCalendarTests, Eras_ContainsThaiBuddhistEra) {
    ThaiBuddhistCalendar cal;
    auto eras = cal.getErasProperty();
    ASSERT_EQ(eras.size(), 1u);
    EXPECT_EQ(eras[0], ThaiBuddhistCalendar::ThaiBuddhistEra);
}
TEST(TaiwanCalendarTests, Year) {
    TaiwanCalendar cal;
    System::DateTime dt(2024,1,1);
    EXPECT_EQ(cal.GetYear(dt), 113);
}
TEST(PersianCalendarTests, LeapYear) {
    PersianCalendar cal;
    // 1403 is a Persian leap year
    EXPECT_TRUE(cal.IsLeapYear(1403));
    EXPECT_FALSE(cal.IsLeapYear(1402));
}
TEST(PersianCalendarTests, CurrentYear) {
    PersianCalendar cal;
    System::DateTime dt(2024,3,20);
    int y = cal.GetYear(dt);
    EXPECT_GE(y, 1402);
    EXPECT_LE(y, 1404);
}
