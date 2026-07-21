// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <exception>
#include <string>

#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when the format of an argument is
     * invalid, or when a composite format string is not well formed.
     */
    class FormatException : public SystemException {
    public:
        /** @brief Initializes a new instance with the default message. */
        FormatException();
        /** @brief Initializes a new instance with the specified error message. */
        explicit FormatException(const char* message);
        /** @brief Initializes a new instance with the specified error message. */
        explicit FormatException(const std::string& message);
        /** @brief Initializes a new instance with the specified message and inner exception. */
        FormatException(const std::string& message, std::exception_ptr inner);
    };

} // namespace System
