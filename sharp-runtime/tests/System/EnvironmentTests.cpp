// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <algorithm>
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/Environment.hpp"
#include "System/IO/DirectoryNotFoundException.hpp"
#include "System/Version.hpp"
#ifndef _WIN32
#include <unistd.h>
#endif

using System::Environment;
using SharpRuntime::longcs;
using SharpRuntime::intcs;

// ---------------------------------------------------------------------------
// NewLine
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, NewLine_IsNonEmpty) {
    EXPECT_FALSE(Environment::NewLine.empty());
}

// ---------------------------------------------------------------------------
// GetCurrentDirectory
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, GetCurrentDirectory_ReturnsNonEmptyString) {
    EXPECT_FALSE(Environment::GetCurrentDirectory().empty());
}

// ---------------------------------------------------------------------------
// GetEnvironmentVariable
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, GetEnvironmentVariable_KnownVar_DoesNotThrow) {
    // PATH is present on Linux/macOS; just verify no exception
    std::string val = Environment::GetEnvironmentVariable("PATH");
    (void)val;
}

TEST(EnvironmentTests, GetEnvironmentVariable_NonExistent_ReturnsEmpty) {
    std::string val = Environment::GetEnvironmentVariable("SHARP_RUNTIME_NONEXISTENT_VAR_XYZ_12345");
    EXPECT_TRUE(val.empty());
}

// getenv("") is unspecified by POSIX; this must not crash and must behave like "not found",
// matching real .NET's GetEnvironmentVariable("") (returns null, no exception).
TEST(EnvironmentTests, GetEnvironmentVariable_EmptyName_ReturnsEmpty) {
    EXPECT_TRUE(Environment::GetEnvironmentVariable("").empty());
}

// ---------------------------------------------------------------------------
// ProcessorCount
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, ProcessorCount_AtLeastOne) {
    EXPECT_GE(Environment::getProcessorCountProperty(), 1);
}

// ---------------------------------------------------------------------------
// Is64Bit helpers
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, Is64BitProcess_ReturnsBool) {
    bool v = Environment::Is64BitProcess;
    (void)v;
}

TEST(EnvironmentTests, Is64BitOperatingSystem_ReturnsBool) {
    bool v = Environment::Is64BitOperatingSystem;
    (void)v;
}

TEST(EnvironmentTests, Is64BitOS_EqualsIs64BitProcess) {
    // This implementation delegates OS check to process check
    EXPECT_EQ(Environment::Is64BitOperatingSystem, Environment::Is64BitProcess);
}

// ---------------------------------------------------------------------------
// MachineName / UserName / TickCount64
// ---------------------------------------------------------------------------
TEST(EnvironmentTests, MachineName_NotEmpty) {
    std::string name = Environment::getMachineNameProperty();
    EXPECT_FALSE(name.empty());
}

#ifndef _WIN32
TEST(EnvironmentTests, MachineName_StripsDomainSuffix) {
    // Real .NET's Unix MachineName truncates at the first '.' to strip the domain suffix
    // (Environment.Unix.cs: hostName.Substring(0, hostName.IndexOf('.'))). Verify the
    // property's return value is never longer than the portion of the raw hostname before
    // its first '.', regardless of whether this particular host's name happens to be
    // fully-qualified.
    char raw[256]{};
    ASSERT_EQ(gethostname(raw, sizeof(raw)), 0);
    std::string rawHost(raw);
    auto dotPos = rawHost.find('.');
    std::string expected = dotPos == std::string::npos ? rawHost : rawHost.substr(0, dotPos);
    EXPECT_EQ(Environment::getMachineNameProperty(), expected);
}
#endif

TEST(EnvironmentTests, UserName_NotEmpty) {
    std::string name = Environment::getUserNameProperty();
    EXPECT_FALSE(name.empty());
}

TEST(EnvironmentTests, TickCount64_Positive) {
    SharpRuntime::longcs t = Environment::getTickCount64Property();
    EXPECT_GT(t, 0LL);
}

TEST(EnvironmentTests, TickCount64_Advances) {
    SharpRuntime::longcs t1 = Environment::getTickCount64Property();
    // int would signed-overflow well before 10,000,000 (true sum is ~5*10^13); volatile long long
    // burns the same CPU time without UB (found via UBSan, ticket 1486).
    volatile long long sink = 0;
    for (int i = 0; i < 10000000; ++i) sink += i;
    SharpRuntime::longcs t2 = Environment::getTickCount64Property();
    (void)sink;
    EXPECT_GE(t2, t1);
}

// ---------------------------------------------------------------------------
// HasShutdownStarted
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, HasShutdownStarted_IsFalse) {
    EXPECT_FALSE(Environment::HasShutdownStarted);
}

// ---------------------------------------------------------------------------
// SetEnvironmentVariable / GetEnvironmentVariable roundtrip
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, SetGet_RoundTrip) {
    Environment::SetEnvironmentVariable("SHARP_TEST_VAR", "hello");
    EXPECT_EQ(Environment::GetEnvironmentVariable("SHARP_TEST_VAR"), "hello");
}

TEST(EnvironmentTests, Set_Empty_RemovesVar) {
    Environment::SetEnvironmentVariable("SHARP_TEST_VAR2", "value");
    Environment::SetEnvironmentVariable("SHARP_TEST_VAR2", "");
    EXPECT_TRUE(Environment::GetEnvironmentVariable("SHARP_TEST_VAR2").empty());
}

// ---------------------------------------------------------------------------
// ExpandEnvironmentVariables
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, ExpandEnvVars_KnownVar) {
    Environment::SetEnvironmentVariable("SHARP_EXPAND_TEST", "world");
    std::string r = Environment::ExpandEnvironmentVariables("hello %SHARP_EXPAND_TEST%");
    EXPECT_EQ(r, "hello world");
}

TEST(EnvironmentTests, ExpandEnvVars_UnknownVar_Preserved) {
    std::string r = Environment::ExpandEnvironmentVariables("%SHARP_NONEXISTENT_XYZ%");
    EXPECT_EQ(r, "%SHARP_NONEXISTENT_XYZ%");
}

TEST(EnvironmentTests, ExpandEnvVars_NoVars_Unchanged) {
    EXPECT_EQ(Environment::ExpandEnvironmentVariables("no vars here"), "no vars here");
}

TEST(EnvironmentTests, ExpandEnvVars_EmptyString_ReturnsEmpty) {
    EXPECT_EQ(Environment::ExpandEnvironmentVariables(""), "");
}

TEST(EnvironmentTests, ExpandEnvVars_MultipleVars) {
    Environment::SetEnvironmentVariable("SHARP_EXPAND_A", "foo");
    Environment::SetEnvironmentVariable("SHARP_EXPAND_B", "bar");
    std::string r = Environment::ExpandEnvironmentVariables("%SHARP_EXPAND_A%-%SHARP_EXPAND_B%");
    EXPECT_EQ(r, "foo-bar");
}

// Real .NET's ExpandEnvironmentVariablesCore (Environment.UnixOrBrowser.cs) has a non-obvious
// property: when a %name% token fails to resolve, only the *opening* '%' is emitted as literal
// text -- the closing '%' is left to double as the opening delimiter of the next token, instead
// of both percents being consumed as one failed pair. Verified against the real algorithm by
// hand-tracing it; a naive "find the next '%' and consume both" scanner (this function's
// previous implementation) does not reproduce this.
TEST(EnvironmentTests, ExpandEnvVars_FailedTokenClosingPercentStartsNextToken) {
    Environment::SetEnvironmentVariable("SHARP_EXPAND_OVERLAP_HOME", "world");
    std::string r = Environment::ExpandEnvironmentVariables("%SHARP_EXPAND_OVERLAP_UNDEFINED%SHARP_EXPAND_OVERLAP_HOME%");
    EXPECT_EQ(r, "%SHARP_EXPAND_OVERLAP_UNDEFINEDworld");
}

TEST(EnvironmentTests, ExpandEnvVars_AdjacentPercents_Unchanged) {
    EXPECT_EQ(Environment::ExpandEnvironmentVariables("%%"), "%%");
}

// ---------------------------------------------------------------------------
// ProcessId
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, ProcessId_Positive) {
    EXPECT_GT(Environment::getProcessIdProperty(), 0);
}

// ---------------------------------------------------------------------------
// TickCount (int)
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, TickCount_Positive) {
    EXPECT_GT(Environment::getTickCountProperty(), 0);
}

// ---------------------------------------------------------------------------
// GetFolderPath
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, GetFolderPath_UserProfile_NonEmpty) {
    std::string p = Environment::GetFolderPath(Environment::SpecialFolder::UserProfile);
    EXPECT_FALSE(p.empty());
}

TEST(EnvironmentTests, GetFolderPath_Desktop_NonEmpty) {
    std::string p = Environment::GetFolderPath(Environment::SpecialFolder::Desktop);
    EXPECT_FALSE(p.empty());
}

// ---------------------------------------------------------------------------
// GetEnvironmentVariable with target
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, GetEnvironmentVariable_WithTarget_SameAsWithout) {
    std::string a = Environment::GetEnvironmentVariable("PATH");
    std::string b = Environment::GetEnvironmentVariable("PATH", System::EnvironmentVariableTarget::Process);
    EXPECT_EQ(a, b);
}

// ---------------------------------------------------------------------------
// IsPrivilegedProcess
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, IsPrivilegedProcess_ReturnsBool) {
    bool v = Environment::getIsPrivilegedProcessProperty();
    (void)v; // just verify it doesn't throw
}

// ---------------------------------------------------------------------------
// SetCurrentDirectory
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, SetCurrentDirectory_ThenGetReflectsChange) {
    std::string original = Environment::GetCurrentDirectory();
    // Change to /tmp and verify
    Environment::SetCurrentDirectory("/tmp");
    EXPECT_EQ(Environment::GetCurrentDirectory(), "/tmp");
    // Restore
    Environment::SetCurrentDirectory(original);
}

TEST(EnvironmentTests, SetCurrentDirectory_NonexistentPath_Throws) {
    // Regression: chdir()'s return value was previously ignored entirely, silently leaving
    // the working directory unchanged instead of surfacing the failure like real .NET does.
    EXPECT_THROW(Environment::SetCurrentDirectory("/this/path/does/not/exist/hopefully"),
                 System::IO::DirectoryNotFoundException);
}

TEST(EnvironmentTests, SetCurrentDirectory_EmptyPath_Throws) {
    // Regression: real .NET's setter calls ArgumentException.ThrowIfNullOrEmpty(value) before
    // touching the OS; this port previously had no such check.
    EXPECT_THROW(Environment::SetCurrentDirectory(""), System::ArgumentException);
}

// ---------------------------------------------------------------------------
// GetEnvironmentVariables
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, GetEnvironmentVariables_ContainsPATH) {
    auto vars = Environment::GetEnvironmentVariables();
    EXPECT_TRUE(vars.count("PATH") > 0);
}

TEST(EnvironmentTests, GetEnvironmentVariables_WithTarget_SameAsWithout) {
    auto a = Environment::GetEnvironmentVariables();
    auto b = Environment::GetEnvironmentVariables(System::EnvironmentVariableTarget::Process);
    EXPECT_EQ(a.size(), b.size());
}

TEST(EnvironmentTests, GetEnvironmentVariables_SetVar_Appears) {
    Environment::SetEnvironmentVariable("SHARP_MAP_TEST", "42");
    auto vars = Environment::GetEnvironmentVariables();
    ASSERT_TRUE(vars.count("SHARP_MAP_TEST") > 0);
    EXPECT_EQ(vars.at("SHARP_MAP_TEST"), "42");
}

// ---------------------------------------------------------------------------
// SetEnvironmentVariable with target
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, SetEnvironmentVariable_WithTarget_SetsForProcess) {
    Environment::SetEnvironmentVariable("SHARP_TARGET_VAR", "ok",
                                        System::EnvironmentVariableTarget::Process);
    EXPECT_EQ(Environment::GetEnvironmentVariable("SHARP_TARGET_VAR"), "ok");
}

// ---------------------------------------------------------------------------
// ProcessPath
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, ProcessPath_NonEmpty) {
    std::string path = Environment::getProcessPathProperty();
    EXPECT_FALSE(path.empty());
}

// ---------------------------------------------------------------------------
// SystemPageSize
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, SystemPageSize_PowerOfTwo) {
    int ps = Environment::getSystemPageSizeProperty();
    EXPECT_GT(ps, 0);
    EXPECT_EQ(ps & (ps - 1), 0); // must be a power of two
}

// ---------------------------------------------------------------------------
// UserDomainName
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, UserDomainName_NonEmpty) {
    EXPECT_FALSE(Environment::getUserDomainNameProperty().empty());
}

// ---------------------------------------------------------------------------
// WorkingSet
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, WorkingSet_NonNegative) {
    EXPECT_GE(Environment::getWorkingSetProperty(), 0LL);
}

// ---------------------------------------------------------------------------
// Command-line args
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, InitializeCommandLine_GetCommandLineArgs_Roundtrip) {
    const char* args[] = { "prog", "--flag", "value" };
    Environment::InitializeCommandLine(3, const_cast<char**>(args));
    auto v = Environment::GetCommandLineArgs();
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], "prog");
    EXPECT_EQ(v[1], "--flag");
    EXPECT_EQ(v[2], "value");
}

TEST(EnvironmentTests, CommandLine_JoinsArgs) {
    const char* args[] = { "prog", "arg1" };
    Environment::InitializeCommandLine(2, const_cast<char**>(args));
    EXPECT_EQ(Environment::getCommandLineProperty(), "prog arg1");
}

TEST(EnvironmentTests, CommandLine_EmptyWhenNotInitialized) {
    Environment::InitializeCommandLine(0, nullptr);
    EXPECT_EQ(Environment::getCommandLineProperty(), "");
}

// ---------------------------------------------------------------------------
// StackTrace (stub)
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, StackTrace_DoesNotThrow) {
    std::string s = Environment::getStackTraceProperty();
    (void)s;
}

// ---------------------------------------------------------------------------
// SpecialFolderOption — enum values
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, SpecialFolderOption_None_IsZero) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolderOption::None), 0);
}

TEST(EnvironmentTests, SpecialFolderOption_DoNotVerify_Is0x4000) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolderOption::DoNotVerify), 0x4000);
}

TEST(EnvironmentTests, SpecialFolderOption_Create_Is0x8000) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolderOption::Create), 0x8000);
}

// ---------------------------------------------------------------------------
// GetFolderPath with SpecialFolderOption
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, GetFolderPath_WithNone_SameAsWithout) {
    auto without = Environment::GetFolderPath(Environment::SpecialFolder::UserProfile);
    auto with    = Environment::GetFolderPath(Environment::SpecialFolder::UserProfile,
                                               Environment::SpecialFolderOption::None);
    EXPECT_EQ(without, with);
}

TEST(EnvironmentTests, GetFolderPath_WithDoNotVerify_SameAsWithout) {
    auto without = Environment::GetFolderPath(Environment::SpecialFolder::Desktop);
    auto with    = Environment::GetFolderPath(Environment::SpecialFolder::Desktop,
                                               Environment::SpecialFolderOption::DoNotVerify);
    EXPECT_EQ(without, with);
}

TEST(EnvironmentTests, GetFolderPath_WithCreate_SameAsWithout) {
    auto without = Environment::GetFolderPath(Environment::SpecialFolder::UserProfile);
    auto with    = Environment::GetFolderPath(Environment::SpecialFolder::UserProfile,
                                               Environment::SpecialFolderOption::Create);
    EXPECT_EQ(without, with);
}

// ---------------------------------------------------------------------------
// ProcessCpuUsage
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, CpuUsage_UserTime_NonNegative) {
    auto usage = Environment::getCpuUsageProperty();
    EXPECT_GE(usage.UserTime.getTotalSecondsProperty(), 0.0);
}

TEST(EnvironmentTests, CpuUsage_PrivilegedTime_NonNegative) {
    auto usage = Environment::getCpuUsageProperty();
    EXPECT_GE(usage.PrivilegedTime.getTotalSecondsProperty(), 0.0);
}

TEST(EnvironmentTests, CpuUsage_TotalTime_EqualsSumOfParts) {
    auto usage = Environment::getCpuUsageProperty();
    System::TimeSpan expected = usage.UserTime + usage.PrivilegedTime;
    EXPECT_EQ(usage.getTotalTimeProperty().getTicksProperty(), expected.getTicksProperty());
}

// ---------------------------------------------------------------------------
// OSVersion
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, OSVersion_PlatformIsValid) {
    auto osv = Environment::getOSVersionProperty();
    // Must be one of the known PlatformID values — not an arbitrary integer
    auto pid = osv.getPlatformProperty();
    bool valid = pid == System::PlatformID::Win32NT
              || pid == System::PlatformID::Unix
              || pid == System::PlatformID::MacOSX
              || pid == System::PlatformID::Other;
    EXPECT_TRUE(valid);
}

TEST(EnvironmentTests, OSVersion_VersionNonNegative) {
    auto osv = Environment::getOSVersionProperty();
    EXPECT_GE(osv.getVersionProperty().Major, 0);
    EXPECT_GE(osv.getVersionProperty().Minor, 0);
}

#ifdef __linux__
TEST(EnvironmentTests, OSVersion_LinuxPlatformIsUnix) {
    auto osv = Environment::getOSVersionProperty();
    EXPECT_EQ(osv.getPlatformProperty(), System::PlatformID::Unix);
}

TEST(EnvironmentTests, OSVersion_LinuxMajorVersionReasonable) {
    auto osv = Environment::getOSVersionProperty();
    // Linux kernel major version has been >= 2 since 1996
    EXPECT_GE(osv.getVersionProperty().Major, 2);
}
#endif

// ---------------------------------------------------------------------------
// UserInteractive
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, UserInteractive_IsTrue) {
    EXPECT_TRUE(Environment::getUserInteractiveProperty());
}

// ---------------------------------------------------------------------------
// SpecialFolder — enum numeric values (match CSIDL constants from .NET)
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, SpecialFolder_Desktop_Is0x0000) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::Desktop), 0x0000);
}
TEST(EnvironmentTests, SpecialFolder_Programs_Is0x0002) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::Programs), 0x0002);
}
TEST(EnvironmentTests, SpecialFolder_Personal_Is0x0005) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::Personal), 0x0005);
}
TEST(EnvironmentTests, SpecialFolder_MyDocuments_SameAsPersonal) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::MyDocuments),
              static_cast<int>(Environment::SpecialFolder::Personal));
}
TEST(EnvironmentTests, SpecialFolder_ApplicationData_Is0x001A) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::ApplicationData), 0x001A);
}
TEST(EnvironmentTests, SpecialFolder_LocalApplicationData_Is0x001C) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::LocalApplicationData), 0x001C);
}
TEST(EnvironmentTests, SpecialFolder_CommonApplicationData_Is0x0023) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::CommonApplicationData), 0x0023);
}
TEST(EnvironmentTests, SpecialFolder_UserProfile_Is0x0028) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::UserProfile), 0x0028);
}
TEST(EnvironmentTests, SpecialFolder_ProgramFiles_Is0x0026) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::ProgramFiles), 0x0026);
}
TEST(EnvironmentTests, SpecialFolder_System_Is0x0025) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::System), 0x0025);
}
TEST(EnvironmentTests, SpecialFolder_Windows_Is0x0024) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::Windows), 0x0024);
}
TEST(EnvironmentTests, SpecialFolder_MyPictures_Is0x0027) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::MyPictures), 0x0027);
}
TEST(EnvironmentTests, SpecialFolder_MyMusic_Is0x000D) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::MyMusic), 0x000D);
}
TEST(EnvironmentTests, SpecialFolder_MyVideos_Is0x000E) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::MyVideos), 0x000E);
}
TEST(EnvironmentTests, SpecialFolder_Fonts_Is0x0014) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::Fonts), 0x0014);
}
TEST(EnvironmentTests, SpecialFolder_Favorites_Is0x0006) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::Favorites), 0x0006);
}
TEST(EnvironmentTests, SpecialFolder_StartMenu_Is0x000B) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::StartMenu), 0x000B);
}
TEST(EnvironmentTests, SpecialFolder_CDBurning_Is0x003B) {
    EXPECT_EQ(static_cast<int>(Environment::SpecialFolder::CDBurning), 0x003B);
}

// ---------------------------------------------------------------------------
// GetFolderPath — additional folders
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, GetFolderPath_ApplicationData_NonEmpty) {
    std::string p = Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData);
    EXPECT_FALSE(p.empty());
}
TEST(EnvironmentTests, GetFolderPath_LocalApplicationData_NonEmpty) {
    std::string p = Environment::GetFolderPath(Environment::SpecialFolder::LocalApplicationData);
    EXPECT_FALSE(p.empty());
}
TEST(EnvironmentTests, GetFolderPath_MyDocuments_NonEmpty) {
    std::string p = Environment::GetFolderPath(Environment::SpecialFolder::MyDocuments);
    EXPECT_FALSE(p.empty());
}
TEST(EnvironmentTests, GetFolderPath_Temp_IsString) {
    std::string p = Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData);
    EXPECT_NE(p, "");
}

// ---------------------------------------------------------------------------
// CurrentManagedThreadId
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, CurrentManagedThreadId_NonNegative) {
    SharpRuntime::intcs id = Environment::getCurrentManagedThreadIdProperty();
    EXPECT_GE(id, 0);
}

TEST(EnvironmentTests, CurrentManagedThreadId_Idempotent) {
    SharpRuntime::intcs a = Environment::getCurrentManagedThreadIdProperty();
    SharpRuntime::intcs b = Environment::getCurrentManagedThreadIdProperty();
    EXPECT_EQ(a, b);
}

// ---------------------------------------------------------------------------
// Version
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, Version_MajorAtLeastOne) {
    System::Version v = Environment::getVersionProperty();
    EXPECT_GE(v.Major, 1);
}

TEST(EnvironmentTests, Version_MinorNonNegative) {
    System::Version v = Environment::getVersionProperty();
    EXPECT_GE(v.Minor, 0);
}

// ---------------------------------------------------------------------------
// EnvironmentVariableTarget — User/Machine are no-ops (matches .NET on non-Windows)
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, GetEnvironmentVariable_UserTarget_ReturnsEmpty) {
    Environment::SetEnvironmentVariable("SHARP_TARGET_NOOP_VAR", "process-value");
    EXPECT_TRUE(Environment::GetEnvironmentVariable(
        "SHARP_TARGET_NOOP_VAR", System::EnvironmentVariableTarget::User).empty());
}

TEST(EnvironmentTests, GetEnvironmentVariable_MachineTarget_ReturnsEmpty) {
    Environment::SetEnvironmentVariable("SHARP_TARGET_NOOP_VAR2", "process-value");
    EXPECT_TRUE(Environment::GetEnvironmentVariable(
        "SHARP_TARGET_NOOP_VAR2", System::EnvironmentVariableTarget::Machine).empty());
}

TEST(EnvironmentTests, SetEnvironmentVariable_UserTarget_DoesNotAffectProcess) {
    Environment::SetEnvironmentVariable("SHARP_TARGET_SET_NOOP", "unset", System::EnvironmentVariableTarget::Process);
    Environment::SetEnvironmentVariable("SHARP_TARGET_SET_NOOP", "user-value",
                                        System::EnvironmentVariableTarget::User);
    EXPECT_EQ(Environment::GetEnvironmentVariable("SHARP_TARGET_SET_NOOP"), "unset");
}

TEST(EnvironmentTests, GetEnvironmentVariables_UserTarget_IsEmpty) {
    auto vars = Environment::GetEnvironmentVariables(System::EnvironmentVariableTarget::User);
    EXPECT_TRUE(vars.empty());
}

TEST(EnvironmentTests, GetEnvironmentVariables_MachineTarget_IsEmpty) {
    auto vars = Environment::GetEnvironmentVariables(System::EnvironmentVariableTarget::Machine);
    EXPECT_TRUE(vars.empty());
}

// ---------------------------------------------------------------------------
// SetEnvironmentVariable — name validation (matches .NET Environment.ValidateVariable)
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, SetEnvironmentVariable_EmptyName_Throws) {
    EXPECT_THROW(Environment::SetEnvironmentVariable("", "value"), System::ArgumentException);
}

TEST(EnvironmentTests, SetEnvironmentVariable_NameContainsEquals_Throws) {
    EXPECT_THROW(Environment::SetEnvironmentVariable("BAD=NAME", "value"), System::ArgumentException);
}

TEST(EnvironmentTests, SetEnvironmentVariable_NameStartsWithNull_Throws) {
    std::string name("\0abc", 4);
    EXPECT_THROW(Environment::SetEnvironmentVariable(name, "value"), System::ArgumentException);
}

TEST(EnvironmentTests, SetEnvironmentVariable_WithUserTarget_EmptyName_Throws) {
    EXPECT_THROW(Environment::SetEnvironmentVariable("", "value", System::EnvironmentVariableTarget::User),
                 System::ArgumentException);
}

// ---------------------------------------------------------------------------
// ExitCode
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, ExitCode_DefaultsToZero) {
    EXPECT_EQ(Environment::getExitCodeProperty(), 0);
}

TEST(EnvironmentTests, ExitCode_SetGet_RoundTrip) {
    Environment::setExitCodeProperty(42);
    EXPECT_EQ(Environment::getExitCodeProperty(), 42);
    Environment::setExitCodeProperty(0); // restore default for other tests
}

// ---------------------------------------------------------------------------
// GetLogicalDrives
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, GetLogicalDrives_NonEmpty) {
    auto drives = Environment::GetLogicalDrives();
    EXPECT_FALSE(drives.empty());
}

#ifndef _WIN32
TEST(EnvironmentTests, GetLogicalDrives_ContainsRoot) {
    auto drives = Environment::GetLogicalDrives();
    EXPECT_NE(std::find(drives.begin(), drives.end(), "/"), drives.end());
}
#endif

// ---------------------------------------------------------------------------
// SystemDirectory
// ---------------------------------------------------------------------------

TEST(EnvironmentTests, SystemDirectory_MatchesGetFolderPathSystem) {
    EXPECT_EQ(Environment::getSystemDirectoryProperty(),
              Environment::GetFolderPath(Environment::SpecialFolder::System));
}
