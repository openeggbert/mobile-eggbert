// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Exception.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when a non-fatal application error occurs.
     *
     * C++ counterpart of .NET System.ApplicationException.
     */
    class ApplicationException : public Exception {
    public:
        /** @brief Initializes a new instance with the default application error message. */
        ApplicationException() : Exception("Error in the application.") {}
        /** @brief Initializes a new instance with the specified error message. */
        explicit ApplicationException(const std::string& message) : Exception(message) {}
        /** @brief Initializes a new instance with the specified message and inner exception. */
        ApplicationException(const std::string& message, std::exception_ptr inner)
            : Exception(message, std::move(inner)) {}
    };

} // namespace System
