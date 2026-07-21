// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/SystemException.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief The exception thrown when an error occurs during a cryptographic operation.
     *
     * C++ counterpart of .NET System.Security.Cryptography.CryptographicException.
     */
    class CryptographicException : public System::SystemException {
    public:
        /** @brief Initializes a new instance with a default message. */
        CryptographicException() : System::SystemException("Error occurred during a cryptographic operation.") {}

        /** @brief Initializes a new instance with the specified native error code as its HResult. */
        explicit CryptographicException(SharpRuntime::intcs hr)
            : System::SystemException("Error occurred during a cryptographic operation.") {
            setHResultProperty(hr);
        }

        /** @brief Initializes a new instance with the specified message. */
        explicit CryptographicException(const std::string& message) : System::SystemException(message) {}

        /** @brief Initializes a new instance with the specified message and inner exception. */
        CryptographicException(const std::string& message, std::exception_ptr innerException)
            : System::SystemException(message, std::move(innerException)) {}
    };

} // namespace System::Security::Cryptography
