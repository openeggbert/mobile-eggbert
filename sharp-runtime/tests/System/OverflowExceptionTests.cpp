// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/OverflowException.hpp"

using System::OverflowException;

TEST(OverflowExceptionTest, DefaultCtor) {
    OverflowException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(OverflowExceptionTest, CStringCtor) {
    OverflowException e("overflow");
    EXPECT_NE(std::string(e.what()).find("overflow"), std::string::npos);
}

TEST(OverflowExceptionTest, StringCtor) {
    OverflowException e(std::string("overflow"));
    EXPECT_NE(std::string(e.what()).find("overflow"), std::string::npos);
}

TEST(OverflowExceptionTest, InnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    OverflowException e("overflow", inner);
    EXPECT_NE(std::string(e.what()).find("overflow"), std::string::npos);
}

TEST(OverflowExceptionTest, IsArithmeticException) {
    OverflowException e;
    EXPECT_NO_THROW({ System::ArithmeticException& ref = e; (void)ref; });
}
