// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Http/HttpContent.hpp"
#include <string>
#include <vector>

namespace System::Net::Http {

/** HTTP content backed by a raw byte array. */
class ByteArrayContent : public HttpContent {
    std::vector<SharpRuntime::bytecs> data_;
    std::string mediaType_;
public:
    /**
     * @brief Constructs ByteArrayContent from a raw byte array.
     * @param data   Byte array to use as the content body.
     * @param mediaType MIME type; defaults to "application/octet-stream".
     */
    explicit ByteArrayContent(const std::vector<SharpRuntime::bytecs>& data,
                              const std::string& mediaType = "application/octet-stream")
        : data_(data), mediaType_(mediaType) {}

    /** Returns the content body interpreted as a UTF-8 string. */
    [[nodiscard]] std::string ReadAsString() const override {
        return std::string(data_.begin(), data_.end());
    }

    /** Returns a copy of the raw byte array content body. */
    [[nodiscard]] std::vector<SharpRuntime::bytecs> ReadAsByteArray() const override {
        return data_;
    }

    /** Returns the MIME type of the content (e.g. "application/octet-stream"). */
    [[nodiscard]] std::string getContentTypeProperty() const override { return mediaType_; }
};

} // namespace System::Net::Http
