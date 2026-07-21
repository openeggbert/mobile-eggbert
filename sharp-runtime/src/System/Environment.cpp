// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Environment.hpp"
#include "System/IO/DirectoryNotFoundException.hpp"

#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <psapi.h>
#  include <shlobj.h>
#  undef GetCurrentDirectory   // windows.h macro collides with our method name
#  undef SetCurrentDirectory   // windows.h macro collides with our method name
#  undef SetEnvironmentVariable // windows.h macro collides with our method name
#elif defined(__EMSCRIPTEN__)
#  include <unistd.h>
#  include <climits>
#else
#  include <unistd.h>
#  include <climits>
#  include <pwd.h>
#  include <time.h>
#  include <sys/resource.h>
#  include <sys/utsname.h>
#  include <cstdio>
#endif
#include <cstring>
#include <sstream>

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
extern char** environ; // POSIX — global process environment array
#endif

namespace System {

namespace {
    // Splits one raw "NAME=value" environment-block entry on its first '='. Returns false (skip
    // this entry) for a malformed block entry with no '=' at all, or one where '=' is the very
    // first character (an empty name) -- matching real .NET's ParseEntry, which explicitly skips
    // "entries starting with '=' and entries with no value" when building the environment
    // dictionary (Environment.Variables.Unix.cs). The previous version of the POSIX loop below
    // only checked for "no '=' at all", not "'=' is the first character", so a corrupted/synthetic
    // environment block entry like "=X" would have produced an empty-string key -- inconsistent
    // with the Windows loop just below it, which already had the `eq > 0` guard.
    bool splitEnvEntry(const std::string& entry, std::string& key, std::string& value) {
        auto eq = entry.find('=');
        if (eq == std::string::npos || eq == 0) return false;
        key = entry.substr(0, eq);
        value = entry.substr(eq + 1);
        return true;
    }
}

// ---------------------------------------------------------------------------
// OSVersion
// ---------------------------------------------------------------------------

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
static int parseNextNumber(const std::string& s, std::size_t& pos) {
    // skip to next digit
    while (pos < s.size() && (s[pos] < '0' || s[pos] > '9')) ++pos;
    int num = 0;
    while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') {
        int d = s[pos++] - '0';
        if (num <= (2147483647 - d) / 10) num = num * 10 + d;
        else return 2147483647; // overflow guard
    }
    return num;
}
#endif

System::OperatingSystem Environment::getOSVersionProperty() {
#if defined(_WIN32)
    OSVERSIONINFOEXW osvi{};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
#if defined(_MSC_VER)
#pragma warning(suppress: 4996)
#endif
    GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&osvi));
    Version v(static_cast<int>(osvi.dwMajorVersion),
               static_cast<int>(osvi.dwMinorVersion),
               static_cast<int>(osvi.dwBuildNumber),
               static_cast<int>(osvi.wServicePackMajor));
    return System::OperatingSystem(PlatformID::Win32NT, v);
#elif defined(__EMSCRIPTEN__)
    return System::OperatingSystem(PlatformID::Other, Version(0, 0));
#else
    // Parse utsname.release (e.g. "6.1.0-28-amd64") for major.minor.build.revision
    struct utsname uts {};
    uname(&uts);
    std::string release(uts.release);
    std::size_t pos = 0;
    int major    = parseNextNumber(release, pos);
    int minor    = parseNextNumber(release, pos);
    int build    = parseNextNumber(release, pos);
    int revision = parseNextNumber(release, pos);
    return System::OperatingSystem(PlatformID::Unix, Version(major, minor, build, revision));
#endif
}

std::string Environment::GetCurrentDirectory() {
#if defined(_WIN32)
    char buf[4096];
    if (GetCurrentDirectoryA(static_cast<DWORD>(sizeof(buf)), buf))
        return std::string(buf);
    return "";
#else
    char buf[4096];
    if (getcwd(buf, sizeof(buf))) return std::string(buf);
    return "";
#endif
}

SharpRuntime::intcs Environment::getProcessorCountProperty() {
#if defined(_WIN32)
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    return static_cast<SharpRuntime::intcs>(si.dwNumberOfProcessors);
#elif defined(__EMSCRIPTEN__)
    return 1; // WebAssembly is single-threaded without pthreads
#else
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return n > 0 ? static_cast<SharpRuntime::intcs>(n) : 1;
#endif
}

std::string Environment::getMachineNameProperty() {
#if defined(_WIN32)
    char buf[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(buf);
    if (GetComputerNameA(buf, &size)) return std::string(buf);
    return "";
#else
    // Real .NET's Unix MachineName truncates at the first '.' to strip the domain suffix
    // (Environment.Unix.cs: `int dotPos = hostName.IndexOf('.'); return dotPos < 0 ? hostName :
    // hostName.Substring(0, dotPos);`) -- gethostname() alone can return a fully-qualified name
    // like "myhost.example.com" on systems where that's how the hostname is configured.
    char buf[HOST_NAME_MAX + 1];
    if (gethostname(buf, sizeof(buf)) != 0) return "";
    std::string hostName(buf);
    auto dotPos = hostName.find('.');
    return dotPos == std::string::npos ? hostName : hostName.substr(0, dotPos);
#endif
}

std::string Environment::getUserNameProperty() {
#if defined(_WIN32)
    char buf[256];
    DWORD size = sizeof(buf);
    if (GetUserNameA(buf, &size)) return std::string(buf);
    return "";
#elif defined(__EMSCRIPTEN__)
    const char* user = std::getenv("USER");
    return user ? std::string(user) : std::string("user");
#else
    const char* user = std::getenv("USER");
    if (user) return std::string(user);
    struct passwd* pw = getpwuid(getuid());
    return pw ? std::string(pw->pw_name) : std::string();
#endif
}

static void validateEnvironmentVariableName(const std::string& name) {
    ArgumentException::ThrowIfNullOrEmpty(name, "variable");
    if (name[0] == '\0')
        throw ArgumentException("The first char in the string is the null character.", "variable");
    if (name.find('=') != std::string::npos)
        throw ArgumentException("Environment variable name cannot contain equal character.", "variable");
}

void Environment::SetEnvironmentVariable(const std::string& name, const std::string& value) {
    validateEnvironmentVariableName(name);
#if defined(_WIN32)
    if (value.empty())
        _putenv_s(name.c_str(), "");
    else
        _putenv_s(name.c_str(), value.c_str());
#else
    if (value.empty())
        ::unsetenv(name.c_str());
    else
        ::setenv(name.c_str(), value.c_str(), 1);
#endif
}

void Environment::SetEnvironmentVariable(const std::string& name, const std::string& value,
                                         EnvironmentVariableTarget target) {
    if (target == EnvironmentVariableTarget::Process) {
        SetEnvironmentVariable(name, value);
        return;
    }
    // No Windows-registry (or other persistent-store) backing in this port, so — matching
    // .NET's own behavior on non-Windows platforms — User/Machine are validated but otherwise no-ops.
    validateEnvironmentVariableName(name);
}

std::vector<std::string> Environment::GetLogicalDrives() {
#if defined(_WIN32)
    std::vector<std::string> drives;
    char buf[512];
    DWORD len = GetLogicalDriveStringsA(static_cast<DWORD>(sizeof(buf)), buf);
    for (char* p = buf; p < buf + len; p += std::strlen(p) + 1)
        drives.emplace_back(p);
    return drives;
#else
    return { "/" };
#endif
}

// Ported from real .NET's Environment.ExpandEnvironmentVariablesCore (Environment.UnixOrBrowser.cs)
// rather than a from-scratch %VAR% scanner, since the reference algorithm has a non-obvious
// property the previous implementation here didn't reproduce: when a %name% token fails to
// resolve, only its opening '%' is treated as consumed literal text -- the closing '%' is left
// to double as the *opening* delimiter of the next token, rather than both percents being
// consumed as one failed pair. E.g. for "%UNDEFINED%HOME%" (UNDEFINED unset, HOME set), real
// .NET produces "%UNDEFINED" + <value of HOME> (the middle '%' is absorbed into the HOME token),
// not "%UNDEFINED%" + "HOME%" with HOME left unexpanded, which is what a naive
// find-the-next-'%'-and-consume-both scanner (the previous version of this function) produced.
std::string Environment::ExpandEnvironmentVariables(const std::string& name) {
    if (name.empty()) return name;

    std::string result;
    result.reserve(name.size());
    size_t lastPos = 0;
    size_t pos;
    while (lastPos < name.size() && (pos = name.find('%', lastPos + 1)) != std::string::npos) {
        if (name[lastPos] == '%') {
            std::string key = name.substr(lastPos + 1, pos - lastPos - 1);
            // getenv("") is unspecified by POSIX ("If name is an empty string ... the behavior
            // is undefined") -- guard it explicitly rather than relying on every libc happening
            // to return null gracefully for "%%".
            const char* val = key.empty() ? nullptr : std::getenv(key.c_str());
            if (val != nullptr) {
                result += val;
                lastPos = pos + 1;
                continue;
            }
        }
        result.append(name, lastPos, pos - lastPos);
        lastPos = pos;
    }
    result.append(name, lastPos, std::string::npos);
    return result;
}

std::string Environment::GetFolderPath(SpecialFolder folder) {
#if defined(_WIN32)
    char buf[MAX_PATH];
    if (SHGetFolderPathA(nullptr, static_cast<int>(folder), nullptr, SHGFP_TYPE_CURRENT, buf) == S_OK)
        return std::string(buf);
    return "";
#else
    const char* home = std::getenv("HOME");
    std::string h = home ? std::string(home) : std::string();
    switch (folder) {
        case SpecialFolder::Personal:          // == MyDocuments (0x0005), returns home
        case SpecialFolder::UserProfile:      return h;
        case SpecialFolder::Desktop:
        case SpecialFolder::DesktopDirectory: return h + "/Desktop";
        case SpecialFolder::MyMusic:          return h + "/Music";
        case SpecialFolder::MyPictures:       return h + "/Pictures";
        case SpecialFolder::MyVideos:         return h + "/Videos";
        case SpecialFolder::ApplicationData:  return h + "/.config";
        case SpecialFolder::LocalApplicationData: return h + "/.local/share";
        case SpecialFolder::CommonApplicationData: return "/etc";
        case SpecialFolder::ProgramFiles:     return "/usr";
        case SpecialFolder::System:           return "/usr/lib";
        case SpecialFolder::Fonts:            return "/usr/share/fonts";
        case SpecialFolder::Templates:        return h + "/Templates";
        default:                              return "";
    }
#endif
}

Environment::ProcessCpuUsage Environment::getCpuUsageProperty() {
#if defined(_WIN32)
    FILETIME creation, exit, kernel, user;
    if (GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user)) {
        auto toTS = [](FILETIME ft) -> TimeSpan {
            longcs ticks = (static_cast<longcs>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
            return TimeSpan(ticks); // FILETIME is in 100-ns units, same as TimeSpan ticks
        };
        return { toTS(user), toTS(kernel) };
    }
    return { TimeSpan::Zero, TimeSpan::Zero };
#elif defined(__EMSCRIPTEN__)
    return { TimeSpan::Zero, TimeSpan::Zero };
#else
    struct rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        auto tvToTS = [](const struct timeval& tv) -> TimeSpan {
            longcs ticks = static_cast<longcs>(tv.tv_sec) * 10000000LL
                         + static_cast<longcs>(tv.tv_usec) * 10LL;
            return TimeSpan(ticks);
        };
        return { tvToTS(usage.ru_utime), tvToTS(usage.ru_stime) };
    }
    return { TimeSpan::Zero, TimeSpan::Zero };
#endif
}

SharpRuntime::intcs Environment::getProcessIdProperty() {
#if defined(_WIN32)
    return static_cast<SharpRuntime::intcs>(GetCurrentProcessId());
#else
    return static_cast<SharpRuntime::intcs>(::getpid());
#endif
}

SharpRuntime::longcs Environment::getTickCount64Property() {
#if defined(_WIN32)
    return static_cast<SharpRuntime::longcs>(GetTickCount64());
#else
    struct timespec ts{};
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return static_cast<SharpRuntime::longcs>(ts.tv_sec) * 1000LL
         + static_cast<SharpRuntime::longcs>(ts.tv_nsec) / 1000000LL;
#endif
}

bool Environment::getIsPrivilegedProcessProperty() {
#if defined(_WIN32)
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return false;
    TOKEN_ELEVATION elev{};
    DWORD size = 0;
    bool result = GetTokenInformation(token, TokenElevation, &elev, sizeof(elev), &size)
                  && elev.TokenIsElevated;
    CloseHandle(token);
    return result;
#elif defined(__EMSCRIPTEN__)
    return false;
#else
    return ::geteuid() == 0;
#endif
}

void Environment::SetCurrentDirectory(const std::string& path) {
    // Real .NET's Environment.CurrentDirectory setter first calls
    // ArgumentException.ThrowIfNullOrEmpty(value) before touching the OS (Environment.cs), then
    // throws when the underlying OS call fails (Environment.Windows.cs:
    // `if (!Interop.Kernel32.SetCurrentDirectory(value))` throws a Win32-error-derived exception,
    // typically DirectoryNotFoundException for a missing path) -- this previously did neither,
    // silently accepting an empty path and ignoring chdir()'s return value entirely instead of
    // surfacing either failure.
    ArgumentException::ThrowIfNullOrEmpty(path, "value");
#if defined(_WIN32)
    if (!SetCurrentDirectoryA(path.c_str()))
        throw System::IO::DirectoryNotFoundException("Could not find a part of the path '" + path + "'.");
#else
    if (chdir(path.c_str()) != 0)
        throw System::IO::DirectoryNotFoundException("Could not find a part of the path '" + path + "'.");
#endif
}

std::string Environment::getProcessPathProperty() {
#if defined(_WIN32)
    char buf[4096]{};
    GetModuleFileNameA(nullptr, buf, sizeof(buf) - 1);
    return std::string(buf);
#elif defined(__EMSCRIPTEN__)
    return "";
#else
    char buf[4096]{};
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) { buf[len] = '\0'; return std::string(buf); }
    return "";
#endif
}

std::map<std::string, std::string> Environment::GetEnvironmentVariables() {
    std::map<std::string, std::string> result;
#if defined(_WIN32)
    LPCH envBlock = GetEnvironmentStringsA();
    if (envBlock) {
        for (LPCH p = envBlock; *p; p += std::strlen(p) + 1) {
            std::string key, value;
            if (splitEnvEntry(p, key, value)) result[key] = value;
        }
        FreeEnvironmentStringsA(envBlock);
    }
#else
    for (char** ep = environ; ep && *ep; ++ep) {
        std::string key, value;
        if (splitEnvEntry(*ep, key, value)) result[key] = value;
    }
#endif
    return result;
}

SharpRuntime::intcs Environment::getSystemPageSizeProperty() {
#if defined(_WIN32)
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    return static_cast<SharpRuntime::intcs>(si.dwPageSize);
#else
    return static_cast<SharpRuntime::intcs>(getpagesize());
#endif
}

std::string Environment::getUserDomainNameProperty() {
#if defined(_WIN32)
    char buf[256]{};
    DWORD size = sizeof(buf);
    if (GetComputerNameExA(ComputerNameDnsDomain, buf, &size) && buf[0])
        return std::string(buf);
    return getMachineNameProperty();
#else
    return getMachineNameProperty();
#endif
}

SharpRuntime::longcs Environment::getWorkingSetProperty() {
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS pmc{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        return static_cast<SharpRuntime::longcs>(pmc.WorkingSetSize);
    return 0;
#elif defined(__linux__)
    FILE* f = std::fopen("/proc/self/status", "r");
    if (!f) return 0;
    char line[256];
    long kb = 0;
    while (std::fgets(line, sizeof(line), f)) {
        if (std::sscanf(line, "VmRSS: %ld kB", &kb) == 1) break;
    }
    std::fclose(f);
    return static_cast<SharpRuntime::longcs>(kb) * 1024LL;
#else
    return 0;
#endif
}

static std::vector<std::string> s_commandLineArgs;

void Environment::InitializeCommandLine(int argc, char** argv) {
    s_commandLineArgs.clear();
    for (int i = 0; i < argc; ++i) s_commandLineArgs.emplace_back(argv[i]);
}

std::vector<std::string> Environment::GetCommandLineArgs() {
    return s_commandLineArgs;
}

std::string Environment::getCommandLineProperty() {
    // Real .NET's Environment.CommandLine (PasteArguments.Paste) re-quotes each argument using
    // the same backslash-doubling rules as Windows' CommandLineToArgvW, so an argument containing
    // whitespace/quotes round-trips unambiguously. This simplified version just space-joins the
    // raw argv entries verbatim -- correct for the common case (no whitespace/quote characters in
    // any argument), but an argument containing a space would be indistinguishable from two
    // separate arguments if this string were re-parsed. Not fixed: full CommandLineToArgvW-style
    // quoting is a Windows-specific escaping scheme with limited practical value for this
    // POSIX-focused runtime's primarily-diagnostic use of this property.
    std::string result;
    for (std::size_t i = 0; i < s_commandLineArgs.size(); ++i) {
        if (i > 0) result += ' ';
        result += s_commandLineArgs[i];
    }
    return result;
}

} // namespace System
