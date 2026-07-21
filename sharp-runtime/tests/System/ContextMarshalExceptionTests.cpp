// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ContextMarshalException.hpp"

using System::ContextMarshalException;
using System::SystemException;

TEST(ContextMarshalExceptionTests2, DefaultCtor_HasMessage) {
    ContextMarshalException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(ContextMarshalExceptionTests2, DefaultCtor_MessageMatchesDotNet) {
    ContextMarshalException ex;
    EXPECT_EQ(std::string(ex.what()), "Attempted to marshal an object across a context boundary.");
}

TEST(ContextMarshalExceptionTests2, MessageCtor_StoresMessage) {
    ContextMarshalException ex("bad marshal");
    EXPECT_EQ(std::string(ex.what()), "bad marshal");
}

TEST(ContextMarshalExceptionTests2, IsA_SystemException) {
    EXPECT_THROW(throw ContextMarshalException(), SystemException);
}

TEST(ContextMarshalExceptionTests2, InnerExceptionCtor_StoresMessage) {
    auto inner = std::make_exception_ptr(std::runtime_error("root cause"));
    ContextMarshalException ex("outer", inner);
    EXPECT_EQ(std::string(ex.what()), "outer");
}
