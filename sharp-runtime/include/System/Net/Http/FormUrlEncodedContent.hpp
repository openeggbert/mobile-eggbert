// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Http/HttpContent.hpp"
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

namespace System::Net::Http {

/**
 * HTTP content encoded as application/x-www-form-urlencoded.
 * 
 * Accepts a list of name-value pairs and encodes them according to
 * RFC 3986 / HTML form encoding: spaces become '+', other special
 * characters are percent-encoded as %XX. Pairs are separated by '&'.
 */
class FormUrlEncodedContent : public HttpContent {
    std::string encoded_;

    static std::string percentEncode(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (unsigned char c : s) {
            if (std::isalnum(static_cast<int>(c)) ||
                c == '-' || c == '_' || c == '.' || c == '*') {
                out += static_cast<char>(c);
            } else if (c == ' ') {
                out += '+';
            } else {
                char buf[4];
                std::snprintf(buf, sizeof(buf), "%%%02X", static_cast<unsigned>(c));
                out += buf;
            }
        }
        return out;
    }

public:
    /**
     * Constructs the content from a list of name-value pairs.
     * @param nameValueCollection  Key-value pairs to encode.
     */
    explicit FormUrlEncodedContent(
        const std::vector<std::pair<std::string, std::string>>& nameValueCollection)
    {
        bool first = true;
        for (const auto& [key, val] : nameValueCollection) {
            if (!first) encoded_ += '&';
            encoded_ += percentEncode(key) + '=' + percentEncode(val);
            first = false;
        }
    }

    /** @return The URL-encoded form string (e.g. "name=Alice&age=30"). */
    [[nodiscard]] std::string ReadAsString() const override { return encoded_; }

    /** @return The URL-encoded form string as a UTF-8 byte array. */
    [[nodiscard]] std::vector<SharpRuntime::bytecs> ReadAsByteArray() const override {
        return std::vector<SharpRuntime::bytecs>(encoded_.begin(), encoded_.end());
    }

    /** @return "application/x-www-form-urlencoded" */
    [[nodiscard]] std::string getContentTypeProperty() const override {
        return "application/x-www-form-urlencoded";
    }
};

} // namespace System::Net::Http
