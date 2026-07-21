// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Exception.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ObjectDisposedException.hpp"

// Additional tests for Exception, ArgumentNullException, ObjectDisposedException
// (basic coverage already in ExceptionTests.cpp / ExceptionRemainingTests.cpp)

TEST(ExceptionNewTests, CStringCtor_MessageMatches) {
    System::Exception e("hello");
    EXPECT_EQ(e.getMessageProperty(), "hello");
}

TEST(ExceptionNewTests, DefaultCtor_MessageEmpty) {
    System::Exception e;
    EXPECT_TRUE(e.getMessageProperty().empty());
}

TEST(ExceptionNewTests, InnerExceptionPtr_StoredAndRetrievable) {
    auto inner = std::make_exception_ptr(std::runtime_error("inner"));
    System::Exception e("outer", inner);
    EXPECT_NE(e.getInnerExceptionProperty(), nullptr);
}

TEST(ExceptionNewTests, StackTrace_AlwaysEmpty) {
    System::Exception e("x");
    EXPECT_TRUE(e.getStackTraceProperty().empty());
}

TEST(ExceptionNewTests, Data_Modifiable) {
    System::Exception e("e");
    e.getDataProperty()["key"] = "val";
    EXPECT_EQ(e.getDataProperty().at("key"), "val");
}

TEST(ArgumentNullExceptionNewTests, DefaultCtor_IsArgumentException) {
    System::ArgumentNullException e;
    EXPECT_NE(dynamic_cast<System::ArgumentException*>(&e), nullptr);
}

TEST(ArgumentNullExceptionNewTests, ParamNameMessageCtor_StoresMessage) {
    System::ArgumentNullException e("param", "custom message");
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(ObjectDisposedExceptionNewTests, ThrowIf_True_Throws) {
    EXPECT_THROW(
        System::ObjectDisposedException::ThrowIf(true, "obj"),
        System::ObjectDisposedException
    );
}

TEST(ObjectDisposedExceptionNewTests, ThrowIf_False_NoThrow) {
    EXPECT_NO_THROW(
        System::ObjectDisposedException::ThrowIf(false, "obj")
    );
}

TEST(ObjectDisposedExceptionNewTests, ObjectName_Stored) {
    System::ObjectDisposedException e("myResource");
    EXPECT_EQ(e.getObjectNameProperty(), "myResource");
}
