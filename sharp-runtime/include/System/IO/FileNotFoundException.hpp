// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "System/IO/IOException.hpp"

namespace System::IO {

    /**
     * @brief The exception that is thrown when an attempt to access a file
     * that does not exist on disk fails.
     *
     * @note Status: Implemented
     */
    class FileNotFoundException : public IOException {
    private:
        std::string fileName_;

    public:
        /** Initializes a FileNotFoundException with a default message. */
        FileNotFoundException();
        /** Initializes a FileNotFoundException with the specified C-string message. */
        explicit FileNotFoundException(const char* message);
        /** Initializes a FileNotFoundException with the specified message. */
        explicit FileNotFoundException(const std::string& message);
        /** Initializes a FileNotFoundException with a message and the name of the file that was not found. */
        FileNotFoundException(const std::string& message, const std::string& fileName);
        /** Initializes a FileNotFoundException with a message and an inner exception. */
        FileNotFoundException(const std::string& message, std::exception_ptr inner);
        /** Initializes a FileNotFoundException with a message, the file name, and an inner exception. */
        FileNotFoundException(const std::string& message, const std::string& fileName, std::exception_ptr inner);

        /** Returns the name of the file that was not found. */
        [[nodiscard]] const std::string& getFileNameProperty() const { return fileName_; }
    };

} // namespace System::IO
