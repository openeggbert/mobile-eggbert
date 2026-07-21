// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::WebSockets {

    /**
     * @brief Indicates the message type.
     *
     * C++ counterpart of .NET System.Net.WebSockets.WebSocketMessageType.
     */
    enum class WebSocketMessageType {
        Text = 0,
        Binary = 1,
        Close = 2,
    };

} // namespace System::Net::WebSockets
