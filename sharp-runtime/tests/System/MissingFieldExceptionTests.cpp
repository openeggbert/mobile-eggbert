// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/MissingFieldException.hpp"

using System::MissingFieldException;

TEST(MissingFieldExceptionTest, DefaultCtor) {
    MissingFieldException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(MissingFieldExceptionTest, MessageCtor) {
    MissingFieldException e("field missing");
    EXPECT_NE(std::string(e.what()).find("field missing"), std::string::npos);
}

TEST(MissingFieldExceptionTest, ClassAndFieldCtor) {
    MissingFieldException e("MyClass", "myField");
    std::string msg(e.what());
    EXPECT_NE(msg.find("MyClass"), std::string::npos);
    EXPECT_NE(msg.find("myField"), std::string::npos);
}

TEST(MissingFieldExceptionTest, InnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    MissingFieldException e("field not found", inner);
    EXPECT_NE(std::string(e.what()).find("field not found"), std::string::npos);
}

TEST(MissingFieldExceptionTest, IsMissingMemberException) {
    MissingFieldException e("test");
    EXPECT_NO_THROW({ System::MissingMemberException& ref = e; (void)ref; });
}

TEST(MissingFieldExceptionTest, ClassFieldCtor_MatchesDotNetMessageFormat) {
    MissingFieldException e("MyClass", "myField");
    EXPECT_EQ(std::string(e.what()), "Field 'MyClass.myField' not found.");
}

TEST(MissingFieldExceptionTest, HResult_MatchesCorEMissingField_NotBaseMissingMember) {
    // MissingFieldException overrides its base MissingMemberException's HResult
    // (COR_E_MISSINGMEMBER, 0x80131512) with its own (COR_E_MISSINGFIELD, 0x80131511).
    constexpr SharpRuntime::intcs corE = static_cast<SharpRuntime::intcs>(0x80131511);
    MissingFieldException defaultCtor;
    MissingFieldException messageCtor("field missing");
    MissingFieldException classFieldCtor("MyClass", "myField");
    MissingFieldException innerCtor("field not found", std::make_exception_ptr(std::runtime_error("cause")));
    EXPECT_EQ(defaultCtor.getHResultProperty(), corE);
    EXPECT_EQ(messageCtor.getHResultProperty(), corE);
    EXPECT_EQ(classFieldCtor.getHResultProperty(), corE);
    EXPECT_EQ(innerCtor.getHResultProperty(), corE);
}
