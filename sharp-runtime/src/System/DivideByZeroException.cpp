// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/DivideByZeroException.hpp"

namespace System {

    namespace {
        constexpr const char* DefaultMsg = "Attempted to divide by zero.";
    }

    DivideByZeroException::DivideByZeroException()
        : ArithmeticException(DefaultMsg) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80020012)); // COR_E_DIVIDEBYZERO
    }

    DivideByZeroException::DivideByZeroException(const char* message)
        : ArithmeticException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80020012)); // COR_E_DIVIDEBYZERO
    }

    DivideByZeroException::DivideByZeroException(const std::string& message)
        : ArithmeticException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80020012)); // COR_E_DIVIDEBYZERO
    }

    DivideByZeroException::DivideByZeroException(
        const std::string& message, std::exception_ptr innerException)
        : ArithmeticException(message, std::move(innerException)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80020012)); // COR_E_DIVIDEBYZERO
    }

} // namespace System
