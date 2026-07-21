// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Http/HttpContent.hpp"
#include "System/IO/Stream.hpp"
#include "System/ArgumentNullException.hpp"
#include <memory>
#include <string>
#include <vector>

namespace System::Net::Http {

/**
 * @brief HTTP content backed by a System::IO::Stream, read eagerly.
 *
 * C++ counterpart of .NET System.Net.Http.StreamContent.
 *
 * @note .NET's StreamContent copies from the source Stream to the destination stream lazily,
 * on demand, at serialization time, and supports re-reading seekable streams from their starting
 * position. This runtime's HttpContent model (ReadAsString()/ReadAsByteArray()) has no
 * serialize-to-destination-stream step, so the source stream is instead read fully into an
 * internal buffer once, at construction time; re-reading (ReadAsString()/ReadAsByteArray() called
 * more than once) simply returns the same buffered copy rather than re-reading the source stream.
 */
class StreamContent : public HttpContent {
    std::vector<SharpRuntime::bytecs> data_;
    std::string mediaType_;

public:
    /**
     * @brief Constructs StreamContent by eagerly reading @p content to the end.
     * @param content   The stream to read the content body from.
     * @param mediaType MIME type; defaults to "application/octet-stream".
     */
    explicit StreamContent(const std::shared_ptr<System::IO::Stream>& content,
                            const std::string& mediaType = "application/octet-stream")
        : mediaType_(mediaType) {
        System::ArgumentNullException::ThrowIfNull(content.get(), "content");

        SharpRuntime::bytecs buffer[4096];
        SharpRuntime::intcs n;
        while ((n = content->Read(buffer, 0, static_cast<SharpRuntime::intcs>(sizeof(buffer)))) > 0) {
            data_.insert(data_.end(), buffer, buffer + n);
        }
    }

    /** Returns the buffered content body interpreted as a UTF-8 string. */
    [[nodiscard]] std::string ReadAsString() const override {
        return std::string(data_.begin(), data_.end());
    }

    /** Returns a copy of the buffered content body as raw bytes. */
    [[nodiscard]] std::vector<SharpRuntime::bytecs> ReadAsByteArray() const override {
        return data_;
    }

    /** Returns the MIME type of the content (e.g. "application/octet-stream"). */
    [[nodiscard]] std::string getContentTypeProperty() const override { return mediaType_; }
};

} // namespace System::Net::Http
