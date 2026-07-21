// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/NetworkInformation/NetworkAddressChangedEventHandler.hpp"
#include "System/Net/NetworkInformation/NetworkAvailabilityChangedEventHandler.hpp"

namespace System::Net::NetworkInformation {

    /**
     * @brief Allows applications to receive notification when network address/availability changes.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.NetworkChange.
     *
     * @note Real OS-level change notification (netlink `RTM_NEWADDR`/`RTM_DELADDR` on Linux, SCNetworkReachability
     * on macOS, `NotifyAddrChange`/`NotifyRouteChange` on Windows) is out of scope for this runtime — this
     * is a stub, matching this codebase's existing AppDomain.UnhandledException event-accessor
     * convention (add/remove accessors that register nothing).
     */
    class NetworkChange {
    public:
        NetworkChange() = default;

        /**
         * @brief Stub — event registration is not functional; provided for API compatibility.
         *
         * C++ counterpart of .NET NetworkChange.NetworkAddressChanged event add accessor.
         */
        static void add_NetworkAddressChanged(const NetworkAddressChangedEventHandler& /*handler*/) {}

        /**
         * @brief Stub — event registration is not functional; provided for API compatibility.
         *
         * C++ counterpart of .NET NetworkChange.NetworkAddressChanged event remove accessor.
         */
        static void remove_NetworkAddressChanged(const NetworkAddressChangedEventHandler& /*handler*/) {}

        /**
         * @brief Stub — event registration is not functional; provided for API compatibility.
         *
         * C++ counterpart of .NET NetworkChange.NetworkAvailabilityChanged event add accessor.
         */
        static void add_NetworkAvailabilityChanged(const NetworkAvailabilityChangedEventHandler& /*handler*/) {}

        /**
         * @brief Stub — event registration is not functional; provided for API compatibility.
         *
         * C++ counterpart of .NET NetworkChange.NetworkAvailabilityChanged event remove accessor.
         */
        static void remove_NetworkAvailabilityChanged(const NetworkAvailabilityChangedEventHandler& /*handler*/) {}
    };

} // namespace System::Net::NetworkInformation
