// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/TimeZoneNotFoundException.hpp"

using System::TimeZoneNotFoundException;

TEST(TimeZoneNotFoundExceptionTest, DefaultCtor) {
    TimeZoneNotFoundException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(TimeZoneNotFoundExceptionTest, DefaultCtorHasMessage) {
    TimeZoneNotFoundException e;
    std::string msg = e.what();
    EXPECT_NE(msg.find("time zone"), std::string::npos);
}

TEST(TimeZoneNotFoundExceptionTest, StringCtor) {
    TimeZoneNotFoundException e("Zone 'Foo' not found");
    EXPECT_NE(std::string(e.what()).find("Foo"), std::string::npos);
}

TEST(TimeZoneNotFoundExceptionTest, InnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("io error"));
    TimeZoneNotFoundException e("zone missing", inner);
    EXPECT_NE(std::string(e.what()).find("zone missing"), std::string::npos);
}

TEST(TimeZoneNotFoundExceptionTest, IsException) {
    TimeZoneNotFoundException e;
    EXPECT_NO_THROW({ System::Exception& ref = e; (void)ref; });
}

TEST(TimeZoneNotFoundExceptionTest, Throwable) {
    EXPECT_THROW({ throw TimeZoneNotFoundException("tz"); }, TimeZoneNotFoundException);
}

TEST(TimeZoneNotFoundExceptionTest, CaughtAsStdException) {
    bool caught = false;
    try { throw TimeZoneNotFoundException("tz"); }
    catch (const std::exception&) { caught = true; }
    EXPECT_TRUE(caught);
}
