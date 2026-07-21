// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Globalization/HijriCalendar.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include <stdexcept>

namespace System::Globalization {

// constexpr definition required for ODR in C++17 when taken by address/ref
constexpr int HijriCalendar::s_monthDays[13];

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

long long HijriCalendar::daysUpToHijriYear(int year) const {
    // Full 30-year cycles
    int numYear30  = ((year - 1) / 30) * 30;
    int yearsLeft  = year - numYear30 - 1;
    // Each 30-year cycle = 10631 days; epoch offset = 227013
    long long days = ((long long)numYear30 * 10631LL) / 30LL + HijriEpoch;
    while (yearsLeft > 0) {
        days += 354 + (IsLeapYear(yearsLeft, CurrentEra) ? 1 : 0);
        --yearsLeft;
    }
    return days;
}

long long HijriCalendar::absoluteDateHijri(int y, int m, int d) const {
    return daysUpToHijriYear(y) + s_monthDays[m - 1] + d - 1;
}

int HijriCalendar::getDatePart(long long ticks, int part) const {
    // absolute day: 1 = 0001-01-01; ticks per day = 864000000000
    long long numDays = ticks / System::DateTime::TicksPerDay + 1;

    // approximate Hijri year from magic formula
    int hijriYear = static_cast<int>(((numDays - HijriEpoch) * 30LL) / 10631LL) + 1;
    if (hijriYear < 1) hijriYear = 1;

    long long daysToYear  = daysUpToHijriYear(hijriYear);
    long long daysInYear  = GetDaysInYear(hijriYear, CurrentEra);

    if (numDays < daysToYear) {
        daysToYear -= GetDaysInYear(--hijriYear, CurrentEra);
    } else if (numDays == daysToYear) {
        daysToYear -= GetDaysInYear(--hijriYear, CurrentEra);
    } else if (numDays > daysToYear + daysInYear) {
        daysToYear += daysInYear;
        ++hijriYear;
    }

    if (part == PartYear) return hijriYear;

    int hijriMonth = 1;
    numDays -= daysToYear;

    if (part == PartDayOfYear) return static_cast<int>(numDays);

    while (hijriMonth <= 12 && numDays > s_monthDays[hijriMonth - 1])
        ++hijriMonth;
    --hijriMonth;

    if (part == PartMonth) return hijriMonth;

    return static_cast<int>(numDays - s_monthDays[hijriMonth - 1]);
}

// ---------------------------------------------------------------------------
// Calendar overrides
// ---------------------------------------------------------------------------

int HijriCalendar::GetEra(const System::DateTime& /*time*/) const { return HijriEra; }

int HijriCalendar::GetYear(const System::DateTime& time) const {
    return getDatePart(time.getTicksProperty(), PartYear);
}

int HijriCalendar::GetMonth(const System::DateTime& time) const {
    return getDatePart(time.getTicksProperty(), PartMonth);
}

int HijriCalendar::GetDayOfMonth(const System::DateTime& time) const {
    return getDatePart(time.getTicksProperty(), PartDay);
}

int HijriCalendar::GetDayOfYear(const System::DateTime& time) const {
    return getDatePart(time.getTicksProperty(), PartDayOfYear);
}

bool HijriCalendar::IsLeapYear(int year, int /*era*/) const {
    if (year < 1 || year > MaxCalendarYear)
        throw System::ArgumentOutOfRangeException("year");
    return ((year * 11) + 14) % 30 < 11;
}

int HijriCalendar::GetDaysInMonth(int year, int month, int era) const {
    if (month < 1 || month > 12) throw System::ArgumentOutOfRangeException("month");
    if (month == 12) return IsLeapYear(year, era) ? 30 : 29;
    return (month % 2 == 1) ? 30 : 29;
}

int HijriCalendar::GetDaysInYear(int year, int era) const {
    return IsLeapYear(year, era) ? 355 : 354;
}

System::DateTime HijriCalendar::AddMonths(const System::DateTime& time, int months) const {
    if (months < -120000 || months > 120000)
        throw System::ArgumentOutOfRangeException("months");
    int y = GetYear(time), m = GetMonth(time), d = GetDayOfMonth(time);
    int i = m - 1 + months;
    if (i >= 0) { m = i % 12 + 1; y += i / 12; }
    else         { m = 12 + (i + 1) % 12; y += (i - 11) / 12; }
    int maxDay = GetDaysInMonth(y, m);
    if (d > maxDay) d = maxDay;
    long long ticks = absoluteDateHijri(y, m, d) * System::DateTime::TicksPerDay
                      + (time.getTicksProperty() % System::DateTime::TicksPerDay);
    return System::DateTime(ticks);
}

System::DateTime HijriCalendar::AddYears(const System::DateTime& time, int years) const {
    // `years * 12` computed directly with no upfront bounds check is real signed-integer-
    // overflow UB in C++ for a merely large years argument (same bug class fixed in
    // Calendar::AddYears/JulianCalendar::AddYears/PersianCalendar::AddYears/
    // HebrewCalendar::AddYears). Validated against the equivalent bound AddMonths (above)
    // already enforces (120000 / 12 = 10000).
    if (years < -10000 || years > 10000)
        throw System::ArgumentOutOfRangeException("years");
    return AddMonths(time, years * 12);
}

System::DateTime HijriCalendar::ToDateTime(int year, int month, int day, int hour, int minute,
                                           int second, int millisecond, int /*era*/) const {
    int daysInMonth = GetDaysInMonth(year, month);
    if (day < 1 || day > daysInMonth) throw System::ArgumentOutOfRangeException("day");

    long long absoluteDate = absoluteDateHijri(year, month, day);
    if (absoluteDate < 0) throw System::ArgumentOutOfRangeException("year");

    long long ticks = absoluteDate * System::DateTime::TicksPerDay
        + hour * System::DateTime::TicksPerHour
        + minute * System::DateTime::TicksPerMinute
        + second * System::DateTime::TicksPerSecond
        + static_cast<long long>(millisecond) * System::DateTime::TicksPerMillisecond;
    return System::DateTime(ticks);
}

} // namespace System::Globalization
