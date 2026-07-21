// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Net/NetworkInformation/NetworkInterfaceType.hpp"
#include "System/Net/NetworkInformation/NetworkInterfaceComponent.hpp"
#include "System/Net/NetworkInformation/OperationalStatus.hpp"
#include "System/Net/NetworkInformation/PhysicalAddress.hpp"

namespace System::Net::NetworkInformation {

    /**
     * @brief Provides configuration and statistical information for a network interface.
     *
     * C++ counterpart of .NET System.Net.NetworkInformation.NetworkInterface.
     *
     * @note This is a reduced-scope port: `GetIPProperties()`/`GetIPStatistics()`/
     * `GetIPv4Statistics()` are not reproduced because their return types
     * (`IPInterfaceProperties`, `IPInterfaceStatistics`, `IPv4InterfaceStatistics`) are out of
     * scope in this codebase (`plan.sqlite3`: part of the large PAL-internal interface-property
     * subsystem, marked `ignore`). Everything else in the public surface is implemented.
     * @note Linux-only: backed by POSIX `getifaddrs()` plus `/sys/class/net/<name>/speed`.
     * Throws `System::PlatformNotSupportedException` on other platforms — same documented-bug
     * category as `System::AppContext` (`/proc/self/exe`) and `System::TimeZoneInfo`
     * (`/usr/share/zoneinfo`).
     */
    class NetworkInterface {
        std::string name_;
        NetworkInterfaceType type_ = NetworkInterfaceType::Unknown;
        OperationalStatus operationalStatus_ = OperationalStatus::Unknown;
        SharpRuntime::longcs speed_ = -1;
        bool isReceiveOnly_ = false;
        bool supportsMulticast_ = false;
        // Only read by Supports() on the Linux PAL (NetworkInterface.cpp); on other
        // platforms Supports() is a PlatformNotSupportedException stub that never
        // reads them, so they'd otherwise trip -Wunused-private-field there.
        [[maybe_unused]] bool supportsIPv4_ = false;
        [[maybe_unused]] bool supportsIPv6_ = false;
        PhysicalAddress physicalAddress_ = PhysicalAddress::None;

        NetworkInterface(std::string name, NetworkInterfaceType type, OperationalStatus status,
                         SharpRuntime::longcs speed, bool supportsMulticast, bool supportsIPv4,
                         bool supportsIPv6, PhysicalAddress physicalAddress)
            : name_(std::move(name)), type_(type), operationalStatus_(status), speed_(speed),
              supportsMulticast_(supportsMulticast), supportsIPv4_(supportsIPv4), supportsIPv6_(supportsIPv6),
              physicalAddress_(std::move(physicalAddress)) {}

    public:
        NetworkInterface() = default;
        virtual ~NetworkInterface() = default;

        /**
         * @brief Returns objects that describe the network interfaces on the local computer.
         * @throws System::PlatformNotSupportedException on non-Linux platforms.
         * @throws NetworkInformationException if the interface list cannot be enumerated.
         */
        [[nodiscard]] static std::vector<std::shared_ptr<NetworkInterface>> GetAllNetworkInterfaces();

        /** @return true if any operational, non-loopback interface is up and running. */
        [[nodiscard]] static bool GetIsNetworkAvailable();

        /** @return The interface index of the IPv6 loopback interface. */
        [[nodiscard]] static SharpRuntime::intcs getIPv6LoopbackInterfaceIndexProperty();

        /** @return The interface index of the IPv4 loopback interface. */
        [[nodiscard]] static SharpRuntime::intcs getLoopbackInterfaceIndexProperty();

        /** @return A stable identifier for the interface (the interface name, on this port). */
        [[nodiscard]] virtual std::string getIdProperty() const { return name_; }

        /** @return The name of the network interface. */
        [[nodiscard]] virtual std::string getNameProperty() const { return name_; }

        /** @return The description of the network interface (the interface name, on this port). */
        [[nodiscard]] virtual std::string getDescriptionProperty() const { return name_; }

        /** @return The current operational state of the network connection. */
        [[nodiscard]] virtual OperationalStatus getOperationalStatusProperty() const { return operationalStatus_; }

        /** @return The reported speed of the interface in bits per second, or -1 if unknown. */
        [[nodiscard]] virtual SharpRuntime::longcs getSpeedProperty() const { return speed_; }

        /** @return true if the interface only receives data packets. */
        [[nodiscard]] virtual bool getIsReceiveOnlyProperty() const { return isReceiveOnly_; }

        /** @return true if the interface is enabled to receive multicast packets. */
        [[nodiscard]] virtual bool getSupportsMulticastProperty() const { return supportsMulticast_; }

        /** @return The physical (MAC) address of this network interface. */
        [[nodiscard]] virtual PhysicalAddress GetPhysicalAddress() const { return physicalAddress_; }

        /** @return The interface type. */
        [[nodiscard]] virtual NetworkInterfaceType getNetworkInterfaceTypeProperty() const { return type_; }

        /** @return true if the interface supports the given component (IPv4 or IPv6). */
        [[nodiscard]] virtual bool Supports(NetworkInterfaceComponent networkInterfaceComponent) const;
    };

} // namespace System::Net::NetworkInformation
