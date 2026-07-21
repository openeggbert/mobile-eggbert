// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::Sockets {

    /**
     * @brief Specifies the type of socket that a Socket instance represents.
     *
     * C++ counterpart of .NET System.Net.Sockets.SocketType.
     */
    enum class SocketType {
        Stream = 1,
        Dgram = 2,
        Raw = 3,
        Rdm = 4,
        Seqpacket = 5,
        Unknown = -1,
    };

} // namespace System::Net::Sockets
