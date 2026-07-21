// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <exception>
#include <string>

#include "System/InvalidOperationException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when accessing an object that was disposed.
     *
     * C++ counterpart of .NET System.ObjectDisposedException.
     */
    class ObjectDisposedException : public InvalidOperationException {
    private:
        std::string objectName_;

        static std::string BuildMessage(const std::string& objectName, const std::string& message);

    public:
        /**
         * @brief Initializes a new instance of the ObjectDisposedException class.
         *
         * Uses a default generic disposed-object message.
         */
        ObjectDisposedException();

        /**
         * @brief Initializes a new instance of the ObjectDisposedException class
         * with the name of the disposed object.
         *
         * @param objectName Name of the disposed object.
         */
        explicit ObjectDisposedException(const char* objectName);

        /**
         * @brief Initializes a new instance of the ObjectDisposedException class
         * with the name of the disposed object.
         *
         * @param objectName Name of the disposed object.
         */
        explicit ObjectDisposedException(const std::string& objectName);

        /**
         * @brief Initializes a new instance of the ObjectDisposedException class
         * with the specified object name and message.
         *
         * @param objectName Name of the disposed object.
         * @param message Error message.
         */
        ObjectDisposedException(const char* objectName, const char* message);

        /**
         * @brief Initializes a new instance of the ObjectDisposedException class
         * with the specified object name and message.
         *
         * @param objectName Name of the disposed object.
         * @param message Error message.
         */
        ObjectDisposedException(const std::string& objectName, const std::string& message);

        /**
         * @brief Initializes a new instance with the specified message and inner exception.
         *
         * C++ counterpart of .NET ObjectDisposedException(string, Exception).
         * @param message Error message.
         * @param inner   The exception that caused this exception.
         */
        ObjectDisposedException(const std::string& message, std::exception_ptr inner);

        /**
         * @brief Gets the name of the disposed object.
         *
         * @return Object name, or empty string if not specified.
         */
        [[nodiscard]] const std::string& getObjectNameProperty() const;

        /**
         * @brief Throws ObjectDisposedException if the specified condition is true.
         *
         * @param condition Condition to evaluate.
         * @param objectName Name of the disposed object to include in the exception.
         */
        static void ThrowIf(bool condition, const char* objectName);

        /**
         * @brief Throws ObjectDisposedException if the specified condition is true.
         *
         * @param condition Condition to evaluate.
         * @param objectName Name of the disposed object to include in the exception.
         */
        static void ThrowIf(bool condition, const std::string& objectName);
    };

} // namespace System
