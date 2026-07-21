// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Exception.hpp"

namespace System::Threading {

    /** The exception thrown when recursive entry into a lock is not allowed by the lock's recursion policy. */
    class LockRecursionException : public System::Exception {
    public:
        /** Initializes a LockRecursionException with a default message. */
        LockRecursionException() : Exception("Recursive lock entry is not allowed.") {}
        /** Initializes a LockRecursionException with the specified message. */
        explicit LockRecursionException(const std::string& message) : Exception(message) {}
        /** Initializes a LockRecursionException with a message and an inner exception. */
        LockRecursionException(const std::string& message, std::exception_ptr inner)
            : Exception(message, std::move(inner)) {}
    };

} // namespace System::Threading
