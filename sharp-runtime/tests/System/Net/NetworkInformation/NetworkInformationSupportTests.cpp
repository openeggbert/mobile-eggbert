// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/Net/NetworkInformation/IPStatus.hpp"
#include "System/Net/NetworkInformation/NetworkInterfaceType.hpp"
#include "System/Net/NetworkInformation/OperationalStatus.hpp"
#include "System/Net/NetworkInformation/NetworkInterfaceComponent.hpp"
#include "System/Net/NetworkInformation/PhysicalAddress.hpp"
#include "System/Net/NetworkInformation/NetworkInformationException.hpp"
#include "System/Net/NetworkInformation/PingException.hpp"
#include "System/Net/NetworkInformation/PingOptions.hpp"
#include "System/Net/NetworkInformation/NetworkAvailabilityEventArgs.hpp"
#include "System/Net/NetworkInformation/NetworkAddressChangedEventHandler.hpp"
#include "System/Net/NetworkInformation/NetworkAvailabilityChangedEventHandler.hpp"
#include "System/Net/NetworkInformation/NetworkChange.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"

using namespace System::Net::NetworkInformation;

// --- Enums ---------------------------------------------------------------------------------

TEST(IPStatusTests, Values) {
    EXPECT_EQ(static_cast<int>(IPStatus::Success), 0);
    EXPECT_EQ(static_cast<int>(IPStatus::TimedOut), 11010);
    EXPECT_EQ(static_cast<int>(IPStatus::Unknown), -1);
}

TEST(NetworkInterfaceTypeTests, Values) {
    EXPECT_EQ(static_cast<int>(NetworkInterfaceType::Ethernet), 6);
    EXPECT_EQ(static_cast<int>(NetworkInterfaceType::Loopback), 24);
    EXPECT_EQ(static_cast<int>(NetworkInterfaceType::Wireless80211), 71);
}

TEST(OperationalStatusTests, Values) {
    EXPECT_EQ(static_cast<int>(OperationalStatus::Up), 1);
    EXPECT_EQ(static_cast<int>(OperationalStatus::Down), 2);
}

TEST(NetworkInterfaceComponentTests, Values) {
    EXPECT_EQ(static_cast<int>(NetworkInterfaceComponent::IPv4), 0);
    EXPECT_EQ(static_cast<int>(NetworkInterfaceComponent::IPv6), 1);
}

// --- PhysicalAddress -------------------------------------------------------------------------

TEST(PhysicalAddressTests, None_IsEmpty) {
    EXPECT_TRUE(PhysicalAddress::None.GetAddressBytes().empty());
}

TEST(PhysicalAddressTests, Constructor_ToString_RoundTrips) {
    PhysicalAddress addr(std::vector<SharpRuntime::bytecs>{0x00, 0x11, 0x22, 0xAA, 0xBB, 0xCC});
    EXPECT_EQ(addr.ToString(), "001122AABBCC");
}

TEST(PhysicalAddressTests, Parse_NoDelimiter) {
    auto addr = PhysicalAddress::Parse("001122AABBCC");
    EXPECT_EQ(addr.ToString(), "001122AABBCC");
}

TEST(PhysicalAddressTests, Parse_HyphenDelimited) {
    auto addr = PhysicalAddress::Parse("00-11-22-AA-BB-CC");
    EXPECT_EQ(addr.ToString(), "001122AABBCC");
}

TEST(PhysicalAddressTests, Parse_ColonDelimited) {
    auto addr = PhysicalAddress::Parse("00:11:22:AA:BB:CC");
    EXPECT_EQ(addr.ToString(), "001122AABBCC");
}

TEST(PhysicalAddressTests, Parse_DotDelimited_CiscoFormat) {
    auto addr = PhysicalAddress::Parse("0011.22AA.BBCC");
    EXPECT_EQ(addr.ToString(), "001122AABBCC");
}

TEST(PhysicalAddressTests, Parse_Invalid_Throws) {
    EXPECT_THROW(PhysicalAddress::Parse("not-a-mac-zz"), System::FormatException);
}

TEST(PhysicalAddressTests, TryParse_Invalid_ReturnsFalse) {
    PhysicalAddress value = PhysicalAddress::None;
    EXPECT_FALSE(PhysicalAddress::TryParse("zz", value));
}

TEST(PhysicalAddressTests, Equals_SameBytes) {
    PhysicalAddress a(std::vector<SharpRuntime::bytecs>{1, 2, 3});
    PhysicalAddress b(std::vector<SharpRuntime::bytecs>{1, 2, 3});
    EXPECT_TRUE(a.Equals(b));
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(PhysicalAddressTests, Equals_DifferentBytes_ReturnsFalse) {
    PhysicalAddress a(std::vector<SharpRuntime::bytecs>{1, 2, 3});
    PhysicalAddress b(std::vector<SharpRuntime::bytecs>{1, 2, 4});
    EXPECT_FALSE(a.Equals(b));
}

// --- NetworkInformationException / PingException -------------------------------------------

TEST(NetworkInformationExceptionTests, ErrorCodeConstructor) {
    NetworkInformationException ex(42);
    EXPECT_EQ(ex.getErrorCodeProperty(), 42);
}

TEST(NetworkInformationExceptionTests, MessageConstructor) {
    NetworkInformationException ex(std::string("boom"));
    EXPECT_STREQ(ex.what(), "boom");
}

TEST(PingExceptionTests, MessageConstructor) {
    PingException ex("ping failed");
    EXPECT_STREQ(ex.what(), "ping failed");
}

// --- PingOptions -----------------------------------------------------------------------------

TEST(PingOptionsTests, DefaultTtl128) {
    PingOptions opts;
    EXPECT_EQ(opts.getTtlProperty(), 128);
    EXPECT_FALSE(opts.getDontFragmentProperty());
}

TEST(PingOptionsTests, Constructor_SetsFields) {
    PingOptions opts(64, true);
    EXPECT_EQ(opts.getTtlProperty(), 64);
    EXPECT_TRUE(opts.getDontFragmentProperty());
}

TEST(PingOptionsTests, Constructor_ZeroTtl_Throws) {
    EXPECT_THROW(PingOptions(0, false), System::ArgumentOutOfRangeException);
}

TEST(PingOptionsTests, SetTtl_Negative_Throws) {
    PingOptions opts;
    EXPECT_THROW(opts.setTtlProperty(-1), System::ArgumentOutOfRangeException);
}

// --- NetworkAvailabilityEventArgs / NetworkChange -------------------------------------------

TEST(NetworkAvailabilityEventArgsTests, IsAvailableProperty) {
    NetworkAvailabilityEventArgs args(true);
    EXPECT_TRUE(args.getIsAvailableProperty());
}

TEST(NetworkChangeTests, EventAccessors_DoNotThrow) {
    NetworkAddressChangedEventHandler h1 = [](void*, System::EventArgs&) {};
    NetworkAvailabilityChangedEventHandler h2 = [](void*, NetworkAvailabilityEventArgs&) {};
    EXPECT_NO_THROW(NetworkChange::add_NetworkAddressChanged(h1));
    EXPECT_NO_THROW(NetworkChange::remove_NetworkAddressChanged(h1));
    EXPECT_NO_THROW(NetworkChange::add_NetworkAvailabilityChanged(h2));
    EXPECT_NO_THROW(NetworkChange::remove_NetworkAvailabilityChanged(h2));
}
