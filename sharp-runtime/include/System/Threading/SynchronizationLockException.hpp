// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/SystemException.hpp"

namespace System::Threading {

    /** The exception thrown when a synchronisation method is invoked from an unsynchronised block of code. */
    class SynchronizationLockException : public System::SystemException {
    public:
        /** Initializes a SynchronizationLockException with a default message. */
        SynchronizationLockException() : SystemException("Object synchronization method was called from an unsynchronized block of code.") {}
        /** Initializes a SynchronizationLockException with the specified message. */
        explicit SynchronizationLockException(const std::string& message) : SystemException(message) {}
        /** Initializes a SynchronizationLockException with a message and an inner exception. */
        SynchronizationLockException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {}
    };

} // namespace System::Threading
