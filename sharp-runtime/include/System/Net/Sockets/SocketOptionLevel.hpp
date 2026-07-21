// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Sockets/ProtocolType.hpp"

namespace System::Net::Sockets {

    /**
     * @brief Defines socket option levels for the Socket class.
     *
     * C++ counterpart of .NET System.Net.Sockets.SocketOptionLevel.
     */
    enum class SocketOptionLevel {
        Socket = 0xffff,
        IP = static_cast<int>(ProtocolType::IP),
        IPv6 = static_cast<int>(ProtocolType::IPv6),
        Tcp = static_cast<int>(ProtocolType::Tcp),
        Udp = static_cast<int>(ProtocolType::Udp),
    };

} // namespace System::Net::Sockets
