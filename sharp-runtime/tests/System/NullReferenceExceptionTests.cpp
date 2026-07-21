// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/NullReferenceException.hpp"

using System::NullReferenceException;

TEST(NullReferenceExceptionTest, DefaultCtor) {
    NullReferenceException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(NullReferenceExceptionTest, CStringCtor) {
    NullReferenceException e("null ref");
    EXPECT_NE(std::string(e.what()).find("null ref"), std::string::npos);
}

TEST(NullReferenceExceptionTest, StringCtor) {
    NullReferenceException e(std::string("null ref"));
    EXPECT_NE(std::string(e.what()).find("null ref"), std::string::npos);
}

TEST(NullReferenceExceptionTest, InnerExceptionCtor) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    NullReferenceException e("null ref", inner);
    EXPECT_NE(std::string(e.what()).find("null ref"), std::string::npos);
}

TEST(NullReferenceExceptionTest, IsSystemException) {
    NullReferenceException e;
    EXPECT_NO_THROW({ System::SystemException& ref = e; (void)ref; });
}

TEST(NullReferenceExceptionTest, HResult_MatchesEPointer) {
    // .NET's NullReferenceException uses HResults.E_POINTER (a standard COM
    // HResult), not a COR_E_* value - verified against NullReferenceException.cs.
    constexpr SharpRuntime::intcs ePointer = static_cast<SharpRuntime::intcs>(0x80004003);
    NullReferenceException defaultCtor;
    NullReferenceException messageCtor("null ref");
    NullReferenceException innerCtor("null ref", std::make_exception_ptr(std::runtime_error("cause")));
    EXPECT_EQ(defaultCtor.getHResultProperty(), ePointer);
    EXPECT_EQ(messageCtor.getHResultProperty(), ePointer);
    EXPECT_EQ(innerCtor.getHResultProperty(), ePointer);
}
