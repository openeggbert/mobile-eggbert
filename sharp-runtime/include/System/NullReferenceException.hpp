// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/SystemException.hpp"
namespace System {

    /**
     * @brief The exception that is thrown when there is an attempt to dereference
     * a null object reference.
     *
     * C++ counterpart of .NET System.NullReferenceException.
     */
    class NullReferenceException : public SystemException {
    public:
        /** @brief Initializes a new instance with the default null-reference message. */
        NullReferenceException();

        /** @brief Initializes a new instance with the specified C-string message. */
        explicit NullReferenceException(const char* message);

        /** @brief Initializes a new instance with the specified message. */
        explicit NullReferenceException(const std::string& message);

        /**
         * @brief Initializes a new instance with a message and an inner exception.
         *
         * C++ counterpart of .NET NullReferenceException(string, Exception).
         */
        NullReferenceException(const std::string& message, std::exception_ptr innerException);
    };

} // namespace System
