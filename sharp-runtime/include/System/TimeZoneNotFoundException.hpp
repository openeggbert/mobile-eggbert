// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Exception.hpp"

namespace System {

    /** @brief The exception that is thrown when a time zone cannot be found. */
    class TimeZoneNotFoundException : public Exception {
    public:
        /** @brief Initializes a new TimeZoneNotFoundException with a default message. */
        TimeZoneNotFoundException() : Exception("The time zone could not be found.") {}
        /** @brief Initializes a new TimeZoneNotFoundException with the specified message. */
        explicit TimeZoneNotFoundException(const std::string& message) : Exception(message) {}
        /** @brief Initializes a new TimeZoneNotFoundException with a message and an inner exception. */
        TimeZoneNotFoundException(const std::string& message, std::exception_ptr inner)
            : Exception(message, std::move(inner)) {}
    };

} // namespace System
