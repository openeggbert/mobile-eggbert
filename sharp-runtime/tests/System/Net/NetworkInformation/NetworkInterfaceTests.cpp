// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <algorithm>
#include "System/Net/NetworkInformation/NetworkInterface.hpp"

using namespace System::Net::NetworkInformation;

TEST(NetworkInterfaceTests, GetAllNetworkInterfaces_ReturnsAtLeastLoopback) {
    auto interfaces = NetworkInterface::GetAllNetworkInterfaces();
    ASSERT_FALSE(interfaces.empty());

    bool foundLoopback = std::any_of(interfaces.begin(), interfaces.end(), [](const auto& iface) {
        return iface->getNetworkInterfaceTypeProperty() == NetworkInterfaceType::Loopback;
    });
    EXPECT_TRUE(foundLoopback);
}

TEST(NetworkInterfaceTests, IdNameDescription_AllEqualInterfaceName) {
    auto interfaces = NetworkInterface::GetAllNetworkInterfaces();
    ASSERT_FALSE(interfaces.empty());
    for (const auto& iface : interfaces) {
        EXPECT_EQ(iface->getIdProperty(), iface->getNameProperty());
        EXPECT_EQ(iface->getDescriptionProperty(), iface->getNameProperty());
        EXPECT_FALSE(iface->getNameProperty().empty());
    }
}

TEST(NetworkInterfaceTests, LoopbackInterface_SupportsIPv4) {
    auto interfaces = NetworkInterface::GetAllNetworkInterfaces();
    auto it = std::find_if(interfaces.begin(), interfaces.end(), [](const auto& iface) {
        return iface->getNetworkInterfaceTypeProperty() == NetworkInterfaceType::Loopback;
    });
    ASSERT_NE(it, interfaces.end());
    EXPECT_TRUE((*it)->Supports(NetworkInterfaceComponent::IPv4));
}

TEST(NetworkInterfaceTests, LoopbackInterfaceIndex_MatchesIPv6LoopbackInterfaceIndex) {
    EXPECT_EQ(NetworkInterface::getLoopbackInterfaceIndexProperty(),
              NetworkInterface::getIPv6LoopbackInterfaceIndexProperty());
    EXPECT_GT(NetworkInterface::getLoopbackInterfaceIndexProperty(), 0);
}

TEST(NetworkInterfaceTests, GetPhysicalAddress_LoopbackIsAllZero) {
    auto interfaces = NetworkInterface::GetAllNetworkInterfaces();
    auto it = std::find_if(interfaces.begin(), interfaces.end(), [](const auto& iface) {
        return iface->getNetworkInterfaceTypeProperty() == NetworkInterfaceType::Loopback;
    });
    ASSERT_NE(it, interfaces.end());
    // Linux reports the loopback link-layer address as 00:00:00:00:00:00, not zero-length.
    auto bytes = (*it)->GetPhysicalAddress().GetAddressBytes();
    EXPECT_TRUE(std::all_of(bytes.begin(), bytes.end(), [](auto b) { return b == 0; }));
}

TEST(NetworkInterfaceTests, GetIsNetworkAvailable_DoesNotThrow) {
    EXPECT_NO_THROW({ NetworkInterface::GetIsNetworkAvailable(); });
}

TEST(NetworkInterfaceTests, OperationalStatus_IsNotUnknownForAnyInterface) {
    auto interfaces = NetworkInterface::GetAllNetworkInterfaces();
    for (const auto& iface : interfaces) {
        EXPECT_NE(iface->getOperationalStatusProperty(), OperationalStatus::Unknown);
    }
}
