// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ObjectDisposedException.hpp"

using System::ObjectDisposedException;

TEST(ObjectDisposedExceptionTest, DefaultCtor) {
    ObjectDisposedException e;
    EXPECT_FALSE(std::string(e.what()).empty());
    EXPECT_TRUE(e.getObjectNameProperty().empty());
}

TEST(ObjectDisposedExceptionTest, ObjectNameCtor) {
    ObjectDisposedException e("MyStream");
    EXPECT_NE(std::string(e.what()).find("MyStream"), std::string::npos);
    EXPECT_EQ(e.getObjectNameProperty(), "MyStream");
}

TEST(ObjectDisposedExceptionTest, ObjectNameAndMessageCtor) {
    ObjectDisposedException e("MyStream", "Stream is closed");
    std::string msg(e.what());
    EXPECT_NE(msg.find("Stream is closed"), std::string::npos);
    EXPECT_EQ(e.getObjectNameProperty(), "MyStream");
}

TEST(ObjectDisposedExceptionTest, MessageAndInnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    ObjectDisposedException e("object disposed", inner);
    EXPECT_NE(std::string(e.what()).find("object disposed"), std::string::npos);
}

TEST(ObjectDisposedExceptionTest, ThrowIf_True) {
    EXPECT_THROW(ObjectDisposedException::ThrowIf(true, "obj"), ObjectDisposedException);
}

TEST(ObjectDisposedExceptionTest, ThrowIf_False) {
    EXPECT_NO_THROW(ObjectDisposedException::ThrowIf(false, "obj"));
}

TEST(ObjectDisposedExceptionTest, IsInvalidOperationException) {
    ObjectDisposedException e("obj");
    EXPECT_NO_THROW({ System::InvalidOperationException& ref = e; (void)ref; });
}

TEST(ObjectDisposedExceptionTest, HResult_MatchesCorEObjectDisposed_NotBaseInvalidOperation) {
    // ObjectDisposedException overrides its base InvalidOperationException's HResult
    // (COR_E_INVALIDOPERATION, 0x80131509) with its own (COR_E_OBJECTDISPOSED, 0x80131622).
    constexpr SharpRuntime::intcs corE = static_cast<SharpRuntime::intcs>(0x80131622);
    ObjectDisposedException defaultCtor;
    ObjectDisposedException objectNameCtor("MyStream");
    ObjectDisposedException objectNameAndMessageCtor("MyStream", "Stream is closed");
    ObjectDisposedException innerCtor("object disposed", std::make_exception_ptr(std::runtime_error("cause")));
    EXPECT_EQ(defaultCtor.getHResultProperty(), corE);
    EXPECT_EQ(objectNameCtor.getHResultProperty(), corE);
    EXPECT_EQ(objectNameAndMessageCtor.getHResultProperty(), corE);
    EXPECT_EQ(innerCtor.getHResultProperty(), corE);
}
