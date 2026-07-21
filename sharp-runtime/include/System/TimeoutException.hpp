// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/SystemException.hpp"
namespace System {
    /**
     * @brief The exception thrown when the time allotted for a process or operation has expired.
     *
     * C++ counterpart of .NET System.TimeoutException.
     */
    class TimeoutException : public SystemException {
    public:
        /** @brief Initializes a new instance with the default timeout message. */
        TimeoutException();
        /** @brief Initializes a new instance with the specified message (const char* overload). */
        explicit TimeoutException(const char* message);
        /** @brief Initializes a new instance with the specified error message. */
        explicit TimeoutException(const std::string& message);
        /** @brief Initializes a new instance with the specified message and inner exception. */
        TimeoutException(const std::string& message, std::exception_ptr innerException);
    };
} // namespace System
