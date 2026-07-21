// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::NetworkInformation {

    /**
     * @brief Specifies types of network interfaces.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.NetworkInterfaceType.
     */
    enum class NetworkInterfaceType {
        Unknown = 1,
        Ethernet = 6,
        TokenRing = 9,
        Fddi = 15,
        BasicIsdn = 20,
        PrimaryIsdn = 21,
        Ppp = 23,
        Loopback = 24,
        Ethernet3Megabit = 26,
        Slip = 28,
        Atm = 37,
        GenericModem = 48,
        FastEthernetT = 62,
        Isdn = 63,
        FastEthernetFx = 69,
        Wireless80211 = 71,
        AsymmetricDsl = 94,
        RateAdaptDsl = 95,
        SymmetricDsl = 96,
        VeryHighSpeedDsl = 97,
        IPOverAtm = 114,
        GigabitEthernet = 117,
        Tunnel = 131,
        MultiRateSymmetricDsl = 143,
        HighPerformanceSerialBus = 144,
        Wman = 237,
        Wwanpp = 243,
        Wwanpp2 = 244,
    };

} // namespace System::Net::NetworkInformation
