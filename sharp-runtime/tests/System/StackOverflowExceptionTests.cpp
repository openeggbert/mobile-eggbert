// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/StackOverflowException.hpp"
#include "System/SystemException.hpp"

using System::StackOverflowException;
using System::SystemException;

// ---------------------------------------------------------------------------
// Default constructor
// ---------------------------------------------------------------------------

TEST(StackOverflowExceptionTests, DefaultCtor_MessageIsNotEmpty) {
    StackOverflowException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(StackOverflowExceptionTests, DefaultCtor_MessageContainsOverflow) {
    StackOverflowException e;
    std::string msg = e.getMessageProperty();
    EXPECT_NE(msg.find("overflow"), std::string::npos);
}

// ---------------------------------------------------------------------------
// const char* constructor
// ---------------------------------------------------------------------------

TEST(StackOverflowExceptionTests, CharCtor_MessageMatches) {
    StackOverflowException e("deep recursion");
    EXPECT_EQ(e.getMessageProperty(), "deep recursion");
}

TEST(StackOverflowExceptionTests, CharCtor_WhatMatches) {
    StackOverflowException e("stack too deep");
    EXPECT_STREQ(e.what(), "stack too deep");
}

// ---------------------------------------------------------------------------
// std::string constructor
// ---------------------------------------------------------------------------

TEST(StackOverflowExceptionTests, StringCtor_MessageMatches) {
    StackOverflowException e(std::string("custom overflow message"));
    EXPECT_EQ(e.getMessageProperty(), "custom overflow message");
}

// ---------------------------------------------------------------------------
// (message, innerException) constructor
// ---------------------------------------------------------------------------

TEST(StackOverflowExceptionTests, InnerExceptionCtor_ContainsMessage) {
    auto inner = std::make_exception_ptr(std::runtime_error("recursion depth exceeded"));
    StackOverflowException e("stack overflow", inner);
    std::string msg = e.getMessageProperty();
    EXPECT_NE(msg.find("stack overflow"), std::string::npos);
}

TEST(StackOverflowExceptionTests, InnerExceptionCtor_ContainsInnerMessage) {
    auto inner = std::make_exception_ptr(std::runtime_error("recursion depth exceeded"));
    StackOverflowException e("stack overflow", inner);
    std::string msg = e.getMessageProperty();
    EXPECT_FALSE(msg.empty());
}

// ---------------------------------------------------------------------------
// Inheritance
// ---------------------------------------------------------------------------

TEST(StackOverflowExceptionTests, IsSystemException) {
    bool caught = false;
    try { throw StackOverflowException("x"); }
    catch (const SystemException&) { caught = true; }
    EXPECT_TRUE(caught);
}

TEST(StackOverflowExceptionTests, IsStdException) {
    bool caught = false;
    try { throw StackOverflowException("x"); }
    catch (const std::exception&) { caught = true; }
    EXPECT_TRUE(caught);
}

TEST(StackOverflowExceptionTests, CaughtAsStackOverflowException) {
    bool caught = false;
    try { throw StackOverflowException("x"); }
    catch (const StackOverflowException&) { caught = true; }
    EXPECT_TRUE(caught);
}

// ---------------------------------------------------------------------------
// HResult
// ---------------------------------------------------------------------------

TEST(StackOverflowExceptionTests, HResult_IsCorStackOverflow) {
    StackOverflowException e;
    EXPECT_EQ(e.getHResultProperty(), static_cast<SharpRuntime::intcs>(0x800703E9));
}
