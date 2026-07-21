// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception thrown when there is an attempt to read or write protected memory.
     *
     * C++ counterpart of .NET System.AccessViolationException.
     */
    class AccessViolationException : public SystemException {
    public:
        /** @brief Initializes a new instance with the default access-violation message. */
        AccessViolationException() : SystemException("Attempted to read or write protected memory. This is often an indication that other memory is corrupt.") {}
        /** @brief Initializes a new instance with the specified error message. */
        explicit AccessViolationException(const std::string& message) : SystemException(message) {}
        /** @brief Initializes a new instance with the specified message and inner exception. */
        AccessViolationException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {}
    };

} // namespace System
