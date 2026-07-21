// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <stdexcept>
#include "System/FormatException.hpp"
#include "System/SystemException.hpp"

TEST(FormatExceptionTests, DefaultCtor_MessageNotEmpty) {
    System::FormatException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(FormatExceptionTests, StringCtor_StoresMessage) {
    System::FormatException e("bad format");
    EXPECT_NE(std::string(e.what()).find("bad format"), std::string::npos);
}

TEST(FormatExceptionTests, InnerExceptionCtor_StoresMessage) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    System::FormatException e("format error", inner);
    EXPECT_NE(std::string(e.what()).find("format error"), std::string::npos);
}

TEST(FormatExceptionTests, IsA_SystemException) {
    System::FormatException e;
    EXPECT_NE(dynamic_cast<System::SystemException*>(&e), nullptr);
}

TEST(FormatExceptionTests, HResult_MatchesCorEFormat) {
    constexpr SharpRuntime::intcs corEFormat = static_cast<SharpRuntime::intcs>(0x80131537);
    EXPECT_EQ(System::FormatException().getHResultProperty(), corEFormat);
}
