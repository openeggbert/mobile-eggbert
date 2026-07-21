// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/String.hpp"
#include "System/StringComparison.hpp"
#include "System/TimeSpan.hpp"
#include "System/Threading/Timeout.hpp"

namespace System::Net::WebSockets {

    class ClientWebSocket;

    /**
     * @brief Options to control the behavior of a ClientWebSocket connection.
     *
     * C++ counterpart of .NET System.Net.WebSockets.ClientWebSocketOptions.
     *
     * @note Reduced scope: no `Credentials`/`Proxy`/`Cookies`/`ClientCertificates`/
     * `RemoteCertificateValidationCallback`/`UseDefaultCredentials`/`HttpVersion(Policy)` — this
     * runtime's `HttpClient` has no credential/proxy/cookie/certificate infrastructure to hook
     * into, and there is no TLS support (matching `HttpClient`'s own documented no-TLS
     * limitation). `DangerousDeflateOptions` (permessage-deflate) is also not reproduced —
     * `WebSocketDeflateOptions` is out of scope in this runtime.
     */
    class ClientWebSocketOptions {
        bool isReadOnly_ = false;
        System::TimeSpan keepAliveInterval_ = System::TimeSpan::FromSeconds(30);
        System::TimeSpan keepAliveTimeout_ = System::TimeSpan::FromTicks(System::Threading::Timeout::InfiniteTimeSpan);
        SharpRuntime::intcs receiveBufferSize_ = 0x1000;
        SharpRuntime::intcs sendBufferSize_ = 0x1000;
        bool collectHttpResponseDetails_ = false;
        std::map<std::string, std::string> requestHeaders_;
        std::vector<std::string> requestedSubProtocols_;

        void throwIfReadOnly() const {
            if (isReadOnly_) {
                throw System::InvalidOperationException("The WebSocket has already been started.");
            }
        }

        static void validateSubprotocol(const std::string& subProtocol) {
            if (subProtocol.empty()) {
                throw System::ArgumentException("Empty string is not a valid subprotocol value.", "subProtocol");
            }
            for (unsigned char c : subProtocol) {
                if (c <= 0x20 || c == 0x7F) {
                    throw System::ArgumentException("The subprotocol contains invalid characters.", "subProtocol");
                }
            }
        }

        friend class ClientWebSocket;
        void setToReadOnly() { isReadOnly_ = true; }

    public:
        ClientWebSocketOptions() = default;

        /** @brief Sets an HTTP header to send during the WebSocket handshake. */
        void SetRequestHeader(const std::string& headerName, const std::string& headerValue) {
            throwIfReadOnly();
            requestHeaders_[headerName] = headerValue;
        }

        /** @return The headers set via SetRequestHeader(), keyed by header name. */
        [[nodiscard]] const std::map<std::string, std::string>& getRequestHeadersProperty() const { return requestHeaders_; }

        /**
         * @brief Adds a sub-protocol to request during the handshake.
         * @note Verified against ClientWebSocketOptions.cs's AddSubProtocol: the duplicate
         * check compares via StringComparison.OrdinalIgnoreCase, not an exact match -- this
         * previously used a case-sensitive comparison, silently allowing e.g. "chat" and
         * "Chat" to both be added as if they were distinct protocols.
         */
        void AddSubProtocol(const std::string& subProtocol) {
            throwIfReadOnly();
            validateSubprotocol(subProtocol);
            for (const auto& existing : requestedSubProtocols_) {
                if (System::String::Equals(existing, subProtocol, System::StringComparison::OrdinalIgnoreCase)) {
                    throw System::ArgumentException("Duplicate protocols are not allowed: " + subProtocol, "subProtocol");
                }
            }
            requestedSubProtocols_.push_back(subProtocol);
        }

        /** @return The sub-protocols requested via AddSubProtocol(). */
        [[nodiscard]] const std::vector<std::string>& getRequestedSubProtocolsProperty() const { return requestedSubProtocols_; }

        /** @return The keep-alive interval, or TimeSpan::Zero/InfiniteTimeSpan to disable keep-alives. */
        [[nodiscard]] System::TimeSpan getKeepAliveIntervalProperty() const { return keepAliveInterval_; }
        /** @brief Sets the keep-alive interval. */
        void setKeepAliveIntervalProperty(System::TimeSpan value) {
            throwIfReadOnly();
            if (value.getTicksProperty() != System::Threading::Timeout::InfiniteTimeSpan && value.getTicksProperty() < 0) {
                throw System::ArgumentOutOfRangeException("value", "The TimeSpan must be non-negative or Timeout.InfiniteTimeSpan.");
            }
            keepAliveInterval_ = value;
        }

        /** @return The timeout to wait for a PONG in response to a PING. */
        [[nodiscard]] System::TimeSpan getKeepAliveTimeoutProperty() const { return keepAliveTimeout_; }
        /** @brief Sets the timeout to wait for a PONG in response to a PING. */
        void setKeepAliveTimeoutProperty(System::TimeSpan value) {
            throwIfReadOnly();
            if (value.getTicksProperty() != System::Threading::Timeout::InfiniteTimeSpan && value.getTicksProperty() < 0) {
                throw System::ArgumentOutOfRangeException("value", "The TimeSpan must be non-negative or Timeout.InfiniteTimeSpan.");
            }
            keepAliveTimeout_ = value;
        }

        /** @brief Sets the internal send/receive buffer sizes to use once connected. */
        void SetBuffer(SharpRuntime::intcs receiveBufferSize, SharpRuntime::intcs sendBufferSize) {
            throwIfReadOnly();
            System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(receiveBufferSize, "receiveBufferSize");
            System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(sendBufferSize, "sendBufferSize");
            receiveBufferSize_ = receiveBufferSize;
            sendBufferSize_ = sendBufferSize;
        }

        /** @return The configured receive buffer size. */
        [[nodiscard]] SharpRuntime::intcs getReceiveBufferSizeProperty() const { return receiveBufferSize_; }
        /** @return The configured send buffer size. */
        [[nodiscard]] SharpRuntime::intcs getSendBufferSizeProperty() const { return sendBufferSize_; }

        /** @return true if HttpStatusCode/HttpResponseHeaders should be captured during ConnectAsync(). */
        [[nodiscard]] bool getCollectHttpResponseDetailsProperty() const { return collectHttpResponseDetails_; }
        /** @brief Sets whether HttpStatusCode/HttpResponseHeaders should be captured during ConnectAsync(). */
        void setCollectHttpResponseDetailsProperty(bool value) {
            throwIfReadOnly();
            collectHttpResponseDetails_ = value;
        }
    };

} // namespace System::Net::WebSockets
