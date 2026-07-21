// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include "System/Net/Sockets/Socket.hpp"
#include "System/Net/WebSockets/ClientWebSocketOptions.hpp"
#include "System/Net/WebSockets/WebSocket.hpp"
#include "System/Net/WebSockets/WebSocketMessageType.hpp"
#include "System/Uri.hpp"

namespace System::Net::WebSockets {

    /**
     * @brief Provides a client for connecting to WebSocket services.
     *
     * C++ counterpart of .NET System.Net.WebSockets.ClientWebSocket.
     *
     * @note Real RFC 6455 implementation over a raw `System::Net::Sockets::Socket` TCP
     * connection: performs the HTTP/1.1 Upgrade handshake (including the
     * `Sec-WebSocket-Accept` SHA-1 digest check) and does real client-masked frame
     * send/receive, including transparent ping/pong and the close handshake.
     * @note `ws://` only — `wss://` throws `System::PlatformNotSupportedException`, matching
     * this runtime's `HttpClient`, which also has no TLS support.
     * @note No permessage-deflate compression (`ClientWebSocketOptions::DangerousDeflateOptions`
     * is not reproduced — see that class's doc comment). `HttpStatusCode`/`HttpResponseHeaders`
     * (gated on `CollectHttpResponseDetails` in .NET) and the `HttpMessageInvoker`-based
     * `ConnectAsync` overload are not reproduced — there is no `HttpMessageInvoker` concept in
     * this runtime's simplified `HttpClient`.
     */
    class ClientWebSocket : public WebSocket {
        ClientWebSocketOptions options_;
        std::unique_ptr<System::Net::Sockets::Socket> socket_;
        WebSocketState state_ = WebSocketState::None;
        bool connectStarted_ = false;
        std::optional<std::string> subProtocol_;
        std::optional<WebSocketCloseStatus> closeStatus_;
        std::optional<std::string> closeStatusDescription_;
        bool sendContinuation_ = false;
        WebSocketMessageType fragmentType_ = WebSocketMessageType::Binary;
        std::vector<SharpRuntime::bytecs> recvLeftover_;
        size_t recvLeftoverPos_ = 0;
        WebSocketMessageType recvLeftoverType_ = WebSocketMessageType::Binary;
        std::mutex sendMutex_;

        void performHandshake(const System::Uri& uri);
        void sendFrame(SharpRuntime::bytecs opcode, const SharpRuntime::bytecs* data, size_t len, bool fin);
        struct RawFrame {
            SharpRuntime::bytecs opcode = 0;
            bool fin = false;
            std::vector<SharpRuntime::bytecs> payload;
        };
        RawFrame readFrame();
        void sendCloseFrame(WebSocketCloseStatus closeStatus, const std::optional<std::string>& statusDescription);

    public:
        ClientWebSocket() = default;
        ~ClientWebSocket() override { Dispose(); }

        /** @return The options used to configure the connection. Must be set before ConnectAsync(). */
        [[nodiscard]] ClientWebSocketOptions& getOptionsProperty() { return options_; }

        [[nodiscard]] std::optional<WebSocketCloseStatus> getCloseStatusProperty() const override { return closeStatus_; }
        [[nodiscard]] std::optional<std::string> getCloseStatusDescriptionProperty() const override {
            return closeStatusDescription_;
        }
        [[nodiscard]] std::optional<std::string> getSubProtocolProperty() const override { return subProtocol_; }
        [[nodiscard]] WebSocketState getStateProperty() const override { return state_; }

        /** @brief Connects to the WebSocket server at @p uri (scheme must be "ws"; "wss" is unsupported — no TLS). */
        System::Threading::Tasks::Task
        ConnectAsync(const System::Uri& uri,
                     System::Threading::CancellationToken cancellationToken = System::Threading::CancellationToken::None());

        void Abort() override;
        System::Threading::Tasks::Task
        CloseAsync(WebSocketCloseStatus closeStatus, const std::optional<std::string>& statusDescription,
                   System::Threading::CancellationToken cancellationToken = System::Threading::CancellationToken::None()) override;
        System::Threading::Tasks::Task CloseOutputAsync(
            WebSocketCloseStatus closeStatus, const std::optional<std::string>& statusDescription,
            System::Threading::CancellationToken cancellationToken = System::Threading::CancellationToken::None()) override;
        void Dispose() override;

        // Un-hide WebSocket's non-virtual whole-buffer convenience overloads — declaring the
        // full-signature overrides below would otherwise hide them (any derived-class overload
        // of a name hides all base-class overloads of that name).
        using WebSocket::ReceiveAsync;
        using WebSocket::SendAsync;

        System::Threading::Tasks::TaskT<WebSocketReceiveResult>
        ReceiveAsync(std::vector<SharpRuntime::bytecs>& buffer, SharpRuntime::intcs offset, SharpRuntime::intcs count,
                     System::Threading::CancellationToken cancellationToken = System::Threading::CancellationToken::None()) override;
        System::Threading::Tasks::Task
        SendAsync(const std::vector<SharpRuntime::bytecs>& buffer, SharpRuntime::intcs offset, SharpRuntime::intcs count,
                  WebSocketMessageType messageType, bool endOfMessage,
                  System::Threading::CancellationToken cancellationToken = System::Threading::CancellationToken::None()) override;
    };

} // namespace System::Net::WebSockets
