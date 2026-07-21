// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/PlatformNotSupportedException.hpp"

using System::PlatformNotSupportedException;

TEST(PlatformNotSupportedExceptionTest, DefaultCtor) {
    PlatformNotSupportedException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(PlatformNotSupportedExceptionTest, MessageCtor) {
    PlatformNotSupportedException e("not supported on this OS");
    EXPECT_NE(std::string(e.what()).find("not supported on this OS"), std::string::npos);
}

TEST(PlatformNotSupportedExceptionTest, InnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    PlatformNotSupportedException e("platform error", inner);
    EXPECT_NE(std::string(e.what()).find("platform error"), std::string::npos);
}

TEST(PlatformNotSupportedExceptionTest, IsNotSupportedException) {
    PlatformNotSupportedException e;
    EXPECT_NO_THROW({ System::NotSupportedException& ref = e; (void)ref; });
}

TEST(PlatformNotSupportedExceptionTest, Throwable) {
    EXPECT_THROW(
        { throw PlatformNotSupportedException("feature not available"); },
        PlatformNotSupportedException
    );
}

TEST(PlatformNotSupportedExceptionTest, HResult_MatchesCorEPlatformNotSupported_NotBaseNotSupported) {
    // PlatformNotSupportedException overrides its base NotSupportedException's HResult
    // (COR_E_NOTSUPPORTED, 0x80131515) with its own (COR_E_PLATFORMNOTSUPPORTED, 0x80131539).
    constexpr SharpRuntime::intcs corE = static_cast<SharpRuntime::intcs>(0x80131539);
    PlatformNotSupportedException defaultCtor;
    PlatformNotSupportedException messageCtor("not supported on this OS");
    PlatformNotSupportedException innerCtor("platform error", std::make_exception_ptr(std::runtime_error("cause")));
    EXPECT_EQ(defaultCtor.getHResultProperty(), corE);
    EXPECT_EQ(messageCtor.getHResultProperty(), corE);
    EXPECT_EQ(innerCtor.getHResultProperty(), corE);
}
