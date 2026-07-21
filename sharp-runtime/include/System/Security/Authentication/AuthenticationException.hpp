// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/SystemException.hpp"

namespace System::Security::Authentication {

    /**
     * @brief Thrown when authentication fails (e.g. from SslStream/NegotiateStream). The
     * authentication process can be retried with different parameters.
     *
     * C++ counterpart of .NET System.Security.Authentication.AuthenticationException.
     */
    class AuthenticationException : public System::SystemException {
    public:
        AuthenticationException() = default;
        explicit AuthenticationException(const std::string& message) : SystemException(message) {}
        AuthenticationException(const std::string& message, std::exception_ptr innerException)
            : SystemException(message, innerException) {}
    };

} // namespace System::Security::Authentication
