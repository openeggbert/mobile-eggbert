// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/IO/IOException.hpp"

namespace System::IO {

    /** The exception thrown when a path or file name is too long. */
    class PathTooLongException : public IOException {
    public:
        /** Initializes a PathTooLongException with a default message. */
        PathTooLongException() : IOException("The specified file name or path is too long, or a component of the specified path is too long.") {}
        /** Initializes a PathTooLongException with the specified message. */
        explicit PathTooLongException(const std::string& message) : IOException(message) {}
        /** Initializes a PathTooLongException with a message and an inner exception. */
        PathTooLongException(const std::string& message, std::exception_ptr inner)
            : IOException(message, std::move(inner)) {}
    };

} // namespace System::IO
