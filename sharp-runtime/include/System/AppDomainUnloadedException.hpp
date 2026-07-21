// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/SystemException.hpp"

namespace System {

    /**
     * @brief The exception thrown when an attempt is made to access an unloaded application domain.
     *
     * C++ counterpart of .NET System.AppDomainUnloadedException.
     */
    class AppDomainUnloadedException : public SystemException {
    public:
        /** @brief Initializes a new instance with the default message. */
        AppDomainUnloadedException() : SystemException("Attempted to access an unloaded AppDomain.") {}
        /** @brief Initializes a new instance with the specified error message. */
        explicit AppDomainUnloadedException(const std::string& message) : SystemException(message) {}
        /** @brief Initializes a new instance with the specified message and inner exception. */
        AppDomainUnloadedException(const std::string& message, std::exception_ptr inner)
            : SystemException(message, std::move(inner)) {}
    };

} // namespace System
