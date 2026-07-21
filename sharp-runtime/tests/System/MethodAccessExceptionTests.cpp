// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/MethodAccessException.hpp"

using System::MethodAccessException;

TEST(MethodAccessExceptionTest, DefaultCtor) {
    MethodAccessException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(MethodAccessExceptionTest, MessageCtor) {
    MethodAccessException e("access denied");
    EXPECT_NE(std::string(e.what()).find("access denied"), std::string::npos);
}

TEST(MethodAccessExceptionTest, InnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    MethodAccessException e("method inaccessible", inner);
    EXPECT_NE(std::string(e.what()).find("method inaccessible"), std::string::npos);
}

TEST(MethodAccessExceptionTest, IsMemberAccessException) {
    MethodAccessException e("test");
    EXPECT_NO_THROW({ System::MemberAccessException& ref = e; (void)ref; });
}

TEST(MethodAccessExceptionTest, HResult_MatchesCorEMethodAccess_NotBaseMemberAccess) {
    // MethodAccessException overrides its base MemberAccessException's HResult
    // (COR_E_MEMBERACCESS, 0x8013151A) with its own (COR_E_METHODACCESS, 0x80131510).
    constexpr SharpRuntime::intcs corEMethodAccess = static_cast<SharpRuntime::intcs>(0x80131510);
    MethodAccessException defaultCtor;
    MethodAccessException messageCtor("access denied");
    MethodAccessException innerCtor("method inaccessible", std::make_exception_ptr(std::runtime_error("cause")));
    EXPECT_EQ(defaultCtor.getHResultProperty(), corEMethodAccess);
    EXPECT_EQ(messageCtor.getHResultProperty(), corEMethodAccess);
    EXPECT_EQ(innerCtor.getHResultProperty(), corEMethodAccess);
}
