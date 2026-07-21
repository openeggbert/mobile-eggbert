// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Net/WebSockets/WebSocketCloseStatus.hpp"
#include "System/Net/WebSockets/WebSocketMessageType.hpp"

namespace System::Net::WebSockets {

    /**
     * @brief Represents the result of performing a single WebSocket::ReceiveAsync() operation.
     *
     * C++ counterpart of .NET System.Net.WebSockets.WebSocketReceiveResult.
     */
    class WebSocketReceiveResult {
        SharpRuntime::intcs count_ = 0;
        bool endOfMessage_ = false;
        WebSocketMessageType messageType_ = WebSocketMessageType::Close;
        std::optional<WebSocketCloseStatus> closeStatus_;
        std::optional<std::string> closeStatusDescription_;

    public:
        /** @brief Default-constructs an empty result — needed so WebSocketReceiveResult can be TaskT<WebSocketReceiveResult>'s result type. */
        WebSocketReceiveResult() = default;

        WebSocketReceiveResult(SharpRuntime::intcs count, WebSocketMessageType messageType, bool endOfMessage)
            : WebSocketReceiveResult(count, messageType, endOfMessage, std::nullopt, std::nullopt) {}

        WebSocketReceiveResult(SharpRuntime::intcs count, WebSocketMessageType messageType, bool endOfMessage,
                                std::optional<WebSocketCloseStatus> closeStatus,
                                std::optional<std::string> closeStatusDescription)
            : count_(count), endOfMessage_(endOfMessage), messageType_(messageType), closeStatus_(std::move(closeStatus)),
              closeStatusDescription_(std::move(closeStatusDescription)) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(count, "count");
        }

        /** @return The number of bytes received. */
        [[nodiscard]] SharpRuntime::intcs getCountProperty() const { return count_; }
        /** @return true if this is the last part of the message. */
        [[nodiscard]] bool getEndOfMessageProperty() const { return endOfMessage_; }
        /** @return The type of message that was received. */
        [[nodiscard]] WebSocketMessageType getMessageTypeProperty() const { return messageType_; }
        /** @return The close status, if this result represents a close message. */
        [[nodiscard]] const std::optional<WebSocketCloseStatus>& getCloseStatusProperty() const { return closeStatus_; }
        /** @return The close status description, if this result represents a close message. */
        [[nodiscard]] const std::optional<std::string>& getCloseStatusDescriptionProperty() const {
            return closeStatusDescription_;
        }
    };

} // namespace System::Net::WebSockets
