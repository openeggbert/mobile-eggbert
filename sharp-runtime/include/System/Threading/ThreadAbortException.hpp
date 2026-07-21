// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/SystemException.hpp"

namespace System::Threading {

    /**
     * @brief The exception thrown when Thread.Abort() is invoked on a thread.
     *
     * C++ counterpart of .NET System.Threading.ThreadAbortException.
     * In .NET 5+, Thread.Abort() always throws PlatformNotSupportedException.
     * This type exists for source-compatibility with pre-.NET 5 code.
     */
    class ThreadAbortException : public System::SystemException {
    public:
        /** @brief Initializes a ThreadAbortException with a default message. */
        ThreadAbortException() : SystemException("Thread was being aborted.") {}
        /**
         * @brief Initializes a ThreadAbortException with the specified message.
         * @param message The error message.
         */
        explicit ThreadAbortException(const std::string& message) : SystemException(message) {}

        /**
         * @brief Gets an object that contains application-specific information related to the thread abort.
         *
         * C++ counterpart of .NET ThreadAbortException.ExceptionState. Always null in this runtime,
         * since Thread.Abort() is not supported (matching .NET 5+ behavior).
         */
        [[nodiscard]] void* getExceptionStateProperty() const noexcept { return nullptr; }
    };

} // namespace System::Threading
