// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/IO/IOException.hpp"

namespace System::IO {

    /**
     * @brief The exception thrown when trying to access a drive that is not available.
     *
     * C++ counterpart of .NET System.IO.DriveNotFoundException.
     */
    class DriveNotFoundException : public IOException {
    public:
        /** Initializes a DriveNotFoundException with a default message. */
        DriveNotFoundException();
        /** Initializes a DriveNotFoundException with the specified C-string message. */
        explicit DriveNotFoundException(const char* message);
        /** Initializes a DriveNotFoundException with the specified message. */
        explicit DriveNotFoundException(const std::string& message);
        /** Initializes a DriveNotFoundException with a message and an inner exception. */
        DriveNotFoundException(const std::string& message, std::exception_ptr inner);
    };

} // namespace System::IO
