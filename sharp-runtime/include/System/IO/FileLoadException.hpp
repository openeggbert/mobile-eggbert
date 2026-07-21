// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/IO/IOException.hpp"

namespace System::IO {

    /** The exception thrown when a managed assembly is found but cannot be loaded. */
    class FileLoadException : public IOException {
        std::string fileName_;
    public:
        /** Initializes a FileLoadException with a default message. */
        FileLoadException() : IOException("Could not load the specified file.") {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131621)); // COR_E_FILELOAD
        }
        /** Initializes a FileLoadException with the specified message. */
        explicit FileLoadException(const std::string& message) : IOException(message) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131621)); // COR_E_FILELOAD
        }
        /** Initializes a FileLoadException with a message and the name of the file that could not be loaded. */
        FileLoadException(const std::string& message, const std::string& fileName)
            : IOException(message), fileName_(fileName) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131621)); // COR_E_FILELOAD
        }
        /** Initializes a FileLoadException with a message and an inner exception. */
        FileLoadException(const std::string& message, std::exception_ptr inner)
            : IOException(message, std::move(inner)) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131621)); // COR_E_FILELOAD
        }
        /** Initializes a FileLoadException with a message, the file name, and an inner exception. */
        FileLoadException(const std::string& message, const std::string& fileName, std::exception_ptr inner)
            : IOException(message, std::move(inner)), fileName_(fileName) {
            setHResultProperty(static_cast<SharpRuntime::intcs>(0x80131621)); // COR_E_FILELOAD
        }

        /** Returns the name of the file that could not be loaded. */
        [[nodiscard]] const std::string& getFileNameProperty() const { return fileName_; }
    };

} // namespace System::IO
