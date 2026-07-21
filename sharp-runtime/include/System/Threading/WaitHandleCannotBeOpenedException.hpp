// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/ApplicationException.hpp"

namespace System::Threading {

    /** The exception thrown when an attempt is made to open a system mutex, semaphore, or event wait handle that does not exist. */
    class WaitHandleCannotBeOpenedException : public System::ApplicationException {
    public:
        /** Initializes a WaitHandleCannotBeOpenedException with a default message. */
        WaitHandleCannotBeOpenedException() : ApplicationException("No handle of the given name exists.") {}
        /** Initializes a WaitHandleCannotBeOpenedException with the specified message. */
        explicit WaitHandleCannotBeOpenedException(const std::string& message) : ApplicationException(message) {}
        /** Initializes a WaitHandleCannotBeOpenedException with a message and an inner exception. */
        WaitHandleCannotBeOpenedException(const std::string& message, std::exception_ptr inner)
            : ApplicationException(message, std::move(inner)) {}
    };

} // namespace System::Threading
