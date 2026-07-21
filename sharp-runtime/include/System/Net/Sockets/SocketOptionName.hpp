// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::Sockets {

    /**
     * @brief Defines socket option names for the Socket class.
     *
     * C++ counterpart of .NET System.Net.Sockets.SocketOptionName.
     */
    enum class SocketOptionName {
        // SocketOptionLevel::Socket
        Debug = 0x0001,
        AcceptConnection = 0x0002,
        ReuseAddress = 0x0004,
        KeepAlive = 0x0008,
        DontRoute = 0x0010,
        Broadcast = 0x0020,
        UseLoopback = 0x0040,
        Linger = 0x0080,
        OutOfBandInline = 0x0100,
        DontLinger = ~0x0080,
        ExclusiveAddressUse = ~0x0004,
        SendBuffer = 0x1001,
        ReceiveBuffer = 0x1002,
        SendLowWater = 0x1003,
        ReceiveLowWater = 0x1004,
        SendTimeout = 0x1005,
        ReceiveTimeout = 0x1006,
        Error = 0x1007,
        Type = 0x1008,
        ReuseUnicastPort = 0x3007,
        MaxConnections = 0x7fffffff,

        // SocketOptionLevel::IP
        IPOptions = 1,
        HeaderIncluded = 2,
        TypeOfService = 3,
        IpTimeToLive = 4,
        MulticastInterface = 9,
        MulticastTimeToLive = 10,
        MulticastLoopback = 11,
        AddMembership = 12,
        DropMembership = 13,
        DontFragment = 14,
        AddSourceMembership = 15,
        DropSourceMembership = 16,
        BlockSource = 17,
        UnblockSource = 18,
        PacketInformation = 19,

        // SocketOptionLevel::IPv6
        HopLimit = 21,
        IPProtectionLevel = 23,
        IPv6Only = 27,

        // SocketOptionLevel::Tcp
        NoDelay = 1,
        BsdUrgent = 2,
        Expedited = 2,
        FastOpen = 15,
        TcpKeepAliveRetryCount = 16,
        TcpKeepAliveTime = 3,
        TcpKeepAliveInterval = 17,

        // SocketOptionLevel::Udp
        NoChecksum = 1,
        ChecksumCoverage = 20,
        UpdateAcceptContext = 0x700B,
        UpdateConnectContext = 0x7010,
    };

} // namespace System::Net::Sockets
