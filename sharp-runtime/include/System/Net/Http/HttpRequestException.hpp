// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <optional>
#include <string>
#include "System/Exception.hpp"
#include "System/Net/HttpStatusCode.hpp"
#include "System/Net/Http/HttpRequestError.hpp"

namespace System::Net::Http {

    /**
     * @brief The exception thrown by HttpClient/HttpMessageInvoker to indicate an error
     * with the HTTP request or response.
     *
     * C++ counterpart of .NET System.Net.Http.HttpRequestException.
     */
    class HttpRequestException : public System::Exception {
        HttpRequestError httpRequestError_ = HttpRequestError::Unknown;
        std::optional<System::Net::HttpStatusCode> statusCode_;

    public:
        /** Creates a new instance with no message. */
        HttpRequestException() = default;

        /** Creates a new instance with the specified message. */
        explicit HttpRequestException(const std::string& message) : System::Exception(message) {}

        /**
         * @brief Creates a new instance with a message and inner exception.
         * @note Real .NET copies `inner.HResult` onto this exception's own HResult when
         * @p inner is non-null. This runtime doesn't propagate HResult from a `std::exception_ptr`
         * inner exception anywhere in the codebase (extracting it needs a rethrow/catch, and no
         * type in this port currently does that for inner exceptions), so this is a known,
         * project-wide gap rather than something specific to this one type.
         */
        HttpRequestException(const std::string& message, std::exception_ptr inner)
            : System::Exception(message, inner) {}

        /** Creates a new instance with a message, inner exception, and HTTP status code. */
        HttpRequestException(const std::string& message, std::exception_ptr inner, System::Net::HttpStatusCode statusCode)
            : System::Exception(message, inner), statusCode_(statusCode) {}

        /** Creates a new instance with an HttpRequestError, message, inner exception, and HTTP status code. */
        explicit HttpRequestException(HttpRequestError httpRequestError, const std::string& message = "",
                                      std::exception_ptr inner = nullptr,
                                      std::optional<System::Net::HttpStatusCode> statusCode = std::nullopt)
            : System::Exception(message, inner), httpRequestError_(httpRequestError), statusCode_(statusCode) {}

        /** @return The HttpRequestError that caused the exception. */
        [[nodiscard]] HttpRequestError getHttpRequestErrorProperty() const { return httpRequestError_; }

        /** @return The HTTP status code to be returned with the exception, or empty if not applicable. */
        [[nodiscard]] std::optional<System::Net::HttpStatusCode> getStatusCodeProperty() const { return statusCode_; }
    };

} // namespace System::Net::Http
