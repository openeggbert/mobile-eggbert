// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/CrashReason.hpp"

using System::CrashReason;

TEST(CrashReasonTests, Unknown_IsZero) {
    EXPECT_EQ(static_cast<int>(CrashReason::Unknown), 0);
}

TEST(CrashReasonTests, UnhandledException_IsOne) {
    EXPECT_EQ(static_cast<int>(CrashReason::UnhandledException), 1);
}

TEST(CrashReasonTests, EnvironmentFailFast_IsTwo) {
    EXPECT_EQ(static_cast<int>(CrashReason::EnvironmentFailFast), 2);
}

TEST(CrashReasonTests, InternalFailFast_IsThree) {
    EXPECT_EQ(static_cast<int>(CrashReason::InternalFailFast), 3);
}

TEST(CrashReasonTests, ValuesAreDistinct) {
    EXPECT_NE(CrashReason::Unknown, CrashReason::UnhandledException);
    EXPECT_NE(CrashReason::EnvironmentFailFast, CrashReason::InternalFailFast);
}
