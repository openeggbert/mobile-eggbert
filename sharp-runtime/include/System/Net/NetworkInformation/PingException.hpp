// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/InvalidOperationException.hpp"

namespace System::Net::NetworkInformation {

    /**
     * @brief The exception thrown when a Ping request fails.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.PingException.
     */
    class PingException : public System::InvalidOperationException {
    public:
        /** @brief Constructs with the given message. */
        explicit PingException(const std::string& message) : InvalidOperationException(message) {}

        /** @brief Constructs with the given message and inner exception. */
        PingException(const std::string& message, std::exception_ptr innerException)
            : InvalidOperationException(message, innerException) {}
    };

} // namespace System::Net::NetworkInformation
