// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/IO/IOException.hpp"
namespace System::IO {
    /** @brief The exception thrown when part of a file or directory cannot be found. @note Status: Implemented */
    class DirectoryNotFoundException : public IOException {
        std::string directoryPath_;
    public:
        /** Initializes a new DirectoryNotFoundException with a default message. */
        DirectoryNotFoundException();
        /** Initializes a new DirectoryNotFoundException with the specified message. */
        explicit DirectoryNotFoundException(const char* message);
        /** Initializes a new DirectoryNotFoundException with the specified message. */
        explicit DirectoryNotFoundException(const std::string& message);
        /** Initializes a DirectoryNotFoundException with a message and an inner exception. */
        DirectoryNotFoundException(const std::string& message, std::exception_ptr inner);
        /** Initializes a DirectoryNotFoundException with a message and the directory path that could not be found. */
        DirectoryNotFoundException(const std::string& message, const std::string& directoryPath);

        /** @return The directory path that could not be found, or an empty string if not set. */
        [[nodiscard]] const std::string& getDirectoryPathProperty() const { return directoryPath_; }
    };
} // namespace System::IO
