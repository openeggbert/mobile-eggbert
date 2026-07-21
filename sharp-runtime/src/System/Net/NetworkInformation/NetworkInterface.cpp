// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/NetworkInformation/NetworkInterface.hpp"
#include "System/Net/NetworkInformation/NetworkInformationException.hpp"
#include "System/PlatformNotSupportedException.hpp"

#if defined(_WIN32)
// No Windows PAL implemented yet.
#elif defined(__EMSCRIPTEN__)
// No network interface enumeration available under Emscripten.
#elif defined(__linux__)
#define SHARP_RUNTIME_NETWORKINTERFACE_LINUX 1
#include <cstdio>
#include <cstring>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <map>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace System::Net::NetworkInformation {

#if defined(SHARP_RUNTIME_NETWORKINTERFACE_LINUX)
namespace {

    SharpRuntime::longcs readSpeed(const std::string& name) {
        std::string path = "/sys/class/net/" + name + "/speed";
        FILE* f = std::fopen(path.c_str(), "r");
        if (f == nullptr) {
            return -1;
        }
        long long value = -1;
        int matched = std::fscanf(f, "%lld", &value);
        std::fclose(f);
        if (matched != 1 || value < 0) {
            return -1;
        }
        return static_cast<SharpRuntime::longcs>(value) * 1'000'000; // Mbps -> bits/sec
    }

    NetworkInterfaceType mapHardwareType(unsigned short arphrdType, bool isLoopback) {
        if (isLoopback) {
            return NetworkInterfaceType::Loopback;
        }
        switch (arphrdType) {
            case ARPHRD_ETHER:
                return NetworkInterfaceType::Ethernet;
            case ARPHRD_LOOPBACK:
                return NetworkInterfaceType::Loopback;
            case ARPHRD_PPP:
                return NetworkInterfaceType::Ppp;
            case ARPHRD_TUNNEL:
            case ARPHRD_TUNNEL6:
            case ARPHRD_SIT:
            case ARPHRD_IPGRE:
                return NetworkInterfaceType::Tunnel;
            case ARPHRD_FDDI:
                return NetworkInterfaceType::Fddi;
            default:
                return NetworkInterfaceType::Unknown;
        }
    }

    struct InterfaceAccumulator {
        NetworkInterfaceType type = NetworkInterfaceType::Unknown;
        OperationalStatus status = OperationalStatus::Unknown;
        SharpRuntime::longcs speed = -1;
        bool supportsMulticast = false;
        bool supportsIPv4 = false;
        bool supportsIPv6 = false;
        PhysicalAddress physicalAddress = PhysicalAddress::None;
    };

} // namespace

std::vector<std::shared_ptr<NetworkInterface>> NetworkInterface::GetAllNetworkInterfaces() {
    ifaddrs* addrs = nullptr;
    if (getifaddrs(&addrs) != 0) {
        throw NetworkInformationException();
    }

    std::vector<std::string> order;
    std::map<std::string, InterfaceAccumulator> byName;

    for (ifaddrs* cur = addrs; cur != nullptr; cur = cur->ifa_next) {
        if (cur->ifa_name == nullptr) {
            continue;
        }
        std::string name(cur->ifa_name);
        auto [it, inserted] = byName.try_emplace(name);
        if (inserted) {
            order.push_back(name);
        }
        InterfaceAccumulator& acc = it->second;

        bool isLoopback = (cur->ifa_flags & IFF_LOOPBACK) != 0;
        acc.supportsMulticast = acc.supportsMulticast || ((cur->ifa_flags & IFF_MULTICAST) != 0);
        bool up = (cur->ifa_flags & IFF_UP) != 0;
        bool running = (cur->ifa_flags & IFF_RUNNING) != 0;
        acc.status = (up && running) ? OperationalStatus::Up : OperationalStatus::Down;

        if (cur->ifa_addr == nullptr) {
            continue;
        }

        if (cur->ifa_addr->sa_family == AF_INET) {
            acc.supportsIPv4 = true;
        } else if (cur->ifa_addr->sa_family == AF_INET6) {
            acc.supportsIPv6 = true;
        } else if (cur->ifa_addr->sa_family == AF_PACKET) {
            const auto* ll = reinterpret_cast<sockaddr_ll*>(cur->ifa_addr);
            acc.type = mapHardwareType(ll->sll_hatype, isLoopback);
            if (ll->sll_halen > 0) {
                std::vector<SharpRuntime::bytecs> mac(ll->sll_addr, ll->sll_addr + ll->sll_halen);
                acc.physicalAddress = PhysicalAddress(mac);
            }
            acc.speed = readSpeed(name);
        }

        if (isLoopback && acc.type == NetworkInterfaceType::Unknown) {
            acc.type = NetworkInterfaceType::Loopback;
        }
    }

    freeifaddrs(addrs);

    std::vector<std::shared_ptr<NetworkInterface>> result;
    result.reserve(order.size());
    for (const std::string& name : order) {
        const InterfaceAccumulator& acc = byName[name];
        result.push_back(std::shared_ptr<NetworkInterface>(new NetworkInterface(
            name, acc.type, acc.status, acc.speed, acc.supportsMulticast, acc.supportsIPv4, acc.supportsIPv6,
            acc.physicalAddress)));
    }
    return result;
}

bool NetworkInterface::GetIsNetworkAvailable() {
    // Verified against NetworkInterfacePal.Linux.cs's GetIsNetworkAvailable: real .NET skips
    // both Loopback AND Tunnel interfaces, not just Loopback -- a machine whose only "up"
    // non-loopback interface is a VPN tunnel previously (silently, incorrectly) reported
    // network availability as true here.
    for (const auto& iface : GetAllNetworkInterfaces()) {
        NetworkInterfaceType type = iface->getNetworkInterfaceTypeProperty();
        if (type == NetworkInterfaceType::Loopback || type == NetworkInterfaceType::Tunnel) {
            continue;
        }
        if (iface->getOperationalStatusProperty() == OperationalStatus::Up) {
            return true;
        }
    }
    return false;
}

SharpRuntime::intcs NetworkInterface::getIPv6LoopbackInterfaceIndexProperty() {
    return getLoopbackInterfaceIndexProperty();
}

SharpRuntime::intcs NetworkInterface::getLoopbackInterfaceIndexProperty() {
    unsigned int index = if_nametoindex("lo");
    if (index == 0) {
        throw NetworkInformationException();
    }
    return static_cast<SharpRuntime::intcs>(index);
}

bool NetworkInterface::Supports(NetworkInterfaceComponent networkInterfaceComponent) const {
    return networkInterfaceComponent == NetworkInterfaceComponent::IPv4 ? supportsIPv4_ : supportsIPv6_;
}

#else

std::vector<std::shared_ptr<NetworkInterface>> NetworkInterface::GetAllNetworkInterfaces() {
    throw System::PlatformNotSupportedException(
        "NetworkInterface.GetAllNetworkInterfaces() is only implemented on Linux in this runtime.");
}

bool NetworkInterface::GetIsNetworkAvailable() {
    throw System::PlatformNotSupportedException(
        "NetworkInterface.GetIsNetworkAvailable() is only implemented on Linux in this runtime.");
}

SharpRuntime::intcs NetworkInterface::getIPv6LoopbackInterfaceIndexProperty() {
    throw System::PlatformNotSupportedException(
        "NetworkInterface.IPv6LoopbackInterfaceIndex is only implemented on Linux in this runtime.");
}

SharpRuntime::intcs NetworkInterface::getLoopbackInterfaceIndexProperty() {
    throw System::PlatformNotSupportedException(
        "NetworkInterface.LoopbackInterfaceIndex is only implemented on Linux in this runtime.");
}

bool NetworkInterface::Supports(NetworkInterfaceComponent) const {
    throw System::PlatformNotSupportedException(
        "NetworkInterface.Supports() is only implemented on Linux in this runtime.");
}

#endif

} // namespace System::Net::NetworkInformation
