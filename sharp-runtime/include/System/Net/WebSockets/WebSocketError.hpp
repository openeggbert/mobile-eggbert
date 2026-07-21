// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::WebSockets {

    /**
     * @brief Defines status codes that indicate the type of error that occurred on a WebSocket.
     *
     * C++ counterpart of .NET System.Net.WebSockets.WebSocketError.
     */
    enum class WebSocketError {
        Success = 0,
        InvalidMessageType = 1,
        Faulted = 2,
        NativeError = 3,
        NotAWebSocket = 4,
        UnsupportedVersion = 5,
        UnsupportedProtocol = 6,
        HeaderError = 7,
        ConnectionClosedPrematurely = 8,
        InvalidState = 9,
    };

} // namespace System::Net::WebSockets
