// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/ArithmeticException.hpp"

namespace System {

    /**
     * @brief The exception thrown when an arithmetic, casting, or conversion operation
     * results in an overflow.
     *
     * C++ counterpart of .NET System.OverflowException.
     * Derives from ArithmeticException.
     */
    class OverflowException : public ArithmeticException {
    public:
        /** @brief Initializes a new instance with the default overflow message. */
        OverflowException();
        /** @brief Initializes a new instance with the specified message (const char* overload). */
        explicit OverflowException(const char* str);
        /** @brief Initializes a new instance with the specified error message. */
        explicit OverflowException(const std::string& message);
        /** @brief Initializes a new instance with the specified message and inner exception. */
        OverflowException(const std::string& message, std::exception_ptr innerException);
    };

} // namespace System
