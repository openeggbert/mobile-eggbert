// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::WebSockets {

    /**
     * @brief Defines the different states a WebSocket instance can be in.
     *
     * C++ counterpart of .NET System.Net.WebSockets.WebSocketState.
     */
    enum class WebSocketState {
        None = 0,
        Connecting = 1,
        Open = 2,
        CloseSent = 3,
        CloseReceived = 4,
        Closed = 5,
        Aborted = 6,
    };

} // namespace System::Net::WebSockets
