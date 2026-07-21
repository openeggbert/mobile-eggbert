// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Http/HttpContent.hpp"
#include "System/Net/Http/HttpRequestException.hpp"
#include "System/Net/HttpStatusCode.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace System::Net::Http {

/** Represents an HTTP response, including status code, headers, and body content. */
class HttpResponseMessage {
    System::Net::HttpStatusCode                    statusCode_;
    std::string                                    reasonPhrase_;
    std::shared_ptr<HttpContent>                   content_;
    std::unordered_map<std::string, std::string>   headers_;
public:
    explicit HttpResponseMessage(
        System::Net::HttpStatusCode statusCode = System::Net::HttpStatusCode::OK)
        : statusCode_(statusCode) {}

    [[nodiscard]] System::Net::HttpStatusCode getStatusCodeProperty() const { return statusCode_; }
    void setStatusCodeProperty(System::Net::HttpStatusCode v)               { statusCode_ = v; }

    [[nodiscard]] const std::string& getReasonPhraseProperty() const { return reasonPhrase_; }
    void setReasonPhraseProperty(const std::string& v)               { reasonPhrase_ = v; }

    [[nodiscard]] std::shared_ptr<HttpContent> getContentProperty() const { return content_; }
    void setContentProperty(std::shared_ptr<HttpContent> v)               { content_ = std::move(v); }

    /** True when StatusCode is in the 200–299 range. */
    [[nodiscard]] bool getIsSuccessStatusCodeProperty() const {
        int c = static_cast<int>(statusCode_);
        return c >= 200 && c < 300;
    }

    /**
     * @brief Throws HttpRequestException if the response indicates failure.
     *
     * C++ counterpart of .NET HttpResponseMessage.EnsureSuccessStatusCode(), which returns
     * `this` to allow fluent chaining (e.g. `response.EnsureSuccessStatusCode().Content...`);
     * returns a const reference to self here for the same purpose, matching this method's
     * existing const-qualification. All existing call sites in this codebase already ignore the
     * return value, so this is a purely additive change.
     * @return *this.
     */
    const HttpResponseMessage& EnsureSuccessStatusCode() const {
        if (getIsSuccessStatusCodeProperty()) return *this;

        size_t firstNonSpace = reasonPhrase_.find_first_not_of(" \t\r\n");
        std::string message = "Response status code does not indicate success: " +
            std::to_string(static_cast<int>(statusCode_));
        message += (firstNonSpace == std::string::npos) ? "." : " (" + reasonPhrase_ + ").";

        throw HttpRequestException(HttpRequestError::Unknown, message, nullptr, statusCode_);
    }

    /** Adds or replaces a response header. */
    void setHeader(const std::string& name, const std::string& value) { headers_[name] = value; }

    /** @return The value of the named header, or "" if not present. */
    [[nodiscard]] std::string getHeader(const std::string& name) const {
        auto it = headers_.find(name);
        return it != headers_.end() ? it->second : "";
    }

    /** @return The map of all response headers. */
    [[nodiscard]] const std::unordered_map<std::string, std::string>& getHeadersProperty() const {
        return headers_;
    }
};

} // namespace System::Net::Http
