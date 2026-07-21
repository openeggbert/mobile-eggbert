// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/TypeAccessException.hpp"

using System::TypeAccessException;

TEST(TypeAccessExceptionTest, DefaultCtor) {
    TypeAccessException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(TypeAccessExceptionTest, DefaultCtorHasMessage) {
    TypeAccessException e;
    EXPECT_NE(std::string(e.what()).find("access"), std::string::npos);
}

TEST(TypeAccessExceptionTest, StringCtor) {
    TypeAccessException e("type X is not accessible");
    EXPECT_NE(std::string(e.what()).find("type X"), std::string::npos);
}

TEST(TypeAccessExceptionTest, InnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    TypeAccessException e("access denied", inner);
    EXPECT_NE(std::string(e.what()).find("access denied"), std::string::npos);
}

TEST(TypeAccessExceptionTest, IsTypeLoadException) {
    TypeAccessException e;
    EXPECT_NO_THROW({ System::TypeLoadException& ref = e; (void)ref; });
}

TEST(TypeAccessExceptionTest, IsException) {
    TypeAccessException e;
    EXPECT_NO_THROW({ System::Exception& ref = e; (void)ref; });
}

TEST(TypeAccessExceptionTest, Throwable) {
    EXPECT_THROW({ throw TypeAccessException("no access"); }, TypeAccessException);
}

TEST(TypeAccessExceptionTest, CaughtAsStdException) {
    bool caught = false;
    try { throw TypeAccessException("no access"); }
    catch (const std::exception&) { caught = true; }
    EXPECT_TRUE(caught);
}

TEST(TypeAccessExceptionTest, HResult_IsCorTypeAccess) {
    TypeAccessException e;
    EXPECT_EQ(e.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80131543));
}
