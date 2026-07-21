// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/MissingMemberException.hpp"

using System::MissingMemberException;

TEST(MissingMemberExceptionTests, DefaultCtor_WhatNotEmpty_New) {
    MissingMemberException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(MissingMemberExceptionTests, DefaultCtor_WhatContainsMember) {
    MissingMemberException e;
    EXPECT_NE(std::string(e.what()).find("member"), std::string::npos);
}

TEST(MissingMemberExceptionTests, StringCtor_MessageStored_New) {
    MissingMemberException e("custom message");
    EXPECT_EQ(std::string(e.what()), "custom message");
}

TEST(MissingMemberExceptionTests, ClassMemberCtor_ContainsClassName) {
    MissingMemberException e("Foo", "Bar");
    EXPECT_NE(std::string(e.what()).find("Foo"), std::string::npos);
}

TEST(MissingMemberExceptionTests, ClassMemberCtor_ContainsMemberName) {
    MissingMemberException e("Foo", "Bar");
    EXPECT_NE(std::string(e.what()).find("Bar"), std::string::npos);
}

TEST(MissingMemberExceptionTests, ClassMemberCtor_ContainsDot) {
    MissingMemberException e("A", "B");
    EXPECT_NE(std::string(e.what()).find("A.B"), std::string::npos);
}

TEST(MissingMemberExceptionTests, InnerExceptionCtor_ContainsBothMessages) {
    auto inner = std::make_exception_ptr(std::runtime_error("inner cause"));
    MissingMemberException e("outer msg", inner);
    std::string msg(e.what());
    EXPECT_NE(msg.find("outer msg"), std::string::npos);
}

TEST(MissingMemberExceptionTests, Catchable_AsStdException) {
    EXPECT_THROW({ throw MissingMemberException(); }, std::exception);
}

TEST(MissingMemberExceptionTests, Catchable_AsMissingMemberException) {
    EXPECT_THROW({ throw MissingMemberException("x"); }, MissingMemberException);
}

TEST(MissingMemberExceptionTests, ClassMemberCtor_MatchesDotNetMessageFormat) {
    MissingMemberException e("Foo", "Bar");
    EXPECT_EQ(std::string(e.what()), "Member 'Foo.Bar' not found.");
}

TEST(MissingMemberExceptionTests, HResult_MatchesCorEMissingMember) {
    constexpr SharpRuntime::intcs corE = static_cast<SharpRuntime::intcs>(0x80131512);
    MissingMemberException defaultCtor;
    MissingMemberException messageCtor("custom message");
    MissingMemberException classMemberCtor("Foo", "Bar");
    MissingMemberException innerCtor("outer msg", std::make_exception_ptr(std::runtime_error("inner cause")));
    EXPECT_EQ(defaultCtor.getHResultProperty(), corE);
    EXPECT_EQ(messageCtor.getHResultProperty(), corE);
    EXPECT_EQ(classMemberCtor.getHResultProperty(), corE);
    EXPECT_EQ(innerCtor.getHResultProperty(), corE);
}
