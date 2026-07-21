// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ComponentModel/Win32Exception.hpp"
#include "System/Net/WebSockets/WebSocketError.hpp"

namespace System::Net::WebSockets {

    using SharpRuntime::intcs;

    /**
     * @brief The exception thrown when an error occurs while performing a WebSocket operation.
     *
     * C++ counterpart of .NET System.Net.WebSockets.WebSocketException.
     *
     * @note Reduced scope: the native-Win32-error-code constructor overloads
     * (`WebSocketException(int nativeError, ...)`) are not reproduced — there is no P/Invoke
     * layer in this runtime to produce a native error code from, matching the same simplification
     * already applied to `NetworkInformationException`.
     */
    class WebSocketException : public System::ComponentModel::Win32Exception {
        WebSocketError errorCode_;

    public:
        /** @brief Constructs with WebSocketError::Faulted and a generic message. */
        WebSocketException() : Win32Exception(0, "An internal WebSocket error occurred."), errorCode_(WebSocketError::Faulted) {}

        /** @brief Constructs from a WebSocketError value with a canned message. */
        explicit WebSocketException(WebSocketError error) : WebSocketException(error, messageFor(error)) {}

        /** @brief Constructs from a WebSocketError value with a custom message. */
        WebSocketException(WebSocketError error, const std::string& message) : Win32Exception(0, message), errorCode_(error) {}

        /** @brief Constructs from a WebSocketError value with a custom message and inner exception. */
        WebSocketException(WebSocketError error, const std::string& message, std::exception_ptr innerException)
            : Win32Exception(0, message), errorCode_(error) {
            (void)innerException; // Win32Exception has no inner-exception-carrying constructor to forward to.
        }

        /** @brief Constructs with a custom message and WebSocketError::Faulted. */
        explicit WebSocketException(const std::string& message) : Win32Exception(0, message), errorCode_(WebSocketError::Faulted) {}

        /** @return The WebSocketError value for this exception. */
        [[nodiscard]] WebSocketError getWebSocketErrorCodeProperty() const { return errorCode_; }

        /** @return The native error code (0 unless explicitly set — see note above). */
        [[nodiscard]] intcs getErrorCodeProperty() const { return getNativeErrorCodeProperty(); }

    private:
        static std::string messageFor(WebSocketError error) {
            switch (error) {
                case WebSocketError::InvalidMessageType: return "The WebSocket message type is invalid.";
                case WebSocketError::Faulted: return "The WebSocket instance is faulted.";
                case WebSocketError::NotAWebSocket: return "The server did not respond with a valid WebSocket handshake.";
                case WebSocketError::UnsupportedVersion: return "The server does not support the requested WebSocket version.";
                case WebSocketError::UnsupportedProtocol: return "The server does not support the requested WebSocket protocol.";
                case WebSocketError::HeaderError: return "The WebSocket handshake response contained invalid headers.";
                case WebSocketError::ConnectionClosedPrematurely: return "The connection was closed prematurely.";
                case WebSocketError::InvalidState: return "The WebSocket is in an invalid state for this operation.";
                default: return "An internal WebSocket error occurred.";
            }
        }
    };

} // namespace System::Net::WebSockets
