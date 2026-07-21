// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <stdexcept>
#include "System/ExecutionEngineException.hpp"
#include "System/SystemException.hpp"

TEST(ExecutionEngineExceptionTests2, DefaultCtor_MessageNotEmpty) {
    System::ExecutionEngineException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(ExecutionEngineExceptionTests2, StringCtor_StoresMessage) {
    System::ExecutionEngineException e("engine failure");
    EXPECT_NE(std::string(e.what()).find("engine failure"), std::string::npos);
}

TEST(ExecutionEngineExceptionTests2, InnerExceptionCtor_StoresMessage) {
    auto inner = std::make_exception_ptr(std::runtime_error("cause"));
    System::ExecutionEngineException e("fatal error", inner);
    EXPECT_NE(std::string(e.what()).find("fatal error"), std::string::npos);
}

TEST(ExecutionEngineExceptionTests2, IsA_SystemException) {
    System::ExecutionEngineException e;
    EXPECT_NE(dynamic_cast<System::SystemException*>(&e), nullptr);
}

TEST(ExecutionEngineExceptionTests2, HResult_MatchesCorEExecutionEngine) {
    constexpr SharpRuntime::intcs corEExecutionEngine = static_cast<SharpRuntime::intcs>(0x80131506);
    System::ExecutionEngineException defaultCtor;
    System::ExecutionEngineException messageCtor("engine failure");
    System::ExecutionEngineException innerCtor("fatal error", std::make_exception_ptr(std::runtime_error("cause")));
    EXPECT_EQ(defaultCtor.getHResultProperty(), corEExecutionEngine);
    EXPECT_EQ(messageCtor.getHResultProperty(), corEExecutionEngine);
    EXPECT_EQ(innerCtor.getHResultProperty(), corEExecutionEngine);
}
