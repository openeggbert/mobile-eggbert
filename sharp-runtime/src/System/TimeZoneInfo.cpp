// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/TimeZoneInfo.hpp"
#include "System/PlatformNotSupportedException.hpp"
#include "System/TimeZoneNotFoundException.hpp"
#include <cstdlib>
#include <ctime>
#include <mutex>

#if defined(_WIN32)
#  include <windows.h>
#elif defined(__EMSCRIPTEN__)
// No timezone database available
#else
#  include <sys/stat.h>
#endif

namespace System {

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
namespace {
// Guards every read (localtime_r/tm_gmtoff/tm_zone) or write (setenv("TZ", ...)) of the
// process-global TZ state. FindSystemTimeZoneById() temporarily overwrites the TZ
// environment variable to query a different zone; without serializing every reader against
// that window too, a concurrent Local() call on another thread could transiently observe
// the wrong zone's offset/name -- a real, reachable bug for any multi-threaded caller, not
// just a race between two FindSystemTimeZoneById() calls (which alone would already need
// this). A function-local static avoids static-initialization-order concerns between
// translation units and works regardless of declaration order within this file.
std::mutex& tzMutex() {
    static std::mutex m;
    return m;
}

// Detects whether the zone currently active via TZ/localtime_r observes DST at
// all during the year, rather than just whether DST happens to be active right
// now (SupportsDaylightSavingTime asks "does this zone have DST rules", not
// "is DST active this instant"). Callers must already hold tzMutex().
bool posixZoneObservesDst() {
    time_t now = time(nullptr);
    struct tm nowUtc{};
    gmtime_r(&now, &nowUtc);
    struct tm jan{};
    jan.tm_year = nowUtc.tm_year; jan.tm_mon = 0; jan.tm_mday = 15; jan.tm_hour = 12;
    struct tm jul{};
    jul.tm_year = nowUtc.tm_year; jul.tm_mon = 6; jul.tm_mday = 15; jul.tm_hour = 12;
    time_t janT = timegm(&jan);
    time_t julT = timegm(&jul);
    struct tm janLocal{};
    localtime_r(&janT, &janLocal);
    struct tm julLocal{};
    localtime_r(&julT, &julLocal);
    return janLocal.tm_isdst > 0 || julLocal.tm_isdst > 0 || janLocal.tm_gmtoff != julLocal.tm_gmtoff;
}
} // namespace
#endif

// ---------------------------------------------------------------------------
// Local()
// ---------------------------------------------------------------------------

const TimeZoneInfo& TimeZoneInfo::Local() {
    static TimeZoneInfo tz = []() -> TimeZoneInfo {
#if defined(_WIN32)
        TIME_ZONE_INFORMATION tzi{};
        DWORD result = GetTimeZoneInformation(&tzi);
        // Bias is minutes west of UTC; negative for east
        long offset_secs = -(tzi.Bias * 60L);
        bool hasDst = (result == TIME_ZONE_ID_DAYLIGHT);
        // Convert StandardName (WCHAR) to narrow string
        char name[64] = "Local";
        WideCharToMultiByte(CP_UTF8, 0, tzi.StandardName, -1, name, sizeof(name), nullptr, nullptr);
        TimeSpan offset = TimeSpan::FromSeconds(static_cast<double>(offset_secs));
        return TimeZoneInfo("Local", offset, name, name, name, hasDst);
#elif defined(__EMSCRIPTEN__)
        return TimeZoneInfo::Utc();
#else
        std::lock_guard<std::mutex> lock(tzMutex());
        time_t t = time(nullptr);
        struct tm local_tm {};
        localtime_r(&t, &local_tm);
        long   offset_secs = local_tm.tm_gmtoff;
        bool   hasDst      = posixZoneObservesDst();
        std::string name   = local_tm.tm_zone ? local_tm.tm_zone : "Local";
        TimeSpan offset    = TimeSpan::FromSeconds(static_cast<double>(offset_secs));
        return TimeZoneInfo("Local", offset, name, name, name, hasDst);
#endif
    }();
    return tz;
}

// ---------------------------------------------------------------------------
// FindSystemTimeZoneById()
// ---------------------------------------------------------------------------

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
static bool zoneFileExists(const std::string& id) {
    if (id.find("..") != std::string::npos) return false;
    std::string path = "/usr/share/zoneinfo/" + id;
    struct stat st {};
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}
#endif

// ---------------------------------------------------------------------------
// IANA ↔ Windows timezone name mapping (CLDR-derived, ~85 common zones)
// Always compiled; used by TryConvertIanaIdToWindowsId / TryConvertWindowsIdToIanaId
// and, on Windows, by FindSystemTimeZoneById.
// ---------------------------------------------------------------------------
namespace {
static const struct { const char* iana; const char* win; } kIanaWindowsMap[] = {
        {"Africa/Abidjan",                  "Greenwich Standard Time"},
        {"Africa/Accra",                    "Greenwich Standard Time"},
        {"Africa/Cairo",                    "Egypt Standard Time"},
        {"Africa/Johannesburg",             "South Africa Standard Time"},
        {"Africa/Lagos",                    "W. Central Africa Standard Time"},
        {"Africa/Nairobi",                  "E. Africa Standard Time"},
        {"America/Anchorage",               "Alaskan Standard Time"},
        {"America/Argentina/Buenos_Aires",  "Argentina Standard Time"},
        {"America/Bogota",                  "SA Pacific Standard Time"},
        {"America/Chicago",                 "Central Standard Time"},
        {"America/Denver",                  "Mountain Standard Time"},
        {"America/Halifax",                 "Atlantic Standard Time"},
        {"America/Lima",                    "SA Pacific Standard Time"},
        {"America/Los_Angeles",             "Pacific Standard Time"},
        {"America/Mexico_City",             "Central Standard Time (Mexico)"},
        {"America/New_York",                "Eastern Standard Time"},
        {"America/Phoenix",                 "US Mountain Standard Time"},
        {"America/Regina",                  "Canada Central Standard Time"},
        {"America/Sao_Paulo",               "E. South America Standard Time"},
        {"America/St_Johns",                "Newfoundland Standard Time"},
        {"America/Toronto",                 "Eastern Standard Time"},
        {"America/Vancouver",               "Pacific Standard Time"},
        {"Asia/Almaty",                     "Central Asia Standard Time"},
        {"Asia/Baghdad",                    "Arabic Standard Time"},
        {"Asia/Bangkok",                    "SE Asia Standard Time"},
        {"Asia/Calcutta",                   "India Standard Time"},
        {"Asia/Colombo",                    "Sri Lanka Standard Time"},
        {"Asia/Dubai",                      "Arabian Standard Time"},
        {"Asia/Hong_Kong",                  "China Standard Time"},
        {"Asia/Jakarta",                    "SE Asia Standard Time"},
        {"Asia/Jerusalem",                  "Israel Standard Time"},
        {"Asia/Kabul",                      "Afghanistan Standard Time"},
        {"Asia/Karachi",                    "Pakistan Standard Time"},
        {"Asia/Kathmandu",                  "Nepal Standard Time"},
        {"Asia/Kolkata",                    "India Standard Time"},
        {"Asia/Krasnoyarsk",                "North Asia Standard Time"},
        {"Asia/Kuala_Lumpur",               "Singapore Standard Time"},
        {"Asia/Kuwait",                     "Arab Standard Time"},
        {"Asia/Muscat",                     "Arabian Standard Time"},
        {"Asia/Novosibirsk",                "N. Central Asia Standard Time"},
        {"Asia/Rangoon",                    "Myanmar Standard Time"},
        {"Asia/Riyadh",                     "Arab Standard Time"},
        {"Asia/Seoul",                      "Korea Standard Time"},
        {"Asia/Shanghai",                   "China Standard Time"},
        {"Asia/Singapore",                  "Singapore Standard Time"},
        {"Asia/Taipei",                     "Taipei Standard Time"},
        {"Asia/Tashkent",                   "West Asia Standard Time"},
        {"Asia/Tehran",                     "Iran Standard Time"},
        {"Asia/Tokyo",                      "Tokyo Standard Time"},
        {"Asia/Ulaanbaatar",                "Ulaanbaatar Standard Time"},
        {"Asia/Vladivostok",                "Vladivostok Standard Time"},
        {"Asia/Yakutsk",                    "Yakutsk Standard Time"},
        {"Asia/Yekaterinburg",              "Ekaterinburg Standard Time"},
        {"Atlantic/Azores",                 "Azores Standard Time"},
        {"Atlantic/Cape_Verde",             "Cape Verde Standard Time"},
        {"Australia/Adelaide",              "Cen. Australia Standard Time"},
        {"Australia/Brisbane",              "E. Australia Standard Time"},
        {"Australia/Darwin",                "AUS Central Standard Time"},
        {"Australia/Hobart",                "Tasmania Standard Time"},
        {"Australia/Perth",                 "W. Australia Standard Time"},
        {"Australia/Sydney",                "AUS Eastern Standard Time"},
        {"Europe/Amsterdam",                "W. Europe Standard Time"},
        {"Europe/Athens",                   "GTB Standard Time"},
        {"Europe/Berlin",                   "W. Europe Standard Time"},
        {"Europe/Brussels",                 "Romance Standard Time"},
        {"Europe/Bucharest",                "GTB Standard Time"},
        {"Europe/Budapest",                 "Central Europe Standard Time"},
        {"Europe/Dublin",                   "GMT Standard Time"},
        {"Europe/Helsinki",                 "FLE Standard Time"},
        {"Europe/Istanbul",                 "Turkey Standard Time"},
        {"Europe/Kiev",                     "FLE Standard Time"},
        {"Europe/Lisbon",                   "GMT Standard Time"},
        {"Europe/London",                   "GMT Standard Time"},
        {"Europe/Madrid",                   "Romance Standard Time"},
        {"Europe/Minsk",                    "Belarus Standard Time"},
        {"Europe/Moscow",                   "Russian Standard Time"},
        {"Europe/Paris",                    "Romance Standard Time"},
        {"Europe/Prague",                   "Central Europe Standard Time"},
        {"Europe/Rome",                     "W. Europe Standard Time"},
        {"Europe/Sofia",                    "FLE Standard Time"},
        {"Europe/Stockholm",                "W. Europe Standard Time"},
        {"Europe/Vienna",                   "W. Europe Standard Time"},
        {"Europe/Warsaw",                   "Central European Standard Time"},
        {"Europe/Zurich",                   "W. Europe Standard Time"},
        {"Pacific/Auckland",                "New Zealand Standard Time"},
        {"Pacific/Fiji",                    "Fiji Standard Time"},
        {"Pacific/Guam",                    "West Pacific Standard Time"},
        {"Pacific/Honolulu",                "Hawaiian Standard Time"},
        {"Pacific/Midway",                  "Samoa Standard Time"},
        {"Pacific/Port_Moresby",            "West Pacific Standard Time"},
        {"Pacific/Tongatapu",               "Tonga Standard Time"},
    {nullptr, nullptr}
};

static const char* ianaToWindows(const std::string& iana) {
    for (const auto* p = kIanaWindowsMap; p->iana; ++p)
        if (iana == p->iana) return p->win;
    return nullptr;
}

static const char* windowsToIana(const std::string& win) {
    for (const auto* p = kIanaWindowsMap; p->iana; ++p)
        if (win == p->win) return p->iana;
    return nullptr;
}
} // anonymous namespace

std::shared_ptr<TimeZoneInfo> TimeZoneInfo::FindSystemTimeZoneById(const std::string& id) {
    if (id == "UTC")   return std::shared_ptr<TimeZoneInfo>(new TimeZoneInfo(Utc()));
    if (id == "Local") return std::shared_ptr<TimeZoneInfo>(new TimeZoneInfo(Local()));

#if defined(_WIN32)
    const char* winName = ianaToWindows(id);
    if (!winName)
        throw System::TimeZoneNotFoundException(
            "The time zone ID '" + id + "' was not found on the local computer.");

    DYNAMIC_TIME_ZONE_INFORMATION dtzi{};
    for (DWORD i = 0; EnumDynamicTimeZoneInformation(i, &dtzi) != ERROR_NO_MORE_ITEMS; ++i) {
        char tzKey[128] = {};
        WideCharToMultiByte(CP_UTF8, 0, dtzi.TimeZoneKeyName, -1, tzKey, sizeof(tzKey), nullptr, nullptr);
        if (std::string(tzKey) != winName) continue;

        SYSTEMTIME st{};
        GetSystemTime(&st);
        TIME_ZONE_INFORMATION tzi{};
        GetTimeZoneInformationForYear(st.wYear, &dtzi, &tzi);
        long offset_secs = -(tzi.Bias * 60L);
        bool hasDst = (tzi.DaylightBias != 0);
        char stdName[64] = {};
        WideCharToMultiByte(CP_UTF8, 0, tzi.StandardName, -1, stdName, sizeof(stdName), nullptr, nullptr);
        TimeSpan offset = TimeSpan::FromSeconds(static_cast<double>(offset_secs));
        return std::shared_ptr<TimeZoneInfo>(
            new TimeZoneInfo(id, offset, id, stdName, stdName, hasDst));
    }
    throw System::TimeZoneNotFoundException(
        "The time zone ID '" + id + "' was not found on the local computer.");
#elif defined(__EMSCRIPTEN__)
    throw System::PlatformNotSupportedException(
        "FindSystemTimeZoneById is not supported on Emscripten.");
#else
    if (!zoneFileExists(id))
        throw System::TimeZoneNotFoundException(
            "The time zone ID '" + id + "' was not found on the local computer.");

    long   offset_secs = 0;
    bool   hasDst      = false;
    std::string abbrev = id;
    {
        std::lock_guard<std::mutex> lock(tzMutex());
        const char* saved = getenv("TZ");
        std::string savedStr = saved ? saved : "";
        setenv("TZ", id.c_str(), 1);
        tzset();
        time_t t = time(nullptr);
        struct tm tm_buf {};
        localtime_r(&t, &tm_buf);
        offset_secs = tm_buf.tm_gmtoff;
        hasDst      = posixZoneObservesDst();
        abbrev      = tm_buf.tm_zone ? tm_buf.tm_zone : id;
        if (!savedStr.empty()) setenv("TZ", savedStr.c_str(), 1);
        else                   unsetenv("TZ");
        tzset();
    }
    TimeSpan offset = TimeSpan::FromSeconds(static_cast<double>(offset_secs));
    return std::shared_ptr<TimeZoneInfo>(
        new TimeZoneInfo(id, offset, id, abbrev, abbrev, hasDst));
#endif
}

// ---------------------------------------------------------------------------
// GetSystemTimeZones()
// ---------------------------------------------------------------------------

std::vector<std::shared_ptr<TimeZoneInfo>> TimeZoneInfo::GetSystemTimeZones() {
    return {
        std::shared_ptr<TimeZoneInfo>(new TimeZoneInfo(Utc())),
        std::shared_ptr<TimeZoneInfo>(new TimeZoneInfo(Local()))
    };
}

bool TimeZoneInfo::TryConvertIanaIdToWindowsId(const std::string& ianaId, std::string& windowsId) {
    const char* win = ianaToWindows(ianaId);
    if (!win) return false;
    windowsId = win;
    return true;
}

bool TimeZoneInfo::TryConvertWindowsIdToIanaId(const std::string& windowsId, std::string& ianaId) {
    const char* iana = windowsToIana(windowsId);
    if (!iana) return false;
    ianaId = iana;
    return true;
}

} // namespace System
