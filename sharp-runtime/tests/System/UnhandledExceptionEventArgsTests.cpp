// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UnhandledExceptionEventArgs.hpp"

using System::UnhandledExceptionEventArgs;

TEST(UnhandledExceptionEventArgsTest, CtorWithNullException) {
    UnhandledExceptionEventArgs args(nullptr, false);
    EXPECT_EQ(args.getExceptionObjectProperty(), nullptr);
    EXPECT_FALSE(args.getIsTerminatingProperty());
}

TEST(UnhandledExceptionEventArgsTest, CtorWithException) {
    auto ex = std::make_exception_ptr(std::runtime_error("boom"));
    UnhandledExceptionEventArgs args(ex, false);
    EXPECT_NE(args.getExceptionObjectProperty(), nullptr);
}

TEST(UnhandledExceptionEventArgsTest, IsTerminatingTrue) {
    UnhandledExceptionEventArgs args(nullptr, true);
    EXPECT_TRUE(args.getIsTerminatingProperty());
}

TEST(UnhandledExceptionEventArgsTest, IsTerminatingFalse) {
    UnhandledExceptionEventArgs args(nullptr, false);
    EXPECT_FALSE(args.getIsTerminatingProperty());
}

TEST(UnhandledExceptionEventArgsTest, ExceptionRethrowable) {
    auto ex = std::make_exception_ptr(std::runtime_error("boom"));
    UnhandledExceptionEventArgs args(ex, true);
    EXPECT_THROW(std::rethrow_exception(args.getExceptionObjectProperty()), std::runtime_error);
}

TEST(UnhandledExceptionEventArgsTest, IsEventArgs) {
    UnhandledExceptionEventArgs args(nullptr, false);
    EXPECT_NO_THROW({ System::EventArgs& ref = args; (void)ref; });
}
