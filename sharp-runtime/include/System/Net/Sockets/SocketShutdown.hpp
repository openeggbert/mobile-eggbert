// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::Sockets {

    /**
     * @brief Defines constants used by the Socket::Shutdown() method.
     *
     * C++ counterpart of .NET System.Net.Sockets.SocketShutdown.
     */
    enum class SocketShutdown {
        Receive = 0x00,
        Send = 0x01,
        Both = 0x02,
    };

} // namespace System::Net::Sockets
