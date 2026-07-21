// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include "System/FormatException.hpp"
#include "System/Uri.hpp"

namespace System::IO {

    /**
     * @brief The exception thrown when an input file or data stream that is supposed to
     * conform to a certain file format specification is malformed.
     *
     * C++ counterpart of .NET System.IO.FileFormatException.
     * Unlike most `System::IO` exceptions, this derives from FormatException, not IOException,
     * matching .NET exactly.
     */
    class FileFormatException : public System::FormatException {
        std::shared_ptr<System::Uri> sourceUri_;

    public:
        /** Initializes a FileFormatException with a default message. */
        FileFormatException() : System::FormatException("Invalid file format.") {}

        /** Initializes a FileFormatException with the specified message. */
        explicit FileFormatException(const std::string& message) : System::FormatException(message) {}

        /** Initializes a FileFormatException with a message and an inner exception. */
        FileFormatException(const std::string& message, std::exception_ptr inner)
            : System::FormatException(message, std::move(inner)) {}

        /**
         * @brief Initializes a FileFormatException with a system-supplied message that
         * includes the given file URI.
         * @param sourceUri The URI of the file that caused this error.
         */
        explicit FileFormatException(const System::Uri& sourceUri)
            : System::FormatException("File '" + sourceUri.ToString() + "' has an invalid file format."),
              sourceUri_(std::make_shared<System::Uri>(sourceUri)) {}

        /**
         * @brief Initializes a FileFormatException with the specified message and source URI.
         * @param sourceUri The URI of the file that caused this error.
         * @param message   The message that describes the error.
         */
        FileFormatException(const System::Uri& sourceUri, const std::string& message)
            : System::FormatException(message), sourceUri_(std::make_shared<System::Uri>(sourceUri)) {}

        /**
         * @brief Initializes a FileFormatException with a system-supplied message that
         * includes the given file URI, and an inner exception.
         * @param sourceUri      The URI of the file that caused this error.
         * @param innerException The exception that is the cause of the current exception.
         */
        FileFormatException(const System::Uri& sourceUri, std::exception_ptr innerException)
            : System::FormatException("File '" + sourceUri.ToString() + "' has an invalid file format.",
                                       std::move(innerException)),
              sourceUri_(std::make_shared<System::Uri>(sourceUri)) {}

        /**
         * @brief Initializes a FileFormatException with the specified message, source URI, and inner exception.
         * @param sourceUri      The URI of the file that caused this error.
         * @param message        The message that describes the error.
         * @param innerException The exception that is the cause of the current exception.
         */
        FileFormatException(const System::Uri& sourceUri, const std::string& message, std::exception_ptr innerException)
            : System::FormatException(message, std::move(innerException)),
              sourceUri_(std::make_shared<System::Uri>(sourceUri)) {}

        /**
         * @brief Gets the URI of the file that caused this exception.
         *
         * C++ counterpart of .NET FileFormatException.SourceUri.
         * @return A pointer to the source URI, or nullptr if none was set.
         */
        [[nodiscard]] const System::Uri* getSourceUriProperty() const { return sourceUri_.get(); }
    };

} // namespace System::IO
