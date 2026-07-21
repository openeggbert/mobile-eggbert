// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::NetworkInformation {

    /**
     * @brief Reports the status of sending an ICMP echo request via Ping.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.IPStatus.
     */
    enum class IPStatus {
        Success = 0,

        DestinationNetworkUnreachable = 11000 + 2,
        DestinationHostUnreachable = 11000 + 3,
        DestinationProtocolUnreachable = 11000 + 4,
        DestinationPortUnreachable = 11000 + 5,
        DestinationProhibited = 11000 + 4,

        NoResources = 11000 + 6,
        BadOption = 11000 + 7,
        HardwareError = 11000 + 8,
        PacketTooBig = 11000 + 9,
        TimedOut = 11000 + 10,
        BadRoute = 11000 + 12,

        TtlExpired = 11000 + 13,
        TtlReassemblyTimeExceeded = 11000 + 14,

        ParameterProblem = 11000 + 15,
        SourceQuench = 11000 + 16,
        BadDestination = 11000 + 18,

        DestinationUnreachable = 11000 + 40,
        TimeExceeded = 11000 + 41,
        BadHeader = 11000 + 42,
        UnrecognizedNextHeader = 11000 + 43,
        IcmpError = 11000 + 44,
        DestinationScopeMismatch = 11000 + 45,
        Unknown = -1,
    };

} // namespace System::Net::NetworkInformation
