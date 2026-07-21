// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/CannotUnloadAppDomainException.hpp"

using System::CannotUnloadAppDomainException;
using System::SystemException;

TEST(CannotUnloadAppDomainExceptionTests, DefaultCtor_HasMessage) {
    CannotUnloadAppDomainException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(CannotUnloadAppDomainExceptionTests, MessageCtor_StoresMessage) {
    CannotUnloadAppDomainException ex("domain failed");
    EXPECT_EQ(std::string(ex.what()), "domain failed");
}

TEST(CannotUnloadAppDomainExceptionTests, IsA_SystemException) {
    EXPECT_THROW(throw CannotUnloadAppDomainException(), SystemException);
}

TEST(CannotUnloadAppDomainExceptionTests, InnerExceptionCtor_StoresMessage) {
    auto inner = std::make_exception_ptr(std::runtime_error("root cause"));
    CannotUnloadAppDomainException ex("unload failed", inner);
    EXPECT_EQ(std::string(ex.what()), "unload failed");
}
