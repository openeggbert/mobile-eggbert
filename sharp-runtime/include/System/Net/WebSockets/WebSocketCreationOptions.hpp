// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/TimeSpan.hpp"
#include "System/Threading/Timeout.hpp"

namespace System::Net::WebSockets {

    /**
     * @brief Options that control how a WebSocket is created.
     *
     * C++ counterpart of .NET System.Net.WebSockets.WebSocketCreationOptions.
     *
     * @note `DangerousDeflateOptions` (per-message-deflate compression) is not reproduced —
     * `WebSocketDeflateOptions` is out of scope in this runtime (marked `ignore` in
     * `plan.sqlite3`), consistent with there being no permessage-deflate support in this
     * runtime's WebSocket implementation.
     */
    class WebSocketCreationOptions {
        bool isServer_ = false;
        std::optional<std::string> subProtocol_;
        System::TimeSpan keepAliveInterval_ = System::TimeSpan::Zero;
        System::TimeSpan keepAliveTimeout_ = System::TimeSpan::FromTicks(System::Threading::Timeout::InfiniteTimeSpan);

        static void validateNotBelowZero(const System::TimeSpan& value, const std::string& paramName) {
            if (value.getTicksProperty() != System::Threading::Timeout::InfiniteTimeSpan && value.getTicksProperty() < 0) {
                throw System::ArgumentOutOfRangeException(paramName, "The TimeSpan must be non-negative or Timeout.InfiniteTimeSpan.");
            }
        }

    public:
        WebSocketCreationOptions() = default;

        /** @return true if this is the server-side of the connection. */
        [[nodiscard]] bool getIsServerProperty() const { return isServer_; }
        /** @brief Sets whether this is the server-side of the connection. */
        void setIsServerProperty(bool value) { isServer_ = value; }

        /** @return The agreed-upon sub-protocol, if any. */
        [[nodiscard]] const std::optional<std::string>& getSubProtocolProperty() const { return subProtocol_; }
        /** @brief Sets the agreed-upon sub-protocol. */
        void setSubProtocolProperty(std::optional<std::string> value) { subProtocol_ = std::move(value); }

        /** @return The keep-alive interval, or TimeSpan::Zero/InfiniteTimeSpan to disable keep-alives. */
        [[nodiscard]] System::TimeSpan getKeepAliveIntervalProperty() const { return keepAliveInterval_; }
        /** @brief Sets the keep-alive interval. */
        void setKeepAliveIntervalProperty(System::TimeSpan value) {
            validateNotBelowZero(value, "KeepAliveInterval");
            keepAliveInterval_ = value;
        }

        /** @return The timeout to wait for a PONG in response to a PING. */
        [[nodiscard]] System::TimeSpan getKeepAliveTimeoutProperty() const { return keepAliveTimeout_; }
        /** @brief Sets the timeout to wait for a PONG in response to a PING. */
        void setKeepAliveTimeoutProperty(System::TimeSpan value) {
            validateNotBelowZero(value, "KeepAliveTimeout");
            keepAliveTimeout_ = value;
        }
    };

} // namespace System::Net::WebSockets
