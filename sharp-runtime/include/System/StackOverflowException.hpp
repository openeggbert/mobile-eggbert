// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when the execution stack overflows
     * because it contains too many nested method calls.
     *
     * C++ counterpart of .NET System.StackOverflowException. Sealed in .NET (cannot
     * be subclassed).
     */
    class StackOverflowException final : public SystemException {
    public:
        /** @brief Initializes a new instance with the default message. */
        StackOverflowException();

        /** @brief Initializes a new instance with the specified message. */
        explicit StackOverflowException(const char* message);

        /** @brief Initializes a new instance with the specified message. */
        explicit StackOverflowException(const std::string& message);

        /**
         * @brief Initializes a new instance with a specified message and a
         * reference to the inner exception that is the cause of this exception.
         *
         * C++ counterpart of .NET StackOverflowException(string, Exception).
         */
        StackOverflowException(const std::string& message, std::exception_ptr innerException);
    };

} // namespace System
