// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Globalization/HebrewCalendar.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/DateTime.hpp"
#include <stdexcept>
#include <algorithm>

namespace System::Globalization {

// ---------------------------------------------------------------------------
// Lookup tables (ported from .NET runtime HebrewCalendar.cs)
// ---------------------------------------------------------------------------

// Two bytes per Gregorian year starting at FirstGregorianTableYear (1583).
// Byte[0]: encoded day/month for January 1st in that year.
// Byte[1]: Hebrew year type (1-6).
// Normal year types: 1=353d, 2=354d, 3=355d.
// Leap year types:   4=383d, 5=384d, 6=385d.
static const unsigned char s_hebrewTable[] = {
    7,3,17,3,
    0,4,11,2,21,6,1,3,13,2,
    25,4,5,3,16,2,27,6,9,1,
    20,2,0,6,11,3,23,4,4,2,
    14,3,27,4,8,2,18,3,28,6,
    11,1,22,5,2,3,12,3,25,4,
    6,2,16,3,26,6,8,2,20,1,
    0,6,11,2,24,4,4,3,15,2,
    25,6,8,1,19,2,29,6,9,3,
    22,4,3,2,13,3,25,4,6,3,
    17,2,27,6,7,3,19,2,31,4,
    11,3,23,4,5,2,15,3,25,6,
    6,2,19,1,29,6,10,2,22,4,
    3,3,14,2,24,6,6,1,17,3,
    28,5,8,3,20,1,32,5,12,3,
    22,6,4,1,16,2,26,6,6,3,
    17,2,0,4,10,3,22,4,3,2,
    14,3,24,6,5,2,17,1,28,6,
    9,2,19,3,31,4,13,2,23,6,
    3,3,15,1,27,5,7,3,17,3,
    29,4,11,2,21,6,3,1,14,2,
    25,6,5,3,16,2,28,4,9,3,
    20,2,0,6,12,1,23,6,4,2,
    14,3,26,4,8,2,18,3,0,4,
    10,3,21,5,1,3,13,1,24,5,
    5,3,15,3,27,4,8,2,19,3,
    29,6,10,2,22,4,3,3,14,2,
    26,4,6,3,18,2,28,6,10,1,
    20,6,2,2,12,3,24,4,5,2,
    16,3,28,4,8,3,19,2,0,6,
    12,1,23,5,3,3,14,3,26,4,
    7,2,17,3,28,6,9,2,21,4,
    1,3,13,2,25,4,5,3,16,2,
    27,6,9,1,19,3,0,5,11,3,
    23,4,4,2,14,3,25,6,7,1,
    18,2,28,6,9,3,21,4,2,2,
    12,3,25,4,6,2,16,3,26,6,
    8,2,20,1,0,6,11,2,22,6,
    4,1,15,2,25,6,6,3,18,1,
    29,5,9,3,22,4,2,3,13,2,
    23,6,4,3,15,2,27,4,7,3,
    19,2,31,4,11,3,21,6,3,2,
    15,1,25,6,6,2,17,3,29,4,
    10,2,20,6,3,1,13,3,24,5,
    4,3,16,1,27,5,7,3,17,3,
    0,4,11,2,21,6,1,3,13,2,
    25,4,5,3,16,2,29,4,9,3,
    19,6,30,2,13,1,23,6,4,2,
    14,3,27,4,8,2,18,3,0,4,
    11,3,22,5,2,3,14,1,26,5,
    6,3,16,3,28,4,10,2,20,6,
    30,3,11,2,24,4,4,3,15,2,
    25,6,8,1,19,2,29,6,9,3,
    22,4,3,2,13,3,25,4,7,2,
    17,3,27,6,9,1,21,5,1,3,
    11,3,23,4,5,2,15,3,25,6,
    6,2,19,1,29,6,10,2,22,4,
    3,3,14,2,24,6,6,1,18,2,
    28,6,8,3,20,4,2,2,12,3,
    24,4,4,3,16,2,26,6,6,3,
    17,2,0,4,10,3,22,4,3,2,
    14,3,24,6,5,2,17,1,28,6,
    9,2,21,4,1,3,13,2,23,6,
    5,1,15,3,27,5,7,3,19,1,
    0,5,10,3,22,4,2,3,13,2,
    24,6,4,3,15,2,27,4,8,3,
    20,4,1,2,11,3,22,6,3,2,
    15,1,25,6,7,2,17,3,29,4,
    10,2,21,6,1,3,13,1,24,5,
    5,3,15,3,27,4,8,2,19,6,
    1,1,12,2,22,6,3,3,14,2,
    26,4,6,3,18,2,28,6,10,1,
    20,6,2,2,12,3,24,4,5,2,
    16,3,28,4,9,2,19,6,30,3,
    12,1,23,5,3,3,14,3,26,4,
    7,2,17,3,28,6,9,2,21,4,
    1,3,13,2,25,4,5,3,16,2,
    27,6,9,1,19,6,30,2,11,3,
    23,4,4,2,14,3,27,4,7,3,
    18,2,28,6,11,1,22,5,2,3,
    12,3,25,4,6,2,16,3,26,6,
    8,2,20,4,30,3,11,2,24,4,
    4,3,15,2,25,6,8,1,18,3,
    29,5,9,3,22,4,3,2,13,3,
    23,6,6,1,17,2,27,6,7,3,
    20,4,1,2,11,3,23,4,5,2,
    15,3,25,6,6,2,19,1,29,6,
    10,2,20,6,3,1,14,2,24,6,
    4,3,17,1,28,5,8,3,20,4,
    1,3,12,2,22,6,2,3,14,2,
    26,4,6,3,17,2,0,4,10,3,
    20,6,1,2,14,1,24,6,5,2,
    15,3,28,4,9,2,19,6,1,1,
    12,3,23,5,3,3,15,1,27,5,
    7,3,17,3,29,4,11,2,21,6,
    1,3,12,2,25,4,5,3,16,2,
    28,4,9,3,19,6,30,2,12,1,
    23,6,4,2,14,3,26,4,8,2,
    18,3,0,4,10,3,22,5,2,3,
    14,1,25,5,6,3,16,3,28,4,
    9,2,20,6,30,3,11,2,23,4,
    4,3,15,2,27,4,7,3,19,2,
    29,6,11,1,21,6,3,2,13,3,
    25,4,6,2,17,3,27,6,9,1,
    20,5,30,3,10,3,22,4,3,2,
    14,3,24,6,5,2,17,1,28,6,
    9,2,21,4,1,3,13,2,23,6,
    5,1,16,2,27,6,7,3,19,4,
    30,2,11,3,23,4,3,3,14,2,
    25,6,5,3,16,2,28,4,9,3,
    21,4,2,2,12,3,23,6,4,2,
    16,1,26,6,8,2,20,4,30,3,
    11,2,22,6,4,1,14,3,25,5,
    6,3,18,1,29,5,9,3,22,4,
    2,3,13,2,23,6,4,3,15,2,
    27,4,7,3,20,4,1,2,11,3,
    21,6,3,2,15,1,25,6,6,2,
    17,3,29,4,10,2,20,6,3,1,
    13,3,24,5,4,3,17,1,28,5,
    8,3,18,6,1,1,12,2,22,6,
    2,3,14,2,26,4,6,3,17,2,
    28,6,10,1,20,6,1,2,12,3,
    24,4,5,2,15,3,28,4,9,2,
    19,6,33,3,12,1,23,5,3,3,
    13,3,25,4,6,2,16,3,26,6,
    8,2,20,4,30,3,11,2,24,4,
    4,3,15,2,25,6,8,1,18,6,
    33,2,9,3,22,4,3,2,13,3,
    25,4,6,3,17,2,27,6,9,1,
    21,5,1,3,11,3,23,4,5,2,
    15,3,25,6,6,2,19,4,33,3,
    10,2,22,4,3,3,14,2,24,6,
    6,1
};

// Month lengths for each of the 6 year types.
// LunarMonthLen[type * 14 + month]  (month 1..13, index 0 and 14 unused per row)
static const unsigned char s_lunarMonthLen[7][14] = {
    {0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},  // type 0 (unused)
    {0,30, 29, 29, 29, 30, 29, 30, 29, 30, 29, 30, 29,  0},  // type 1: 353 days (common deficient)
    {0,30, 29, 30, 29, 30, 29, 30, 29, 30, 29, 30, 29,  0},  // type 2: 354 days (common regular)
    {0,30, 30, 30, 29, 30, 29, 30, 29, 30, 29, 30, 29,  0},  // type 3: 355 days (common complete)
    {0,30, 29, 29, 29, 30, 30, 29, 30, 29, 30, 29, 30, 29},  // type 4: 383 days (leap deficient)
    {0,30, 29, 30, 29, 30, 30, 29, 30, 29, 30, 29, 30, 29},  // type 5: 384 days (leap regular)
    {0,30, 30, 30, 29, 30, 30, 29, 30, 29, 30, 29, 30, 29},  // type 6: 385 days (leap complete)
};

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

int HebrewCalendar::getLunarMonthDay(int gregorianYear, DateBuffer& lunarDate) {
    int index = gregorianYear - FirstGregorianTableYear;
    if (index < 0 || index > (LastGregorianTableYear - FirstGregorianTableYear))
        throw System::ArgumentOutOfRangeException("year");

    lunarDate.day   = s_hebrewTable[index * 2];
    int yearType    = s_hebrewTable[index * 2 + 1];

    // Decode the month
    switch (lunarDate.day) {
        case 0:  lunarDate.month = 5; lunarDate.day = 1;  break; // Shvat 1
        case 30: lunarDate.month = 3;                     break; // Kislev 30
        case 31: lunarDate.month = 5; lunarDate.day = 2;  break; // Shvat 2
        case 32: lunarDate.month = 5; lunarDate.day = 3;  break; // Shvat 3
        case 33: lunarDate.month = 3; lunarDate.day = 29; break; // Kislev 29
        default: lunarDate.month = 4;                     break; // Tevet (general case)
    }
    return yearType;
}

int HebrewCalendar::getHebrewYearType(int year, int era) {
    if (era != CurrentEra && era != HebrewEra)
        throw System::ArgumentOutOfRangeException("era");
    if (year < MinHebrewYear || year > MaxHebrewYear)
        throw System::ArgumentOutOfRangeException("year");
    return s_hebrewTable[(year - HebrewYearOf1AD - FirstGregorianTableYear) * 2 + 1];
}

// Absolute Gregorian date: day 1 = 0001-01-01.
long long HebrewCalendar::getAbsoluteDate(int year, int month, int day) {
    static const int daysBeforeMonth[] = {0,0,31,59,90,120,151,181,212,243,273,304,334};
    long long y = year - 1;
    long long days = y * 365LL + y / 4 - y / 100 + y / 400;
    days += daysBeforeMonth[month];
    if (month > 2) {
        bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        if (leap) ++days;
    }
    return days + day;
}

int HebrewCalendar::getDayDifference(int yearType, int m1, int d1, int m2, int d2) {
    if (m1 == m2) return d1 - d2;
    bool swap = (m1 > m2);
    if (swap) { std::swap(m1, m2); std::swap(d1, d2); }

    int days = s_lunarMonthLen[yearType][m1] - d1;
    ++m1;
    while (m1 < m2) days += s_lunarMonthLen[yearType][m1++];
    days += d2;
    return swap ? days : -days;
}

int HebrewCalendar::getDatePart(long long ticks, int part) {
    System::DateTime dt(ticks);
    int gYear = dt.getYearProperty(), gMonth = dt.getMonthProperty(), gDay = dt.getDayProperty();

    DateBuffer lunarDate;
    lunarDate.year  = gYear + HebrewYearOf1AD;
    int yearType    = getLunarMonthDay(gYear, lunarDate);

    DateBuffer result;
    result.year  = lunarDate.year;
    result.month = lunarDate.month;
    result.day   = lunarDate.day;

    long long absDate  = getAbsoluteDate(gYear, gMonth, gDay);
    long long absJan1  = getAbsoluteDate(gYear, 1, 1);

    if (gMonth == 1 && gDay == 1) {
        if (part == PartYear)  return result.year;
        if (part == PartMonth) return result.month;
        return result.day;
    }

    long long numDays = absDate - absJan1;

    if ((numDays + lunarDate.day) <= s_lunarMonthLen[yearType][lunarDate.month]) {
        result.day += static_cast<int>(numDays);
        if (part == PartYear)  return result.year;
        if (part == PartMonth) return result.month;
        return result.day;
    }

    // Move past the current partial month
    numDays -= (s_lunarMonthLen[yearType][lunarDate.month] - lunarDate.day);
    result.month++;
    result.day = 1;

    if (numDays > 1) {
        while (numDays > s_lunarMonthLen[yearType][result.month]) {
            numDays -= s_lunarMonthLen[yearType][result.month++];
            if (result.month > 13 || s_lunarMonthLen[yearType][result.month] == 0) {
                result.year++;
                yearType = s_hebrewTable[(gYear + 1 - FirstGregorianTableYear) * 2 + 1];
                result.month = 1;
            }
        }
        result.day += static_cast<int>(numDays - 1);
    }

    if (part == PartYear)  return result.year;
    if (part == PartMonth) return result.month;
    return result.day;
}

System::DateTime HebrewCalendar::hebrewToGregorian(int hy, int hm, int hd,
                                                     int hour, int min, int sec, int ms) {
    int gregorianYear = hy - HebrewYearOf1AD;
    DateBuffer jan1;
    int yearType = getLunarMonthDay(gregorianYear, jan1);

    long long timeTicks = static_cast<long long>(hour) * System::DateTime::TicksPerHour
                        + static_cast<long long>(min)  * System::DateTime::TicksPerMinute
                        + static_cast<long long>(sec)  * System::DateTime::TicksPerSecond
                        + static_cast<long long>(ms)   * System::DateTime::TicksPerMillisecond;

    if (hm == jan1.month && hd == jan1.day) {
        long long base = (getAbsoluteDate(gregorianYear, 1, 1) - 1) * System::DateTime::TicksPerDay;
        return System::DateTime(base + timeTicks);
    }

    int days = getDayDifference(yearType, hm, hd, jan1.month, jan1.day);
    long long jan1Ticks = (getAbsoluteDate(gregorianYear, 1, 1) - 1) * System::DateTime::TicksPerDay;
    return System::DateTime(jan1Ticks + static_cast<long long>(days) * System::DateTime::TicksPerDay + timeTicks);
}

// ---------------------------------------------------------------------------
// Calendar overrides
// ---------------------------------------------------------------------------

int HebrewCalendar::GetYear(const System::DateTime& time) const {
    return getDatePart(time.getTicksProperty(), PartYear);
}

int HebrewCalendar::GetMonth(const System::DateTime& time) const {
    return getDatePart(time.getTicksProperty(), PartMonth);
}

int HebrewCalendar::GetDayOfMonth(const System::DateTime& time) const {
    return getDatePart(time.getTicksProperty(), PartDay);
}

int HebrewCalendar::GetDayOfYear(const System::DateTime& time) const {
    int year = GetYear(time);
    // Handle the edge case for first supported year
    System::DateTime beginOfYear;
    if (year == MinHebrewYear) {
        beginOfYear = System::DateTime(1582, 9, 27); // Hebrew 5343/01/01 in Gregorian
    } else {
        beginOfYear = hebrewToGregorian(year, 1, 1, 0, 0, 0, 0);
    }
    return static_cast<int>((time.getTicksProperty() - beginOfYear.getTicksProperty())
                            / System::DateTime::TicksPerDay) + 1;
}

bool HebrewCalendar::IsLeapYear(int year, int era) const {
    if (era != CurrentEra && era != HebrewEra)
        throw System::ArgumentOutOfRangeException("era");
    if (year < MinHebrewYear || year > MaxHebrewYear)
        throw System::ArgumentOutOfRangeException("year");
    return ((7LL * year + 1) % 19) < 7;
}

int HebrewCalendar::GetMonthsInYear(int year, int era) const {
    return IsLeapYear(year, era) ? 13 : 12;
}

int HebrewCalendar::GetDaysInMonth(int year, int month, int era) const {
    int yearType = getHebrewYearType(year, era);
    int maxMonth = IsLeapYear(year, era) ? 13 : 12;
    if (month < 1 || month > maxMonth) throw System::ArgumentOutOfRangeException("month");
    int days = s_lunarMonthLen[yearType][month];
    if (days == 0) throw System::ArgumentOutOfRangeException("month");
    return days;
}

int HebrewCalendar::GetDaysInYear(int year, int era) const {
    int yearType = getHebrewYearType(year, era);
    return (yearType < 4) ? (352 + yearType) : (382 + yearType - 3);
}

System::DateTime HebrewCalendar::AddMonths(const System::DateTime& time, int months) const {
    // `int i = m + months;` below (and, in the negative branch, `-months`) is real signed-
    // integer-overflow UB in C++ for a months argument as simple as INT_MAX/INT_MIN -- real
    // .NET's identical-looking arithmetic relies on C#'s well-defined unchecked wraparound,
    // eventually caught by a try/catch around the whole method that converts any downstream
    // ArgumentException into ArgumentOutOfRangeException(nameof(months), ...). This port has
    // no such wrapper (and C++ wraparound isn't guaranteed for signed types), so validate
    // upfront instead, matching the same 120000-month bound Calendar::AddMonths and this
    // port's other calendar-specific AddMonths overrides already use.
    if (months < -120000 || months > 120000)
        throw System::ArgumentOutOfRangeException("months", std::to_string(months),
            "Valid values are between -120000 and 120000, inclusive.");
    int y = GetYear(time), m = GetMonth(time), d = GetDayOfMonth(time);
    if (months >= 0) {
        int i = m + months;
        while (i > GetMonthsInYear(y, CurrentEra)) i -= GetMonthsInYear(y++, CurrentEra);
        m = i;
    } else {
        int i = m + months;
        if (i <= 0) {
            months = -months - m;
            --y;
            while (months > GetMonthsInYear(y, CurrentEra)) months -= GetMonthsInYear(y--, CurrentEra);
            m = GetMonthsInYear(y, CurrentEra) - months;
        } else {
            m = i;
        }
    }
    int maxDay = GetDaysInMonth(y, m);
    if (d > maxDay) d = maxDay;
    long long ticks = hebrewToGregorian(y, m, d, 0, 0, 0, 0).getTicksProperty()
                    + (time.getTicksProperty() % System::DateTime::TicksPerDay);
    return System::DateTime(ticks);
}

System::DateTime HebrewCalendar::AddYears(const System::DateTime& time, int years) const {
    // `GetYear(time) + years` computed directly in int32 is real signed-integer-overflow UB
    // in C++ for a years argument near INT_MAX/INT_MIN -- real .NET's `y += years` relies on
    // C#'s well-defined unchecked wraparound, immediately followed by
    // CheckHebrewYearValue(y, ...) validating the (possibly-wrapped) result is in
    // [MinHebrewYear, MaxHebrewYear]. Computing the sum in a wider type first avoids the
    // overflow while preserving the same effective validation (the narrow Hebrew year range
    // means any genuinely out-of-range sum is rejected either way).
    long long y64 = static_cast<long long>(GetYear(time)) + static_cast<long long>(years);
    if (y64 < MinHebrewYear || y64 > MaxHebrewYear)
        throw System::ArgumentOutOfRangeException("years");
    int y = static_cast<int>(y64);
    int m = GetMonth(time), d = GetDayOfMonth(time);
    if (m > GetMonthsInYear(y, CurrentEra)) m = GetMonthsInYear(y, CurrentEra);
    int maxDay = GetDaysInMonth(y, m);
    if (d > maxDay) d = maxDay;
    long long ticks = hebrewToGregorian(y, m, d, 0, 0, 0, 0).getTicksProperty()
                    + (time.getTicksProperty() % System::DateTime::TicksPerDay);
    return System::DateTime(ticks);
}

System::DateTime HebrewCalendar::ToDateTime(int year, int month, int day, int hour, int minute,
                                            int second, int millisecond, int era) const {
    // Real .NET validates era (via CheckHebrewYearValue -> CheckEraRange) before anything
    // else in ToDateTime. The previous version of this method discarded `era` entirely and
    // called GetDaysInMonth(year, month) with GetDaysInMonth's *default* era argument
    // (CurrentEra), so an invalid era passed by the caller was silently never checked.
    // Passing era through here restores that validation (GetDaysInMonth -> getHebrewYearType
    // already throws ArgumentOutOfRangeException("era") for anything other than
    // CurrentEra/HebrewEra).
    int daysInMonth = GetDaysInMonth(year, month, era);
    if (day < 1 || day > daysInMonth) throw System::ArgumentOutOfRangeException("day");
    return hebrewToGregorian(year, month, day, hour, minute, second, millisecond);
}

} // namespace System::Globalization
