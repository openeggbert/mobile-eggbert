// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::Sockets {

    /**
     * @brief Specifies the addressing scheme that an instance of the Socket class can use.
     *
     * C++ counterpart of .NET System.Net.Sockets.AddressFamily.
     */
    enum class AddressFamily {
        Unknown = -1,
        Unspecified = 0,
        Unix = 1,
        InterNetwork = 2,
        ImpLink = 3,
        Pup = 4,
        Chaos = 5,
        NS = 6,
        Ipx = NS,
        Iso = 7,
        Osi = Iso,
        Ecma = 8,
        DataKit = 9,
        Ccitt = 10,
        Sna = 11,
        DecNet = 12,
        DataLink = 13,
        Lat = 14,
        HyperChannel = 15,
        AppleTalk = 16,
        NetBios = 17,
        VoiceView = 18,
        FireFox = 19,
        Banyan = 21,
        Atm = 22,
        InterNetworkV6 = 23,
        Cluster = 24,
        Ieee12844 = 25,
        Irda = 26,
        NetworkDesigners = 28,
        Max = 29,
        Packet = 65536,
        ControllerAreaNetwork = 65537,
    };

} // namespace System::Net::Sockets
