// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when an attempt is made to marshal an object
     * across a context boundary when the target context does not support the operation.
     *
     * C++ counterpart of .NET System.ContextMarshalException.
     */
    class ContextMarshalException : public SystemException {
    public:
        /**
         * @brief Initializes a new instance with the default context-marshal message.
         *
         * C++ counterpart of .NET ContextMarshalException().
         */
        ContextMarshalException()
            : SystemException("Attempted to marshal an object across a context boundary.") {}

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET ContextMarshalException(string).
         * @param message A description of the error.
         */
        explicit ContextMarshalException(const char* message)
            : SystemException(message) {}

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET ContextMarshalException(string).
         * @param message A description of the error.
         */
        explicit ContextMarshalException(const std::string& message)
            : SystemException(message) {}

        /**
         * @brief Initializes a new instance with a specified error message and a
         * reference to the inner exception that caused this exception.
         *
         * C++ counterpart of .NET ContextMarshalException(string, Exception).
         * @param message        A description of the error.
         * @param innerException The exception that is the cause of this exception.
         */
        ContextMarshalException(const std::string& message, std::exception_ptr innerException)
            : SystemException(message, std::move(innerException)) {}
    };

} // namespace System
