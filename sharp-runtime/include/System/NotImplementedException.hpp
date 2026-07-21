// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when a requested method or operation
     * is not implemented.
     *
     * C++ counterpart of .NET System.NotImplementedException.
     */
    class NotImplementedException : public SystemException {
    public:
        /** @brief Initializes a new instance with a default message. */
        NotImplementedException();

        /** @brief Initializes a new instance with the specified error message (C string). */
        explicit NotImplementedException(const char* message);

        /** @brief Initializes a new instance with the specified error message. */
        explicit NotImplementedException(const std::string& message);

        /** @brief Initializes a new instance with the specified message and inner exception. */
        NotImplementedException(const std::string& message, std::exception_ptr inner);
    };

} // namespace System
