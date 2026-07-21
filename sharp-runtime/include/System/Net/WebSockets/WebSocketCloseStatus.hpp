// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::WebSockets {

    /**
     * @brief Defines the close status codes used by the WebSocket protocol (RFC 6455 §7.4).
     *
     * C++ counterpart of .NET System.Net.WebSockets.WebSocketCloseStatus.
     */
    enum class WebSocketCloseStatus {
        NormalClosure = 1000,
        EndpointUnavailable = 1001,
        ProtocolError = 1002,
        InvalidMessageType = 1003,
        Empty = 1005,
        InvalidPayloadData = 1007,
        PolicyViolation = 1008,
        MessageTooBig = 1009,
        MandatoryExtension = 1010,
        InternalServerError = 1011,
    };

} // namespace System::Net::WebSockets
