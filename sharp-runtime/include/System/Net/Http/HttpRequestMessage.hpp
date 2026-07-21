// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Http/HttpMethod.hpp"
#include "System/Net/Http/HttpContent.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace System::Net::Http {

/**
 * @brief Represents an HTTP request message, mirroring .NET System.Net.Http.HttpRequestMessage.
 *
 * @note Deliberately simplified relative to real .NET: RequestUri is a plain string rather than
 * a System::Uri (avoiding a broad, unrequested refactor of this and other Net.Http call sites),
 * headers are a raw name/value map rather than the ported HttpRequestHeaders type (this
 * project's Net/Net.Http subsystems intentionally keep several independent, simplified
 * header-bag designs rather than unifying them -- see NEXT.md's "Do not do yet" section), and
 * Version/VersionPolicy are absent since this runtime's HttpClient only supports HTTP/1.1 over
 * plain TCP.
 */
class HttpRequestMessage {
    HttpMethod                                     method_;
    std::string                                    uri_;
    std::shared_ptr<HttpContent>                   content_;
    std::unordered_map<std::string, std::string>   headers_;
public:
    /** Constructs an HttpRequestMessage with the default GET method and an empty URI. */
    HttpRequestMessage() : method_(HttpMethod::Get()) {}

    /**
     * @brief Constructs an HttpRequestMessage with the given method and URI.
     * @param method HTTP method (verb).
     * @param uri    Request URI string.
     */
    HttpRequestMessage(const HttpMethod& method, const std::string& uri)
        : method_(method), uri_(uri) {}

    /** Returns the HTTP method of this request. */
    [[nodiscard]] const HttpMethod& getMethodProperty() const { return method_; }
    /** Sets the HTTP method of this request. */
    void setMethodProperty(const HttpMethod& v)               { method_ = v; }

    /** Returns the request URI string. */
    [[nodiscard]] const std::string& getRequestUriProperty() const { return uri_; }
    /** Sets the request URI string. */
    void setRequestUriProperty(const std::string& v)               { uri_ = v; }

    /** Returns the content body of this request (may be null). */
    [[nodiscard]] std::shared_ptr<HttpContent> getContentProperty() const { return content_; }
    /** Sets the content body of this request. */
    void setContentProperty(std::shared_ptr<HttpContent> v)               { content_ = std::move(v); }

    /**
     * @brief Adds or replaces a request header.
     * @param name  Header name.
     * @param value Header value.
     */
    void setHeader(const std::string& name, const std::string& value) { headers_[name] = value; }

    /** Returns the map of all request headers. */
    [[nodiscard]] const std::unordered_map<std::string, std::string>& getHeadersProperty() const {
        return headers_;
    }
};

} // namespace System::Net::Http
