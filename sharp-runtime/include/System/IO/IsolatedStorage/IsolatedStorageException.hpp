// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <exception>
#include "System/Exception.hpp"

namespace System::IO::IsolatedStorage
{
    /**
     * @brief Represents errors related to isolated storage operations.
     *
     * @note Status: IMPLEMENTED
     */
    class IsolatedStorageException : public System::Exception
    {
    public:
        /** Initializes a new instance of the IsolatedStorageException class with a default message. */
        IsolatedStorageException();

        /**
         * @brief Initializes a new instance of the IsolatedStorageException class.
         * @param message Exception message.
         */
        explicit IsolatedStorageException(const std::string& message);

        /** Initializes a new instance of the IsolatedStorageException class with a message and an inner exception. */
        IsolatedStorageException(const std::string& message, std::exception_ptr inner);
    };
}