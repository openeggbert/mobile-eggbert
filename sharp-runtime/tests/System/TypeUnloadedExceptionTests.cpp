// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/TypeUnloadedException.hpp"

using System::TypeUnloadedException;

TEST(TypeUnloadedExceptionTest, DefaultCtor) {
    TypeUnloadedException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(TypeUnloadedExceptionTest, DefaultCtorHasMessage) {
    TypeUnloadedException e;
    EXPECT_NE(std::string(e.what()).find("unload"), std::string::npos);
}

TEST(TypeUnloadedExceptionTest, StringCtor) {
    TypeUnloadedException e("type Foo was unloaded");
    EXPECT_NE(std::string(e.what()).find("Foo"), std::string::npos);
}

TEST(TypeUnloadedExceptionTest, InnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    TypeUnloadedException e("unloaded", inner);
    EXPECT_NE(std::string(e.what()).find("unloaded"), std::string::npos);
}

TEST(TypeUnloadedExceptionTest, IsSystemException) {
    TypeUnloadedException e;
    EXPECT_NO_THROW({ System::SystemException& ref = e; (void)ref; });
}

TEST(TypeUnloadedExceptionTest, Throwable) {
    EXPECT_THROW({ throw TypeUnloadedException("x"); }, TypeUnloadedException);
}

TEST(TypeUnloadedExceptionTest, CaughtAsStdException) {
    bool caught = false;
    try { throw TypeUnloadedException("x"); }
    catch (const std::exception&) { caught = true; }
    EXPECT_TRUE(caught);
}

TEST(TypeUnloadedExceptionTest, HResult_IsCorTypeUnloaded) {
    TypeUnloadedException e;
    EXPECT_EQ(e.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80131013));
}
