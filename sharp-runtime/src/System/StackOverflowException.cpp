// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/StackOverflowException.hpp"

namespace System {

    StackOverflowException::StackOverflowException()
        : SystemException("Operation caused a stack overflow.") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x800703E9)); // COR_E_STACKOVERFLOW
    }

    StackOverflowException::StackOverflowException(const char* message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x800703E9)); // COR_E_STACKOVERFLOW
    }

    StackOverflowException::StackOverflowException(const std::string& message)
        : SystemException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x800703E9)); // COR_E_STACKOVERFLOW
    }

    StackOverflowException::StackOverflowException(const std::string& message, std::exception_ptr innerException)
        : SystemException(message, std::move(innerException)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x800703E9)); // COR_E_STACKOVERFLOW
    }

} // namespace System
