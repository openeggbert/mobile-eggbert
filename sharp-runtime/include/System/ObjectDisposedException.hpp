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
