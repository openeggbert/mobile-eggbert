// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "System/IO/IOException.hpp"

namespace System::IO {

    /**
     * @brief The exception that is thrown when reading is attempted past the
     * end of a stream.
     *
     * @note Status: Implemented
     */
    class EndOfStreamException : public IOException {
    public:
        /** Initializes an EndOfStreamException with a default message. */
        EndOfStreamException();
        /** Initializes an EndOfStreamException with the specified C-string message. */
        explicit EndOfStreamException(const char* message);
        /** Initializes an EndOfStreamException with the specified message. */
        explicit EndOfStreamException(const std::string& message);
        /** Initializes an EndOfStreamException with a message and an inner exception. */
        EndOfStreamException(const std::string& message, std::exception_ptr inner);
    };

} // namespace System::IO
