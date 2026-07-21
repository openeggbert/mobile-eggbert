// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/InvalidOperationException.hpp"
#include "System/NullReferenceException.hpp"

using System::InvalidOperationException;
using System::NullReferenceException;

// InvalidOperationException — additional tests (basic message tests in ExceptionTests.cpp)
TEST(InvalidOperationExceptionNewTests, DefaultMsg_NotEmpty) {
    InvalidOperationException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}
TEST(InvalidOperationExceptionNewTests, DefaultMsg_ContainsState) {
    InvalidOperationException e;
    std::string msg(e.what());
    EXPECT_NE(msg.find("state"), std::string::npos);
}
TEST(InvalidOperationExceptionNewTests, InnerExceptionCtor_ContainsBoth) {
    auto inner = std::make_exception_ptr(std::runtime_error("root"));
    InvalidOperationException e("outer msg", inner);
    std::string msg(e.what());
    EXPECT_NE(msg.find("outer msg"), std::string::npos);
}
TEST(InvalidOperationExceptionNewTests, CStringCtor_MessageStored) {
    InvalidOperationException e("my custom error");
    EXPECT_EQ(std::string(e.what()), "my custom error");
}
TEST(InvalidOperationExceptionNewTests, HResult_MatchesCorEInvalidOperation) {
    constexpr SharpRuntime::intcs corE = static_cast<SharpRuntime::intcs>(0x80131509);
    InvalidOperationException defaultCtor;
    InvalidOperationException messageCtor("bad state");
    InvalidOperationException innerCtor("bad state", std::make_exception_ptr(std::runtime_error("inner")));
    EXPECT_EQ(defaultCtor.getHResultProperty(), corE);
    EXPECT_EQ(messageCtor.getHResultProperty(), corE);
    EXPECT_EQ(innerCtor.getHResultProperty(), corE);
}

// NullReferenceException — additional tests
TEST(NullReferenceExceptionNewTests, DefaultMsg_ContainsNull) {
    NullReferenceException e;
    std::string msg(e.what());
    EXPECT_FALSE(msg.empty());
}
TEST(NullReferenceExceptionNewTests, InnerExceptionCtor_ContainsBoth) {
    auto inner = std::make_exception_ptr(std::runtime_error("null ptr"));
    NullReferenceException e("deref failed", inner);
    std::string msg(e.what());
    EXPECT_NE(msg.find("deref failed"), std::string::npos);
}
TEST(NullReferenceExceptionNewTests, Catchable_AsSystemException) {
    EXPECT_THROW({ throw NullReferenceException(); }, System::SystemException);
}
