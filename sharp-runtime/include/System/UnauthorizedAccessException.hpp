// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SystemException.hpp"
#include <string>

namespace System {

    /**
     * @brief The exception thrown when access to a file or resource is denied by the OS.
     *
     * C++ counterpart of .NET System.UnauthorizedAccessException.
     * Derives from SystemException; thrown for permission or I/O access violations.
     */
    class UnauthorizedAccessException : public SystemException {
    public:
        /** @brief Initializes a new instance with the default unauthorized-access message. */
        UnauthorizedAccessException();
        /** @brief Initializes a new instance with the specified message (const char* overload). */
        explicit UnauthorizedAccessException(const char* str);
        /** @brief Initializes a new instance with the specified error message. */
        explicit UnauthorizedAccessException(const std::string& str);
        /** @brief Initializes a new instance with the specified message and inner exception. */
        UnauthorizedAccessException(const std::string& str, std::exception_ptr inner);
    };

} // namespace System
