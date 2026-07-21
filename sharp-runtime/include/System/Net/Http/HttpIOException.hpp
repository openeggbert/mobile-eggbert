// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/IO/IOException.hpp"
#include "System/Net/Http/HttpRequestError.hpp"

namespace System::Net::Http {

    /**
     * @brief An exception thrown when an error occurs while reading the response.
     *
     * C++ counterpart of .NET System.Net.Http.HttpIOException.
     */
    class HttpIOException : public System::IO::IOException {
        HttpRequestError httpRequestError_;

        static std::string httpRequestErrorToString(HttpRequestError error) {
            switch (error) {
                case HttpRequestError::Unknown:                     return "Unknown";
                case HttpRequestError::NameResolutionError:         return "NameResolutionError";
                case HttpRequestError::ConnectionError:             return "ConnectionError";
                case HttpRequestError::SecureConnectionError:       return "SecureConnectionError";
                case HttpRequestError::HttpProtocolError:           return "HttpProtocolError";
                case HttpRequestError::ExtendedConnectNotSupported: return "ExtendedConnectNotSupported";
                case HttpRequestError::VersionNegotiationError:     return "VersionNegotiationError";
                case HttpRequestError::UserAuthenticationError:     return "UserAuthenticationError";
                case HttpRequestError::ProxyTunnelError:            return "ProxyTunnelError";
                case HttpRequestError::InvalidResponse:             return "InvalidResponse";
                case HttpRequestError::ResponseEnded:               return "ResponseEnded";
                case HttpRequestError::ConfigurationLimitExceeded:  return "ConfigurationLimitExceeded";
                default:                                            return "Unknown";
            }
        }

        static std::string combineMessage(const std::string& message, HttpRequestError error) {
            // When no message is supplied, real .NET's base Exception.Message falls back to a
            // generic "Exception of type '...' was thrown." rather than an empty string; this
            // port instead reuses IOException's own default text (matching the argless
            // IOException() constructor) since it is more informative and this project doesn't
            // chase verbatim fallback-message parity. Without this fallback, an empty `message`
            // produced a bare " (Unknown)" string with no base text at all.
            const std::string& base_ = message.empty() ? std::string("I/O error occurred.") : message;
            return base_ + " (" + httpRequestErrorToString(error) + ")";
        }

    public:
        /**
         * @brief Initializes a new instance of the HttpIOException class.
         * @param httpRequestError The HttpRequestError that caused the exception.
         * @param message          The message string describing the error.
         * @param innerException   The exception that is the cause of the current exception.
         *
         * @note .NET's Message property appends " ({HttpRequestError})" to the base message
         * dynamically via an override; here the combined string is computed once at construction
         * and stored directly, since this runtime's Exception::what()/getMessageProperty() read a
         * private field rather than dispatching through a virtual Message getter.
         */
        explicit HttpIOException(HttpRequestError httpRequestError, const std::string& message = "",
                                  std::exception_ptr innerException = nullptr)
            : System::IO::IOException(combineMessage(message, httpRequestError), innerException),
              httpRequestError_(httpRequestError) {}

        /** @return The HttpRequestError that caused the exception. */
        [[nodiscard]] HttpRequestError getHttpRequestErrorProperty() const { return httpRequestError_; }
    };

} // namespace System::Net::Http
