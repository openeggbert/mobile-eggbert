// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/TimeoutException.hpp"
namespace System {
    TimeoutException::TimeoutException() : SystemException("The operation has timed out.") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131505)); // COR_E_TIMEOUT
    }
    TimeoutException::TimeoutException(const char* message) : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131505)); // COR_E_TIMEOUT
    }
    TimeoutException::TimeoutException(const std::string& message) : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131505)); // COR_E_TIMEOUT
    }
    TimeoutException::TimeoutException(const std::string& message, std::exception_ptr innerException)
        : SystemException(message, std::move(innerException)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131505)); // COR_E_TIMEOUT
    }
} // namespace System
