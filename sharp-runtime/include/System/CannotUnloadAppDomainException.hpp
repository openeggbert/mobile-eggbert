// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception thrown when an attempt to unload an application domain fails.
     */
    class CannotUnloadAppDomainException : public SystemException {
    public:
        /** @brief Initializes a new instance with the default message. */
        CannotUnloadAppDomainException() : SystemException("Attempt to unload the AppDomain failed.") {}
        /** @brief Initializes a new instance with the specified error message. */
        explicit CannotUnloadAppDomainException(const std::string& message) : SystemException(message) {}
        /** @brief Initializes a new instance with the specified message and inner exception. */
        CannotUnloadAppDomainException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {}
    };

} // namespace System
