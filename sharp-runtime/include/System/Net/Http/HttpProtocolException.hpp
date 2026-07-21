// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/Net/Http/HttpIOException.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http {

    /**
     * @brief The exception thrown when an HTTP/2 or an HTTP/3 protocol error occurs.
     *
     * C++ counterpart of .NET System.Net.Http.HttpProtocolException.
     *
     * @note This runtime's HttpClient only supports HTTP/1.1 over plain TCP (see HttpClient's
     * class doc comment), so nothing in this runtime currently throws this exception; it exists
     * for API-surface completeness and for future HTTP/2/HTTP/3 work. .NET's internal
     * CreateHttp2StreamException/CreateHttp3ConnectionException/etc. factory methods are not
     * ported: they depend on HTTP/2 and HTTP/3 protocol machinery (Http2ProtocolErrorCode,
     * Http3ErrorCode, QUIC) this runtime doesn't implement.
     */
    class HttpProtocolException : public HttpIOException {
        SharpRuntime::longcs errorCode_;

    public:
        /**
         * @brief Initializes a new instance with the specified error code, message, and inner exception.
         * @param errorCode      The HTTP/2 or HTTP/3 error code.
         * @param message        The error message that explains the reason for the exception.
         * @param innerException The exception that is the cause of the current exception.
         */
        HttpProtocolException(SharpRuntime::longcs errorCode, const std::string& message, std::exception_ptr innerException)
            : HttpIOException(HttpRequestError::HttpProtocolError, message, innerException),
              errorCode_(errorCode) {}

        /** @return The HTTP/2 or HTTP/3 error code associated with this exception. */
        [[nodiscard]] SharpRuntime::longcs getErrorCodeProperty() const { return errorCode_; }
    };

} // namespace System::Net::Http
