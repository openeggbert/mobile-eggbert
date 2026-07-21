// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Http/HttpContent.hpp"
#include <string>
#include <vector>

namespace System::Net::Http {

/** HTTP content backed by a plain text string, mirroring .NET System.Net.Http.StringContent. */
class StringContent : public HttpContent {
    std::string content_;
    std::string mediaType_;
    std::string charset_;
public:
    /**
     * @brief Constructs StringContent from a plain text string.
     * @param content   The text body.
     * @param charset   Character encoding; defaults to "utf-8".
     * @param mediaType MIME type; defaults to "text/plain".
     */
    explicit StringContent(const std::string& content,
                           const std::string& charset   = "utf-8",
                           const std::string& mediaType = "text/plain")
        : content_(content), mediaType_(mediaType), charset_(charset) {}

    /** Returns the content body as a string. */
    [[nodiscard]] std::string ReadAsString() const override { return content_; }

    /** Returns the content body as a raw byte array (UTF-8 encoding assumed). */
    [[nodiscard]] std::vector<SharpRuntime::bytecs> ReadAsByteArray() const override {
        return std::vector<SharpRuntime::bytecs>(content_.begin(), content_.end());
    }

    /** Returns the MIME type of the content (e.g. "text/plain"). */
    [[nodiscard]] std::string getContentTypeProperty() const override { return mediaType_; }
    /** Returns the character set of the content (e.g. "utf-8"). */
    [[nodiscard]] std::string getCharSetProperty()     const override { return charset_; }
};

} // namespace System::Net::Http
