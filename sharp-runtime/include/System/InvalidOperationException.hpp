// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when a method call is invalid for
     * the object's current state.
     *
     * C++ counterpart of .NET System.InvalidOperationException.
     */
    class InvalidOperationException : public SystemException {
    public:
        /** @brief Initializes a new instance with the default invalid-operation message. */
        InvalidOperationException();

        /** @brief Initializes a new instance with the specified C-string message. */
        explicit InvalidOperationException(const char* message);

        /** @brief Initializes a new instance with the specified message. */
        explicit InvalidOperationException(const std::string& message);

        /**
         * @brief Initializes a new instance with a message and an inner exception.
         *
         * C++ counterpart of .NET InvalidOperationException(string, Exception).
         */
        InvalidOperationException(const std::string& message, std::exception_ptr innerException);
    };

} // namespace System