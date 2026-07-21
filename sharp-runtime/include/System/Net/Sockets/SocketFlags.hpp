// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::Sockets {

    /**
     * @brief Provides constant values for socket messages. [Flags]
     *
     * C++ counterpart of .NET System.Net.Sockets.SocketFlags.
     */
    enum class SocketFlags {
        None = 0x0000,
        OutOfBand = 0x0001,
        Peek = 0x0002,
        DontRoute = 0x0004,
        Truncated = 0x0100,
        ControlDataTruncated = 0x0200,
        Broadcast = 0x0400,
        Multicast = 0x0800,
        Partial = 0x8000,
    };

    /** @brief Bitwise OR for the [Flags] SocketFlags enum. */
    constexpr SocketFlags operator|(SocketFlags a, SocketFlags b) {
        return static_cast<SocketFlags>(static_cast<int>(a) | static_cast<int>(b));
    }

    /** @brief Bitwise AND for the [Flags] SocketFlags enum. */
    constexpr SocketFlags operator&(SocketFlags a, SocketFlags b) {
        return static_cast<SocketFlags>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Net::Sockets
