// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/MissingMemberException.hpp"

namespace System {

    /**
     * @brief The exception thrown when there is an attempt to dynamically access a field
     * that does not exist.
     *
     * C++ counterpart of .NET System.MissingFieldException.
     */
    class MissingFieldException : public MissingMemberException {
    public:
        /**
         * @brief Initializes a new instance with the default missing-field message.
         *
         * C++ counterpart of .NET MissingFieldException().
         */
        MissingFieldException() : MissingMemberException("Attempted to access a field that does not exist.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131511)); // COR_E_MISSINGFIELD
        }

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET MissingFieldException(string).
         */
        explicit MissingFieldException(const std::string& message) : MissingMemberException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131511)); // COR_E_MISSINGFIELD
        }

        /**
         * @brief Initializes a new instance with the class and field name.
         *
         * C++ counterpart of .NET MissingFieldException(string className, string fieldName).
         * Matches .NET's dynamic Message format ("Field '{ClassName}.{FieldName}' not found.").
         */
        MissingFieldException(const std::string& className, const std::string& fieldName)
            : MissingMemberException("Field '" + className + "." + fieldName + "' not found.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131511)); // COR_E_MISSINGFIELD
        }

        /**
         * @brief Initializes a new instance with the specified message and inner exception.
         *
         * C++ counterpart of .NET MissingFieldException(string, Exception).
         */
        MissingFieldException(const std::string& message, std::exception_ptr innerException)
            : MissingMemberException(message, std::move(innerException)) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131511)); // COR_E_MISSINGFIELD
        }
    };

} // namespace System
