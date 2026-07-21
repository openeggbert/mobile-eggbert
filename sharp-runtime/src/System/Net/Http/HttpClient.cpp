// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)

#include "System/Net/Http/HttpClient.hpp"
#include "System/Net/Http/ByteArrayContent.hpp"
#include "System/Net/Http/HttpClientHandler.hpp"
#include "System/Net/Http/HttpRequestException.hpp"
#include "System/Net/Http/StringContent.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/UriFormatException.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace System::Net::Http {

// ---------------------------------------------------------------------------
// URL parser
// ---------------------------------------------------------------------------

// Verified against real .NET: a malformed request URI fails Uri construction with
// System.UriFormatException (a FormatException); requesting a scheme HttpClient can't
// dispatch (here, anything but "http" -- HTTPS/TLS is out of scope for this runtime, see
// CLAUDE.md's documented deviations) is a System.NotSupportedException-shaped failure, not a
// format error. Previously every failure path here threw std::invalid_argument -- an
// unrelated std:: exception type invisible to code catching System::Exception&/
// System::FormatException&, escaping straight out of a public API.
HttpClient::ParsedUrl HttpClient::parseUrl(const std::string& url) {
    ParsedUrl result;

    size_t schemeEnd = url.find("://");
    if (schemeEnd == std::string::npos)
        throw System::UriFormatException("HttpClient: missing scheme in URL: " + url);

    result.scheme = url.substr(0, schemeEnd);
    for (char& c : result.scheme)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    if (result.scheme != "http")
        throw System::NotSupportedException(
            "HttpClient: only HTTP is supported (not '" + result.scheme + "'). "
            "HTTPS requires TLS which is not yet implemented.");

    size_t hostStart = schemeEnd + 3;
    size_t pathStart = url.find('/', hostStart);

    std::string hostPort;
    if (pathStart == std::string::npos) {
        hostPort    = url.substr(hostStart);
        result.path = "/";
    } else {
        hostPort    = url.substr(hostStart, pathStart - hostStart);
        result.path = url.substr(pathStart);
    }
    if (result.path.empty()) result.path = "/";

    if (!hostPort.empty() && hostPort[0] == '[') {
        // IPv6 literal in bracket notation, e.g. "[::1]" or "[::1]:8080". A bare
        // `rfind(':')` split is wrong here -- an IPv6 address itself contains colons, so
        // splitting on the last colon in "[::1]" (no port) grabs the second colon of the
        // address instead of a port separator, corrupting both host and port.
        size_t closeBracket = hostPort.find(']');
        if (closeBracket == std::string::npos)
            throw System::UriFormatException("HttpClient: unterminated IPv6 literal in URL: " + url);
        result.host = hostPort.substr(1, closeBracket - 1);
        if (closeBracket + 1 < hostPort.size() && hostPort[closeBracket + 1] == ':') {
            try {
                result.port = std::stoi(hostPort.substr(closeBracket + 2));
            } catch (...) {
                throw System::UriFormatException("HttpClient: invalid port in URL: " + url);
            }
        } else {
            result.port = 80;
        }
    } else {
        size_t colonPos = hostPort.rfind(':');
        if (colonPos != std::string::npos) {
            result.host = hostPort.substr(0, colonPos);
            try {
                result.port = std::stoi(hostPort.substr(colonPos + 1));
            } catch (...) {
                throw System::UriFormatException("HttpClient: invalid port in URL: " + url);
            }
        } else {
            result.host = hostPort;
            result.port = 80;
        }
    }

    if (result.host.empty())
        throw System::UriFormatException("HttpClient: empty host in URL: " + url);

    return result;
}

// A status line that can't be parsed (no space to find the code, or a non-numeric code) must
// not silently report success -- this previously defaulted the status code to 200 (OK)
// whenever the line had no space at all, making a garbled/empty response from the server look
// exactly like a successful one to calling code.
HttpClient::ParsedStatusLine HttpClient::parseStatusLine(const std::string& statusLine) {
    ParsedStatusLine result;
    size_t sp1 = statusLine.find(' ');
    if (sp1 == std::string::npos)
        throw HttpRequestException("HttpClient: malformed status line: '" + statusLine + "'");
    size_t sp2 = statusLine.find(' ', sp1 + 1);
    std::string codeStr = (sp2 != std::string::npos)
        ? statusLine.substr(sp1 + 1, sp2 - sp1 - 1)
        : statusLine.substr(sp1 + 1);
    try {
        result.statusCode = std::stoi(codeStr);
    } catch (...) {
        throw HttpRequestException("HttpClient: malformed status line: '" + statusLine + "'");
    }
    if (sp2 != std::string::npos) result.reason = statusLine.substr(sp2 + 1);
    return result;
}

// ---------------------------------------------------------------------------
// HttpClient public methods
// ---------------------------------------------------------------------------

// The actual socket-level send/receive implementation lives in HttpClientHandler
// (System::Net::Http::HttpClientHandler, see HttpClientHandler.cpp) so that HttpClient can be
// pointed at a custom HttpMessageHandler/DelegatingHandler chain instead (ticket 1496).

HttpClient::HttpClient() : handler_(std::make_shared<HttpClientHandler>()) {}

HttpClient::HttpClient(std::shared_ptr<HttpMessageHandler> handler) : handler_(std::move(handler)) {
    System::ArgumentNullException::ThrowIfNull(handler_.get(), "handler");
}

HttpClient::~HttpClient() = default;

std::shared_ptr<HttpResponseMessage> HttpClient::Send(
    std::shared_ptr<HttpRequestMessage> request)
{
    // Verified against HttpClient.cs's CheckRequestBeforeSend: every Send/SendAsync overload
    // validates the request is non-null before touching it. Previously this dereferenced
    // request immediately with no check -- a null-pointer dereference (UB/crash) instead of
    // a catchable exception.
    System::ArgumentNullException::ThrowIfNull(request.get(), "request");

    std::string url = request->getRequestUriProperty();
    if (url.empty()) url = baseAddress_;
    else if (url.find("://") == std::string::npos && !baseAddress_.empty())
        url = baseAddress_ + url;
    request->setRequestUriProperty(url);

    // DefaultRequestHeaders are merged onto the request here (client-level defaults never
    // override a header the caller already set on this specific request) before handing off
    // to the handler chain -- matching real .NET's HttpClient applying DefaultRequestHeaders
    // ahead of invoking the handler pipeline.
    for (const auto& [k, v] : defaultHeaders_) {
        if (request->getHeadersProperty().find(k) == request->getHeadersProperty().end())
            request->setHeader(k, v);
    }

    return handler_->Send(std::move(request));
}

std::shared_ptr<HttpResponseMessage> HttpClient::Get(const std::string& url) {
    auto req = std::make_shared<HttpRequestMessage>(
        HttpMethod::Get(), url.empty() ? baseAddress_ : url);
    return Send(req);
}

std::shared_ptr<HttpResponseMessage> HttpClient::Post(
    const std::string& url, std::shared_ptr<HttpContent> content)
{
    auto req = std::make_shared<HttpRequestMessage>(
        HttpMethod::Post(), url.empty() ? baseAddress_ : url);
    req->setContentProperty(std::move(content));
    return Send(req);
}

std::string HttpClient::GetString(const std::string& url) {
    auto resp = Get(url);
    resp->EnsureSuccessStatusCode();
    return resp->getContentProperty() ? resp->getContentProperty()->ReadAsString() : "";
}

std::vector<SharpRuntime::bytecs> HttpClient::GetByteArray(const std::string& url) {
    auto resp = Get(url);
    resp->EnsureSuccessStatusCode();
    return resp->getContentProperty()
        ? resp->getContentProperty()->ReadAsByteArray()
        : std::vector<SharpRuntime::bytecs>{};
}

System::Threading::Tasks::TaskT<std::shared_ptr<HttpResponseMessage>>
HttpClient::SendAsync(std::shared_ptr<HttpRequestMessage> request) {
    return System::Threading::Tasks::TaskT<std::shared_ptr<HttpResponseMessage>>(
        [this, request]() { return Send(request); });
}

System::Threading::Tasks::TaskT<std::shared_ptr<HttpResponseMessage>>
HttpClient::GetAsync(const std::string& url) {
    return System::Threading::Tasks::TaskT<std::shared_ptr<HttpResponseMessage>>(
        [this, url]() { return Get(url); });
}

System::Threading::Tasks::TaskT<std::shared_ptr<HttpResponseMessage>>
HttpClient::PostAsync(const std::string& url, std::shared_ptr<HttpContent> content) {
    return System::Threading::Tasks::TaskT<std::shared_ptr<HttpResponseMessage>>(
        [this, url, content]() { return Post(url, content); });
}

System::Threading::Tasks::TaskT<std::string>
HttpClient::GetStringAsync(const std::string& url) {
    return System::Threading::Tasks::TaskT<std::string>(
        [this, url]() { return GetString(url); });
}

System::Threading::Tasks::TaskT<std::vector<SharpRuntime::bytecs>>
HttpClient::GetByteArrayAsync(const std::string& url) {
    return System::Threading::Tasks::TaskT<std::vector<SharpRuntime::bytecs>>(
        [this, url]() { return GetByteArray(url); });
}

void HttpClient::setDefaultHeader(const std::string& name, const std::string& value) {
    defaultHeaders_[name] = value;
}

std::string HttpClient::getDefaultHeader(const std::string& name) const {
    auto it = defaultHeaders_.find(name);
    return it != defaultHeaders_.end() ? it->second : "";
}

} // namespace System::Net::Http
