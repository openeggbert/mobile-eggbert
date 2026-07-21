// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/SystemException.hpp"

namespace System::Threading {

    /** The exception thrown when the Release method is called on a semaphore whose count is already at the maximum. */
    class SemaphoreFullException : public System::SystemException {
    public:
        /** Initializes a SemaphoreFullException with a default message. */
        SemaphoreFullException() : SystemException("Adding the specified count to the semaphore would cause it to exceed its maximum count.") {}
        /** Initializes a SemaphoreFullException with the specified message. */
        explicit SemaphoreFullException(const std::string& message) : SystemException(message) {}
        /** Initializes a SemaphoreFullException with a message and an inner exception. */
        SemaphoreFullException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {}
    };

} // namespace System::Threading
