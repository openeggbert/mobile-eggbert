// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Exception.hpp"

namespace System::Diagnostics {

    /**
     * @brief Exception thrown when the program executes an instruction that was thought to be
     * unreachable.
     *
     * C++ counterpart of .NET System.Diagnostics.UnreachableException.
     */
    class UnreachableException : public System::Exception {
    public:
        /** @brief Initializes a new instance with the default error message. */
        UnreachableException() : Exception("The program executed an instruction that was thought to be unreachable.") {}
        /** @brief Initializes a new instance with a specified error message. */
        explicit UnreachableException(const std::string& message) : Exception(message) {}
        /** @brief Initializes a new instance with a specified error message and inner exception. */
        UnreachableException(const std::string& message, std::exception_ptr inner)
            : Exception(message, std::move(inner)) {}
    };

} // namespace System::Diagnostics
