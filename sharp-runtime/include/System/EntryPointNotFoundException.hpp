// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include "System/TypeLoadException.hpp"

namespace System {

    /**
     * @brief The exception that is thrown when an attempt to load a class fails
     * because the entry point was not found.
     *
     * C++ counterpart of .NET System.EntryPointNotFoundException.
     */
    class EntryPointNotFoundException : public TypeLoadException {
    public:
        /** @brief Initializes a new instance with the default entry-point-not-found message. */
        EntryPointNotFoundException() : TypeLoadException("Entry point was not found.") {}

        /** @brief Initializes a new instance with the specified message. */
        explicit EntryPointNotFoundException(const std::string& message) : TypeLoadException(message) {}

        /**
         * @brief Initializes a new instance with a message and an inner exception.
         *
         * C++ counterpart of .NET EntryPointNotFoundException(string, Exception).
         */
        EntryPointNotFoundException(const std::string& message, std::exception_ptr inner)
            : TypeLoadException(message, std::move(inner)) {}
    };

} // namespace System
