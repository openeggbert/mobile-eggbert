// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/Runtime/InteropServices/Architecture.hpp"
#include "System/Runtime/InteropServices/OSPlatform.hpp"
#include "System/Runtime/InteropServices/RuntimeInformation.hpp"

using namespace System::Runtime::InteropServices;

TEST(ArchitectureTests, ValuesAreDistinct) {
    EXPECT_NE(Architecture::X86, Architecture::X64);
    EXPECT_NE(Architecture::Arm, Architecture::Arm64);
}

TEST(OSPlatformTests, WellKnownValues_AreDistinct) {
    EXPECT_NE(OSPlatform::Windows, OSPlatform::Linux);
    EXPECT_NE(OSPlatform::Linux, OSPlatform::OSX);
    EXPECT_NE(OSPlatform::OSX, OSPlatform::FreeBSD);
}

TEST(OSPlatformTests, Create_CaseInsensitiveEquality) {
    auto a = OSPlatform::Create("Linux");
    EXPECT_EQ(a, OSPlatform::Linux);
}

TEST(OSPlatformTests, Create_Empty_Throws) {
    EXPECT_THROW(OSPlatform::Create(""), System::ArgumentException);
}

TEST(OSPlatformTests, ToString_ReturnsName) {
    EXPECT_EQ(OSPlatform::Linux.ToString(), "LINUX");
}

// GetHashCode() must satisfy the Equals/GetHashCode contract: values that compare Equals()
// (case-insensitively) must hash equal.
TEST(OSPlatformTests, GetHashCode_MatchesForCaseInsensitivelyEqualValues) {
    auto a = OSPlatform::Create("Linux");
    EXPECT_EQ(a.GetHashCode(), OSPlatform::Linux.GetHashCode());
}

TEST(OSPlatformTests, GetHashCode_DiffersForDistinctValues) {
    EXPECT_NE(OSPlatform::Linux.GetHashCode(), OSPlatform::Windows.GetHashCode());
}

TEST(RuntimeInformationTests, IsOSPlatform_MatchesLinuxOnThisSandbox) {
    EXPECT_TRUE(RuntimeInformation::IsOSPlatform(OSPlatform::Linux));
    EXPECT_FALSE(RuntimeInformation::IsOSPlatform(OSPlatform::Windows));
}

TEST(RuntimeInformationTests, OSDescription_IsNonEmpty) {
    EXPECT_FALSE(RuntimeInformation::getOSDescriptionProperty().empty());
}

TEST(RuntimeInformationTests, ProcessArchitecture_MatchesOSArchitecture) {
    EXPECT_EQ(RuntimeInformation::getProcessArchitectureProperty(), RuntimeInformation::getOSArchitectureProperty());
}

// OSArchitecture queries the real kernel architecture via uname() (matching real .NET's
// Interop.Sys.GetOSArchitecture()), not just an alias for ProcessArchitecture -- on this x86_64
// Linux sandbox it must resolve to X64 specifically, not merely "equal to whatever
// ProcessArchitecture happens to return" (which would pass vacuously if both were wrong the same way).
TEST(RuntimeInformationTests, OSArchitecture_IsX64OnThisSandbox) {
    EXPECT_EQ(RuntimeInformation::getOSArchitectureProperty(), Architecture::X64);
}
