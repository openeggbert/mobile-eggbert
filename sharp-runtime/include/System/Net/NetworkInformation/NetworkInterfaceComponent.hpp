// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net::NetworkInformation {

    /**
     * @brief Specifies the list of networking components that are supported on a network interface.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.NetworkInterfaceComponent.
     */
    enum class NetworkInterfaceComponent {
        /** Internet Protocol version 4 is supported. */
        IPv4,
        /** Internet Protocol version 6 is supported. */
        IPv6
    };

} // namespace System::Net::NetworkInformation
