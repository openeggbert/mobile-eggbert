// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when there is not enough memory to
     * continue the execution of a program.
     *
     * C++ counterpart of .NET System.OutOfMemoryException.
     * Derives from SystemException and is the base class of
     * InsufficientMemoryException.
     *
     * Unlike .NET, this C++ port cannot intercept actual allocator failures;
     * the class is provided for source-compatibility with ported C# code that
     * catches or throws OutOfMemoryException explicitly.
     */
    class OutOfMemoryException : public SystemException {
    public:
        /**
         * @brief Initializes a new instance with the default message.
         *
         * C++ counterpart of .NET OutOfMemoryException().
         * The default message is "Insufficient memory to continue the
         * execution of the program."
         */
        OutOfMemoryException();

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET OutOfMemoryException(string).
         * @param message A string that describes the error.
         */
        explicit OutOfMemoryException(const char* message);

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET OutOfMemoryException(string).
         * @param message A string that describes the error.
         */
        explicit OutOfMemoryException(const std::string& message);

        /**
         * @brief Initializes a new instance with a specified error message and
         * a reference to the inner exception that caused this exception.
         *
         * C++ counterpart of .NET OutOfMemoryException(string, Exception).
         * @param message A string that describes the error.
         * @param inner   The exception that is the cause of this exception.
         */
        OutOfMemoryException(const std::string& message, std::exception_ptr inner);
    };

} // namespace System
