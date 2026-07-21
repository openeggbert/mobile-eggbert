// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when an attempt is made to store an
     * element of the wrong type within an array.
     *
     * C++ counterpart of .NET System.ArrayTypeMismatchException.
     */
    class ArrayTypeMismatchException : public SystemException {
    public:
        /**
         * @brief Initializes a new instance with the default message.
         *
         * C++ counterpart of .NET ArrayTypeMismatchException().
         */
        ArrayTypeMismatchException()
            : SystemException("Attempted to access an element as a type incompatible with the array.") {}

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET ArrayTypeMismatchException(string).
         * @param message A string that describes the error.
         */
        explicit ArrayTypeMismatchException(const std::string& message)
            : SystemException(message) {}

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * @param message A string that describes the error.
         */
        explicit ArrayTypeMismatchException(const char* message)
            : SystemException(message) {}

        /**
         * @brief Initializes a new instance with the specified message and
         * inner exception.
         *
         * C++ counterpart of .NET ArrayTypeMismatchException(string, Exception).
         * @param message A string that describes the error.
         * @param inner   The exception that is the cause of this exception.
         */
        ArrayTypeMismatchException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {}
    };

} // namespace System
