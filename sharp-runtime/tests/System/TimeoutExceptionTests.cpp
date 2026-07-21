// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/TimeoutException.hpp"

using System::TimeoutException;

TEST(TimeoutExceptionTest, DefaultCtor) {
    TimeoutException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(TimeoutExceptionTest, DefaultCtorMessageContainsTimeout) {
    TimeoutException e;
    std::string msg = e.what();
    EXPECT_NE(msg.find("timed out"), std::string::npos);
}

TEST(TimeoutExceptionTest, CStringCtor) {
    TimeoutException e("custom timeout");
    EXPECT_NE(std::string(e.what()).find("custom timeout"), std::string::npos);
}

TEST(TimeoutExceptionTest, StringCtor) {
    TimeoutException e(std::string("op timed out"));
    EXPECT_NE(std::string(e.what()).find("op timed out"), std::string::npos);
}

TEST(TimeoutExceptionTest, InnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    TimeoutException e("timeout with inner", inner);
    EXPECT_NE(std::string(e.what()).find("timeout with inner"), std::string::npos);
}

TEST(TimeoutExceptionTest, IsSystemException) {
    TimeoutException e;
    EXPECT_NO_THROW({ System::SystemException& ref = e; (void)ref; });
}

TEST(TimeoutExceptionTest, IsException) {
    TimeoutException e;
    EXPECT_NO_THROW({ System::Exception& ref = e; (void)ref; });
}

TEST(TimeoutExceptionTest, Throwable) {
    EXPECT_THROW({ throw TimeoutException("timeout"); }, TimeoutException);
}

TEST(TimeoutExceptionTest, CaughtAsStdException) {
    bool caught = false;
    try { throw TimeoutException("timeout"); }
    catch (const std::exception&) { caught = true; }
    EXPECT_TRUE(caught);
}

TEST(TimeoutExceptionTest, HResult_IsCorTimeout) {
    TimeoutException e;
    EXPECT_EQ(e.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80131505));
}
