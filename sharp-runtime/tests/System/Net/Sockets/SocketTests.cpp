// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <thread>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Net/IPEndPoint.hpp"
#include "System/Net/Sockets/Socket.hpp"

using namespace System::Net::Sockets;
using System::Net::IPAddress;
using System::Net::IPEndPoint;

TEST(SocketTests, ConstructAndProperties) {
    Socket socket(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    EXPECT_EQ(socket.getAddressFamilyProperty(), AddressFamily::InterNetwork);
    EXPECT_EQ(socket.getSocketTypeProperty(), SocketType::Stream);
    EXPECT_EQ(socket.getProtocolTypeProperty(), ProtocolType::Tcp);
    EXPECT_FALSE(socket.getConnectedProperty());
    EXPECT_FALSE(socket.getIsBoundProperty());
    EXPECT_GE(socket.getHandleProperty(), 0);
}

TEST(SocketTests, TcpBindListenAcceptConnectSendReceive) {
    Socket listener(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    listener.Bind(IPEndPoint(IPAddress::Loopback, 0));
    listener.Listen();
    EXPECT_TRUE(listener.getIsBoundProperty());

    auto local = listener.getLocalEndPointProperty();
    ASSERT_NE(local, nullptr);
    auto* localIp = dynamic_cast<IPEndPoint*>(local.get());
    ASSERT_NE(localIp, nullptr);
    intcs port = localIp->getPortProperty();
    ASSERT_GT(port, 0);

    std::shared_ptr<Socket> acceptedSocket;
    std::thread serverThread([&]() { acceptedSocket = listener.Accept(); });

    Socket client(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    client.Connect(IPAddress::Loopback, port);
    serverThread.join();

    ASSERT_NE(acceptedSocket, nullptr);
    EXPECT_TRUE(client.getConnectedProperty());

    std::vector<SharpRuntime::bytecs> sendBuf{'h', 'i', '!'};
    intcs sent = client.Send(sendBuf);
    EXPECT_EQ(sent, 3);

    std::vector<SharpRuntime::bytecs> recvBuf(16);
    intcs received = acceptedSocket->Receive(recvBuf);
    ASSERT_EQ(received, 3);
    EXPECT_EQ(recvBuf[0], 'h');
    EXPECT_EQ(recvBuf[1], 'i');
    EXPECT_EQ(recvBuf[2], '!');

    client.Close();
    acceptedSocket->Close();
}

// Regression test for a wave-3 audit finding: Send/Receive/SendTo/ReceiveFrom cast
// SocketFlags directly to the native int flags argument with no translation. On Linux the
// only flag whose bit position coincidentally matches .NET's SocketFlags is exercised here
// (SocketFlags::Peek == MSG_PEEK); a raw cast would have worked for this specific flag by
// coincidence too, so this mainly guards against a future refactor reintroducing the raw
// cast for flags whose bit positions do NOT coincide (e.g. Truncated/0x0100, which collides
// with Linux's unrelated MSG_WAITALL). Verified against pal_networking.c's
// ConvertSocketFlagsPalToPlatform.
TEST(SocketTests, Receive_PeekFlag_DoesNotConsumeData) {
    Socket listener(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    listener.Bind(IPEndPoint(IPAddress::Loopback, 0));
    listener.Listen();

    auto local = listener.getLocalEndPointProperty();
    auto* localIp = dynamic_cast<IPEndPoint*>(local.get());
    intcs port = localIp->getPortProperty();

    std::shared_ptr<Socket> acceptedSocket;
    std::thread serverThread([&]() { acceptedSocket = listener.Accept(); });

    Socket client(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    client.Connect(IPAddress::Loopback, port);
    serverThread.join();

    std::vector<SharpRuntime::bytecs> sendBuf{'p', 'e', 'e', 'k'};
    client.Send(sendBuf);

    std::vector<SharpRuntime::bytecs> peekBuf(16);
    intcs peeked = acceptedSocket->Receive(peekBuf, 0, 4, SocketFlags::Peek);
    ASSERT_EQ(peeked, 4);
    EXPECT_EQ(peekBuf[0], 'p');

    // A normal Receive afterward must still see the same bytes -- Peek must not have
    // consumed them. If SocketFlags::Peek's bit (0x0002) were misrouted to an unrelated
    // native flag, this would either block (data actually consumed) or read garbage.
    std::vector<SharpRuntime::bytecs> recvBuf(16);
    intcs received = acceptedSocket->Receive(recvBuf, 0, 4, SocketFlags::None);
    ASSERT_EQ(received, 4);
    EXPECT_EQ(recvBuf[0], 'p');
    EXPECT_EQ(recvBuf[1], 'e');
    EXPECT_EQ(recvBuf[2], 'e');
    EXPECT_EQ(recvBuf[3], 'k');

    client.Close();
    acceptedSocket->Close();
}

// Regression test for a wave-3 audit finding: Send/Receive/SendTo/ReceiveFrom used to do
// buffer.data() + offset with no bounds check against buffer.size() -- a genuine
// out-of-bounds heap read (Send) or write (Receive) when offset+count exceeded the buffer,
// instead of a clean ArgumentOutOfRangeException. Verified against Socket.Tasks.cs's
// ValidateBufferArguments.
TEST(SocketTests, Send_OffsetCountOutOfRange_Throws) {
    Socket listener(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    listener.Bind(IPEndPoint(IPAddress::Loopback, 0));
    listener.Listen();
    auto local = listener.getLocalEndPointProperty();
    auto* localIp = dynamic_cast<IPEndPoint*>(local.get());
    intcs port = localIp->getPortProperty();

    std::shared_ptr<Socket> acceptedSocket;
    std::thread serverThread([&]() { acceptedSocket = listener.Accept(); });
    Socket client(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    client.Connect(IPAddress::Loopback, port);
    serverThread.join();

    std::vector<SharpRuntime::bytecs> buf{'a', 'b', 'c'};
    EXPECT_THROW(client.Send(buf, 2, 5, SocketFlags::None), System::ArgumentOutOfRangeException); // offset+count > size
    EXPECT_THROW(client.Send(buf, 10, 1, SocketFlags::None), System::ArgumentOutOfRangeException); // offset > size
    EXPECT_THROW(client.Send(buf, -1, 1, SocketFlags::None), System::ArgumentOutOfRangeException); // negative offset
    EXPECT_THROW(client.Send(buf, 0, -1, SocketFlags::None), System::ArgumentOutOfRangeException); // negative count

    client.Close();
    acceptedSocket->Close();
}

TEST(SocketTests, Receive_OffsetCountOutOfRange_Throws) {
    Socket listener(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    listener.Bind(IPEndPoint(IPAddress::Loopback, 0));
    listener.Listen();
    auto local = listener.getLocalEndPointProperty();
    auto* localIp = dynamic_cast<IPEndPoint*>(local.get());
    intcs port = localIp->getPortProperty();

    std::shared_ptr<Socket> acceptedSocket;
    std::thread serverThread([&]() { acceptedSocket = listener.Accept(); });
    Socket client(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    client.Connect(IPAddress::Loopback, port);
    serverThread.join();

    std::vector<SharpRuntime::bytecs> buf(4);
    EXPECT_THROW(client.Receive(buf, 2, 5, SocketFlags::None), System::ArgumentOutOfRangeException);
    EXPECT_THROW(client.Receive(buf, -1, 1, SocketFlags::None), System::ArgumentOutOfRangeException);

    client.Close();
    acceptedSocket->Close();
}

TEST(SocketTests, UdpSendToReceiveFrom) {
    Socket receiver(AddressFamily::InterNetwork, SocketType::Dgram, ProtocolType::Udp);
    receiver.Bind(IPEndPoint(IPAddress::Loopback, 0));
    auto local = receiver.getLocalEndPointProperty();
    auto* localIp = dynamic_cast<IPEndPoint*>(local.get());
    ASSERT_NE(localIp, nullptr);
    intcs port = localIp->getPortProperty();

    Socket sender(AddressFamily::InterNetwork, SocketType::Dgram, ProtocolType::Udp);
    std::vector<SharpRuntime::bytecs> payload{9, 8, 7};
    IPEndPoint dest(IPAddress::Loopback, port);
    intcs sent = sender.SendTo(payload, dest);
    EXPECT_EQ(sent, 3);

    std::vector<SharpRuntime::bytecs> recvBuf(16);
    IPEndPoint anyTemplate;
    SocketReceiveFromResult result = receiver.ReceiveFrom(recvBuf, anyTemplate);
    EXPECT_EQ(result.ReceivedBytes, 3);
    ASSERT_NE(result.RemoteEndPoint, nullptr);
    auto* remoteIp = dynamic_cast<System::Net::IPEndPoint*>(result.RemoteEndPoint.get());
    ASSERT_NE(remoteIp, nullptr);
    EXPECT_EQ(remoteIp->getAddressProperty(), IPAddress::Loopback);
}

TEST(SocketTests, BlockingAndTimeoutProperties) {
    Socket socket(AddressFamily::InterNetwork, SocketType::Dgram, ProtocolType::Udp);
    EXPECT_TRUE(socket.getBlockingProperty());
    socket.setBlockingProperty(false);
    EXPECT_FALSE(socket.getBlockingProperty());

    socket.setReceiveTimeoutProperty(1234);
    // Linux rounds SO_RCVTIMEO to whole milliseconds via timeval; just verify it's set and sane.
    EXPECT_GE(socket.getReceiveTimeoutProperty(), 0);
}

TEST(SocketTests, SetSocketOption_ReuseAddress) {
    Socket socket(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    socket.SetSocketOption(SocketOptionLevel::Socket, SocketOptionName::ReuseAddress, true);
    EXPECT_NE(socket.GetSocketOption(SocketOptionLevel::Socket, SocketOptionName::ReuseAddress), 0);
}

TEST(SocketTests, LingerState) {
    Socket socket(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    LingerOption linger(true, 5);
    socket.setLingerStateProperty(linger);
    LingerOption readBack = socket.getLingerStateProperty();
    EXPECT_TRUE(readBack.getEnabledProperty());
    EXPECT_EQ(readBack.getLingerTimeProperty(), 5);
}

TEST(SocketTests, NoDelayProperty) {
    Socket socket(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    socket.setNoDelayProperty(true);
    EXPECT_TRUE(socket.getNoDelayProperty());
    socket.setNoDelayProperty(false);
    EXPECT_FALSE(socket.getNoDelayProperty());
}

TEST(SocketTests, PollDetectsReadable) {
    Socket receiver(AddressFamily::InterNetwork, SocketType::Dgram, ProtocolType::Udp);
    receiver.Bind(IPEndPoint(IPAddress::Loopback, 0));
    auto local = std::dynamic_pointer_cast<IPEndPoint>(receiver.getLocalEndPointProperty());
    ASSERT_NE(local, nullptr);

    EXPECT_FALSE(receiver.Poll(1000, SelectMode::SelectRead));

    Socket sender(AddressFamily::InterNetwork, SocketType::Dgram, ProtocolType::Udp);
    std::vector<SharpRuntime::bytecs> payload{1};
    sender.SendTo(payload, *local);

    EXPECT_TRUE(receiver.Poll(2'000'000, SelectMode::SelectRead));
}

// Regression test for a code-audit finding (ticket 240): Poll(-1, mode) is documented (matching
// real .NET's Socket.Poll/SocketPal.Unix.cs Poll) to mean "wait indefinitely", but this port
// previously built a timeval from microSeconds unconditionally -- for -1, truncating division
// gave tv_sec=0 and tv_usec=-1, an invalid timeval that made select() fail with EINVAL, so
// Poll(-1, ...) always returned false immediately instead of blocking. Data is sent before
// polling so a correct fix returns promptly (true) rather than actually blocking forever if the
// fix were wrong.
TEST(SocketTests, Poll_InfiniteTimeout_DoesNotReturnFalseImmediately) {
    Socket receiver(AddressFamily::InterNetwork, SocketType::Dgram, ProtocolType::Udp);
    receiver.Bind(IPEndPoint(IPAddress::Loopback, 0));
    auto local = std::dynamic_pointer_cast<IPEndPoint>(receiver.getLocalEndPointProperty());
    ASSERT_NE(local, nullptr);

    Socket sender(AddressFamily::InterNetwork, SocketType::Dgram, ProtocolType::Udp);
    std::vector<SharpRuntime::bytecs> payload{1};
    sender.SendTo(payload, *local);

    EXPECT_TRUE(receiver.Poll(-1, SelectMode::SelectRead));
}

// Regression test for a code-audit finding (ticket 240): Connect(host, port) had no test
// coverage at all before this fix. Verified against Socket.cs's Connect(string,int) ->
// Connect(IPAddress[],int), which resolves the host and tries every returned address in turn.
TEST(SocketTests, Connect_ByHostnameLiteral_ConnectsSuccessfully) {
    Socket listener(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    listener.Bind(IPEndPoint(IPAddress::Loopback, 0));
    listener.Listen();
    auto local = std::dynamic_pointer_cast<IPEndPoint>(listener.getLocalEndPointProperty());
    ASSERT_NE(local, nullptr);
    intcs port = local->getPortProperty();

    std::shared_ptr<Socket> acceptedSocket;
    std::thread serverThread([&]() { acceptedSocket = listener.Accept(); });

    Socket client(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    client.Connect("127.0.0.1", port);
    serverThread.join();

    ASSERT_NE(acceptedSocket, nullptr);
    EXPECT_TRUE(client.getConnectedProperty());
}

// Regression test for the same audit finding: this port previously connected only to
// addresses[0], with no address-family filtering at all, so a family-mismatched resolved
// address (e.g. connecting an IPv4-only socket to a literal IPv6 host) would be handed straight
// to connect() rather than being skipped as real .NET's CanTryAddressFamily check does.
TEST(SocketTests, Connect_ByHostname_NoMatchingAddressFamily_Throws) {
    Socket client(AddressFamily::InterNetworkV6, SocketType::Stream, ProtocolType::Tcp);
    EXPECT_THROW(client.Connect("127.0.0.1", 12345), System::ArgumentException);
}

TEST(SocketTests, AcceptAsyncAndConnectAsync) {
    Socket listener(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    listener.Bind(IPEndPoint(IPAddress::Loopback, 0));
    listener.Listen();
    auto local = std::dynamic_pointer_cast<IPEndPoint>(listener.getLocalEndPointProperty());
    ASSERT_NE(local, nullptr);

    auto acceptTask = listener.AcceptAsync();

    Socket client(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
    auto connectTask = client.ConnectAsync(*local);
    EXPECT_TRUE(connectTask.getResultProperty());

    auto acceptedSocket = acceptTask.getResultProperty();
    ASSERT_NE(acceptedSocket, nullptr);
}

TEST(SocketTests, StaticOSSupports) {
    EXPECT_TRUE(Socket::getOSSupportsIPv4Property());
}

TEST(SocketTests, MoveConstructor) {
    Socket original(AddressFamily::InterNetwork, SocketType::Dgram, ProtocolType::Udp);
    intcs originalHandle = original.getHandleProperty();
    Socket moved(std::move(original));
    EXPECT_EQ(moved.getHandleProperty(), originalHandle);
}
