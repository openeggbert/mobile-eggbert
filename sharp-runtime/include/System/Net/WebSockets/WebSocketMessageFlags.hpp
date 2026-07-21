// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::WebSockets {

    /**
     * @brief Flags for controlling how the WebSocket should send a message. [Flags]
     *
     * C++ counterpart of .NET System.Net.WebSockets.WebSocketMessageFlags.
     */
    enum class WebSocketMessageFlags {
        None = 0,
        EndOfMessage = 1,
        DisableCompression = 2,
    };

    /** @brief Bitwise OR for the [Flags] WebSocketMessageFlags enum. */
    constexpr WebSocketMessageFlags operator|(WebSocketMessageFlags a, WebSocketMessageFlags b) {
        return static_cast<WebSocketMessageFlags>(static_cast<int>(a) | static_cast<int>(b));
    }

    /** @brief Bitwise AND for the [Flags] WebSocketMessageFlags enum. */
    constexpr WebSocketMessageFlags operator&(WebSocketMessageFlags a, WebSocketMessageFlags b) {
        return static_cast<WebSocketMessageFlags>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Net::WebSockets
