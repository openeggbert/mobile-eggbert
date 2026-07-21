// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/Exception.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when time zone information is invalid.
     *
     * C++ counterpart of .NET System.InvalidTimeZoneException.
     * Derives from Exception.
     *
     * Thrown when a time zone identifier is found in the registry but the
     * registry data is corrupted, or when the time zone data cannot be read
     * from the operating system.
     */
    class InvalidTimeZoneException : public Exception {
    public:
        /**
         * @brief Initializes a new instance with the default error message.
         *
         * C++ counterpart of .NET InvalidTimeZoneException().
         */
        InvalidTimeZoneException()
            : Exception("The time zone information is invalid.") {}

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET InvalidTimeZoneException(string).
         * @param message A string that describes the error.
         */
        explicit InvalidTimeZoneException(const char* message)
            : Exception(message) {}

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET InvalidTimeZoneException(string).
         * @param message A string that describes the error.
         */
        explicit InvalidTimeZoneException(const std::string& message)
            : Exception(message) {}

        /**
         * @brief Initializes a new instance with a specified error message and
         * a reference to the inner exception that caused this exception.
         *
         * C++ counterpart of .NET InvalidTimeZoneException(string, Exception).
         * @param message A string that describes the error.
         * @param inner   The exception that is the cause of this exception.
         */
        InvalidTimeZoneException(const std::string& message, std::exception_ptr inner)
            : Exception(message, std::move(inner)) {}
    };

} // namespace System
