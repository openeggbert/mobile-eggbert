// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/BadImageFormatException.hpp"
#include "System/SystemException.hpp"

using System::BadImageFormatException;

TEST(BadImageFormatExceptionTests2, DefaultCtor_WhatNotEmpty) {
    BadImageFormatException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(BadImageFormatExceptionTests2, IsA_SystemException) {
    EXPECT_THROW(throw BadImageFormatException(), System::SystemException);
}

TEST(BadImageFormatExceptionTests2, MessageCtor_WhatContainsMessage) {
    BadImageFormatException ex("corrupt binary");
    EXPECT_NE(std::string(ex.what()).find("corrupt binary"), std::string::npos);
}

TEST(BadImageFormatExceptionTests2, FileNameCtor_StoresFileName) {
    BadImageFormatException ex("bad format", "foo.dll");
    EXPECT_EQ(ex.getFileNameProperty(), "foo.dll");
}

TEST(BadImageFormatExceptionTests2, DefaultCtor_FileNameEmpty) {
    BadImageFormatException ex;
    EXPECT_TRUE(ex.getFileNameProperty().empty());
}

TEST(BadImageFormatExceptionTests2, InnerExceptionCtor_WhatContainsMessage) {
    auto inner = std::make_exception_ptr(std::runtime_error("root"));
    BadImageFormatException ex("bad dll", inner);
    EXPECT_NE(std::string(ex.what()).find("bad dll"), std::string::npos);
}

TEST(BadImageFormatExceptionTests2, FileNameAndInnerCtor_StoresFileName) {
    auto inner = std::make_exception_ptr(std::runtime_error("root"));
    BadImageFormatException ex("bad", "bar.dll", inner);
    EXPECT_EQ(ex.getFileNameProperty(), "bar.dll");
}

TEST(BadImageFormatExceptionTests2, FusionLog_IsEmpty) {
    BadImageFormatException ex("msg", "lib.dll");
    EXPECT_TRUE(ex.getFusionLogProperty().empty());
}
