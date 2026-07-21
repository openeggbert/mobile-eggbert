// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <array>
#include <cstring>
#include <thread>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Convert.hpp"
#include "System/Net/IPEndPoint.hpp"
#include "System/Net/Sockets/Socket.hpp"
#include "System/Net/WebSockets/ClientWebSocket.hpp"
#include "System/PlatformNotSupportedException.hpp"
#include "System/Uri.hpp"

using namespace System::Net::WebSockets;
using System::Net::IPAddress;
using System::Net::IPEndPoint;
using System::Net::Sockets::Socket;

namespace {

// Minimal test-only SHA-1 (RFC 3174), duplicated from the production implementation so the mock
// server below can compute a correct Sec-WebSocket-Accept without depending on ClientWebSocket's
// private internals.
std::array<uint8_t, 20> testSha1(const std::string& input) {
    uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE, h3 = 0x10325476, h4 = 0xC3D2E1F0;
    std::vector<uint8_t> msg(input.begin(), input.end());
    uint64_t bitLen = static_cast<uint64_t>(msg.size()) * 8;
    msg.push_back(0x80);
    while (msg.size() % 64 != 56) msg.push_back(0);
    for (int i = 7; i >= 0; --i) msg.push_back(static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFF));

    for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        std::array<uint32_t, 80> w{};
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(msg[chunk + i * 4]) << 24) | (static_cast<uint32_t>(msg[chunk + i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(msg[chunk + i * 4 + 2]) << 8) | static_cast<uint32_t>(msg[chunk + i * 4 + 3]);
        }
        for (int i = 16; i < 80; ++i) {
            uint32_t v = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
            w[i] = (v << 1) | (v >> 31);
        }
        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
        for (int i = 0; i < 80; ++i) {
            uint32_t f, k;
            if (i < 20) { f = (b & c) | ((~b) & d); k = 0x5A827999; }
            else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else { f = b ^ c ^ d; k = 0xCA62C1D6; }
            uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
            e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = temp;
        }
        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
    }
    std::array<uint8_t, 20> digest{};
    uint32_t hs[5] = {h0, h1, h2, h3, h4};
    for (int i = 0; i < 5; ++i) {
        digest[i * 4] = static_cast<uint8_t>((hs[i] >> 24) & 0xFF);
        digest[i * 4 + 1] = static_cast<uint8_t>((hs[i] >> 16) & 0xFF);
        digest[i * 4 + 2] = static_cast<uint8_t>((hs[i] >> 8) & 0xFF);
        digest[i * 4 + 3] = static_cast<uint8_t>(hs[i] & 0xFF);
    }
    return digest;
}

std::vector<SharpRuntime::bytecs> toBytes(const std::string& s) { return std::vector<SharpRuntime::bytecs>(s.begin(), s.end()); }

std::string readUntilHeadersEnd(Socket& socket) {
    std::string response;
    std::vector<SharpRuntime::bytecs> one(1);
    while (response.size() < 4 || response.compare(response.size() - 4, 4, "\r\n\r\n") != 0) {
        SharpRuntime::intcs n = socket.Receive(one);
        if (n == 0) break;
        response += static_cast<char>(one[0]);
    }
    return response;
}

std::vector<SharpRuntime::bytecs> buildServerFrame(SharpRuntime::bytecs opcode, const std::string& payload, bool fin = true) {
    std::vector<SharpRuntime::bytecs> frame;
    frame.push_back(static_cast<SharpRuntime::bytecs>((fin ? 0x80 : 0x00) | opcode));
    frame.push_back(static_cast<SharpRuntime::bytecs>(payload.size())); // test payloads are always < 126 bytes
    frame.insert(frame.end(), payload.begin(), payload.end());
    return frame;
}

// Reads one (possibly client-masked) frame from `socket`, returning the opcode and unmasked payload.
std::pair<SharpRuntime::bytecs, std::string> readClientFrame(Socket& socket) {
    std::vector<SharpRuntime::bytecs> header(2);
    SharpRuntime::intcs headerBytes = socket.Receive(header);
    EXPECT_EQ(headerBytes, 2);
    SharpRuntime::bytecs opcode = header[0] & 0x0F;
    bool masked = (header[1] & 0x80) != 0;
    uint64_t len = header[1] & 0x7F;
    SharpRuntime::bytecs maskKey[4] = {0, 0, 0, 0};
    if (masked) {
        std::vector<SharpRuntime::bytecs> maskBuf(4);
        socket.Receive(maskBuf);
        std::memcpy(maskKey, maskBuf.data(), 4);
    }
    std::vector<SharpRuntime::bytecs> payload(static_cast<size_t>(len));
    if (len > 0) {
        size_t total = 0;
        while (total < len) {
            std::vector<SharpRuntime::bytecs> chunk(static_cast<size_t>(len) - total);
            SharpRuntime::intcs n = socket.Receive(chunk);
            std::memcpy(payload.data() + total, chunk.data(), static_cast<size_t>(n));
            total += static_cast<size_t>(n);
        }
    }
    if (masked) {
        for (size_t i = 0; i < payload.size(); ++i) payload[i] = static_cast<SharpRuntime::bytecs>(payload[i] ^ maskKey[i % 4]);
    }
    return {opcode, std::string(payload.begin(), payload.end())};
}

} // namespace

TEST(ClientWebSocketTests, FullHandshakeSendReceiveClose) {
    Socket listener(System::Net::Sockets::AddressFamily::InterNetwork, System::Net::Sockets::SocketType::Stream,
                     System::Net::Sockets::ProtocolType::Tcp);
    listener.Bind(IPEndPoint(IPAddress::Loopback, 0));
    listener.Listen();
    auto local = std::dynamic_pointer_cast<IPEndPoint>(listener.getLocalEndPointProperty());
    ASSERT_NE(local, nullptr);
    SharpRuntime::intcs port = local->getPortProperty();

    std::thread serverThread([&]() {
        auto serverSocket = listener.Accept();

        std::string request = readUntilHeadersEnd(*serverSocket);
        size_t keyPos = request.find("Sec-WebSocket-Key: ");
        ASSERT_NE(keyPos, std::string::npos);
        size_t keyStart = keyPos + std::string("Sec-WebSocket-Key: ").size();
        size_t keyEnd = request.find("\r\n", keyStart);
        std::string key = request.substr(keyStart, keyEnd - keyStart);

        auto digest = testSha1(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
        std::string accept = System::Convert::ToBase64String(std::vector<SharpRuntime::bytecs>(digest.begin(), digest.end()));

        std::string response = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
                                "Sec-WebSocket-Accept: " +
                                accept + "\r\n\r\n";
        serverSocket->Send(toBytes(response));

        auto [opcode, payload] = readClientFrame(*serverSocket);
        EXPECT_EQ(opcode, 0x1); // text
        EXPECT_EQ(payload, "hello");

        auto echoFrame = buildServerFrame(0x1, payload);
        serverSocket->Send(echoFrame);

        auto [closeOpcode, closePayload] = readClientFrame(*serverSocket);
        EXPECT_EQ(closeOpcode, 0x8);
        auto closeFrame = buildServerFrame(0x8, closePayload);
        serverSocket->Send(closeFrame);

        serverSocket->Close();
    });

    ClientWebSocket client;
    System::Uri uri("ws://127.0.0.1:" + std::to_string(port) + "/");
    client.ConnectAsync(uri).Wait();
    EXPECT_EQ(client.getStateProperty(), WebSocketState::Open);

    std::vector<SharpRuntime::bytecs> sendBuf = toBytes("hello");
    client.SendAsync(sendBuf, WebSocketMessageType::Text).Wait();

    std::vector<SharpRuntime::bytecs> recvBuf(64);
    auto result = client.ReceiveAsync(recvBuf).getResultProperty();
    EXPECT_EQ(result.getMessageTypeProperty(), WebSocketMessageType::Text);
    EXPECT_TRUE(result.getEndOfMessageProperty());
    std::string received(recvBuf.begin(), recvBuf.begin() + result.getCountProperty());
    EXPECT_EQ(received, "hello");

    client.CloseAsync(WebSocketCloseStatus::NormalClosure, std::string("bye")).Wait();
    EXPECT_EQ(client.getStateProperty(), WebSocketState::Closed);

    serverThread.join();
}

// Regression test (ticket 264): readFrame() used to read the 64-bit extended payload length
// straight off the wire with no upper bound, so a malicious/misbehaving server sending a huge
// length would make readExact()'s buffer.resize(n) attempt a correspondingly huge allocation --
// throwing a raw std::length_error/std::bad_alloc (invisible to code catching
// System::Exception&) instead of a clean WebSocketException. The server here sends a frame
// header claiming a length near UINT64_MAX and never sends any payload bytes at all; the client
// must reject based on the length alone, before attempting to read (or allocate for) a payload.
TEST(ClientWebSocketTests, ReceiveAsync_HugeFrameLength_ThrowsWebSocketException) {
    Socket listener(System::Net::Sockets::AddressFamily::InterNetwork, System::Net::Sockets::SocketType::Stream,
                     System::Net::Sockets::ProtocolType::Tcp);
    listener.Bind(IPEndPoint(IPAddress::Loopback, 0));
    listener.Listen();
    auto local = std::dynamic_pointer_cast<IPEndPoint>(listener.getLocalEndPointProperty());
    ASSERT_NE(local, nullptr);
    SharpRuntime::intcs port = local->getPortProperty();

    std::thread serverThread([&]() {
        auto serverSocket = listener.Accept();

        std::string request = readUntilHeadersEnd(*serverSocket);
        size_t keyPos = request.find("Sec-WebSocket-Key: ");
        ASSERT_NE(keyPos, std::string::npos);
        size_t keyStart = keyPos + std::string("Sec-WebSocket-Key: ").size();
        size_t keyEnd = request.find("\r\n", keyStart);
        std::string key = request.substr(keyStart, keyEnd - keyStart);

        auto digest = testSha1(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
        std::string accept = System::Convert::ToBase64String(std::vector<SharpRuntime::bytecs>(digest.begin(), digest.end()));
        std::string response = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
                                "Sec-WebSocket-Accept: " +
                                accept + "\r\n\r\n";
        serverSocket->Send(toBytes(response));

        std::vector<SharpRuntime::bytecs> frame;
        frame.push_back(0x82); // fin=1, opcode=0x2 (binary)
        frame.push_back(0xFF); // unmasked (server frames aren't masked), length marker = 127 (64-bit extended)
        uint64_t hugeLen = 0xFFFFFFFFFFFFFFFFULL;
        for (int i = 7; i >= 0; --i) frame.push_back(static_cast<SharpRuntime::bytecs>((hugeLen >> (i * 8)) & 0xFF));
        serverSocket->Send(frame);

        serverSocket->Close();
    });

    ClientWebSocket client;
    System::Uri uri("ws://127.0.0.1:" + std::to_string(port) + "/");
    client.ConnectAsync(uri).Wait();
    EXPECT_EQ(client.getStateProperty(), WebSocketState::Open);

    std::vector<SharpRuntime::bytecs> recvBuf(64);
    EXPECT_THROW(client.ReceiveAsync(recvBuf).getResultProperty(), WebSocketException);

    serverThread.join();
}

// Regression test for a wave-3 audit finding: SendAsync/ReceiveAsync used to do
// buffer.data() + offset with no bounds check against buffer.size() -- an out-of-bounds
// read (Send) or write (Receive) whenever offset+count exceeded the buffer. Verified
// against WebSocketValidate.cs's ValidateBuffer. Argument validation happens synchronously
// before any socket I/O (matching real .NET's async-method-validates-synchronously
// convention, confirmed in ManagedWebSocket.cs), so this doesn't need a live connection.
TEST(ClientWebSocketTests, SendAsync_OffsetCountOutOfRange_Throws) {
    ClientWebSocket client;
    std::vector<SharpRuntime::bytecs> buf{'a', 'b', 'c'};
    EXPECT_THROW(client.SendAsync(buf, 2, 5, WebSocketMessageType::Text, true),
                 System::ArgumentOutOfRangeException);
    EXPECT_THROW(client.SendAsync(buf, -1, 1, WebSocketMessageType::Text, true),
                 System::ArgumentOutOfRangeException);
    EXPECT_THROW(client.SendAsync(buf, 0, -1, WebSocketMessageType::Text, true),
                 System::ArgumentOutOfRangeException);
}

TEST(ClientWebSocketTests, ReceiveAsync_OffsetCountOutOfRange_Throws) {
    ClientWebSocket client;
    std::vector<SharpRuntime::bytecs> buf(4);
    EXPECT_THROW(client.ReceiveAsync(buf, 2, 5), System::ArgumentOutOfRangeException);
    EXPECT_THROW(client.ReceiveAsync(buf, -1, 1), System::ArgumentOutOfRangeException);
}

TEST(ClientWebSocketTests, WrongSchemeThrows) {
    ClientWebSocket client;
    System::Uri uri("wss://127.0.0.1:1/");
    EXPECT_THROW(client.ConnectAsync(uri).Wait(), System::PlatformNotSupportedException);
}

// Regression test for a wave-3 audit finding: AddSubProtocol's duplicate check compared
// case-sensitively, silently allowing "chat" and "Chat" to both be added as if they were
// distinct protocols. Verified against ClientWebSocketOptions.cs's AddSubProtocol, which
// compares via StringComparison.OrdinalIgnoreCase.
TEST(ClientWebSocketTests, AddSubProtocol_DuplicateDifferingOnlyByCase_Throws) {
    ClientWebSocket client;
    client.getOptionsProperty().AddSubProtocol("chat");
    EXPECT_THROW(client.getOptionsProperty().AddSubProtocol("Chat"), System::ArgumentException);
}
