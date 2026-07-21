// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <stdexcept>
#include <thread>
#include "System/Runtime/ExceptionServices/ExceptionDispatchInfo.hpp"

using System::Runtime::ExceptionServices::ExceptionDispatchInfo;

TEST(ExceptionDispatchInfoTests, CaptureAndThrow_PreservesException) {
    std::exception_ptr captured;
    try {
        throw std::runtime_error("original failure");
    } catch (...) {
        captured = std::current_exception();
    }

    auto info = ExceptionDispatchInfo::Capture(captured);
    EXPECT_THROW(info.Throw(), std::runtime_error);

    try {
        info.Throw();
        FAIL() << "expected throw";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "original failure");
    }
}

TEST(ExceptionDispatchInfoTests, GetSourceException_ReturnsCaptured) {
    std::exception_ptr captured;
    try {
        throw std::logic_error("boom");
    } catch (...) {
        captured = std::current_exception();
    }
    auto info = ExceptionDispatchInfo::Capture(captured);
    EXPECT_EQ(info.getSourceExceptionProperty(), captured);
}

TEST(ExceptionDispatchInfoTests, StaticThrow_RethrowsImmediately) {
    std::exception_ptr captured;
    try {
        throw std::runtime_error("static throw");
    } catch (...) {
        captured = std::current_exception();
    }
    EXPECT_THROW(ExceptionDispatchInfo::Throw(captured), std::runtime_error);
}

TEST(ExceptionDispatchInfoTests, CrossThreadRethrow) {
    std::exception_ptr captured;
    std::thread worker([&]() {
        try {
            throw std::runtime_error("from worker thread");
        } catch (...) {
            captured = std::current_exception();
        }
    });
    worker.join();

    auto info = ExceptionDispatchInfo::Capture(captured);
    EXPECT_THROW(info.Throw(), std::runtime_error);
}
