// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/SystemException.hpp"

using System::SystemException;

TEST(SystemExceptionTest, DefaultCtor) {
    SystemException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(SystemExceptionTest, CStringCtor) {
    SystemException e("system error");
    EXPECT_NE(std::string(e.what()).find("system error"), std::string::npos);
}

TEST(SystemExceptionTest, StringCtor) {
    SystemException e(std::string("system error 2"));
    EXPECT_NE(std::string(e.what()).find("system error 2"), std::string::npos);
}

TEST(SystemExceptionTest, InnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    SystemException e("outer", inner);
    EXPECT_NE(std::string(e.what()).find("outer"), std::string::npos);
}

TEST(SystemExceptionTest, IsException) {
    SystemException e;
    EXPECT_NO_THROW({ System::Exception& ref = e; (void)ref; });
}

TEST(SystemExceptionTest, Throwable) {
    EXPECT_THROW({ throw SystemException("test"); }, SystemException);
}

TEST(SystemExceptionTest, CaughtAsStdException) {
    bool caught = false;
    try { throw SystemException("test"); }
    catch (const std::exception&) { caught = true; }
    EXPECT_TRUE(caught);
}

TEST(SystemExceptionTest, HResult_IsCorSystem) {
    SystemException e;
    EXPECT_EQ(e.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80131501));
}
