// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)

// Platform-specific socket includes — must not appear in the public header. Moved verbatim
// from HttpClient.cpp when the socket-level send/receive logic was extracted into this
// dedicated HttpMessageHandler implementation (ticket 1496).
#if defined(__EMSCRIPTEN__)
#  include "System/PlatformNotSupportedException.hpp"
#elif defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else // POSIX
#  include <sys/socket.h>
#  include <netdb.h>
#  include <unistd.h>
#endif

#include "System/Net/Http/HttpClientHandler.hpp"
#include "System/Net/Http/ByteArrayContent.hpp"
#include "System/Net/Http/HttpClient.hpp"
#include "System/Net/Http/HttpRequestException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/Uri.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace System::Net::Http {

// ---------------------------------------------------------------------------
// Platform typedefs
// ---------------------------------------------------------------------------

#if !defined(__EMSCRIPTEN__)
#  if defined(_WIN32)
using SocketFd = SOCKET;
static constexpr SocketFd kInvalidSocket = INVALID_SOCKET;
static void platformClose(SocketFd fd) { closesocket(fd); }
#  else
using SocketFd = int;
static constexpr SocketFd kInvalidSocket = -1;
static void platformClose(SocketFd fd) { ::close(fd); }
#  endif
#endif // !__EMSCRIPTEN__

// ---------------------------------------------------------------------------
// Socket helpers (POSIX + Windows; not compiled on Emscripten)
// ---------------------------------------------------------------------------

#if !defined(__EMSCRIPTEN__)

static SocketFd connectToHost(const std::string& host, int port) {
    struct addrinfo hints{};
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = nullptr;
    std::string portStr  = std::to_string(port);
    int rv = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res);
    if (rv != 0)
        throw HttpRequestException("HttpClient: DNS resolution failed for '" + host + "'.");

    SocketFd fd = kInvalidSocket;
    for (auto* p = res; p != nullptr; p = p->ai_next) {
        fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd == kInvalidSocket) continue;
        if (::connect(fd, p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0) break;
        platformClose(fd);
        fd = kInvalidSocket;
    }
    freeaddrinfo(res);

    if (fd == kInvalidSocket)
        throw HttpRequestException(
            "HttpClient: could not connect to " + host + ":" + std::to_string(port));
    return fd;
}

static void sendAll(SocketFd fd, const std::string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        int n = static_cast<int>(
            ::send(fd, data.c_str() + sent,
                   static_cast<int>(data.size() - sent), 0));
        if (n <= 0) throw HttpRequestException("HttpClient: send() failed.");
        sent += static_cast<size_t>(n);
    }
}

// Read one \r\n-terminated line from the socket, using buf as a carry buffer.
static std::string recvLine(SocketFd fd, std::string& buf) {
    while (true) {
        size_t pos = buf.find("\r\n");
        if (pos != std::string::npos) {
            std::string line = buf.substr(0, pos);
            buf = buf.substr(pos + 2);
            return line;
        }
        char tmp[2048];
        int n = static_cast<int>(::recv(fd, tmp, sizeof(tmp), 0));
        if (n <= 0) {
            std::string line = buf;
            buf.clear();
            return line;
        }
        buf.append(tmp, static_cast<size_t>(n));
    }
}

// Read exactly `count` bytes, drawing from buf first.
//
// If the connection closes (recv() returns <= 0) before `count` bytes have arrived, this
// throws rather than returning the truncated data collected so far -- a dropped connection
// mid-response (e.g. server crash, network failure while streaming a Content-Length-bounded or
// chunked body) must surface as a catchable error, not a "successful" but incomplete response.
// Real .NET surfaces this as an IOException with HttpRequestError.ResponseEnded ("The response
// ended prematurely."); mirrored here via the matching HttpRequestException overload for
// consistency with every other malformed-response failure path in this file.
static std::vector<uint8_t> recvExact(SocketFd fd, std::string& buf, size_t count) {
    while (buf.size() < count) {
        char tmp[4096];
        size_t want = std::min(sizeof(tmp), count - buf.size());
        int n = static_cast<int>(::recv(fd, tmp, static_cast<int>(want), 0));
        if (n <= 0) {
            throw HttpRequestException(HttpRequestError::ResponseEnded,
                "HttpClient: connection closed before the full response body was received.");
        }
        buf.append(tmp, static_cast<size_t>(n));
    }
    size_t take = std::min(buf.size(), count);
    std::vector<uint8_t> result(buf.begin(), buf.begin() + static_cast<long>(take));
    buf = buf.substr(take);
    return result;
}

// Read until connection closes.
static std::vector<uint8_t> recvAll(SocketFd fd, std::string& buf) {
    char tmp[4096];
    for (;;) {
        int n = static_cast<int>(::recv(fd, tmp, sizeof(tmp), 0));
        if (n <= 0) break;
        buf.append(tmp, static_cast<size_t>(n));
    }
    std::vector<uint8_t> result(buf.begin(), buf.end());
    buf.clear();
    return result;
}

static std::string strToLower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

static std::string trimValue(const std::string& s) {
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos) return "";
    size_t end   = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

#endif // !defined(__EMSCRIPTEN__)

// ---------------------------------------------------------------------------
// HttpClientHandler
// ---------------------------------------------------------------------------

HttpClientHandler::HttpClientHandler() {
#if defined(_WIN32) && !defined(__EMSCRIPTEN__)
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

HttpClientHandler::~HttpClientHandler() {
#if defined(_WIN32) && !defined(__EMSCRIPTEN__)
    WSACleanup();
#endif
}

std::shared_ptr<HttpResponseMessage> HttpClientHandler::Send(std::shared_ptr<HttpRequestMessage> request) {
    System::ArgumentNullException::ThrowIfNull(request.get(), "request");
#if defined(__EMSCRIPTEN__)
    (void)request;
    throw System::PlatformNotSupportedException(
        "HttpClient is not supported on Emscripten (no TCP sockets in single-threaded Wasm).");
#else
    const std::string& absoluteUrl = request->getRequestUriProperty();
    HttpClient::ParsedUrl purl = HttpClient::parseUrl(absoluteUrl);

    // Cookie support (ticket 1498): attach a Cookie request header built from the container
    // for this URI, unless the caller already set one explicitly.
    std::unordered_map<std::string, std::string> reqHeaders = request->getHeadersProperty();
    std::unique_ptr<System::Uri> uri;
    if (useCookies_ && cookieContainer_ && reqHeaders.find("Cookie") == reqHeaders.end()) {
        uri = std::make_unique<System::Uri>(absoluteUrl);
        std::string cookieHeader = cookieContainer_->GetCookieHeader(*uri);
        if (!cookieHeader.empty()) reqHeaders["Cookie"] = cookieHeader;
    }

    SocketFd fd = connectToHost(purl.host, purl.port);

    // Build request body
    std::shared_ptr<HttpContent> content = request->getContentProperty();
    std::string body;
    if (content) body = content->ReadAsString();

    // Build request headers
    std::string method = request->getMethodProperty().getMethodProperty();
    std::ostringstream req;
    req << method << " " << purl.path << " HTTP/1.1\r\n";
    req << "Host: " << purl.host;
    if (purl.port != 80) req << ":" << purl.port;
    req << "\r\n";
    req << "User-Agent: SharpRuntime-HttpClient/1.0\r\n";
    req << "Accept: */*\r\n";
    req << "Connection: close\r\n";

    for (const auto& [k, v] : reqHeaders) req << k << ": " << v << "\r\n";

    if (content && !body.empty()) {
        std::string ct = content->getContentTypeProperty();
        std::string cs = content->getCharSetProperty();
        if (!ct.empty()) {
            req << "Content-Type: " << ct;
            if (!cs.empty()) req << "; charset=" << cs;
            req << "\r\n";
        }
        req << "Content-Length: " << body.size() << "\r\n";
    }
    req << "\r\n";
    if (!body.empty()) req << body;

    sendAll(fd, req.str());

    // Parse response
    std::string buf;

    // Status line: HTTP/1.1 200 OK
    std::string statusLine = recvLine(fd, buf);
    HttpClient::ParsedStatusLine parsedStatus = HttpClient::parseStatusLine(statusLine);
    int statusCode = parsedStatus.statusCode;

    auto resp = std::make_shared<HttpResponseMessage>(
        static_cast<System::Net::HttpStatusCode>(statusCode));
    resp->setReasonPhraseProperty(parsedStatus.reason);

    // Response headers
    std::string transferEncoding;
    long long   contentLength = -1;
    std::vector<std::string> setCookieValues;

    for (;;) {
        std::string line = recvLine(fd, buf);
        if (line.empty()) break;
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string name  = line.substr(0, colon);
        std::string value = trimValue(line.substr(colon + 1));
        resp->setHeader(name, value);

        std::string nameLower = strToLower(name);
        if (nameLower == "transfer-encoding") transferEncoding = strToLower(value);
        if (nameLower == "set-cookie") {
            // A response may carry several Set-Cookie headers; HttpResponseMessage's header
            // map (like real .NET's dictionary-of-string) stores one value per name, so the
            // last one would otherwise silently overwrite the rest. Collect every occurrence
            // here specifically for cookie extraction below, in addition to the single-value
            // resp->setHeader() call above (kept for the general getHeader("Set-Cookie") case).
            setCookieValues.push_back(value);
        }
        if (nameLower == "content-length") {
            // Unguarded std::stoll() on a server-controlled value -- a malformed or
            // out-of-range Content-Length (e.g. non-numeric, or too large for long long) threw
            // a raw std::invalid_argument/std::out_of_range straight out of Send()/GetAsync()/
            // etc., invisible to code catching System::Exception&/HttpRequestException&. Same
            // bug class parseUrl()/parseStatusLine() were already fixed for; this one was
            // missed.
            try {
                contentLength = std::stoll(value);
            } catch (...) {
                throw HttpRequestException("HttpClient: malformed Content-Length header: '" + value + "'");
            }
        }
    }

    // Response body
    bool noBody = (method == "HEAD")
               || (statusCode >= 100 && statusCode < 200)
               || statusCode == 204
               || statusCode == 304;

    std::vector<uint8_t> bodyBytes;

    if (!noBody) {
        if (transferEncoding == "chunked") {
            for (;;) {
                std::string chunkLine = recvLine(fd, buf);
                // Strip optional chunk extensions (;...)
                size_t semi = chunkLine.find(';');
                if (semi != std::string::npos) chunkLine = chunkLine.substr(0, semi);
                if (chunkLine.empty()) break;
                // Same unguarded-std::sto*-on-server-controlled-input bug as the Content-Length
                // header above: a malformed (non-hex) or out-of-range chunk-size line threw a
                // raw std::invalid_argument/std::out_of_range instead of a clean
                // HttpRequestException.
                size_t chunkSize;
                try {
                    chunkSize = std::stoul(chunkLine, nullptr, 16);
                } catch (...) {
                    throw HttpRequestException("HttpClient: malformed chunk size: '" + chunkLine + "'");
                }
                if (chunkSize == 0) { recvLine(fd, buf); break; } // trailing CRLF
                auto chunk = recvExact(fd, buf, chunkSize);
                bodyBytes.insert(bodyBytes.end(), chunk.begin(), chunk.end());
                recvLine(fd, buf); // chunk-trailing CRLF
            }
        } else if (contentLength >= 0) {
            bodyBytes = recvExact(fd, buf, static_cast<size_t>(contentLength));
        } else {
            bodyBytes = recvAll(fd, buf);
        }
    }

    platformClose(fd);

    if (!bodyBytes.empty())
        resp->setContentProperty(
            std::make_shared<ByteArrayContent>(
                std::vector<SharpRuntime::bytecs>(bodyBytes.begin(), bodyBytes.end())));

    if (useCookies_ && cookieContainer_ && !setCookieValues.empty()) {
        if (!uri) uri = std::make_unique<System::Uri>(absoluteUrl);
        for (const auto& sc : setCookieValues) {
            try {
                cookieContainer_->SetCookies(*uri, sc);
            } catch (const System::Net::CookieException&) {
                // A malformed Set-Cookie header from the server is ignored, matching real
                // .NET's lenient CookieContainer.SetCookies behavior -- it must not fail the
                // whole request just because one cookie attribute string was garbled.
            }
        }
    }

    return resp;
#endif
}

} // namespace System::Net::Http
