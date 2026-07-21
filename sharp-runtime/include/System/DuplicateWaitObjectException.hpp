// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/ArgumentException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when a duplicate object exists in an
     * array where multiple distinct objects are required.
     *
     * C++ counterpart of .NET System.DuplicateWaitObjectException.
     */
    class DuplicateWaitObjectException : public ArgumentException {
    public:
        /** @brief Initializes a new instance with the default duplicate-object message. */
        DuplicateWaitObjectException() : ArgumentException("Duplicate objects in argument.") {}

        /**
         * @brief Initializes a new instance with the name of the offending parameter.
         *
         * C++ counterpart of .NET DuplicateWaitObjectException(string parameterName).
         */
        explicit DuplicateWaitObjectException(const std::string& parameterName)
            : ArgumentException("Duplicate objects in argument.", parameterName) {}

        /**
         * @brief Initializes a new instance with the parameter name and a custom message.
         *
         * C++ counterpart of .NET DuplicateWaitObjectException(string, string).
         */
        DuplicateWaitObjectException(const std::string& parameterName, const std::string& message)
            : ArgumentException(message, parameterName) {}

        /**
         * @brief Initializes a new instance with a message and an inner exception.
         *
         * C++ counterpart of .NET DuplicateWaitObjectException(string, Exception).
         */
        DuplicateWaitObjectException(const std::string& message, std::exception_ptr innerException)
            : ArgumentException(message, std::move(innerException)) {}
    };

} // namespace System
