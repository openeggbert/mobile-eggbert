// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <exception>
#include <string>

#include "System/SystemException.hpp"

namespace System::IO {

    /**
     * @brief The exception that is thrown when an I/O error occurs.
     *
     * @note Status: Implemented
     */
    class IOException : public System::SystemException {
    public:
        /** Initializes an IOException with the specified C-string message. */
        explicit IOException(const char* message);
        /** Initializes an IOException with the specified message. */
        explicit IOException(const std::string& message);
    };

} // namespace System::IO
