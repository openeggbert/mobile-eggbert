// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArrayTypeMismatchException.hpp"
#include "System/SystemException.hpp"

using System::ArrayTypeMismatchException;

TEST(ArrayTypeMismatchExceptionTests2, DefaultCtor_WhatNotEmpty) {
    ArrayTypeMismatchException ex;
    EXPECT_FALSE(std::string(ex.what()).empty());
}

// Regression: the default message previously used the class doc-comment's summary text
// ("Attempted to store an element...") instead of real .NET's actual SR.Arg_ArrayTypeMismatchException
// resource string.
TEST(ArrayTypeMismatchExceptionTests2, DefaultCtor_MatchesDotNetResourceString) {
    ArrayTypeMismatchException ex;
    EXPECT_EQ(std::string(ex.what()),
              "Attempted to access an element as a type incompatible with the array.");
}

TEST(ArrayTypeMismatchExceptionTests2, IsA_SystemException) {
    EXPECT_THROW(throw ArrayTypeMismatchException(), System::SystemException);
}

TEST(ArrayTypeMismatchExceptionTests2, MessageCtor_WhatContainsMessage) {
    ArrayTypeMismatchException ex("wrong type stored");
    EXPECT_NE(std::string(ex.what()).find("wrong type stored"), std::string::npos);
}

TEST(ArrayTypeMismatchExceptionTests2, CharPtrCtor_WhatContainsMessage) {
    ArrayTypeMismatchException ex("type error");
    EXPECT_NE(std::string(ex.what()).find("type error"), std::string::npos);
}

TEST(ArrayTypeMismatchExceptionTests2, InnerExceptionCtor_WhatContainsMessage) {
    auto inner = std::make_exception_ptr(std::runtime_error("root"));
    ArrayTypeMismatchException ex("mismatch", inner);
    EXPECT_NE(std::string(ex.what()).find("mismatch"), std::string::npos);
}
