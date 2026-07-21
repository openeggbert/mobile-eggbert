// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <initializer_list>
#include <optional>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Net/WebSockets/WebSocketCloseStatus.hpp"
#include "System/Net/WebSockets/WebSocketException.hpp"
#include "System/Net/WebSockets/WebSocketMessageType.hpp"
#include "System/Net/WebSockets/WebSocketReceiveResult.hpp"
#include "System/Net/WebSockets/WebSocketState.hpp"
#include "System/Threading/CancellationToken.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "System/TimeSpan.hpp"

namespace System::Net::WebSockets {

    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;

    /**
     * @brief Provides the base class for WebSocket implementations.
     *
     * C++ counterpart of .NET System.Net.WebSockets.WebSocket.
     *
     * @note `ArraySegment<byte>`/`Memory<byte>` overloads are unified into a single
     * `(vector<bytecs>&, offset, count)` signature — this runtime's established convention for
     * buffer + range, already used by `Socket::Send`/`Receive`. `CreateFromStream`/
     * `CreateClientWebSocket` (which construct .NET's internal `ManagedWebSocket` over an
     * arbitrary `Stream`) are not reproduced, since that internal type is out of scope here
     * (`plan.sqlite3`: `ManagedWebSocket` marked `ignore`) — use `ClientWebSocket` instead.
     */
    class WebSocket {
    public:
        virtual ~WebSocket() = default;

        /** @return The close status sent by the remote endpoint, or empty if not yet closed. */
        [[nodiscard]] virtual std::optional<WebSocketCloseStatus> getCloseStatusProperty() const = 0;
        /** @return The close status description sent by the remote endpoint, if any. */
        [[nodiscard]] virtual std::optional<std::string> getCloseStatusDescriptionProperty() const = 0;
        /** @return The agreed-upon sub-protocol, if any. */
        [[nodiscard]] virtual std::optional<std::string> getSubProtocolProperty() const = 0;
        /** @return The current state of the WebSocket connection. */
        [[nodiscard]] virtual WebSocketState getStateProperty() const = 0;

        /** @brief Aborts the connection and cancels any pending I/O. */
        virtual void Abort() = 0;

        /** @brief Closes the connection, performing the full closing handshake. */
        virtual System::Threading::Tasks::Task CloseAsync(WebSocketCloseStatus closeStatus,
                                                            const std::optional<std::string>& statusDescription,
                                                            System::Threading::CancellationToken cancellationToken =
                                                                System::Threading::CancellationToken::None()) = 0;

        /** @brief Sends a close frame without waiting for the remote endpoint's close frame. */
        virtual System::Threading::Tasks::Task
        CloseOutputAsync(WebSocketCloseStatus closeStatus, const std::optional<std::string>& statusDescription,
                          System::Threading::CancellationToken cancellationToken = System::Threading::CancellationToken::None()) = 0;

        /** @brief Releases resources used by this WebSocket. */
        virtual void Dispose() = 0;

        /**
         * @brief Receives data into @p buffer[offset, offset+count).
         *
         * @warning Lifetime contract: implementations (e.g. `ClientWebSocket`) run this
         * operation on a real background thread that starts immediately and writes into
         * @p buffer asynchronously (this runtime's `Task` has no GC to keep a captured
         * reference's target alive, unlike real .NET's `Memory<byte>`/array-backed overloads).
         * The caller MUST keep @p buffer alive, unmoved, and untouched by any other code until
         * the returned `Task` completes — destroying, reallocating (e.g. via `push_back`), or
         * concurrently writing to @p buffer before completion is a use-after-free / data race.
         */
        virtual System::Threading::Tasks::TaskT<WebSocketReceiveResult>
        ReceiveAsync(std::vector<bytecs>& buffer, intcs offset, intcs count,
                     System::Threading::CancellationToken cancellationToken = System::Threading::CancellationToken::None()) = 0;

        /** @brief Receives data into the entire buffer. Same lifetime contract as the 4-arg overload. */
        System::Threading::Tasks::TaskT<WebSocketReceiveResult> ReceiveAsync(std::vector<bytecs>& buffer) {
            return ReceiveAsync(buffer, 0, static_cast<intcs>(buffer.size()));
        }

        /**
         * @brief Sends @p buffer[offset, offset+count) as a message of the given type.
         *
         * @warning Same lifetime contract as `ReceiveAsync`: the operation runs on a real
         * background thread that starts immediately and reads @p buffer asynchronously by
         * reference. The caller MUST keep @p buffer alive and unmoved until the returned `Task`
         * completes.
         */
        virtual System::Threading::Tasks::Task
        SendAsync(const std::vector<bytecs>& buffer, intcs offset, intcs count, WebSocketMessageType messageType,
                  bool endOfMessage,
                  System::Threading::CancellationToken cancellationToken = System::Threading::CancellationToken::None()) = 0;

        /** @brief Sends the entire buffer as a single, complete message of the given type. */
        System::Threading::Tasks::Task SendAsync(const std::vector<bytecs>& buffer, WebSocketMessageType messageType) {
            return SendAsync(buffer, 0, static_cast<intcs>(buffer.size()), messageType, true);
        }

        /** @return The default keep-alive interval used by ClientWebSocket. */
        [[nodiscard]] static System::TimeSpan getDefaultKeepAliveIntervalProperty() {
            return System::TimeSpan::FromSeconds(30);
        }

        /** @brief Allocates a buffer sized for both a client's send and receive needs. */
        [[nodiscard]] static std::vector<bytecs> CreateClientBuffer(intcs receiveBufferSize, intcs sendBufferSize) {
            System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(receiveBufferSize, "receiveBufferSize");
            System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(sendBufferSize, "sendBufferSize");
            return std::vector<bytecs>(static_cast<size_t>(std::max(receiveBufferSize, sendBufferSize)));
        }

        /** @brief Allocates a buffer sized for a server's receive needs. */
        [[nodiscard]] static std::vector<bytecs> CreateServerBuffer(intcs receiveBufferSize) {
            System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(receiveBufferSize, "receiveBufferSize");
            return std::vector<bytecs>(static_cast<size_t>(receiveBufferSize));
        }

    protected:
        /** @throws WebSocketException(WebSocketError::InvalidState) if @p state is not one of @p validStates. */
        static void ThrowOnInvalidState(WebSocketState state, std::initializer_list<WebSocketState> validStates) {
            for (WebSocketState valid : validStates) {
                if (state == valid) return;
            }
            throw WebSocketException(WebSocketError::InvalidState, "The WebSocket is in an invalid state for this operation.");
        }

        /** @return true if @p state is a terminal state (Closed or Aborted). */
        [[nodiscard]] static bool IsStateTerminal(WebSocketState state) {
            return state == WebSocketState::Closed || state == WebSocketState::Aborted;
        }
    };

} // namespace System::Net::WebSockets
