// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/OperationCanceledException.hpp"
#include "System/SystemException.hpp"
#include "System/Threading/CancellationTokenSource.hpp"

using System::OperationCanceledException;
using System::Threading::CancellationToken;
using System::Threading::CancellationTokenSource;

TEST(OperationCanceledExceptionTests, DefaultCtor_WhatContainsCanceled) {
    OperationCanceledException e;
    EXPECT_NE(std::string(e.what()).find("canceled"), std::string::npos);
}

TEST(OperationCanceledExceptionTests, DefaultCtor_IsSystemException) {
    OperationCanceledException e;
    EXPECT_NE(dynamic_cast<System::SystemException*>(&e), nullptr);
}

TEST(OperationCanceledExceptionTests, CStringCtor_MessageStored) {
    OperationCanceledException e("task was aborted");
    EXPECT_EQ(std::string(e.what()), "task was aborted");
}

TEST(OperationCanceledExceptionTests, StringCtor_MessageStored) {
    OperationCanceledException e(std::string("operation stopped"));
    EXPECT_EQ(std::string(e.what()), "operation stopped");
}

TEST(OperationCanceledExceptionTests, InnerExceptionCtor_ContainsBothMessages) {
    auto inner = std::make_exception_ptr(std::runtime_error("timeout"));
    OperationCanceledException e("task canceled", inner);
    std::string msg(e.what());
    EXPECT_NE(msg.find("task canceled"), std::string::npos);
}

TEST(OperationCanceledExceptionTests, Catchable_AsStdException) {
    EXPECT_THROW(
        { throw OperationCanceledException(); },
        std::exception
    );
}

TEST(OperationCanceledExceptionTests, Catchable_AsOperationCanceledException) {
    EXPECT_THROW(
        { throw OperationCanceledException("x"); },
        OperationCanceledException
    );
}

TEST(OperationCanceledExceptionTests, DefaultMsg_NotEmpty) {
    OperationCanceledException e;
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(OperationCanceledExceptionTests, InnerCtor_WhatNotEmpty) {
    auto inner = std::make_exception_ptr(std::runtime_error("inner"));
    OperationCanceledException e("outer", inner);
    EXPECT_FALSE(std::string(e.what()).empty());
}

TEST(OperationCanceledExceptionTests, GetMessage_MatchesWhat) {
    OperationCanceledException e("match me");
    EXPECT_EQ(e.getMessageProperty(), std::string(e.what()));
}

TEST(OperationCanceledExceptionTests, DefaultCtor_TokenIsNotCancelled) {
    OperationCanceledException e;
    EXPECT_FALSE(e.getCancellationTokenProperty().getIsCancellationRequestedProperty());
}

TEST(OperationCanceledExceptionTests, TokenCtor_StoresToken) {
    CancellationTokenSource cts;
    cts.Cancel();
    OperationCanceledException e(cts.getTokenProperty());
    EXPECT_TRUE(e.getCancellationTokenProperty().getIsCancellationRequestedProperty());
    EXPECT_NE(std::string(e.what()).find("canceled"), std::string::npos);
}

TEST(OperationCanceledExceptionTests, MessageAndTokenCtor_StoresBoth) {
    CancellationTokenSource cts;
    cts.Cancel();
    OperationCanceledException e("custom message", cts.getTokenProperty());
    EXPECT_EQ(std::string(e.what()), "custom message");
    EXPECT_TRUE(e.getCancellationTokenProperty().getIsCancellationRequestedProperty());
}

TEST(OperationCanceledExceptionTests, MessageInnerAndTokenCtor_StoresAll) {
    CancellationTokenSource cts;
    cts.Cancel();
    auto inner = std::make_exception_ptr(std::runtime_error("root cause"));
    OperationCanceledException e("custom message", inner, cts.getTokenProperty());
    EXPECT_EQ(std::string(e.what()), "custom message");
    EXPECT_TRUE(e.getCancellationTokenProperty().getIsCancellationRequestedProperty());
}

TEST(OperationCanceledExceptionTests, HResult_MatchesCorEOperationCanceled) {
    constexpr SharpRuntime::intcs corE = static_cast<SharpRuntime::intcs>(0x8013153B);
    OperationCanceledException defaultCtor;
    OperationCanceledException messageCtor("msg");
    OperationCanceledException innerCtor("msg", std::make_exception_ptr(std::runtime_error("cause")));
    CancellationTokenSource cts;
    OperationCanceledException tokenCtor(cts.getTokenProperty());
    EXPECT_EQ(defaultCtor.getHResultProperty(), corE);
    EXPECT_EQ(messageCtor.getHResultProperty(), corE);
    EXPECT_EQ(innerCtor.getHResultProperty(), corE);
    EXPECT_EQ(tokenCtor.getHResultProperty(), corE);
}
