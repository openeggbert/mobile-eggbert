// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/MissingMethodException.hpp"
#include "System/MissingMemberException.hpp"

using System::MissingMethodException;

TEST(MissingMethodExceptionTests, DefaultCtor_WhatNotEmpty_New) {
    MissingMethodException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(MissingMethodExceptionTests, DefaultCtor_WhatContainsMethod) {
    MissingMethodException e;
    EXPECT_NE(std::string(e.what()).find("method"), std::string::npos);
}

TEST(MissingMethodExceptionTests, DefaultCtor_IsAMissingMemberException) {
    MissingMethodException e;
    EXPECT_NE(dynamic_cast<System::MissingMemberException*>(&e), nullptr);
}

TEST(MissingMethodExceptionTests, StringCtor_MessageStored) {
    MissingMethodException e("no such method");
    EXPECT_EQ(std::string(e.what()), "no such method");
}

TEST(MissingMethodExceptionTests, ClassMethodCtor_ContainsClassName) {
    MissingMethodException e("MyClass", "MyMethod");
    EXPECT_NE(std::string(e.what()).find("MyClass"), std::string::npos);
}

TEST(MissingMethodExceptionTests, ClassMethodCtor_ContainsMethodName) {
    MissingMethodException e("MyClass", "MyMethod");
    EXPECT_NE(std::string(e.what()).find("MyMethod"), std::string::npos);
}

TEST(MissingMethodExceptionTests, ClassMethodCtor_ContainsDot) {
    MissingMethodException e("A", "B");
    EXPECT_NE(std::string(e.what()).find("A.B"), std::string::npos);
}

TEST(MissingMethodExceptionTests, InnerExceptionCtor_ContainsBothMessages) {
    auto inner = std::make_exception_ptr(std::runtime_error("root cause"));
    MissingMethodException e("method gone", inner);
    std::string msg(e.what());
    EXPECT_NE(msg.find("method gone"), std::string::npos);
}

TEST(MissingMethodExceptionTests, Catchable_AsStdException) {
    EXPECT_THROW({ throw MissingMethodException(); }, std::exception);
}

TEST(MissingMethodExceptionTests, Catchable_AsMissingMemberException) {
    EXPECT_THROW({ throw MissingMethodException(); }, System::MissingMemberException);
}

TEST(MissingMethodExceptionTests, ClassMethodCtor_MatchesDotNetMessageFormat) {
    MissingMethodException e("MyClass", "MyMethod");
    EXPECT_EQ(std::string(e.what()), "Method 'MyClass.MyMethod' not found.");
}

TEST(MissingMethodExceptionTests, HResult_MatchesCorEMissingMethod_NotBaseMissingMember) {
    // MissingMethodException overrides its base MissingMemberException's HResult
    // (COR_E_MISSINGMEMBER, 0x80131512) with its own (COR_E_MISSINGMETHOD, 0x80131513).
    constexpr SharpRuntime::intcs corE = static_cast<SharpRuntime::intcs>(0x80131513);
    MissingMethodException defaultCtor;
    MissingMethodException messageCtor("method missing");
    MissingMethodException classMethodCtor("MyClass", "MyMethod");
    MissingMethodException innerCtor("method not found", std::make_exception_ptr(std::runtime_error("cause")));
    EXPECT_EQ(defaultCtor.getHResultProperty(), corE);
    EXPECT_EQ(messageCtor.getHResultProperty(), corE);
    EXPECT_EQ(classMethodCtor.getHResultProperty(), corE);
    EXPECT_EQ(innerCtor.getHResultProperty(), corE);
}
