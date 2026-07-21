// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception thrown when a program contains invalid IL code or metadata.
     *
     * C++ counterpart of .NET System.InvalidProgramException.
     */
    class InvalidProgramException final : public SystemException {
    public:
        /**
         * @brief Initializes a new instance with the default invalid-program message.
         *
         * C++ counterpart of .NET InvalidProgramException().
         */
        InvalidProgramException()
            : SystemException("Common Language Runtime detected an invalid program.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013153A)); // COR_E_INVALIDPROGRAM
        }

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET InvalidProgramException(string).
         * @param message A description of the error.
         */
        explicit InvalidProgramException(const std::string& message)
            : SystemException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013153A)); // COR_E_INVALIDPROGRAM
        }

        /**
         * @brief Initializes a new instance with a specified error message and a
         * reference to the inner exception that is the cause of this exception.
         *
         * C++ counterpart of .NET InvalidProgramException(string, Exception).
         * @param message The error message that explains the reason for the exception.
         * @param inner   The exception that is the cause of the current exception.
         */
        InvalidProgramException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x8013153A)); // COR_E_INVALIDPROGRAM
        }
    };

} // namespace System
