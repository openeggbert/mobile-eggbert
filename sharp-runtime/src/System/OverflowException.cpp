// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/OverflowException.hpp"

namespace System {

    OverflowException::OverflowException()
        : ArithmeticException("Arithmetic operation resulted in an overflow.") {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131516)); // COR_E_OVERFLOW
    }

    OverflowException::OverflowException(const char* str)
        : ArithmeticException(str) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131516)); // COR_E_OVERFLOW
    }

    OverflowException::OverflowException(const std::string& message)
        : ArithmeticException(message) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131516)); // COR_E_OVERFLOW
    }

    OverflowException::OverflowException(const std::string& message, std::exception_ptr innerException)
        : ArithmeticException(message, std::move(innerException)) {
        setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131516)); // COR_E_OVERFLOW
    }

} // namespace System
