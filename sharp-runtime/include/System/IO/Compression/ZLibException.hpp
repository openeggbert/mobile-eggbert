// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <utility>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/IO/IOException.hpp"

namespace System::IO::Compression {

    using SharpRuntime::intcs;

    /**
     * @brief The exception that is thrown when ZLib returns an error code indicating an
     * unrecoverable error.
     *
     * C++ counterpart of .NET System.IO.Compression.ZLibException.
     */
    class ZLibException : public System::IO::IOException {
    private:
        std::string zlibErrorContext_;
        intcs       zlibErrorCode_ = 0;
        std::string zlibErrorMessage_;

    public:
        /** Initializes a ZLibException with a default message. */
        ZLibException() = default;

        /** Initializes a ZLibException with the specified message. */
        explicit ZLibException(const std::string& message) : System::IO::IOException(message) {}

        /** Initializes a ZLibException with a message and an inner exception. */
        ZLibException(const std::string& message, std::exception_ptr inner)
            : System::IO::IOException(message, std::move(inner)) {}

        /**
         * @brief The preferred constructor to use.
         * @param message A human-readable error description.
         * @param zlibErrorContext A description of the context within zlib where the error occurred (e.g. the function name).
         * @param zlibErrorCode The error code returned by a ZLib function that caused this exception.
         * @param zlibErrorMessage The string provided by ZLib as error information (unlocalized).
         */
        ZLibException(const std::string& message, const std::string& zlibErrorContext,
                       intcs zlibErrorCode, const std::string& zlibErrorMessage)
            : System::IO::IOException(message),
              zlibErrorContext_(zlibErrorContext),
              zlibErrorCode_(zlibErrorCode),
              zlibErrorMessage_(zlibErrorMessage) {}

        /** Returns a description of the context within zlib where the error occurred. */
        [[nodiscard]] const std::string& getZlibErrorContextProperty() const { return zlibErrorContext_; }
        /** Returns the error code returned by the ZLib function that caused this exception. */
        [[nodiscard]] intcs getZlibErrorCodeProperty() const { return zlibErrorCode_; }
        /** Returns the string provided by ZLib as error information (unlocalized). */
        [[nodiscard]] const std::string& getZlibErrorMessageProperty() const { return zlibErrorMessage_; }
    };

} // namespace System::IO::Compression
