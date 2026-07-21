// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/Security/Cryptography/CryptographicException.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Thrown when a decryption operation with an authenticated cipher (e.g. AesGcm,
     * AesCcm, ChaCha20Poly1305) has an authentication tag mismatch.
     *
     * C++ counterpart of .NET System.Security.Cryptography.AuthenticationTagMismatchException.
     */
    class AuthenticationTagMismatchException final : public CryptographicException {
    public:
        AuthenticationTagMismatchException() : CryptographicException("The computed authentication tag did not match the input authentication tag.") {}
        explicit AuthenticationTagMismatchException(const std::string& message) : CryptographicException(message) {}
        AuthenticationTagMismatchException(const std::string& message, std::exception_ptr innerException)
            : CryptographicException(message, std::move(innerException)) {}
    };

} // namespace System::Security::Cryptography
