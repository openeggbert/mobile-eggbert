// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <exception>
#include <string>

#include "System/ArithmeticException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when there is an attempt to divide an
     * integral or Decimal value by zero.
     *
     * C++ counterpart of .NET System.DivideByZeroException.
     */
    class DivideByZeroException : public ArithmeticException {
    public:
        /** @brief Initializes a new instance with the default divide-by-zero message. */
        DivideByZeroException();

        /** @brief Initializes a new instance with the specified C-string message. */
        explicit DivideByZeroException(const char* message);

        /** @brief Initializes a new instance with the specified message. */
        explicit DivideByZeroException(const std::string& message);

        /**
         * @brief Initializes a new instance with a message and an inner exception.
         *
         * C++ counterpart of .NET DivideByZeroException(string, Exception).
         */
        DivideByZeroException(const std::string& message, std::exception_ptr innerException);
    };

} // namespace System
