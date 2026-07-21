// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/RankException.hpp"

using System::RankException;

TEST(RankExceptionTest, DefaultCtor) {
    RankException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(RankExceptionTest, MessageCtor) {
    RankException e("wrong number of dimensions");
    EXPECT_NE(std::string(e.what()).find("wrong number of dimensions"), std::string::npos);
}

TEST(RankExceptionTest, InnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    RankException e("rank error", inner);
    EXPECT_NE(std::string(e.what()).find("rank error"), std::string::npos);
}

TEST(RankExceptionTest, IsSystemException) {
    RankException e;
    EXPECT_NO_THROW({ System::SystemException& ref = e; (void)ref; });
}

TEST(RankExceptionTest, Throwable) {
    EXPECT_THROW({ throw RankException("bad rank"); }, RankException);
}

TEST(RankExceptionTest, HResult_MatchesCorERank) {
    constexpr SharpRuntime::intcs corE = static_cast<SharpRuntime::intcs>(0x80131517);
    RankException defaultCtor;
    RankException messageCtor("wrong number of dimensions");
    RankException innerCtor("rank error", std::make_exception_ptr(std::runtime_error("cause")));
    EXPECT_EQ(defaultCtor.getHResultProperty(), corE);
    EXPECT_EQ(messageCtor.getHResultProperty(), corE);
    EXPECT_EQ(innerCtor.getHResultProperty(), corE);
}
