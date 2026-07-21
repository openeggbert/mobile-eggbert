// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/SystemException.hpp"

namespace System::Threading {

    /**
     * @brief The exception thrown when a method is invoked on a Thread that is in an invalid ThreadState.
     *
     * C++ counterpart of .NET System.Threading.ThreadStateException.
     */
    class ThreadStateException : public System::SystemException {
    public:
        /** @brief Initializes a ThreadStateException with a default message. */
        ThreadStateException() : SystemException("Thread is in an invalid state for the operation being executed.") {}
        /**
         * @brief Initializes a ThreadStateException with the specified message.
         * @param message The error message.
         */
        explicit ThreadStateException(const std::string& message) : SystemException(message) {}
        /**
         * @brief Initializes a ThreadStateException with a message and an inner exception.
         * @param message The error message.
         * @param inner   The exception that caused this exception.
         */
        ThreadStateException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {}
    };

} // namespace System::Threading
