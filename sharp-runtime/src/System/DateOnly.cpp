// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)

#include "System/DateOnly.hpp"
#include "System/DateTime.hpp"
#include "System/TimeOnly.hpp"
#include "System/FormatException.hpp"
#include <algorithm>
#include <cstdio>

namespace System {

DateOnly::DateOnly(intcs year, intcs month, intcs day)
    : year_(year), month_(month), day_(day) {
    // Delegates validation to DateTime's constructor (the single source of truth for
    // calendar-date validity in this codebase), matching .NET's own
    // DateOnly(int,int,int) => DayNumberFromDateTime(new DateTime(year, month, day)).
    (void)DateTime(year, month, day);
}

const DateOnly DateOnly::MinValue{1, 1, 1};
const DateOnly DateOnly::MaxValue{9999, 12, 31};

// Julian Day Number helpers for day-accurate date arithmetic
static int dateToJDN(int y, int m, int d) {
    return d - 32075
         + 1461 * (y + 4800 + (m - 14) / 12) / 4
         + 367  * (m - 2   - (m - 14) / 12 * 12) / 12
         - 3    * ((y + 4900 + (m - 14) / 12) / 100) / 4;
}

static void jdnToDate(int jdn, int& y, int& m, int& d) {
    int l = jdn + 68569;
    int n = 4 * l / 146097;
    l = l - (146097 * n + 3) / 4;
    int i = 4000 * (l + 1) / 1461001;
    l = l - 1461 * i / 4 + 31;
    int j = 80 * l / 2447;
    d = l - 2447 * j / 80;
    l = j / 11;
    m = j + 2 - 12 * l;
    y = 100 * (n - 49) + i + l;
}

// JDN of 0001-01-01 (DateOnly epoch)
static constexpr int JDN_EPOCH = 1721426;

DayOfWeek DateOnly::getDayOfWeekProperty() const {
    // JDN % 7: 0=Monday … 6=Sunday → map to .NET Sunday=0
    int jdn = dateToJDN(year_, month_, day_);
    return static_cast<DayOfWeek>((jdn + 1) % 7);
}

intcs DateOnly::getDayOfYearProperty() const {
    return dateToJDN(year_, month_, day_) - dateToJDN(year_, 1, 1) + 1;
}

intcs DateOnly::getDayNumberProperty() const {
    return dateToJDN(year_, month_, day_) - JDN_EPOCH;
}

DateOnly DateOnly::FromDayNumber(intcs dayNumber) {
    int y, m, d;
    jdnToDate(dayNumber + JDN_EPOCH, y, m, d);
    return DateOnly(y, m, d);
}

intcs DateOnly::CompareTo(const DateOnly& other) const {
    intcs a = getDayNumberProperty(), b = other.getDayNumberProperty();
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

DateOnly DateOnly::AddDays(intcs n) const {
    int y, m, d;
    jdnToDate(dateToJDN(year_, month_, day_) + n, y, m, d);
    return DateOnly(y, m, d);
}

DateOnly DateOnly::AddMonths(intcs n) const {
    int y = year_, m = month_ + n, d = day_;
    while (m > 12) { m -= 12; ++y; }
    while (m < 1)  { m += 12; --y; }
    static const int dpm[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    bool leap = (y % 4 == 0) && (y % 100 != 0 || y % 400 == 0);
    int maxDay = (m == 2 && leap) ? 29 : dpm[m];
    if (d > maxDay) d = maxDay;
    return DateOnly(y, m, d);
}

DateOnly DateOnly::AddYears(intcs n) const {
    return AddMonths(n * 12);
}

DateOnly DateOnly::FromDateTime(const DateTime& dt) {
    return DateOnly(dt.getYearProperty(), dt.getMonthProperty(), dt.getDayProperty());
}

DateTime DateOnly::ToDateTime(const TimeOnly& time) const {
    return DateTime(year_, month_, day_,
                     time.getHourProperty(), time.getMinuteProperty(),
                     time.getSecondProperty(), time.getMillisecondProperty());
}

bool DateOnly::TryParse(const std::string& s, DateOnly& result) {
    int y, m, d;
    if (s.size() < 10 || s[4] != '-' || s[7] != '-') return false;
    if (std::sscanf(s.c_str(), "%d-%d-%d", &y, &m, &d) != 3) return false;
    if (y < 1 || y > 9999 || m < 1 || m > 12 || d < 1 || d > 31) return false;
    try {
        result = DateOnly(y, m, d);
    } catch (...) {
        // Rejects dates with a day that doesn't exist in the given month/year
        // (e.g. February 30, or February 29 in a non-leap year).
        return false;
    }
    return true;
}

DateOnly DateOnly::Parse(const std::string& s) {
    DateOnly result;
    if (!TryParse(s, result))
        throw FormatException("String was not recognized as a valid DateOnly: " + s);
    return result;
}

std::string DateOnly::ToString(const std::string& format) const {
    std::string result;
    result.reserve(format.size() + 4);
    auto pad = [](int n, int w) -> std::string {
        std::string s = std::to_string(n);
        while (static_cast<int>(s.size()) < w) s = "0" + s;
        return s;
    };
    size_t i = 0;
    while (i < format.size()) {
        char c = format[i];
        auto run = [&](char ch) {
            size_t k = i + 1;
            while (k < format.size() && format[k] == ch) ++k;
            return static_cast<int>(k - i);
        };
        if (c == 'y') {
            int n = run('y');
            result += (n >= 4) ? pad(year_, 4) : pad(year_ % 100, 2);
            i += n;
        } else if (c == 'M') {
            int n = run('M');
            result += (n >= 2) ? pad(month_, 2) : std::to_string(month_);
            i += n;
        } else if (c == 'd') {
            int n = run('d');
            result += (n >= 2) ? pad(day_, 2) : std::to_string(day_);
            i += n;
        } else if (c == '\'') {
            ++i;
            while (i < format.size() && format[i] != '\'') result += format[i++];
            if (i < format.size()) ++i;
        } else {
            result += c;
            ++i;
        }
    }
    return result;
}

} // namespace System
