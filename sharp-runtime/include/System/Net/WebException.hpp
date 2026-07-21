// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/InvalidOperationException.hpp"
#include "System/Net/WebExceptionStatus.hpp"

namespace System::Net {

    /**
     * The exception that is thrown when an error occurs while accessing a resource
     * using a WebRequest-derived or WebClient-derived API.
     *
     * C++ counterpart of .NET System.Net.WebException.
     *
     * @note .NET's Response property (a WebResponse?) is not ported: WebRequest/WebResponse
     * are legacy pre-HttpClient APIs and are out of scope in this runtime (see plan.sqlite3 —
     * classified ignore, superseded by the already-ported System::Net::Http::HttpClient).
     */
    class WebException : public System::InvalidOperationException {
        WebExceptionStatus status_ = WebExceptionStatus::UnknownError;

    public:
        /** Creates a new instance with no message and Status = UnknownError. */
        WebException() = default;

        /** Creates a new instance with the specified message. */
        explicit WebException(const std::string& message) : System::InvalidOperationException(message) {}

        /** Creates a new instance with a message and inner exception. */
        WebException(const std::string& message, std::exception_ptr innerException)
            : System::InvalidOperationException(message, innerException) {}

        /** Creates a new instance with a message and status. */
        WebException(const std::string& message, WebExceptionStatus status)
            : System::InvalidOperationException(message), status_(status) {}

        /** Creates a new instance with a message, inner exception, and status. */
        WebException(const std::string& message, std::exception_ptr innerException, WebExceptionStatus status)
            : System::InvalidOperationException(message, innerException), status_(status) {}

        /** @return The status of this exception. */
        [[nodiscard]] WebExceptionStatus getStatusProperty() const { return status_; }
    };

} // namespace System::Net
