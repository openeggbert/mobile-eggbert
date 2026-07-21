// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Net/Sockets/IPPacketInformation.hpp"
#include "System/Net/Sockets/LingerOption.hpp"
#include "System/Net/Sockets/MulticastOption.hpp"
#include "System/Net/Sockets/ProtocolType.hpp"
#include "System/Net/Sockets/SelectMode.hpp"
#include "System/Net/Sockets/SendPacketsElement.hpp"
#include "System/Net/Sockets/SocketFlags.hpp"
#include "System/Net/Sockets/SocketOptionLevel.hpp"
#include "System/Net/Sockets/SocketOptionName.hpp"
#include "System/Net/Sockets/SocketReceiveFromResult.hpp"
#include "System/Net/Sockets/SocketReceiveMessageFromResult.hpp"
#include "System/Net/Sockets/SocketShutdown.hpp"
#include "System/Net/Sockets/SocketType.hpp"
#include "System/Net/Sockets/UdpReceiveResult.hpp"
#include "System/Net/Sockets/UnixDomainSocketEndPoint.hpp"

using namespace System::Net::Sockets;
using System::Net::IPAddress;
using System::Net::IPEndPoint;

// --- Enums -----------------------------------------------------------------------------------

TEST(ProtocolTypeTests, Values) {
    EXPECT_EQ(static_cast<int>(ProtocolType::Tcp), 6);
    EXPECT_EQ(static_cast<int>(ProtocolType::Udp), 17);
    EXPECT_EQ(static_cast<int>(ProtocolType::Unknown), -1);
}

TEST(SelectModeTests, Values) {
    EXPECT_EQ(static_cast<int>(SelectMode::SelectRead), 0);
    EXPECT_EQ(static_cast<int>(SelectMode::SelectWrite), 1);
    EXPECT_EQ(static_cast<int>(SelectMode::SelectError), 2);
}

TEST(SocketShutdownTests, Values) {
    EXPECT_EQ(static_cast<int>(SocketShutdown::Receive), 0x00);
    EXPECT_EQ(static_cast<int>(SocketShutdown::Send), 0x01);
    EXPECT_EQ(static_cast<int>(SocketShutdown::Both), 0x02);
}

TEST(SocketTypeTests, Values) {
    EXPECT_EQ(static_cast<int>(SocketType::Stream), 1);
    EXPECT_EQ(static_cast<int>(SocketType::Dgram), 2);
    EXPECT_EQ(static_cast<int>(SocketType::Unknown), -1);
}

TEST(SocketFlagsTests, BitwiseOperators) {
    auto combined = SocketFlags::Peek | SocketFlags::DontRoute;
    EXPECT_EQ(static_cast<int>(combined), 0x0006);
    EXPECT_EQ(combined & SocketFlags::Peek, SocketFlags::Peek);
}

TEST(SocketOptionLevelTests, Values) {
    EXPECT_EQ(static_cast<int>(SocketOptionLevel::Socket), 0xffff);
    EXPECT_EQ(static_cast<int>(SocketOptionLevel::Tcp), 6);
    EXPECT_EQ(static_cast<int>(SocketOptionLevel::Udp), 17);
}

TEST(SocketOptionNameTests, Values) {
    EXPECT_EQ(static_cast<int>(SocketOptionName::ReuseAddress), 0x0004);
    EXPECT_EQ(static_cast<int>(SocketOptionName::Linger), 0x0080);
    EXPECT_EQ(static_cast<int>(SocketOptionName::NoDelay), 1);
}

// --- Structs ---------------------------------------------------------------------------------

TEST(IPPacketInformationTests, BasicAccessors) {
    IPPacketInformation info(IPAddress::Loopback, 3);
    EXPECT_EQ(info.getAddressProperty(), IPAddress::Loopback);
    EXPECT_EQ(info.getInterfaceProperty(), 3);
    EXPECT_EQ(info, IPPacketInformation(IPAddress::Loopback, 3));
    EXPECT_NE(info, IPPacketInformation(IPAddress::Loopback, 4));
}

TEST(IPPacketInformationTests, GetHashCode_EqualInstancesMatch) {
    IPPacketInformation a(IPAddress::Loopback, 3);
    IPPacketInformation b(IPAddress::Loopback, 3);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(IPPacketInformationTests, GetHashCode_DifferingInterfaceDiffers) {
    IPPacketInformation a(IPAddress::Loopback, 3);
    IPPacketInformation b(IPAddress::Loopback, 4);
    EXPECT_NE(a.GetHashCode(), b.GetHashCode());
}

TEST(SocketReceiveFromResultTests, DefaultAndAssign) {
    SocketReceiveFromResult result;
    result.ReceivedBytes = 42;
    EXPECT_EQ(result.ReceivedBytes, 42);
    EXPECT_EQ(result.RemoteEndPoint, nullptr);
}

TEST(SocketReceiveMessageFromResultTests, DefaultAndAssign) {
    SocketReceiveMessageFromResult result;
    result.ReceivedBytes = 10;
    result.SocketFlags = SocketFlags::Truncated;
    EXPECT_EQ(result.ReceivedBytes, 10);
    EXPECT_EQ(result.SocketFlags, SocketFlags::Truncated);
}

TEST(UdpReceiveResultTests, BasicAccessors) {
    std::vector<SharpRuntime::bytecs> buffer{1, 2, 3};
    IPEndPoint remote(IPAddress::Loopback, 1234);
    UdpReceiveResult result(buffer, remote);
    EXPECT_EQ(result.getBufferProperty(), buffer);
    EXPECT_EQ(result.getRemoteEndPointProperty(), remote);
    EXPECT_EQ(result, UdpReceiveResult(buffer, remote));
}

TEST(UdpReceiveResultTests, GetHashCode_EqualInstancesMatch) {
    std::vector<SharpRuntime::bytecs> buffer{1, 2, 3};
    IPEndPoint remote(IPAddress::Loopback, 1234);
    UdpReceiveResult a(buffer, remote);
    UdpReceiveResult b(buffer, remote);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(UdpReceiveResultTests, GetHashCode_DifferingBufferDiffers) {
    IPEndPoint remote(IPAddress::Loopback, 1234);
    UdpReceiveResult a(std::vector<SharpRuntime::bytecs>{1, 2, 3}, remote);
    UdpReceiveResult b(std::vector<SharpRuntime::bytecs>{4, 5, 6}, remote);
    EXPECT_NE(a.GetHashCode(), b.GetHashCode());
}

// --- LingerOption / MulticastOption -----------------------------------------------------------

TEST(LingerOptionTests, BasicAccessors) {
    LingerOption option(true, 30);
    EXPECT_TRUE(option.getEnabledProperty());
    EXPECT_EQ(option.getLingerTimeProperty(), 30);
    option.setEnabledProperty(false);
    option.setLingerTimeProperty(10);
    EXPECT_FALSE(option.getEnabledProperty());
    EXPECT_EQ(option.getLingerTimeProperty(), 10);
    EXPECT_EQ(option, LingerOption(false, 10));
}

TEST(LingerOptionTests, GetHashCode_EqualInstancesMatch) {
    LingerOption a(true, 30);
    LingerOption b(true, 30);
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(LingerOptionTests, GetHashCode_DifferingTimeDiffers) {
    LingerOption a(true, 30);
    LingerOption b(true, 45);
    EXPECT_NE(a.GetHashCode(), b.GetHashCode());
}

TEST(MulticastOptionTests, GroupOnly) {
    MulticastOption option(IPAddress::Loopback);
    EXPECT_EQ(option.getGroupProperty(), IPAddress::Loopback);
    EXPECT_EQ(option.getLocalAddressProperty(), IPAddress::Any);
}

TEST(MulticastOptionTests, WithInterfaceIndex) {
    MulticastOption option(IPAddress::Loopback, 5);
    EXPECT_EQ(option.getInterfaceIndexProperty(), 5);
}

TEST(MulticastOptionTests, NegativeInterfaceIndex_Throws) {
    EXPECT_THROW((MulticastOption(IPAddress::Loopback, -1)), System::ArgumentOutOfRangeException);
}

TEST(IPv6MulticastOptionTests, BasicAccessors) {
    IPv6MulticastOption option(IPAddress::IPv6Loopback, 7);
    EXPECT_EQ(option.getGroupProperty(), IPAddress::IPv6Loopback);
    EXPECT_EQ(option.getInterfaceIndexProperty(), 7);
}

// --- SendPacketsElement ------------------------------------------------------------------------

TEST(SendPacketsElementTests, FilePathElement) {
    SendPacketsElement element(std::string("/tmp/foo.bin"));
    ASSERT_TRUE(element.getFilePathProperty().has_value());
    EXPECT_EQ(*element.getFilePathProperty(), "/tmp/foo.bin");
    EXPECT_EQ(element.getCountProperty(), 0);
    EXPECT_FALSE(element.getEndOfPacketProperty());
}

TEST(SendPacketsElementTests, BufferElement_WholeBuffer) {
    std::vector<SharpRuntime::bytecs> buffer{1, 2, 3, 4};
    SendPacketsElement element(buffer);
    EXPECT_EQ(element.getBufferProperty(), buffer);
    EXPECT_EQ(element.getCountProperty(), 4);
    EXPECT_EQ(element.getOffsetProperty(), 0);
}

TEST(SendPacketsElementTests, BufferElement_PartialRange) {
    std::vector<SharpRuntime::bytecs> buffer{1, 2, 3, 4, 5};
    SendPacketsElement element(buffer, 1, 2, true);
    EXPECT_EQ(element.getCountProperty(), 2);
    EXPECT_EQ(element.getOffsetProperty(), 1);
    EXPECT_TRUE(element.getEndOfPacketProperty());
}

TEST(SendPacketsElementTests, BufferElement_OutOfRange_Throws) {
    std::vector<SharpRuntime::bytecs> buffer{1, 2, 3};
    EXPECT_THROW((SendPacketsElement(buffer, 0, 10)), System::ArgumentOutOfRangeException);
}

// --- UnixDomainSocketEndPoint ------------------------------------------------------------------

TEST(UnixDomainSocketEndPointTests, BasicPath) {
    UnixDomainSocketEndPoint ep("/tmp/my.sock");
    EXPECT_EQ(ep.getPathProperty(), "/tmp/my.sock");
    EXPECT_EQ(ep.ToString(), "/tmp/my.sock");
    EXPECT_EQ(ep.getAddressFamilyProperty(), AddressFamily::Unix);
}

TEST(UnixDomainSocketEndPointTests, EmptyPath_Throws) {
    EXPECT_THROW(UnixDomainSocketEndPoint(std::string("")), System::ArgumentOutOfRangeException);
}

TEST(UnixDomainSocketEndPointTests, TooLongPath_Throws) {
    std::string longPath(200, 'a');
    EXPECT_THROW(UnixDomainSocketEndPoint{longPath}, System::ArgumentOutOfRangeException);
}

TEST(UnixDomainSocketEndPointTests, SerializeRoundTrip) {
    UnixDomainSocketEndPoint ep("/tmp/roundtrip.sock");
    auto address = ep.Serialize();
    auto recreated = ep.Create(address);
    auto* uds = dynamic_cast<UnixDomainSocketEndPoint*>(recreated.get());
    ASSERT_NE(uds, nullptr);
    EXPECT_EQ(uds->getPathProperty(), "/tmp/roundtrip.sock");
}

TEST(UnixDomainSocketEndPointTests, Equality) {
    UnixDomainSocketEndPoint a("/tmp/a.sock");
    UnixDomainSocketEndPoint b("/tmp/a.sock");
    UnixDomainSocketEndPoint c("/tmp/b.sock");
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(UnixDomainSocketEndPointTests, GetHashCode_EqualInstancesMatch) {
    UnixDomainSocketEndPoint a("/tmp/a.sock");
    UnixDomainSocketEndPoint b("/tmp/a.sock");
    EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
}

TEST(UnixDomainSocketEndPointTests, GetHashCode_DifferingPathDiffers) {
    UnixDomainSocketEndPoint a("/tmp/a.sock");
    UnixDomainSocketEndPoint c("/tmp/b.sock");
    EXPECT_NE(a.GetHashCode(), c.GetHashCode());
}
