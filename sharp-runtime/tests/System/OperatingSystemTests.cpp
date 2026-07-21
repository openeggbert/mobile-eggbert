// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/OperatingSystem.hpp"

using System::OperatingSystem;
using System::PlatformID;
using System::Version;

TEST(OperatingSystemTest, ConstructAndGetPlatform) {
    OperatingSystem os(PlatformID::Unix, Version(5, 15, 0, 0));
    EXPECT_EQ(os.getPlatformProperty(), PlatformID::Unix);
}

TEST(OperatingSystemTest, VersionString) {
    OperatingSystem os(PlatformID::Unix, Version(5, 15, 0, 0));
    std::string vs = os.getVersionStringProperty();
    EXPECT_NE(vs.find("Unix"), std::string::npos);
    EXPECT_NE(vs.find("5"), std::string::npos);
}

TEST(OperatingSystemTest, ToString) {
    OperatingSystem os(PlatformID::Unix, Version(1, 0, 0, 0));
    EXPECT_EQ(os.ToString(), os.getVersionStringProperty());
}

TEST(OperatingSystemTest, VersionString_Other) {
    // PlatformID::Other is not a dead/legacy enum value here: Environment::getOSVersionProperty()
    // constructs it for Emscripten builds. Previously fell through to "Unknown ", not real
    // .NET's "Other ".
    OperatingSystem os(PlatformID::Other, Version(0, 0));
    EXPECT_EQ(os.getVersionStringProperty(), "Other 0.0");
}

TEST(OperatingSystemTest, VersionString_Win32S) {
    OperatingSystem os(PlatformID::Win32S, Version(1, 0));
    EXPECT_EQ(os.getVersionStringProperty(), "Microsoft Win32S 1.0");
}

TEST(OperatingSystemTest, VersionString_WinCE) {
    OperatingSystem os(PlatformID::WinCE, Version(5, 0));
    EXPECT_EQ(os.getVersionStringProperty(), "Microsoft Windows CE 5.0");
}

TEST(OperatingSystemTest, VersionString_Xbox) {
    OperatingSystem os(PlatformID::Xbox, Version(1, 0));
    EXPECT_EQ(os.getVersionStringProperty(), "Xbox 1.0");
}

TEST(OperatingSystemTest, VersionString_Win32Windows_Version95) {
    // Real .NET: major<4, or major==4 && minor==0 -> "95"; otherwise "98".
    OperatingSystem os(PlatformID::Win32Windows, Version(4, 0));
    EXPECT_EQ(os.getVersionStringProperty(), "Microsoft Windows 95 4.0");
}

TEST(OperatingSystemTest, VersionString_Win32Windows_Version98) {
    OperatingSystem os(PlatformID::Win32Windows, Version(4, 10));
    EXPECT_EQ(os.getVersionStringProperty(), "Microsoft Windows 98 4.10");
}

TEST(OperatingSystemTest, Clone) {
    OperatingSystem os(PlatformID::Unix, Version(1, 0, 0, 0));
    auto clone = os.Clone();
    EXPECT_EQ(clone->getPlatformProperty(), PlatformID::Unix);
}

TEST(OperatingSystemTest, IsWindowsAndIsLinux_OneTrue) {
    // Exactly one of these must be true on the build host
    EXPECT_TRUE(OperatingSystem::IsWindows() || OperatingSystem::IsLinux() || OperatingSystem::IsMacOS());
}

TEST(OperatingSystemTest, IsOSPlatform_Linux) {
    if (OperatingSystem::IsLinux()) {
        EXPECT_TRUE(OperatingSystem::IsOSPlatform("linux"));
        EXPECT_TRUE(OperatingSystem::IsOSPlatform("LINUX"));
    }
}

TEST(OperatingSystemTest, IsOSPlatform_Unknown) {
    EXPECT_FALSE(OperatingSystem::IsOSPlatform("unknown_os_xyz"));
}

TEST(OperatingSystemTest, IsAndroid_FalseOnNonAndroidBuild) {
    EXPECT_FALSE(OperatingSystem::IsAndroid());
}

TEST(OperatingSystemTest, IsIOS_FalseOnNonAppleBuild) {
    EXPECT_FALSE(OperatingSystem::IsIOS());
}

// Regression test for a wave-3 audit finding: IsAndroid() was hardcoded false, so it and
// IsLinux() could both report true simultaneously on an Android build (Android's kernel also
// defines __linux__) -- real .NET treats them as mutually exclusive OSPlatform values.
// Verified against IsMacOS()'s existing analogous exclusion of iOS.
TEST(OperatingSystemTest, IsAndroidAndIsLinux_MutuallyExclusive) {
    EXPECT_FALSE(OperatingSystem::IsAndroid() && OperatingSystem::IsLinux());
}

TEST(OperatingSystemTest, IsIOSAndIsMacOS_MutuallyExclusive) {
    EXPECT_FALSE(OperatingSystem::IsIOS() && OperatingSystem::IsMacOS());
}

TEST(OperatingSystemTest, ServicePack_Empty) {
    OperatingSystem os(PlatformID::Unix, Version(1, 0, 0, 0));
    EXPECT_TRUE(os.getServicePackProperty().empty());
}
