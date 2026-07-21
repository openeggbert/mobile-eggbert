// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/SystemException.hpp"
namespace System {
    /**
     * @brief The exception thrown for invalid casting or explicit conversion.
     *
     * C++ counterpart of .NET System.InvalidCastException.
     */
    class InvalidCastException : public SystemException {
    public:
        /** @brief Initializes a new instance with the default invalid-cast message. */
        InvalidCastException();
        /** @brief Initializes a new instance with the specified message (const char* overload). */
        explicit InvalidCastException(const char* message);
        /** @brief Initializes a new instance with the specified error message. */
        explicit InvalidCastException(const std::string& message);
        /** @brief Initializes a new instance with the specified message and inner exception. */
        InvalidCastException(const std::string& message, std::exception_ptr innerException);
        /**
         * @brief Initializes a new instance with the specified message and a custom HResult error code.
         *
         * C++ counterpart of .NET InvalidCastException(string, int).
         * @param message   A description of the error.
         * @param errorCode The HResult that describes the error.
         */
        InvalidCastException(const std::string& message, SharpRuntime::intcs errorCode);
    };
} // namespace System
