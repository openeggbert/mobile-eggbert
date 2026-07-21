// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Security/Authentication/AuthenticationException.hpp"

namespace System::Security::Authentication {

    /**
     * @brief Thrown when authentication fails due to invalid credentials, and is expected to
     * fail again if retried on the same underlying stream.
     *
     * C++ counterpart of .NET System.Security.Authentication.InvalidCredentialException.
     */
    class InvalidCredentialException : public AuthenticationException {
    public:
        InvalidCredentialException() = default;
        explicit InvalidCredentialException(const std::string& message) : AuthenticationException(message) {}
        InvalidCredentialException(const std::string& message, std::exception_ptr innerException)
            : AuthenticationException(message, innerException) {}
    };

} // namespace System::Security::Authentication
