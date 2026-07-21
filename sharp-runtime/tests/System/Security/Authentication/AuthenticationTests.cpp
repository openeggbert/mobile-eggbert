// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Security/Authentication/AuthenticationException.hpp"
#include "System/Security/Authentication/InvalidCredentialException.hpp"
#include "System/Security/Authentication/SslProtocols.hpp"

using namespace System::Security::Authentication;

TEST(SslProtocolsTests, Values) {
    EXPECT_EQ(static_cast<int>(SslProtocols::None), 0);
    EXPECT_EQ(static_cast<int>(SslProtocols::Tls12), 0xC00);
    EXPECT_EQ(static_cast<int>(SslProtocols::Tls13), 0x3000);
}

TEST(SslProtocolsTests, BitwiseOperators) {
    auto combined = SslProtocols::Tls12 | SslProtocols::Tls13;
    EXPECT_EQ(combined & SslProtocols::Tls12, SslProtocols::Tls12);
    EXPECT_EQ(combined & SslProtocols::Tls13, SslProtocols::Tls13);
    EXPECT_EQ(combined & SslProtocols::Tls11, SslProtocols::None);
}

TEST(AuthenticationExceptionTests, DefaultConstructor) {
    EXPECT_NO_THROW(AuthenticationException());
}

TEST(AuthenticationExceptionTests, CustomMessage) {
    AuthenticationException ex("auth failed");
    EXPECT_STREQ(ex.what(), "auth failed");
}

TEST(AuthenticationExceptionTests, IsThrowableAndCatchableAsSystemException) {
    EXPECT_THROW(
        { try { throw AuthenticationException("x"); }
          catch (const System::SystemException&) { throw; } },
        AuthenticationException);
}

TEST(InvalidCredentialExceptionTests, IsAuthenticationException) {
    InvalidCredentialException ex("bad creds");
    const AuthenticationException& base = ex;
    EXPECT_STREQ(base.what(), "bad creds");
}

TEST(InvalidCredentialExceptionTests, CatchableAsAuthenticationException) {
    EXPECT_THROW(
        { try { throw InvalidCredentialException("bad creds"); }
          catch (const AuthenticationException&) { throw; } },
        InvalidCredentialException);
}
