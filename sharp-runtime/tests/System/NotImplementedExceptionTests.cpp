// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/NotImplementedException.hpp"

using System::NotImplementedException;

TEST(NotImplementedExceptionTest, DefaultCtor) {
    NotImplementedException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(NotImplementedExceptionTest, CStringCtor) {
    NotImplementedException e("not done yet");
    EXPECT_NE(std::string(e.what()).find("not done yet"), std::string::npos);
}

TEST(NotImplementedExceptionTest, StringCtor) {
    NotImplementedException e(std::string("not done yet"));
    EXPECT_NE(std::string(e.what()).find("not done yet"), std::string::npos);
}

TEST(NotImplementedExceptionTest, InnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    NotImplementedException e("not implemented", inner);
    EXPECT_NE(std::string(e.what()).find("not implemented"), std::string::npos);
}

TEST(NotImplementedExceptionTest, IsSystemException) {
    NotImplementedException e;
    EXPECT_NO_THROW({ System::SystemException& ref = e; (void)ref; });
}
