// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when an invoked method is not
     * supported, or when there is an attempt to read, seek, or write to a
     * stream that does not support the invoked functionality.
     *
     * C++ counterpart of .NET System.NotSupportedException.
     */
    class NotSupportedException : public SystemException {
    public:
        /** @brief Initializes a new instance with the default message. */
        NotSupportedException();

        /** @brief Initializes a new instance with the specified message. */
        explicit NotSupportedException(const char* message);

        /** @brief Initializes a new instance with the specified message. */
        explicit NotSupportedException(const std::string& message);

        /**
         * @brief Initializes a new instance with a specified message and a
         * reference to the inner exception that is the cause of this exception.
         *
         * C++ counterpart of .NET NotSupportedException(string, Exception).
         */
        NotSupportedException(const std::string& message, std::exception_ptr innerException);
    };

} // namespace System
