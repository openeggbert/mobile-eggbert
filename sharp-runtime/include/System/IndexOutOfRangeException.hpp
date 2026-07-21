// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <exception>
#include <string>

#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when an attempt is made to access an element
     * of an array or collection with an index that is outside its bounds.
     *
     * C++ counterpart of .NET System.IndexOutOfRangeException.
     */
    class IndexOutOfRangeException : public SystemException {
    public:
        /**
         * @brief Initializes a new instance with the default message
         * "Index was outside the bounds of the array."
         *
         * C++ counterpart of .NET IndexOutOfRangeException().
         */
        IndexOutOfRangeException();

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET IndexOutOfRangeException(string).
         * @param message A description of the error.
         */
        explicit IndexOutOfRangeException(const char* message);

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET IndexOutOfRangeException(string).
         * @param message A description of the error.
         */
        explicit IndexOutOfRangeException(const std::string& message);

        /**
         * @brief Initializes a new instance with a specified error message
         * and a reference to the inner exception that caused this exception.
         *
         * C++ counterpart of .NET IndexOutOfRangeException(string, Exception).
         * @param message        A description of the error.
         * @param innerException The exception that is the cause of this exception.
         */
        IndexOutOfRangeException(const std::string& message, std::exception_ptr innerException);
    };

} // namespace System
