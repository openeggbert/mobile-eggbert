// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>
#include <memory>

#include "System/ArgumentException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when a null reference is passed to a
     * method that does not accept it as a valid argument.
     *
     * C++ counterpart of .NET System.ArgumentNullException.
     */
    class ArgumentNullException : public ArgumentException {
    public:
        /** @brief Initializes a new instance with the default null-argument message. */
        ArgumentNullException();

        /**
         * @brief Initializes a new instance with the name of the null parameter.
         *
         * C++ counterpart of .NET ArgumentNullException(string paramName).
         */
        explicit ArgumentNullException(const char* paramName);

        /**
         * @brief Initializes a new instance with the parameter name and a custom message.
         *
         * C++ counterpart of .NET ArgumentNullException(string paramName, string message).
         */
        ArgumentNullException(const char* paramName, const char* message);

        /**
         * @brief Initializes a new instance with the name of the null parameter.
         *
         * C++ counterpart of .NET ArgumentNullException(string paramName).
         */
        explicit ArgumentNullException(const std::string& paramName);

        /**
         * @brief Initializes a new instance with the parameter name and a custom message.
         *
         * C++ counterpart of .NET ArgumentNullException(string paramName, string message).
         */
        ArgumentNullException(const std::string& paramName, const std::string& message);

        /**
         * @brief Initializes a new instance with a message and an inner exception.
         *
         * C++ counterpart of .NET ArgumentNullException(string message, Exception innerException).
         */
        ArgumentNullException(const std::string& message, std::exception_ptr innerException);

        /**
         * @brief Throws an ArgumentNullException if @p argument is null.
         *
         * C++ counterpart of .NET ArgumentNullException.ThrowIfNull.
         * @param argument  The pointer to validate as non-null.
         * @param paramName The name of the parameter with which @p argument corresponds.
         * @throws ArgumentNullException if @p argument is null.
         */
        template<typename T>
        static void ThrowIfNull(T* argument, const std::string& paramName = "") {
            if (argument == nullptr)
                throw ArgumentNullException(paramName);
        }
    };

} // namespace System
