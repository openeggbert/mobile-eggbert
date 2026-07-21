// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/SystemException.hpp"

namespace System::Threading {

    /**
     * @brief The exception thrown when a thread is interrupted while it is in a waiting state.
     *
     * C++ counterpart of .NET System.Threading.ThreadInterruptedException.
     */
    class ThreadInterruptedException : public System::SystemException {
    public:
        /** @brief Initializes a ThreadInterruptedException with a default message. */
        ThreadInterruptedException() : SystemException("Thread was interrupted from a waiting state.") {}
        /**
         * @brief Initializes a ThreadInterruptedException with the specified message.
         * @param message The error message.
         */
        explicit ThreadInterruptedException(const std::string& message) : SystemException(message) {}
        /**
         * @brief Initializes a ThreadInterruptedException with a message and an inner exception.
         * @param message The error message.
         * @param inner   The exception that caused this exception.
         */
        ThreadInterruptedException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {}
    };

} // namespace System::Threading
