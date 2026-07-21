// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Net/WebSockets/WebSocketMessageType.hpp"

namespace System::Net::WebSockets {

    /**
     * @brief Represents the result of a single WebSocket::ReceiveAsync() operation.
     *
     * C++ counterpart of .NET System.Net.WebSockets.ValueWebSocketReceiveResult (a readonly struct).
     */
    class ValueWebSocketReceiveResult {
        SharpRuntime::intcs count_ = 0;
        WebSocketMessageType messageType_ = WebSocketMessageType::Close;
        bool endOfMessage_ = false;

    public:
        ValueWebSocketReceiveResult() = default;

        ValueWebSocketReceiveResult(SharpRuntime::intcs count, WebSocketMessageType messageType, bool endOfMessage)
            : count_(count), messageType_(messageType), endOfMessage_(endOfMessage) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(count, "count");
            // Real .NET validates messageType against the same range it packs into 32 minus 1
            // bits alongside count/endOfMessage: if ((uint)messageType > (uint)WebSocketMessageType.Close).
            if (static_cast<SharpRuntime::intcs>(messageType) < 0 ||
                static_cast<SharpRuntime::intcs>(messageType) > static_cast<SharpRuntime::intcs>(WebSocketMessageType::Close)) {
                throw System::ArgumentOutOfRangeException("messageType");
            }
        }

        /** @return The number of bytes received. */
        [[nodiscard]] SharpRuntime::intcs getCountProperty() const { return count_; }
        /** @return true if this is the last part of the message. */
        [[nodiscard]] bool getEndOfMessageProperty() const { return endOfMessage_; }
        /** @return The type of message that was received. */
        [[nodiscard]] WebSocketMessageType getMessageTypeProperty() const { return messageType_; }
    };

} // namespace System::Net::WebSockets
