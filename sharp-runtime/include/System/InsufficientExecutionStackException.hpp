// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception thrown when there is insufficient execution stack available
     * to allow most methods to execute.
     *
     * C++ counterpart of .NET System.InsufficientExecutionStackException.
     */
    class InsufficientExecutionStackException final : public SystemException {
    public:
        /**
         * @brief Initializes a new instance with the default insufficient-stack message.
         *
         * C++ counterpart of .NET InsufficientExecutionStackException().
         */
        InsufficientExecutionStackException()
            : SystemException("Insufficient stack to continue executing the program safely. This can happen from having too many functions on the call stack or function on the stack using too much stack space.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131578)); // COR_E_INSUFFICIENTEXECUTIONSTACK
        }

        /**
         * @brief Initializes a new instance with the specified error message.
         *
         * C++ counterpart of .NET InsufficientExecutionStackException(string).
         * @param message A description of the error.
         */
        explicit InsufficientExecutionStackException(const std::string& message)
            : SystemException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131578)); // COR_E_INSUFFICIENTEXECUTIONSTACK
        }

        /**
         * @brief Initializes a new instance with a specified error message and a
         * reference to the inner exception that is the cause of this exception.
         *
         * C++ counterpart of .NET InsufficientExecutionStackException(string, Exception).
         * @param message The error message that explains the reason for the exception.
         * @param inner   The exception that is the cause of the current exception.
         */
        InsufficientExecutionStackException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131578)); // COR_E_INSUFFICIENTEXECUTIONSTACK
        }
    };

} // namespace System
