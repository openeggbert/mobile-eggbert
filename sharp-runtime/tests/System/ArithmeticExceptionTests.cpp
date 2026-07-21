// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArithmeticException.hpp"
#include "System/SystemException.hpp"

using System::ArithmeticException;

TEST(ArithmeticExceptionTests2, DefaultCtor_WhatNotEmpty) {
    ArithmeticException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(ArithmeticExceptionTests2, IsA_SystemException) {
    EXPECT_THROW(throw ArithmeticException(), System::SystemException);
}

TEST(ArithmeticExceptionTests2, MessageCtor_WhatContainsMessage) {
    ArithmeticException ex("overflow occurred");
    EXPECT_NE(std::string(ex.what()).find("overflow occurred"), std::string::npos);
}

TEST(ArithmeticExceptionTests2, CharPtrCtor_WhatContainsMessage) {
    ArithmeticException ex("divide error");
    EXPECT_NE(std::string(ex.what()).find("divide error"), std::string::npos);
}

TEST(ArithmeticExceptionTests2, InnerExceptionCtor_WhatContainsMessage) {
    auto inner = std::make_exception_ptr(std::runtime_error("root"));
    ArithmeticException ex("arithmetic fault", inner);
    EXPECT_NE(std::string(ex.what()).find("arithmetic fault"), std::string::npos);
}
