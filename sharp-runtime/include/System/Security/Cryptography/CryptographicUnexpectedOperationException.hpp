// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/Security/Cryptography/CryptographicException.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief The exception thrown when an unexpected operation occurs during a cryptographic
     * transform (e.g. reading the hash value before finalizing it).
     *
     * C++ counterpart of .NET System.Security.Cryptography.CryptographicUnexpectedOperationException.
     */
    class CryptographicUnexpectedOperationException : public CryptographicException {
    public:
        CryptographicUnexpectedOperationException()
            : CryptographicException("Error occurred during a cryptographic operation.") {}
        explicit CryptographicUnexpectedOperationException(const std::string& message)
            : CryptographicException(message) {}
        CryptographicUnexpectedOperationException(const std::string& message, std::exception_ptr innerException)
            : CryptographicException(message, std::move(innerException)) {}
    };

} // namespace System::Security::Cryptography
