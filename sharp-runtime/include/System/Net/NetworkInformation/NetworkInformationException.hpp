// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cerrno>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ComponentModel/Win32Exception.hpp"

namespace System::Net::NetworkInformation {

    using SharpRuntime::intcs;

    /**
     * @brief Provides NetworkInformation exceptions to the application.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.NetworkInformationException.
     *
     * @note .NET's default constructor uses `Marshal.GetLastPInvokeError()`; this runtime has no
     * P/Invoke layer, so the practical POSIX equivalent — the current `errno` — is used instead.
     * @note The internal `(string message, Exception innerException)` constructor isn't
     * reproduced: `Win32Exception` (the base class) has no inner-exception-carrying constructor to
     * forward to.
     */
    class NetworkInformationException : public System::ComponentModel::Win32Exception {
    public:
        /** @brief Constructs with the current `errno` as the native error code. */
        NetworkInformationException() : Win32Exception(errno) {}

        /** @brief Constructs with the given native error code. */
        explicit NetworkInformationException(intcs errorCode) : Win32Exception(errorCode) {}

        /** @brief Constructs with a custom message and no specific native error code. */
        explicit NetworkInformationException(const std::string& message) : Win32Exception(0, message) {}

        /** @return The native error code (same as getNativeErrorCodeProperty()). */
        [[nodiscard]] intcs getErrorCodeProperty() const { return getNativeErrorCodeProperty(); }
    };

} // namespace System::Net::NetworkInformation
