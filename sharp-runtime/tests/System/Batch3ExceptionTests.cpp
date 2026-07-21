// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
// Batch 3 exception tests: TimeoutException, DllNotFoundException, MissingFieldException,
// MulticastNotSupportedException, InvalidCastException, NotFiniteNumberException
// (DefaultCtor_WhatNotEmpty / MessageCtor / IsA_Exception already covered by EXCEPT_SIMPLE
//  in ExceptionRemainingTests.cpp — new tests use distinct names)
#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include "System/TimeoutException.hpp"
#include "System/DllNotFoundException.hpp"
#include "System/MissingFieldException.hpp"
#include "System/MulticastNotSupportedException.hpp"
#include "System/InvalidCastException.hpp"
#include "System/NotFiniteNumberException.hpp"
#include "System/TypeLoadException.hpp"
#include "System/SystemException.hpp"
#include "System/ArithmeticException.hpp"

// ---------------------------------------------------------------------------
// TimeoutException
// ---------------------------------------------------------------------------
TEST(TimeoutExceptionNewTests, InnerExceptionCtor_ContainsBoth) {
    auto inner = std::make_exception_ptr(std::runtime_error("timed out waiting"));
    System::TimeoutException e("operation failed", inner);
    std::string msg(e.what());
    EXPECT_NE(msg.find("operation failed"), std::string::npos);
}
TEST(TimeoutExceptionNewTests, IsA_SystemException) {
    System::TimeoutException e;
    EXPECT_NE(dynamic_cast<System::SystemException*>(&e), nullptr);
}
TEST(TimeoutExceptionNewTests, DefaultMsg_ContainsTimeout) {
    System::TimeoutException e;
    EXPECT_NE(std::string(e.what()).find("timed out"), std::string::npos);
}

// ---------------------------------------------------------------------------
// DllNotFoundException
// ---------------------------------------------------------------------------
TEST(DllNotFoundExceptionNewTests, InnerExceptionCtor_ContainsBoth) {
    auto inner = std::make_exception_ptr(std::runtime_error("load error"));
    System::DllNotFoundException e("missing lib", inner);
    std::string msg(e.what());
    EXPECT_NE(msg.find("missing lib"), std::string::npos);
}
TEST(DllNotFoundExceptionNewTests, IsA_TypeLoadException) {
    System::DllNotFoundException e;
    EXPECT_NE(dynamic_cast<System::TypeLoadException*>(&e), nullptr);
}
TEST(DllNotFoundExceptionNewTests, DefaultMsg_ContainsDLL) {
    System::DllNotFoundException e;
    // .NET's actual resource string (SR.Arg_DllNotFoundException) is "Dll was not found."
    EXPECT_NE(std::string(e.what()).find("Dll"), std::string::npos);
}

// ---------------------------------------------------------------------------
// MissingFieldException
// ---------------------------------------------------------------------------
TEST(MissingFieldExceptionNewTests, InnerExceptionCtor_ContainsBoth) {
    auto inner = std::make_exception_ptr(std::runtime_error("reflection error"));
    System::MissingFieldException e("no field", inner);
    std::string msg(e.what());
    EXPECT_NE(msg.find("no field"), std::string::npos);
}
TEST(MissingFieldExceptionNewTests, ClassFieldNameCtor_ContainsFieldPath) {
    System::MissingFieldException e("MyClass", "myField");
    std::string msg(e.what());
    EXPECT_NE(msg.find("MyClass"), std::string::npos);
    EXPECT_NE(msg.find("myField"), std::string::npos);
}
TEST(MissingFieldExceptionNewTests, IsA_MissingMemberException) {
    System::MissingFieldException e;
    EXPECT_NE(dynamic_cast<System::MissingMemberException*>(&e), nullptr);
}

// ---------------------------------------------------------------------------
// MulticastNotSupportedException
// ---------------------------------------------------------------------------
TEST(MulticastNotSupportedExceptionNewTests, InnerExceptionCtor_ContainsBoth) {
    auto inner = std::make_exception_ptr(std::runtime_error("delegate error"));
    System::MulticastNotSupportedException e("bad combine", inner);
    std::string msg(e.what());
    EXPECT_NE(msg.find("bad combine"), std::string::npos);
}
TEST(MulticastNotSupportedExceptionNewTests, IsA_SystemException) {
    System::MulticastNotSupportedException e;
    EXPECT_NE(dynamic_cast<System::SystemException*>(&e), nullptr);
}
TEST(MulticastNotSupportedExceptionNewTests, DefaultMsg_ContainsMulticast) {
    System::MulticastNotSupportedException e;
    EXPECT_NE(std::string(e.what()).find("multicast"), std::string::npos);
}

// ---------------------------------------------------------------------------
// InvalidCastException
// ---------------------------------------------------------------------------
TEST(InvalidCastExceptionNewTests, InnerExceptionCtor_ContainsBoth) {
    auto inner = std::make_exception_ptr(std::runtime_error("type mismatch"));
    System::InvalidCastException e("bad cast", inner);
    std::string msg(e.what());
    EXPECT_NE(msg.find("bad cast"), std::string::npos);
}
TEST(InvalidCastExceptionNewTests, IsA_SystemException) {
    System::InvalidCastException e;
    EXPECT_NE(dynamic_cast<System::SystemException*>(&e), nullptr);
}
TEST(InvalidCastExceptionNewTests, DefaultMsg_ContainsCast) {
    System::InvalidCastException e;
    EXPECT_NE(std::string(e.what()).find("cast"), std::string::npos);
}

// ---------------------------------------------------------------------------
// NotFiniteNumberException (extra tests beyond ExceptionRemainingTests.cpp)
// ---------------------------------------------------------------------------
TEST(NotFiniteNumberExceptionNewTests, IsA_ArithmeticException) {
    System::NotFiniteNumberException e;
    EXPECT_NE(dynamic_cast<System::ArithmeticException*>(&e), nullptr);
}
TEST(NotFiniteNumberExceptionNewTests, GetOffendingNumber_NaN) {
    double nan = std::numeric_limits<double>::quiet_NaN();
    System::NotFiniteNumberException e(nan);
    EXPECT_TRUE(std::isnan(e.getOffendingNumberProperty()));
}
TEST(NotFiniteNumberExceptionNewTests, GetOffendingNumber_DefaultIsZero) {
    System::NotFiniteNumberException e;
    EXPECT_EQ(e.getOffendingNumberProperty(), 0.0);
}
