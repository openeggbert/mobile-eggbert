// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/WebSockets/ClientWebSocket.hpp"
#include <array>
#include <cstring>
#include <map>
#include <random>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Convert.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Net/IPEndPoint.hpp"
#include "System/Net/WebSockets/WebSocketException.hpp"
#include "System/PlatformNotSupportedException.hpp"

namespace System::Net::WebSockets {

using SharpRuntime::bytecs;
using SharpRuntime::intcs;

namespace {

    // --- Minimal, self-contained SHA-1 (RFC 3174) -------------------------------------------
    // Not System::Security::Cryptography::SHA1 (that namespace is not yet ported/decided) —
    // this is a private implementation detail needed only for the Sec-WebSocket-Accept digest
    // in the RFC 6455 handshake, kept file-local rather than exposed as a public crypto API.
    std::array<uint8_t, 20> sha1(const std::string& input) {
        uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE, h3 = 0x10325476, h4 = 0xC3D2E1F0;

        std::vector<uint8_t> msg(input.begin(), input.end());
        uint64_t bitLen = static_cast<uint64_t>(msg.size()) * 8;
        msg.push_back(0x80);
        while (msg.size() % 64 != 56) msg.push_back(0);
        for (int i = 7; i >= 0; --i) msg.push_back(static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFF));

        for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
            std::array<uint32_t, 80> w{};
            for (int i = 0; i < 16; ++i) {
                w[i] = (static_cast<uint32_t>(msg[chunk + i * 4]) << 24) |
                       (static_cast<uint32_t>(msg[chunk + i * 4 + 1]) << 16) |
                       (static_cast<uint32_t>(msg[chunk + i * 4 + 2]) << 8) |
                       static_cast<uint32_t>(msg[chunk + i * 4 + 3]);
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
                e = d;
                d = c;
                c = (b << 30) | (b >> 2);
                b = a;
                a = temp;
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

    std::string toLower(std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    std::vector<bytecs> toBytes(const std::string& s) { return std::vector<bytecs>(s.begin(), s.end()); }

    constexpr const char* kWebSocketGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    std::array<bytecs, 16> randomBytes16() {
        std::random_device rd;
        std::array<bytecs, 16> bytes{};
        for (auto& b : bytes) b = static_cast<bytecs>(rd() & 0xFF);
        return bytes;
    }

    uint32_t randomMaskingKey() {
        std::random_device rd;
        return rd();
    }

} // namespace

void ClientWebSocket::performHandshake(const System::Uri& uri) {
    const std::string& scheme = uri.getSchemeProperty();
    if (scheme == "wss") {
        throw System::PlatformNotSupportedException(
            "wss:// requires TLS, which this runtime's HttpClient/WebSocket implementations do not support.");
    }
    if (scheme != "ws") {
        throw System::ArgumentException("The URI scheme must be 'ws' or 'wss'.", "uri");
    }

    intcs port = uri.getPortProperty();
    if (port <= 0) port = 80;

    socket_ = std::make_unique<System::Net::Sockets::Socket>(
        System::Net::Sockets::AddressFamily::InterNetwork, System::Net::Sockets::SocketType::Stream,
        System::Net::Sockets::ProtocolType::Tcp);
    socket_->Connect(uri.getHostProperty(), port);

    auto keyBytes = randomBytes16();
    std::string secWebSocketKey = System::Convert::ToBase64String(std::vector<bytecs>(keyBytes.begin(), keyBytes.end()));

    std::string request = "GET " + uri.getPathAndQueryProperty() + " HTTP/1.1\r\n";
    request += "Host: " + uri.getHostProperty() + ":" + std::to_string(port) + "\r\n";
    request += "Upgrade: websocket\r\n";
    request += "Connection: Upgrade\r\n";
    request += "Sec-WebSocket-Key: " + secWebSocketKey + "\r\n";
    request += "Sec-WebSocket-Version: 13\r\n";
    for (const auto& [name, value] : options_.getRequestHeadersProperty()) {
        request += name + ": " + value + "\r\n";
    }
    const auto& subProtocols = options_.getRequestedSubProtocolsProperty();
    if (!subProtocols.empty()) {
        std::string joined;
        for (size_t i = 0; i < subProtocols.size(); ++i) {
            if (i > 0) joined += ", ";
            joined += subProtocols[i];
        }
        request += "Sec-WebSocket-Protocol: " + joined + "\r\n";
    }
    request += "\r\n";

    auto requestBytes = toBytes(request);
    socket_->Send(requestBytes);

    // Read the HTTP response headers, one byte at a time, until "\r\n\r\n".
    std::string response;
    std::vector<bytecs> one(1);
    while (response.size() < 4 || response.compare(response.size() - 4, 4, "\r\n\r\n") != 0) {
        intcs n = socket_->Receive(one);
        if (n == 0) {
            throw WebSocketException(WebSocketError::ConnectionClosedPrematurely,
                                      "The connection was closed before the WebSocket handshake completed.");
        }
        response += static_cast<char>(one[0]);
        if (response.size() > 16384) {
            throw WebSocketException(WebSocketError::HeaderError, "The WebSocket handshake response is too large.");
        }
    }

    size_t lineEnd = response.find("\r\n");
    std::string statusLine = response.substr(0, lineEnd);
    if (statusLine.find(" 101 ") == std::string::npos) {
        throw WebSocketException(WebSocketError::NotAWebSocket,
                                  "The server did not respond with a valid WebSocket handshake: " + statusLine);
    }

    std::map<std::string, std::string> headers;
    size_t pos = lineEnd + 2;
    while (pos < response.size()) {
        size_t next = response.find("\r\n", pos);
        if (next == std::string::npos || next == pos) break;
        std::string line = response.substr(pos, next - pos);
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            headers[toLower(trim(line.substr(0, colon)))] = trim(line.substr(colon + 1));
        }
        pos = next + 2;
    }

    auto upgradeIt = headers.find("upgrade");
    auto acceptIt = headers.find("sec-websocket-accept");
    if (upgradeIt == headers.end() || toLower(upgradeIt->second) != "websocket" || acceptIt == headers.end()) {
        throw WebSocketException(WebSocketError::NotAWebSocket, "The WebSocket handshake response is missing required headers.");
    }

    auto expectedDigest = sha1(secWebSocketKey + kWebSocketGuid);
    std::string expectedAccept =
        System::Convert::ToBase64String(std::vector<bytecs>(expectedDigest.begin(), expectedDigest.end()));
    if (acceptIt->second != expectedAccept) {
        throw WebSocketException(WebSocketError::HeaderError, "Sec-WebSocket-Accept did not match the expected value.");
    }

    auto protoIt = headers.find("sec-websocket-protocol");
    if (protoIt != headers.end()) {
        subProtocol_ = protoIt->second;
    }

    state_ = WebSocketState::Open;
}

System::Threading::Tasks::Task
ClientWebSocket::ConnectAsync(const System::Uri& uri, System::Threading::CancellationToken /*cancellationToken*/) {
    if (connectStarted_) {
        throw System::InvalidOperationException("ConnectAsync may only be called once.");
    }
    connectStarted_ = true;
    state_ = WebSocketState::Connecting;
    options_.setToReadOnly();

    return System::Threading::Tasks::Task([this, uri]() {
        try {
            performHandshake(uri);
        } catch (...) {
            Dispose();
            throw;
        }
    });
}

void ClientWebSocket::sendFrame(bytecs opcode, const bytecs* data, size_t len, bool fin) {
    std::lock_guard<std::mutex> lock(sendMutex_);

    std::vector<bytecs> frame;
    frame.push_back(static_cast<bytecs>((fin ? 0x80 : 0x00) | (opcode & 0x0F)));

    uint32_t maskingKey = randomMaskingKey();
    bytecs maskBytes[4] = {
        static_cast<bytecs>((maskingKey >> 24) & 0xFF),
        static_cast<bytecs>((maskingKey >> 16) & 0xFF),
        static_cast<bytecs>((maskingKey >> 8) & 0xFF),
        static_cast<bytecs>(maskingKey & 0xFF),
    };

    if (len <= 125) {
        frame.push_back(static_cast<bytecs>(0x80 | len));
    } else if (len <= 0xFFFF) {
        frame.push_back(static_cast<bytecs>(0x80 | 126));
        frame.push_back(static_cast<bytecs>((len >> 8) & 0xFF));
        frame.push_back(static_cast<bytecs>(len & 0xFF));
    } else {
        frame.push_back(static_cast<bytecs>(0x80 | 127));
        for (int i = 7; i >= 0; --i) frame.push_back(static_cast<bytecs>((static_cast<uint64_t>(len) >> (i * 8)) & 0xFF));
    }

    frame.insert(frame.end(), maskBytes, maskBytes + 4);

    size_t frameHeaderLen = frame.size();
    frame.resize(frameHeaderLen + len);
    for (size_t i = 0; i < len; ++i) {
        frame[frameHeaderLen + i] = static_cast<bytecs>(data[i] ^ maskBytes[i % 4]);
    }

    socket_->Send(frame);
}

namespace {

    void readExact(System::Net::Sockets::Socket& socket, std::vector<bytecs>& buffer, size_t n) {
        buffer.resize(n);
        size_t total = 0;
        while (total < n) {
            std::vector<bytecs> chunk(n - total);
            intcs received = socket.Receive(chunk, 0, static_cast<intcs>(n - total), System::Net::Sockets::SocketFlags::None);
            if (received == 0) {
                throw WebSocketException(WebSocketError::ConnectionClosedPrematurely,
                                          "The remote endpoint closed the connection.");
            }
            std::memcpy(buffer.data() + total, chunk.data(), static_cast<size_t>(received));
            total += static_cast<size_t>(received);
        }
    }

    // Verified against WebSocketValidate.cs's ValidateBuffer: negative offset/count and an
    // offset/count exceeding the buffer are all rejected. Previously SendAsync/ReceiveAsync
    // did buffer.data() + offset / std::memcpy(..., count) with no bounds check at all -- an
    // out-of-bounds read (Send) or write (Receive) whenever offset+count exceeded the buffer.
    template<typename Buf>
    void validateWebSocketBuffer(const Buf& buffer, intcs offset, intcs count) {
        if (offset < 0 || static_cast<size_t>(offset) > buffer.size())
            throw System::ArgumentOutOfRangeException("offset");
        if (count < 0 || static_cast<size_t>(count) > buffer.size() - static_cast<size_t>(offset))
            throw System::ArgumentOutOfRangeException("count");
    }

} // namespace

ClientWebSocket::RawFrame ClientWebSocket::readFrame() {
    std::vector<bytecs> header;
    readExact(*socket_, header, 2);

    bool fin = (header[0] & 0x80) != 0;
    bytecs opcode = header[0] & 0x0F;
    bool masked = (header[1] & 0x80) != 0;
    uint64_t len = header[1] & 0x7F;

    if (len == 126) {
        std::vector<bytecs> ext;
        readExact(*socket_, ext, 2);
        len = (static_cast<uint64_t>(ext[0]) << 8) | ext[1];
    } else if (len == 127) {
        std::vector<bytecs> ext;
        readExact(*socket_, ext, 8);
        len = 0;
        for (int i = 0; i < 8; ++i) len = (len << 8) | ext[static_cast<size_t>(i)];
    }

    // The 64-bit extended length comes directly off the wire with no upper bound otherwise --
    // a malicious or misbehaving server could send an arbitrarily large value (e.g. close to
    // UINT64_MAX) and readExact()'s buffer.resize(n) would attempt a correspondingly huge
    // allocation, throwing a raw std::length_error/std::bad_alloc (invisible to code catching
    // System::Exception&) instead of a clean, catchable WebSocketException. 256 MiB is a
    // generous cap for a single WebSocket frame -- comfortably above any realistic legitimate
    // message -- chosen defensively, matching the existing 16384-byte cap already applied to
    // the handshake response a few lines up in this same file.
    constexpr uint64_t kMaxFramePayloadBytes = 256ull * 1024 * 1024;
    if (len > kMaxFramePayloadBytes) {
        throw WebSocketException(WebSocketError::HeaderError,
                                  "The WebSocket frame payload length exceeds the maximum allowed size.");
    }

    bytecs maskKey[4] = {0, 0, 0, 0};
    if (masked) {
        std::vector<bytecs> maskBuf;
        readExact(*socket_, maskBuf, 4);
        std::memcpy(maskKey, maskBuf.data(), 4);
    }

    RawFrame result;
    result.fin = fin;
    result.opcode = opcode;
    readExact(*socket_, result.payload, static_cast<size_t>(len));
    if (masked) {
        for (size_t i = 0; i < result.payload.size(); ++i) {
            result.payload[i] = static_cast<bytecs>(result.payload[i] ^ maskKey[i % 4]);
        }
    }
    return result;
}

void ClientWebSocket::sendCloseFrame(WebSocketCloseStatus closeStatus, const std::optional<std::string>& statusDescription) {
    std::vector<bytecs> payload;
    auto code = static_cast<uint16_t>(closeStatus);
    payload.push_back(static_cast<bytecs>((code >> 8) & 0xFF));
    payload.push_back(static_cast<bytecs>(code & 0xFF));
    if (statusDescription.has_value()) {
        auto bytes = toBytes(*statusDescription);
        payload.insert(payload.end(), bytes.begin(), bytes.end());
    }
    sendFrame(0x8, payload.data(), payload.size(), true);
}

System::Threading::Tasks::Task
ClientWebSocket::SendAsync(const std::vector<bytecs>& buffer, intcs offset, intcs count, WebSocketMessageType messageType,
                            bool endOfMessage, System::Threading::CancellationToken /*cancellationToken*/) {
    validateWebSocketBuffer(buffer, offset, count);
    return System::Threading::Tasks::Task([this, &buffer, offset, count, messageType, endOfMessage]() {
        WebSocket::ThrowOnInvalidState(state_, {WebSocketState::Open, WebSocketState::CloseReceived});
        bytecs opcode;
        if (sendContinuation_) {
            opcode = 0x0;
        } else {
            opcode = messageType == WebSocketMessageType::Text ? 0x1 : 0x2;
        }
        sendFrame(opcode, buffer.data() + offset, static_cast<size_t>(count), endOfMessage);
        sendContinuation_ = !endOfMessage;
    });
}

System::Threading::Tasks::TaskT<WebSocketReceiveResult>
ClientWebSocket::ReceiveAsync(std::vector<bytecs>& buffer, intcs offset, intcs count,
                               System::Threading::CancellationToken /*cancellationToken*/) {
    validateWebSocketBuffer(buffer, offset, count);
    return System::Threading::Tasks::TaskT<WebSocketReceiveResult>([this, &buffer, offset, count]() {
        WebSocket::ThrowOnInvalidState(state_, {WebSocketState::Open, WebSocketState::CloseSent});

        if (recvLeftoverPos_ < recvLeftover_.size()) {
            size_t remaining = recvLeftover_.size() - recvLeftoverPos_;
            size_t n = std::min(static_cast<size_t>(count), remaining);
            std::memcpy(buffer.data() + offset, recvLeftover_.data() + recvLeftoverPos_, n);
            recvLeftoverPos_ += n;
            bool isLast = recvLeftoverPos_ == recvLeftover_.size();
            if (isLast) {
                recvLeftover_.clear();
                recvLeftoverPos_ = 0;
            }
            return WebSocketReceiveResult(static_cast<intcs>(n), recvLeftoverType_, isLast);
        }

        while (true) {
            RawFrame frame = readFrame();
            switch (frame.opcode) {
                case 0x9: { // Ping
                    sendFrame(0xA, frame.payload.data(), frame.payload.size(), true);
                    continue;
                }
                case 0xA: // Pong
                    continue;
                case 0x8: { // Close
                    std::optional<WebSocketCloseStatus> receivedStatus;
                    std::optional<std::string> receivedDescription;
                    if (frame.payload.size() >= 2) {
                        uint16_t code = (static_cast<uint16_t>(frame.payload[0]) << 8) | frame.payload[1];
                        receivedStatus = static_cast<WebSocketCloseStatus>(code);
                        if (frame.payload.size() > 2) {
                            receivedDescription = std::string(frame.payload.begin() + 2, frame.payload.end());
                        }
                    }
                    closeStatus_ = receivedStatus;
                    closeStatusDescription_ = receivedDescription;
                    state_ = state_ == WebSocketState::CloseSent ? WebSocketState::Closed : WebSocketState::CloseReceived;
                    return WebSocketReceiveResult(0, WebSocketMessageType::Close, true, receivedStatus, receivedDescription);
                }
                default: { // Continuation(0x0)/Text(0x1)/Binary(0x2)
                    WebSocketMessageType type =
                        frame.opcode == 0x0 ? fragmentType_ : (frame.opcode == 0x1 ? WebSocketMessageType::Text : WebSocketMessageType::Binary);
                    if (frame.opcode != 0x0) fragmentType_ = type;

                    if (frame.payload.size() <= static_cast<size_t>(count)) {
                        std::memcpy(buffer.data() + offset, frame.payload.data(), frame.payload.size());
                        return WebSocketReceiveResult(static_cast<intcs>(frame.payload.size()), type, frame.fin);
                    }

                    std::memcpy(buffer.data() + offset, frame.payload.data(), static_cast<size_t>(count));
                    recvLeftover_.assign(frame.payload.begin() + count, frame.payload.end());
                    recvLeftoverPos_ = 0;
                    recvLeftoverType_ = type;
                    return WebSocketReceiveResult(count, type, false);
                }
            }
        }
    });
}

System::Threading::Tasks::Task
ClientWebSocket::CloseOutputAsync(WebSocketCloseStatus closeStatus, const std::optional<std::string>& statusDescription,
                                    System::Threading::CancellationToken /*cancellationToken*/) {
    return System::Threading::Tasks::Task([this, closeStatus, statusDescription]() {
        WebSocket::ThrowOnInvalidState(state_, {WebSocketState::Open, WebSocketState::CloseReceived});
        sendCloseFrame(closeStatus, statusDescription);
        state_ = state_ == WebSocketState::CloseReceived ? WebSocketState::Closed : WebSocketState::CloseSent;
    });
}

System::Threading::Tasks::Task
ClientWebSocket::CloseAsync(WebSocketCloseStatus closeStatus, const std::optional<std::string>& statusDescription,
                              System::Threading::CancellationToken /*cancellationToken*/) {
    // Unlike ConnectAsync/SendAsync/ReceiveAsync (which all consistently comment out this same
    // parameter), this one used to be captured by name and threaded into the lambda without
    // ever actually being checked anywhere in the close-handshake wait loop below -- silently
    // misleading, since the parameter's presence implies cancellation works here specifically.
    // Wiring up real cancellation would need to interrupt the loop's blocking socket read
    // mid-flight, which only makes sense done consistently across all of this file's async
    // methods at once, not as a one-off partial fix for just this one; left for a follow-up
    // ticket, matching the honest "not implemented" stance the other three already take.
    return System::Threading::Tasks::Task([this, closeStatus, statusDescription]() {
        if (state_ == WebSocketState::Open) {
            sendCloseFrame(closeStatus, statusDescription);
            state_ = WebSocketState::CloseSent;
        }
        if (state_ == WebSocketState::CloseSent) {
            while (state_ != WebSocketState::Closed) {
                RawFrame frame = readFrame();
                if (frame.opcode == 0x8) {
                    if (frame.payload.size() >= 2) {
                        uint16_t code = (static_cast<uint16_t>(frame.payload[0]) << 8) | frame.payload[1];
                        closeStatus_ = static_cast<WebSocketCloseStatus>(code);
                        if (frame.payload.size() > 2) {
                            closeStatusDescription_ = std::string(frame.payload.begin() + 2, frame.payload.end());
                        }
                    }
                    state_ = WebSocketState::Closed;
                }
                // Any other frame received while waiting for the close handshake is discarded.
            }
        }
    });
}

void ClientWebSocket::Abort() {
    if (state_ != WebSocketState::Aborted && state_ != WebSocketState::Closed) {
        state_ = WebSocketState::Aborted;
    }
    Dispose();
}

void ClientWebSocket::Dispose() {
    if (socket_) {
        socket_->Close();
        socket_.reset();
    }
    if (state_ != WebSocketState::Aborted) {
        state_ = WebSocketState::Closed;
    }
}

} // namespace System::Net::WebSockets
