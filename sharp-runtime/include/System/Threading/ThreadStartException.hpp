// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/SystemException.hpp"

namespace System::Threading {

    /**
     * @brief The exception thrown when a failure occurs in a managed thread after the
     * underlying operating system thread has been started but before the thread is
     * ready to execute user code.
     *
     * C++ counterpart of .NET System.Threading.ThreadStartException.
     * Constructors are internal in .NET — only the runtime creates instances.
     */
    class ThreadStartException : public System::SystemException {
    public:
        /** @brief Initializes a ThreadStartException with a default message. */
        ThreadStartException()
            : SystemException("Thread failed to start.") {}
        /**
         * @brief Initializes a ThreadStartException with the specified message.
         * @param message The error message.
         */
        explicit ThreadStartException(const std::string& message)
            : SystemException(message) {}
        /**
         * @brief Initializes a ThreadStartException with a message and an inner exception.
         * @param message The error message.
         * @param inner   The exception that caused this exception.
         */
        ThreadStartException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {}
    };

} // namespace System::Threading
