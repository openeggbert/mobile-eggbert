// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SystemException.hpp"
#include <string>

namespace System {

    /**
     * @brief The exception that is thrown when one of the arguments provided to a method is not valid.
     *
     * C++ counterpart of .NET System.ArgumentException.
     */
    class ArgumentException : public SystemException {
        std::string paramName_;

    public:
        /**
         * @brief Initializes a new instance of the ArgumentException class.
         *
         * C++ counterpart of .NET ArgumentException().
         */
        ArgumentException();

        /**
         * @brief Initializes a new instance with the specified error message.
         * @param message A message that describes the error.
         */
        explicit ArgumentException(const char* message);

        /**
         * @brief Initializes a new instance with the specified error message.
         * @param message A message that describes the error.
         */
        explicit ArgumentException(const std::string& message);

        /**
         * @brief Initializes a new instance with a message and a reference to the inner exception.
         * @param message        A message that describes the error.
         * @param innerException The exception that is the cause of the current exception.
         */
        ArgumentException(const std::string& message, std::exception_ptr innerException);

        /**
         * @brief Initializes a new instance with a message and the name of the parameter that caused this exception.
         * @param message   A message that describes the error.
         * @param paramName The name of the parameter that caused the current exception.
         */
        ArgumentException(const char* message, const char* paramName);

        /**
         * @brief Initializes a new instance with a message and the name of the parameter that caused this exception.
         * @param message   A message that describes the error.
         * @param paramName The name of the parameter that caused the current exception.
         */
        ArgumentException(const std::string& message, const std::string& paramName);

        /**
         * @brief Initializes a new instance with a message, parameter name, and inner exception.
         * @param message        A message that describes the error.
         * @param paramName      The name of the parameter that caused the current exception.
         * @param innerException The exception that is the cause of the current exception.
         */
        ArgumentException(const std::string& message, const std::string& paramName,
                          std::exception_ptr innerException);

        /**
         * @brief Gets the name of the parameter that caused the current exception.
         *
         * C++ counterpart of .NET ArgumentException.ParamName.
         * @return The parameter name, or an empty string if none was specified.
         */
        [[nodiscard]] virtual const std::string& getParamNameProperty() const noexcept {
            return paramName_;
        }

        /**
         * @brief Throws an ArgumentException if @p argument is empty.
         *
         * C++ counterpart of .NET ArgumentException.ThrowIfNullOrEmpty.
         * Note: C++ std::string cannot be null; only the empty check applies.
         * @param argument  The string argument to validate as non-empty.
         * @param paramName The name of the parameter with which @p argument corresponds.
         * @throws ArgumentException if @p argument is empty.
         */
        static void ThrowIfNullOrEmpty(const std::string& argument,
                                       const std::string& paramName = "");

        /**
         * @brief Throws an ArgumentException if @p argument is empty or consists only of white-space characters.
         *
         * C++ counterpart of .NET ArgumentException.ThrowIfNullOrWhiteSpace.
         * Note: C++ std::string cannot be null; only the empty/whitespace check applies.
         * @param argument  The string argument to validate.
         * @param paramName The name of the parameter with which @p argument corresponds.
         * @throws ArgumentException if @p argument is empty or whitespace-only.
         */
        static void ThrowIfNullOrWhiteSpace(const std::string& argument,
                                            const std::string& paramName = "");

    protected:
        /** @brief Tag type selecting the "message is already fully composed" constructor below. */
        struct AlreadyComposedTag {};

        /**
         * @brief Constructs from a message a derived class has already fully composed (e.g.
         * with additional suffix text appended after the parameter-name marker), storing
         * @p paramName directly without appending " (Parameter 'x')" to @p composedMessage a
         * second time.
         *
         * For use by derived classes (e.g. ArgumentOutOfRangeException) whose real .NET
         * counterpart appends its own suffix (e.g. "Actual value was X.") AFTER the
         * "(Parameter 'x')" marker that the two-argument (message, paramName) constructor
         * would otherwise append last.
         */
        ArgumentException(const std::string& composedMessage, const std::string& paramName,
                          AlreadyComposedTag);

        /**
         * @brief Appends " (Parameter 'paramName')" to @p message, matching the exact format
         * the (message, paramName) constructors use, for use by derived classes composing a
         * fuller final message before calling the AlreadyComposedTag constructor above.
         */
        static std::string AppendParamNameSuffix(const std::string& message,
                                                  const std::string& paramName);
    };

} // namespace System
