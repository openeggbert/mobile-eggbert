// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/UriFormatException.hpp"

using System::UriFormatException;

TEST(UriFormatExceptionTest, DefaultCtor) {
    UriFormatException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(UriFormatExceptionTest, DefaultCtor_MatchesFormatExceptionDefault) {
    // .NET's UriFormatException() delegates to base FormatException() with no
    // Uri-specific text - it does not mention "URI" in the default message.
    UriFormatException e;
    EXPECT_EQ(std::string(e.what()), "One of the identified items was in an invalid format.");
}

TEST(UriFormatExceptionTest, StringCtor) {
    UriFormatException e("bad uri: foo://");
    EXPECT_NE(std::string(e.what()).find("foo"), std::string::npos);
}

TEST(UriFormatExceptionTest, InnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("parse error"));
    UriFormatException e("invalid", inner);
    EXPECT_NE(std::string(e.what()).find("invalid"), std::string::npos);
}

TEST(UriFormatExceptionTest, IsFormatException) {
    UriFormatException e;
    EXPECT_NO_THROW({ System::FormatException& ref = e; (void)ref; });
}

TEST(UriFormatExceptionTest, Throwable) {
    EXPECT_THROW({ throw UriFormatException("bad"); }, UriFormatException);
}
