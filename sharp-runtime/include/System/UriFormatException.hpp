// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/FormatException.hpp"

namespace System {

    /**
     * @brief The exception thrown when an invalid Uniform Resource Identifier (URI) is detected.
     *
     * C++ counterpart of .NET System.UriFormatException.
     * Derives from FormatException.
     */
    class UriFormatException : public FormatException {
    public:
        /**
         * @brief Initializes a new instance with the default message.
         *
         * C++ counterpart of .NET UriFormatException(), which delegates to the base
         * FormatException() with no Uri-specific text ("One of the identified items
         * was in an invalid format.").
         */
        UriFormatException() : FormatException() {}
        /** @brief Initializes a new instance with the specified error message. */
        explicit UriFormatException(const std::string& message) : FormatException(message) {}
        /** @brief Initializes a new instance with the specified message and inner exception. */
        UriFormatException(const std::string& message, std::exception_ptr inner)
            : FormatException(message, std::move(inner)) {}
    };

} // namespace System
