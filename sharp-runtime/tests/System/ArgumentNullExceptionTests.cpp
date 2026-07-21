// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentException.hpp"

using System::ArgumentNullException;

TEST(ArgumentNullExceptionTests, DefaultCtor_WhatNotEmpty) {
    ArgumentNullException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

TEST(ArgumentNullExceptionTests, IsA_ArgumentException) {
    EXPECT_THROW(throw ArgumentNullException(), System::ArgumentException);
}

TEST(ArgumentNullExceptionTests, HResult_MatchesEPointer) {
    // Regression: previously inherited ArgumentException's HResult; .NET's real value
    // (ArgumentNullException.cs) is E_POINTER, not COR_E_ARGUMENT -- "Use E_POINTER,
    // COM used that for null pointers."
    ArgumentNullException ex;
    EXPECT_EQ(ex.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x80004003));
}

TEST(ArgumentNullExceptionTests, ParamNameCtor_StoresParamName) {
    ArgumentNullException ex("myParam");
    EXPECT_EQ(ex.getParamNameProperty(), "myParam");
}

TEST(ArgumentNullExceptionTests, ParamNameCtor_WhatContainsParamName) {
    ArgumentNullException ex("myParam");
    EXPECT_NE(std::string(ex.what()).find("myParam"), std::string::npos);
}

TEST(ArgumentNullExceptionTests, ParamNameAndMessage_ParamNameStored) {
    ArgumentNullException ex("myParam", "custom message");
    EXPECT_EQ(ex.getParamNameProperty(), "myParam");
}

TEST(ArgumentNullExceptionTests, ParamNameAndMessage_WhatContainsMessage) {
    ArgumentNullException ex("myParam", "custom message");
    EXPECT_NE(std::string(ex.what()).find("custom message"), std::string::npos);
}

TEST(ArgumentNullExceptionTests, MessageAndInner_WhatContainsMessage) {
    auto inner = std::make_exception_ptr(std::runtime_error("root cause"));
    ArgumentNullException ex("something was null", inner);
    EXPECT_NE(std::string(ex.what()).find("something was null"), std::string::npos);
}

TEST(ArgumentNullExceptionTests, ThrowIfNull_NullThrows) {
    int* p = nullptr;
    EXPECT_THROW(ArgumentNullException::ThrowIfNull(p, "p"), ArgumentNullException);
}

TEST(ArgumentNullExceptionTests, ThrowIfNull_NonNullNoThrow) {
    int x = 42;
    EXPECT_NO_THROW(ArgumentNullException::ThrowIfNull(&x, "x"));
}

TEST(ArgumentNullExceptionTests, ThrowIfNull_ParamNameInException) {
    int* p = nullptr;
    try {
        ArgumentNullException::ThrowIfNull(p, "myPtr");
        FAIL();
    } catch (const ArgumentNullException& e) {
        EXPECT_EQ(e.getParamNameProperty(), "myPtr");
    }
}
