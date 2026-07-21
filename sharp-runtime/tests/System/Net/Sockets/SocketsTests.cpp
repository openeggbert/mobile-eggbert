// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <vector>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/Net/IPAddress.hpp"
#include "System/Net/IPEndPoint.hpp"
#include "System/Net/Sockets/NetworkStream.hpp"
#include "System/Net/Sockets/SocketError.hpp"
#include "System/Net/Sockets/SocketException.hpp"
#include "System/Net/Sockets/TcpClient.hpp"
#include "System/Net/Sockets/UdpClient.hpp"
#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
#  include <cerrno>
#  include <sys/socket.h>
#  include <sys/un.h>
#  include <unistd.h>
#endif

using System::Net::IPAddress;
using System::Net::IPEndPoint;
using System::Net::Sockets::NetworkStream;
using System::Net::Sockets::TcpClient;
using System::Net::Sockets::TcpListener;
using System::Net::Sockets::UdpClient;

// 2026-07-13 resource-management fix: each of these owns a raw socket fd closed by its
// destructor -- an implicit shallow copy previously let two instances' destructors both close
// the same fd. Compile-time check, matching how the original audit finding was itself verified.
static_assert(!std::is_copy_constructible_v<TcpClient>, "TcpClient must not be copy-constructible");
static_assert(!std::is_copy_assignable_v<TcpClient>, "TcpClient must not be copy-assignable");
static_assert(!std::is_copy_constructible_v<TcpListener>, "TcpListener must not be copy-constructible");
static_assert(!std::is_copy_assignable_v<TcpListener>, "TcpListener must not be copy-assignable");
static_assert(!std::is_copy_constructible_v<UdpClient>, "UdpClient must not be copy-constructible");
static_assert(!std::is_copy_assignable_v<UdpClient>, "UdpClient must not be copy-assignable");
static_assert(!std::is_copy_constructible_v<NetworkStream>, "NetworkStream must not be copy-constructible");
static_assert(!std::is_copy_assignable_v<NetworkStream>, "NetworkStream must not be copy-assignable");

// ===========================================================================
// TcpClient
// ===========================================================================

TEST(TcpClientTests, DefaultConstructor_NoThrow) {
    EXPECT_NO_THROW(TcpClient{});
}

TEST(TcpClientTests, EndPointConstructor_NoThrow) {
    IPEndPoint ep(IPAddress::Parse("127.0.0.1"), 8080);
    EXPECT_NO_THROW(TcpClient{ep});
}

TEST(TcpClientTests, Connect_InvalidHostname_Throws) {
    TcpClient client;
    EXPECT_THROW(client.Connect("this.host.does.not.exist.invalid.example", 80), System::Net::Sockets::SocketException);
}

TEST(TcpClientTests, Connect_ConnectionRefused_Throws) {
    // Port 1 on loopback is almost certainly not listening.
    TcpClient client;
    EXPECT_THROW(client.Connect("127.0.0.1", 1), System::Net::Sockets::SocketException);
}

TEST(TcpClientTests, Connect_ConnectionRefused_SocketErrorCodeIsConnectionRefused) {
    // Regression: SocketException previously carried the raw POSIX errno (Linux's
    // ECONNREFUSED=111) reinterpreted directly as a SocketError value instead of being
    // translated into the WSA-numbered SocketError space (SocketError::ConnectionRefused =
    // 10061), so SocketErrorCode never matched any real SocketError constant.
    TcpClient client;
    try {
        client.Connect("127.0.0.1", 1);
        FAIL() << "expected SocketException";
    } catch (const System::Net::Sockets::SocketException& ex) {
        EXPECT_EQ(ex.getSocketErrorCodeProperty(), System::Net::Sockets::SocketError::ConnectionRefused);
    }
}

TEST(SocketExceptionTests, DefaultConstructor_CapturesLastErrno) {
    // Real .NET's parameterless SocketException() captures the last OS socket error
    // (Interop.Sys.GetLastErrorInfo() on Unix). This port had no equivalent constructor at all;
    // added one that reads errno and reuses the same TranslateErrno() helper every other socket
    // call site uses. Force a known errno (ENOTSOCK, via a bad fstat-style call) then construct.
#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)
    errno = ENOTSOCK;
    System::Net::Sockets::SocketException ex;
    EXPECT_EQ(ex.getSocketErrorCodeProperty(), System::Net::Sockets::SocketError::NotSocket);
#endif
}

TEST(TcpClientTests, Connect_EndPoint_ConnectionRefused_Throws) {
    TcpClient client;
    IPEndPoint ep(IPAddress::Loopback, 1);
    EXPECT_THROW(client.Connect(ep), System::Net::Sockets::SocketException);
}

TEST(TcpClientTests, Close_NoThrow) {
    TcpClient client;
    EXPECT_NO_THROW(client.Close());
}

TEST(TcpClientTests, Connected_ReturnsFalse_WhenNotConnected) {
    TcpClient client;
    EXPECT_FALSE(client.getConnectedProperty());
}

TEST(TcpClientTests, Available_ReturnsZero_WhenNotConnected) {
    TcpClient client;
    EXPECT_EQ(0, client.Available());
}

TEST(TcpClientTests, GetStream_Throws_WhenNotConnected) {
    TcpClient client;
    EXPECT_THROW((void)client.GetStream(), System::InvalidOperationException);
}

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)

// Regression tests for ticket 336: real .NET's TcpClient.GetStream() caches and returns the SAME
// NetworkStream instance on every call (TCPClient.cs: `return _dataStream ??= new
// NetworkStream(Client, true);`). This port previously created a brand-new NetworkStream (a
// fresh dup()'d fd) on every call -- and on Windows outright transferred fd_ away on the first
// call, throwing InvalidOperationException on any second call.

TEST(TcpClientTests, GetStream_ReturnsSameCachedInstance_OnRepeatedCalls) {
    IPEndPoint listenEp(IPAddress::Parse("127.0.0.1"), 0);
    TcpListener listener(listenEp);
    listener.Start();
    int port = listener.getLocalEndpointProperty().getPortProperty();

    std::thread serverThread([&]() { (void)listener.AcceptTcpClient(); });
    TcpClient client;
    client.Connect("127.0.0.1", port);
    serverThread.join();
    listener.Stop();

    auto s1 = client.GetStream();
    auto s2 = client.GetStream();
    EXPECT_EQ(s1.get(), s2.get());
}

TEST(TcpClientTests, GetStream_NewInstance_AfterReconnect) {
    IPEndPoint listenEp(IPAddress::Parse("127.0.0.1"), 0);
    TcpListener listener(listenEp);
    listener.Start();
    int port = listener.getLocalEndpointProperty().getPortProperty();

    TcpClient client;
    {
        std::thread serverThread([&]() { (void)listener.AcceptTcpClient(); });
        client.Connect("127.0.0.1", port);
        serverThread.join();
    }
    auto first = client.GetStream();
    client.Close();

    {
        std::thread serverThread([&]() { (void)listener.AcceptTcpClient(); });
        client.Connect("127.0.0.1", port);
        serverThread.join();
    }
    auto second = client.GetStream();
    listener.Stop();

    EXPECT_NE(first.get(), second.get());
}

#endif // !_WIN32 && !__EMSCRIPTEN__

// ===========================================================================
// TcpListener
// ===========================================================================

TEST(TcpListenerTests, EndPointConstructor_NoThrow) {
    IPEndPoint ep(IPAddress::Parse("0.0.0.0"), 0);
    EXPECT_NO_THROW(TcpListener{ep});
}

TEST(TcpListenerTests, AddressPortConstructor_NoThrow) {
    IPAddress addr = IPAddress::Parse("0.0.0.0");
    EXPECT_NO_THROW(TcpListener(addr, 0));
}

TEST(TcpListenerTests, Start_Stop_NoThrow) {
    IPEndPoint ep(IPAddress::Parse("0.0.0.0"), 0);
    TcpListener listener(ep);
    EXPECT_NO_THROW(listener.Start());
    EXPECT_NO_THROW(listener.Stop());
}

TEST(TcpListenerTests, Start_AssignsPort_WhenPortZero) {
    IPEndPoint ep(IPAddress::Parse("0.0.0.0"), 0);
    TcpListener listener(ep);
    listener.Start();
    EXPECT_GT(listener.getLocalEndpointProperty().getPortProperty(), 0);
    listener.Stop();
}

TEST(TcpListenerTests, Stop_NoThrow_WhenNotStarted) {
    IPEndPoint ep(IPAddress::Parse("0.0.0.0"), 0);
    TcpListener listener(ep);
    EXPECT_NO_THROW(listener.Stop());
}

// ===========================================================================
// UdpClient
// ===========================================================================

TEST(UdpClientTests, DefaultConstructor_NoThrow) {
    EXPECT_NO_THROW(UdpClient{});
}

TEST(UdpClientTests, PortConstructor_BindsToFreePort) {
    // Port 0 → OS picks a free port.
    EXPECT_NO_THROW(UdpClient{0});
}

TEST(UdpClientTests, EndPointConstructor_NoThrow) {
    IPEndPoint ep(IPAddress::Parse("0.0.0.0"), 0);
    EXPECT_NO_THROW(UdpClient{ep});
}

TEST(UdpClientTests, ClientProperty_TrueAfterConstruction) {
    UdpClient client;
    EXPECT_TRUE(client.getClientProperty());
}

TEST(UdpClientTests, ClientProperty_FalseAfterClose) {
    UdpClient client;
    client.Close();
    EXPECT_FALSE(client.getClientProperty());
}

TEST(UdpClientTests, Connect_EndPoint_NoThrow) {
    // UDP connect just sets the default remote — no real connection established.
    UdpClient client;
    IPEndPoint ep(IPAddress::Loopback, 12345);
    EXPECT_NO_THROW(client.Connect(ep));
}

TEST(UdpClientTests, Connect_InvalidHostname_Throws) {
    UdpClient client;
    EXPECT_THROW(client.Connect("this.host.does.not.exist.invalid.example", 5000), System::Net::Sockets::SocketException);
}

TEST(UdpClientTests, Send_ThrowsWithoutConnect) {
    UdpClient client;
    std::vector<SharpRuntime::bytecs> data = {0x01, 0x02, 0x03};
    EXPECT_THROW((void)client.Send(data, 3), System::InvalidOperationException);
}

// Regression: Send() previously cast `bytes` straight to size_t with no bounds check at all,
// letting an oversized (or negative, wrapping to huge) count read past the end of the datagram
// buffer -- confirmed via a standalone ASan repro (heap-buffer-overflow) before this fix.
TEST(UdpClientTests, Send_BytesExceedsBufferSize_Throws) {
    UdpClient client;
    IPEndPoint ep(IPAddress::Loopback, 12345);
    client.Connect(ep);
    std::vector<SharpRuntime::bytecs> data = {0x01, 0x02, 0x03};
    EXPECT_THROW((void)client.Send(data, 1000), System::ArgumentOutOfRangeException);
}

TEST(UdpClientTests, Send_NegativeBytes_Throws) {
    UdpClient client;
    IPEndPoint ep(IPAddress::Loopback, 12345);
    client.Connect(ep);
    std::vector<SharpRuntime::bytecs> data = {0x01, 0x02, 0x03};
    EXPECT_THROW((void)client.Send(data, -1), System::ArgumentOutOfRangeException);
}

TEST(UdpClientTests, Close_NoThrow) {
    UdpClient client;
    EXPECT_NO_THROW(client.Close());
}

// ===========================================================================
// NetworkStream — POSIX-only tests (socketpair is not available on Windows/Emscripten)
// ===========================================================================

#if !defined(_WIN32) && !defined(__EMSCRIPTEN__)

TEST(NetworkStreamTests, CanRead_True) {
    int fds[2];
    ASSERT_EQ(0, ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    NetworkStream ns(fds[0]);
    EXPECT_TRUE(ns.getCanReadProperty());
    ::close(fds[1]);
    ns.Close();
}

TEST(NetworkStreamTests, CanWrite_True) {
    int fds[2];
    ASSERT_EQ(0, ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    NetworkStream ns(fds[0]);
    EXPECT_TRUE(ns.getCanWriteProperty());
    ::close(fds[1]);
    ns.Close();
}

TEST(NetworkStreamTests, ReadWrite_RoundTrip) {
    int fds[2];
    ASSERT_EQ(0, ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    NetworkStream writer(fds[0]);
    NetworkStream reader(fds[1]);

    SharpRuntime::bytecs out[3] = {0xAA, 0xBB, 0xCC};
    writer.Write(out, 0, 3);

    SharpRuntime::bytecs in[3] = {};
    auto n = reader.Read(in, 0, 3);
    EXPECT_EQ(3, n);
    EXPECT_EQ(0xAA, in[0]);
    EXPECT_EQ(0xBB, in[1]);
    EXPECT_EQ(0xCC, in[2]);
}

TEST(NetworkStreamTests, GetLength_Throws) {
    int fds[2];
    ASSERT_EQ(0, ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    NetworkStream ns(fds[0]);
    EXPECT_THROW((void)ns.getLengthProperty(), System::NotSupportedException);
    ::close(fds[1]);
}

TEST(NetworkStreamTests, CanRead_False_AfterClose) {
    int fds[2];
    ASSERT_EQ(0, ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    NetworkStream ns(fds[0]);
    ::close(fds[1]);
    ns.Close();
    EXPECT_FALSE(ns.getCanReadProperty());
}

// Regression: Read/Write had no argument validation at all, unlike this codebase's other Stream
// implementations (e.g. FileStream::Read). A negative `count` wrapped to a huge size_t once cast
// for the recv()/send() call, letting the kernel write past the end of a small destination
// buffer -- confirmed via an ASan repro (stack-buffer-overflow) before this fix.
TEST(NetworkStreamTests, Read_NegativeCount_Throws) {
    int fds[2];
    ASSERT_EQ(0, ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    NetworkStream ns(fds[0]);
    SharpRuntime::bytecs buffer[4] = {};
    EXPECT_THROW(ns.Read(buffer, 0, -1), System::ArgumentOutOfRangeException);
    ::close(fds[1]);
}

TEST(NetworkStreamTests, Read_NegativeOffset_Throws) {
    int fds[2];
    ASSERT_EQ(0, ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    NetworkStream ns(fds[0]);
    SharpRuntime::bytecs buffer[4] = {};
    EXPECT_THROW(ns.Read(buffer, -1, 1), System::ArgumentOutOfRangeException);
    ::close(fds[1]);
}

TEST(NetworkStreamTests, Write_NegativeCount_Throws) {
    int fds[2];
    ASSERT_EQ(0, ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    NetworkStream ns(fds[0]);
    SharpRuntime::bytecs buffer[4] = {1, 2, 3, 4};
    EXPECT_THROW(ns.Write(buffer, 0, -1), System::ArgumentOutOfRangeException);
    ::close(fds[1]);
}

TEST(NetworkStreamTests, Write_NegativeOffset_Throws) {
    int fds[2];
    ASSERT_EQ(0, ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds));
    NetworkStream ns(fds[0]);
    SharpRuntime::bytecs buffer[4] = {1, 2, 3, 4};
    EXPECT_THROW(ns.Write(buffer, -1, 1), System::ArgumentOutOfRangeException);
    ::close(fds[1]);
}

#endif // !_WIN32 && !__EMSCRIPTEN__
