// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/NotSupportedException.hpp"
#include "System/SystemException.hpp"

using System::NotSupportedException;
using System::SystemException;

// ---------------------------------------------------------------------------
// Default constructor
// ---------------------------------------------------------------------------

TEST(NotSupportedExceptionTests, DefaultCtor_MessageIsNotEmpty) {
    NotSupportedException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(NotSupportedExceptionTests, DefaultCtor_MessageContainsSupported) {
    NotSupportedException e;
    std::string msg = e.getMessageProperty();
    EXPECT_NE(msg.find("supported"), std::string::npos);
}

// ---------------------------------------------------------------------------
// const char* constructor
// ---------------------------------------------------------------------------

TEST(NotSupportedExceptionTests, CharCtor_MessageMatches) {
    NotSupportedException e("read not supported");
    EXPECT_EQ(e.getMessageProperty(), "read not supported");
}

TEST(NotSupportedExceptionTests, CharCtor_WhatMatches) {
    NotSupportedException e("write not supported");
    EXPECT_STREQ(e.what(), "write not supported");
}

// ---------------------------------------------------------------------------
// std::string constructor
// ---------------------------------------------------------------------------

TEST(NotSupportedExceptionTests, StringCtor_MessageMatches) {
    NotSupportedException e(std::string("seek not supported"));
    EXPECT_EQ(e.getMessageProperty(), "seek not supported");
}

// ---------------------------------------------------------------------------
// (message, innerException) constructor
// ---------------------------------------------------------------------------

TEST(NotSupportedExceptionTests, InnerExceptionCtor_ContainsMessage) {
    auto inner = std::make_exception_ptr(std::runtime_error("disk error"));
    NotSupportedException e("operation not supported", inner);
    std::string msg = e.getMessageProperty();
    EXPECT_NE(msg.find("operation not supported"), std::string::npos);
}

TEST(NotSupportedExceptionTests, InnerExceptionCtor_ContainsInnerMessage) {
    auto inner = std::make_exception_ptr(std::runtime_error("disk error"));
    NotSupportedException e("operation not supported", inner);
    std::string msg = e.getMessageProperty();
    EXPECT_FALSE(msg.empty());
}

// ---------------------------------------------------------------------------
// Inheritance
// ---------------------------------------------------------------------------

TEST(NotSupportedExceptionTests, IsSystemException) {
    bool caught = false;
    try { throw NotSupportedException("x"); }
    catch (const SystemException&) { caught = true; }
    EXPECT_TRUE(caught);
}

TEST(NotSupportedExceptionTests, IsStdException) {
    bool caught = false;
    try { throw NotSupportedException("x"); }
    catch (const std::exception&) { caught = true; }
    EXPECT_TRUE(caught);
}

TEST(NotSupportedExceptionTests, CaughtAsNotSupportedException) {
    bool caught = false;
    try { throw NotSupportedException("x"); }
    catch (const NotSupportedException&) { caught = true; }
    EXPECT_TRUE(caught);
}
