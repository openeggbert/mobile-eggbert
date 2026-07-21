// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/SystemException.hpp"

using System::ArgumentException;

TEST(ArgumentExceptionTests, DefaultCtor_WhatNotEmpty) {
    ArgumentException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(ArgumentExceptionTests, IsA_SystemException) {
    EXPECT_THROW(throw ArgumentException(), System::SystemException);
}

TEST(ArgumentExceptionTests, HResult_MatchesCorEArgument) {
    // Regression: previously never set, so it kept the base Exception's default
    // (COR_E_EXCEPTION) instead of .NET's HResults.COR_E_ARGUMENT (ArgumentException.cs).
    ArgumentException ex;
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80070057));
}

TEST(ArgumentExceptionTests, MessageCtor_WhatContainsMessage) {
    ArgumentException ex("bad argument");
    EXPECT_NE(std::string(ex.what()).find("bad argument"), std::string::npos);
}

TEST(ArgumentExceptionTests, MessageAndParamName_WhatContainsBoth) {
    ArgumentException ex("invalid value", "myParam");
    std::string w = ex.what();
    EXPECT_NE(w.find("invalid value"), std::string::npos);
    EXPECT_NE(w.find("myParam"), std::string::npos);
}

TEST(ArgumentExceptionTests, ParamName_StoredCorrectly) {
    ArgumentException ex("msg", "theParam");
    EXPECT_EQ(ex.getParamNameProperty(), "theParam");
}

TEST(ArgumentExceptionTests, ParamName_EmptyWhenNotSet) {
    ArgumentException ex("msg");
    EXPECT_TRUE(ex.getParamNameProperty().empty());
}

TEST(ArgumentExceptionTests, InnerException_Ctor) {
    auto inner = std::make_exception_ptr(std::runtime_error("root cause"));
    ArgumentException ex("outer msg", inner);
    EXPECT_NE(std::string(ex.what()).find("outer msg"), std::string::npos);
}

TEST(ArgumentExceptionTests, ThrowIfNullOrEmpty_EmptyThrows) {
    EXPECT_THROW(ArgumentException::ThrowIfNullOrEmpty("", "arg"), ArgumentException);
}

TEST(ArgumentExceptionTests, ThrowIfNullOrEmpty_NonEmptyNoThrow) {
    EXPECT_NO_THROW(ArgumentException::ThrowIfNullOrEmpty("hello", "arg"));
}

TEST(ArgumentExceptionTests, ThrowIfNullOrWhiteSpace_WhitespaceThrows) {
    EXPECT_THROW(ArgumentException::ThrowIfNullOrWhiteSpace("   ", "arg"), ArgumentException);
}

TEST(ArgumentExceptionTests, ThrowIfNullOrWhiteSpace_NonWhitespaceNoThrow) {
    EXPECT_NO_THROW(ArgumentException::ThrowIfNullOrWhiteSpace("hello", "arg"));
}

TEST(ArgumentExceptionTests, ThrowIfNullOrEmpty_ParamNameInException) {
    try {
        ArgumentException::ThrowIfNullOrEmpty("", "myArg");
        FAIL();
    } catch (const ArgumentException& e) {
        EXPECT_EQ(e.getParamNameProperty(), "myArg");
    }
}
