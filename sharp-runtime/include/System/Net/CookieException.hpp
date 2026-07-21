// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/FormatException.hpp"

namespace System::Net {

    /**
     * @brief The exception thrown as a result of a failure while accessing the properties of a
     * System::Net::Cookie object, or while parsing cookie header text.
     *
     * C++ counterpart of .NET System.Net.CookieException. Derives from FormatException.
     */
    class CookieException : public FormatException {
    public:
        /** @brief Initializes a new instance with the default message. */
        CookieException() : FormatException("Invalid cookie.") {}
        /** @brief Initializes a new instance with the specified error message. */
        explicit CookieException(const std::string& message) : FormatException(message) {}
        /** @brief Initializes a new instance with the specified message and inner exception. */
        CookieException(const std::string& message, std::exception_ptr inner)
            : FormatException(message, std::move(inner)) {}
    };

} // namespace System::Net
