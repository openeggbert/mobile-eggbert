// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/MissingMemberException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when there is an attempt to dynamically
     * access a method that does not exist.
     *
     * C++ counterpart of .NET System.MissingMethodException.
     */
    class MissingMethodException : public MissingMemberException {
    public:
        /**
         * @brief Initializes a new instance with the default message.
         *
         * C++ counterpart of .NET MissingMethodException().
         */
        MissingMethodException()
            : MissingMemberException("Attempted to access a method that does not exist.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131513)); // COR_E_MISSINGMETHOD
        }

        /**
         * @brief Initializes a new instance with the specified message.
         *
         * C++ counterpart of .NET MissingMethodException(string).
         */
        explicit MissingMethodException(const std::string& message)
            : MissingMemberException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131513)); // COR_E_MISSINGMETHOD
        }

        /**
         * @brief Initializes a new instance with a message and an inner exception.
         *
         * C++ counterpart of .NET MissingMethodException(string, Exception).
         */
        MissingMethodException(const std::string& message, std::exception_ptr innerException)
            : MissingMemberException(message, std::move(innerException)) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131513)); // COR_E_MISSINGMETHOD
        }

        /**
         * @brief Initializes a new instance with the class name and the method name.
         *
         * C++ counterpart of .NET MissingMethodException(string className, string methodName).
         * Matches .NET's dynamic Message format ("Method '{ClassName}.{MethodName}' not found.").
         */
        MissingMethodException(const std::string& className, const std::string& methodName)
            : MissingMemberException("Method '" + className + "." + methodName + "' not found.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131513)); // COR_E_MISSINGMETHOD
        }
    };

} // namespace System
