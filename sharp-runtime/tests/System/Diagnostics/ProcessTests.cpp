// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>

#include "System/Diagnostics/Process.hpp"
#include "System/InvalidOperationException.hpp"

using System::Diagnostics::Process;
using System::Diagnostics::ProcessStartInfo;

TEST(ProcessTests, Start_TrueExitsZero) {
    Process p = Process::Start("/bin/true");
    p.WaitForExit();
    EXPECT_TRUE(p.getHasExitedProperty());
    EXPECT_EQ(p.getExitCodeProperty(), 0);
}

TEST(ProcessTests, Start_FalseExitsNonZero) {
    Process p = Process::Start("/bin/false");
    p.WaitForExit();
    EXPECT_TRUE(p.getHasExitedProperty());
    EXPECT_NE(p.getExitCodeProperty(), 0);
}

TEST(ProcessTests, Start_WithArgumentsString) {
    ProcessStartInfo si("/bin/echo", "hello world");
    si.setRedirectStandardOutputProperty(true);
    Process p = Process::Start(si);
    p.WaitForExit();
    EXPECT_EQ(p.getExitCodeProperty(), 0);
    EXPECT_EQ(p.getStandardOutputTextProperty(), "hello world\n");
}

TEST(ProcessTests, Start_WithArgumentList) {
    ProcessStartInfo si("/bin/echo");
    si.getArgumentListProperty().push_back("a b");
    si.getArgumentListProperty().push_back("c");
    si.setRedirectStandardOutputProperty(true);
    Process p = Process::Start(si);
    p.WaitForExit();
    EXPECT_EQ(p.getStandardOutputTextProperty(), "a b c\n");
}

TEST(ProcessTests, RedirectStandardError) {
    ProcessStartInfo si("/bin/sh");
    si.getArgumentListProperty().push_back("-c");
    si.getArgumentListProperty().push_back("echo err-text 1>&2");
    si.setRedirectStandardErrorProperty(true);
    Process p = Process::Start(si);
    p.WaitForExit();
    EXPECT_EQ(p.getStandardErrorTextProperty(), "err-text\n");
}

TEST(ProcessTests, ExitCode_BeforeExit_Throws) {
    ProcessStartInfo si("/bin/sh");
    si.getArgumentListProperty().push_back("-c");
    si.getArgumentListProperty().push_back("sleep 5");
    Process p = Process::Start(si);
    EXPECT_THROW(p.getExitCodeProperty(), System::InvalidOperationException);
    p.Kill();
    p.WaitForExit();
}

TEST(ProcessTests, WaitForExit_Timeout_ReturnsFalseThenTrue) {
    ProcessStartInfo si("/bin/sh");
    si.getArgumentListProperty().push_back("-c");
    si.getArgumentListProperty().push_back("sleep 5");
    Process p = Process::Start(si);
    EXPECT_FALSE(p.WaitForExit(50));
    p.Kill();
    EXPECT_TRUE(p.WaitForExit(2000));
    EXPECT_NE(p.getExitCodeProperty(), 0);
}

TEST(ProcessTests, Kill_TerminatesLongRunningProcess) {
    ProcessStartInfo si("/bin/sh");
    si.getArgumentListProperty().push_back("-c");
    si.getArgumentListProperty().push_back("sleep 30");
    Process p = Process::Start(si);
    EXPECT_FALSE(p.getHasExitedProperty());
    p.Kill();
    EXPECT_TRUE(p.WaitForExit(2000));
}

TEST(ProcessTests, GetCurrentProcess_HasValidId) {
    Process p = Process::GetCurrentProcess();
    EXPECT_GT(p.getIdProperty(), 0);
    EXPECT_FALSE(p.getHasExitedProperty());
}

TEST(ProcessTests, WorkingDirectory_IsRespected) {
    ProcessStartInfo si("/bin/pwd");
    si.setWorkingDirectoryProperty("/tmp");
    si.setRedirectStandardOutputProperty(true);
    Process p = Process::Start(si);
    p.WaitForExit();
    EXPECT_EQ(p.getStandardOutputTextProperty(), "/tmp\n");
}

TEST(ProcessTests, StandardOutputText_WithoutRedirect_Throws) {
    Process p = Process::Start("/bin/true");
    p.WaitForExit();
    EXPECT_THROW(p.getStandardOutputTextProperty(), System::InvalidOperationException);
}
