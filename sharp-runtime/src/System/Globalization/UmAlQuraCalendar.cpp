// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Globalization/UmAlQuraCalendar.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/DateTime.hpp"
#include <stdexcept>

namespace System::Globalization {

// ---------------------------------------------------------------------------
// Table: Hijri year 1318 = index 0, 1319 = index 1, ..., 1501 = index 183
// flags: bit (month-1) set → that month has 30 days; clear → 29 days.
// gYear/gMonth/gDay: Gregorian start date of that Hijri year.
// ---------------------------------------------------------------------------
const UmAlQuraCalendar::DateMapping UmAlQuraCalendar::s_yearInfo[184] = {
    {0x02EA,1900,4,30},{0x06E9,1901,4,19},{0x0ED2,1902,4,9},{0x0EA4,1903,3,30},
    {0x0D4A,1904,3,18},{0x0A96,1905,3,7},{0x0536,1906,2,24},{0x0AB5,1907,2,13},
    {0x0DAA,1908,2,3},{0x0BA4,1909,1,23},{0x0B49,1910,1,12},{0x0A93,1911,1,1},
    {0x052B,1911,12,21},{0x0A57,1912,12,9},{0x04B6,1913,11,29},{0x0AB5,1914,11,18},
    {0x05AA,1915,11,8},{0x0D55,1916,10,27},{0x0D2A,1917,10,17},{0x0A56,1918,10,6},
    {0x04AE,1919,9,25},{0x095D,1920,9,13},{0x02EC,1921,9,3},{0x06D5,1922,8,23},
    {0x06AA,1923,8,13},{0x0555,1924,8,1},{0x04AB,1925,7,21},{0x095B,1926,7,10},
    {0x02BA,1927,6,30},{0x0575,1928,6,18},{0x0BB2,1929,6,8},{0x0764,1930,5,29},
    {0x0749,1931,5,18},{0x0655,1932,5,6},{0x02AB,1933,4,25},{0x055B,1934,4,14},
    {0x0ADA,1935,4,4},{0x06D4,1936,3,24},{0x0EC9,1937,3,13},{0x0D92,1938,3,3},
    {0x0D25,1939,2,20},{0x0A4D,1940,2,9},{0x02AD,1941,1,28},{0x056D,1942,1,17},
    {0x0B6A,1943,1,7},{0x0B52,1943,12,28},{0x0AA5,1944,12,16},{0x0A4B,1945,12,5},
    {0x0497,1946,11,24},{0x0937,1947,11,13},{0x02B6,1948,11,2},{0x0575,1949,10,22},
    {0x0D6A,1950,10,12},{0x0D52,1951,10,2},{0x0A96,1952,9,20},{0x092D,1953,9,9},
    {0x025D,1954,8,29},{0x04DD,1955,8,18},{0x0ADA,1956,8,7},{0x05D4,1957,7,28},
    {0x0DA9,1958,7,17},{0x0D52,1959,7,7},{0x0AAA,1960,6,25},{0x04D6,1961,6,14},
    {0x09B6,1962,6,3},{0x0374,1963,5,24},{0x0769,1964,5,12},{0x0752,1965,5,2},
    {0x06A5,1966,4,21},{0x054B,1967,4,10},{0x0AAB,1968,3,29},{0x055A,1969,3,19},
    {0x0AD5,1970,3,8},{0x0DD2,1971,2,26},{0x0DA4,1972,2,16},{0x0D49,1973,2,4},
    {0x0A95,1974,1,24},{0x052D,1975,1,13},{0x0A5D,1976,1,2},{0x055A,1976,12,22},
    {0x0AD5,1977,12,11},{0x06AA,1978,12,1},{0x0695,1979,11,20},{0x052B,1980,11,8},
    {0x0A57,1981,10,28},{0x04AE,1982,10,18},{0x0976,1983,10,7},{0x056C,1984,9,26},
    {0x0B55,1985,9,15},{0x0AAA,1986,9,5},{0x0A55,1987,8,25},{0x04AD,1988,8,13},
    {0x095D,1989,8,2},{0x02DA,1990,7,23},{0x05D9,1991,7,12},{0x0DB2,1992,7,1},
    {0x0BA4,1993,6,21},{0x0B4A,1994,6,10},{0x0A55,1995,5,30},{0x02B5,1996,5,18},
    {0x0575,1997,5,7},{0x0B6A,1998,4,27},{0x0BD2,1999,4,17},{0x0BC4,2000,4,6},
    {0x0B89,2001,3,26},{0x0A95,2002,3,15},{0x052D,2003,3,4},{0x05AD,2004,2,21},
    {0x0B6A,2005,2,10},{0x06D4,2006,1,31},{0x0DC9,2007,1,20},{0x0D92,2008,1,10},
    {0x0AA6,2008,12,29},{0x0956,2009,12,18},{0x02AE,2010,12,7},{0x056D,2011,11,26},
    {0x036A,2012,11,15},{0x0B55,2013,11,4},{0x0AAA,2014,10,25},{0x094D,2015,10,14},
    {0x049D,2016,10,2},{0x095D,2017,9,21},{0x02BA,2018,9,11},{0x05B5,2019,8,31},
    {0x05AA,2020,8,20},{0x0D55,2021,8,9},{0x0A9A,2022,7,30},{0x092E,2023,7,19},
    {0x026E,2024,7,7},{0x055D,2025,6,26},{0x0ADA,2026,6,16},{0x06D4,2027,6,6},
    {0x06A5,2028,5,25},{0x054B,2029,5,14},{0x0A97,2030,5,3},{0x054E,2031,4,23},
    {0x0AAE,2032,4,11},{0x05AC,2033,4,1},{0x0BA9,2034,3,21},{0x0D92,2035,3,11},
    {0x0B25,2036,2,28},{0x064B,2037,2,16},{0x0CAB,2038,2,5},{0x055A,2039,1,26},
    {0x0B55,2040,1,15},{0x06D2,2041,1,4},{0x0EA5,2041,12,24},{0x0E4A,2042,12,14},
    {0x0A95,2043,12,3},{0x052D,2044,11,21},{0x0AAD,2045,11,10},{0x036C,2046,10,31},
    {0x0759,2047,10,20},{0x06D2,2048,10,9},{0x0695,2049,9,28},{0x052D,2050,9,17},
    {0x0A5B,2051,9,6},{0x04BA,2052,8,26},{0x09BA,2053,8,15},{0x03B4,2054,8,5},
    {0x0B69,2055,7,25},{0x0B52,2056,7,14},{0x0AA6,2057,7,3},{0x04B6,2058,6,22},
    {0x096D,2059,6,11},{0x02EC,2060,5,31},{0x06D9,2061,5,20},{0x0EB2,2062,5,10},
    {0x0D54,2063,4,30},{0x0D2A,2064,4,18},{0x0A56,2065,4,7},{0x04AE,2066,3,27},
    {0x096D,2067,3,16},{0x0D6A,2068,3,5},{0x0B54,2069,2,23},{0x0B29,2070,2,12},
    {0x0A93,2071,2,1},{0x052B,2072,1,21},{0x0A57,2073,1,9},{0x0536,2073,12,30},
    {0x0AB5,2074,12,19},{0x06AA,2075,12,9},{0x0E93,2076,11,27},
    {0,2077,11,17}  // sentinel for year 1501 (end marker)
};

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void UmAlQuraCalendar::checkYear(int year, int era) {
    if (era != CurrentEra && era != UmAlQuraEra) throw System::ArgumentOutOfRangeException("era");
    if (year < MinCalendarYear || year > MaxCalendarYear)
        throw System::ArgumentOutOfRangeException("year");
}

void UmAlQuraCalendar::checkYearMonth(int year, int month, int era) {
    checkYear(year, era);
    if (month < 1 || month > 12) throw System::ArgumentOutOfRangeException("month");
}

// Absolute Gregorian date (day 1 = 0001-01-01)
long long UmAlQuraCalendar::absoluteDate(int y, int m, int d) {
    static const int pre[] = {0,0,31,59,90,120,151,181,212,243,273,304,334};
    long long yy = y - 1;
    long long days = yy * 365LL + yy / 4 - yy / 100 + yy / 400 + pre[m];
    if (m > 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) ++days;
    return days + d;
}

int UmAlQuraCalendar::daysInYear(int year) {
    int b = s_yearInfo[year - MinCalendarYear].flags;
    int days = 0;
    for (int m = 0; m < 12; ++m) { days += 29 + (b & 1); b >>= 1; }
    return days;
}

void UmAlQuraCalendar::gregorianToHijri(const System::DateTime& time,
                                         int& hy, int& hm, int& hd) {
    // Approximate starting index (Hijri year ≈ 354-355 days)
    long long minTicks = (absoluteDate(1900, 4, 30) - 1) * System::DateTime::TicksPerDay;
    int index = static_cast<int>((time.getTicksProperty() - minTicks)
                                 / System::DateTime::TicksPerDay) / 355;
    if (index < 0) index = 0;
    if (index > static_cast<int>(183 - 1)) index = 183 - 1;

    // Advance until we're past the target date
    while (index < 183) {
        const DateMapping& dm = s_yearInfo[index + 1];
        long long dmTicks = (absoluteDate(dm.gYear, dm.gMonth, dm.gDay) - 1)
                            * System::DateTime::TicksPerDay;
        if (time.getTicksProperty() < dmTicks) break;
        ++index;
    }

    // Check if we're exactly at the start of (index+1)
    {
        const DateMapping& dm = s_yearInfo[index];
        long long dmTicks = (absoluteDate(dm.gYear, dm.gMonth, dm.gDay) - 1)
                            * System::DateTime::TicksPerDay;
        if (time.getTicksProperty() < dmTicks) --index;
    }

    const DateMapping& entry = s_yearInfo[index];
    long long startTicks = (absoluteDate(entry.gYear, entry.gMonth, entry.gDay) - 1)
                            * System::DateTime::TicksPerDay;
    double nDays = static_cast<double>((time.getTicksProperty() - startTicks)
                                        / System::DateTime::TicksPerDay);

    hy = index + MinCalendarYear;
    hm = 1;
    hd = 1;
    int b = entry.flags;
    int dpm = 29 + (b & 1);
    while (nDays >= dpm) {
        nDays -= dpm;
        b >>= 1;
        dpm = 29 + (b & 1);
        ++hm;
    }
    hd += static_cast<int>(nDays);
}

void UmAlQuraCalendar::hijriToGregorian(int hy, int hm, int hd,
                                         int& gy, int& gm, int& gd) {
    int nDays = hd - 1;
    int index = hy - MinCalendarYear;
    long long startAbs = absoluteDate(s_yearInfo[index].gYear,
                                      s_yearInfo[index].gMonth,
                                      s_yearInfo[index].gDay);
    int b = s_yearInfo[index].flags;
    for (int m = 1; m < hm; ++m) { nDays += 29 + (b & 1); b >>= 1; }

    // Convert absolute date back to Gregorian y/m/d
    long long absDay = startAbs + nDays;
    // Reverse absolute-date formula via DateTime
    System::DateTime result(absDay * System::DateTime::TicksPerDay
                            - System::DateTime::TicksPerDay); // abs day 1 = tick 0
    gy = result.getYearProperty();
    gm = result.getMonthProperty();
    gd = result.getDayProperty();
}

long long UmAlQuraCalendar::absoluteDateUAQ(int year, int month, int day) {
    int gy, gm, gd;
    hijriToGregorian(year, month, day, gy, gm, gd);
    return absoluteDate(gy, gm, gd);
}

// ---------------------------------------------------------------------------
// Calendar overrides
// ---------------------------------------------------------------------------

intcs UmAlQuraCalendar::GetEra(const System::DateTime& /*time*/) const { return UmAlQuraEra; }

int UmAlQuraCalendar::getDatePart(const System::DateTime& time, int part) const {
    int hy, hm, hd;
    gregorianToHijri(time, hy, hm, hd);
    if (part == PartYear)  return hy;
    if (part == PartMonth) return hm;
    if (part == PartDay)   return hd;
    // DayOfYear
    return static_cast<int>(absoluteDateUAQ(hy, hm, hd) - absoluteDateUAQ(hy, 1, 1) + 1);
}

intcs UmAlQuraCalendar::GetYear(const System::DateTime& time) const {
    return getDatePart(time, PartYear);
}

intcs UmAlQuraCalendar::GetMonth(const System::DateTime& time) const {
    return getDatePart(time, PartMonth);
}

intcs UmAlQuraCalendar::GetDayOfMonth(const System::DateTime& time) const {
    return getDatePart(time, PartDay);
}

intcs UmAlQuraCalendar::GetDayOfYear(const System::DateTime& time) const {
    return getDatePart(time, PartDayOfYear);
}

bool UmAlQuraCalendar::IsLeapYear(intcs year, intcs era) const {
    checkYear(year, era);
    return daysInYear(year) == 355;
}

intcs UmAlQuraCalendar::GetDaysInMonth(intcs year, intcs month, intcs era) const {
    checkYearMonth(year, month, era);
    int flags = s_yearInfo[year - MinCalendarYear].flags;
    return ((flags >> (month - 1)) & 1) ? 30 : 29;
}

intcs UmAlQuraCalendar::GetDaysInYear(intcs year, intcs era) const {
    checkYear(year, era);
    return daysInYear(year);
}

System::DateTime UmAlQuraCalendar::AddMonths(const System::DateTime& time, intcs months) const {
    if (months < -120000 || months > 120000) throw System::ArgumentOutOfRangeException("months");
    int y = GetYear(time), m = GetMonth(time), d = GetDayOfMonth(time);
    int i = m - 1 + months;
    if (i >= 0) { m = i % 12 + 1; y += i / 12; }
    else         { m = 12 + (i + 1) % 12; y += (i - 11) / 12; }
    int maxDay = GetDaysInMonth(y, m);
    if (d > maxDay) d = maxDay;
    long long ticks = absoluteDateUAQ(y, m, d) * System::DateTime::TicksPerDay
                      - System::DateTime::TicksPerDay  // abs day 1 has tick 0
                      + (time.getTicksProperty() % System::DateTime::TicksPerDay);
    return System::DateTime(ticks);
}

System::DateTime UmAlQuraCalendar::AddYears(const System::DateTime& time, intcs years) const {
    // `years * 12` computed directly with no upfront bounds check is real signed-integer-
    // overflow UB in C++ for a merely large years argument (same bug class fixed in
    // Calendar::AddYears/JulianCalendar::AddYears/PersianCalendar::AddYears/
    // HebrewCalendar::AddYears/HijriCalendar::AddYears). Validated against the equivalent
    // bound AddMonths (above) already enforces (120000 / 12 = 10000).
    if (years < -10000 || years > 10000)
        throw System::ArgumentOutOfRangeException("years");
    return AddMonths(time, years * 12);
}

System::DateTime UmAlQuraCalendar::ToDateTime(intcs year, intcs month, intcs day, intcs hour, intcs minute,
                                              intcs second, intcs millisecond, intcs era) const {
    checkYearMonth(year, month, era);
    int daysInMonth = GetDaysInMonth(year, month, era);
    if (day < 1 || day > daysInMonth) throw System::ArgumentOutOfRangeException("day");

    long long ticks = absoluteDateUAQ(year, month, day) * System::DateTime::TicksPerDay
                      - System::DateTime::TicksPerDay // abs day 1 has tick 0
                      + hour * System::DateTime::TicksPerHour
                      + minute * System::DateTime::TicksPerMinute
                      + second * System::DateTime::TicksPerSecond
                      + static_cast<long long>(millisecond) * System::DateTime::TicksPerMillisecond;
    return System::DateTime(ticks);
}

} // namespace System::Globalization
