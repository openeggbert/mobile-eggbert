// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Http/HttpContent.hpp"
#include "System/ReadOnlyMemory.hpp"
#include <string>
#include <vector>

namespace System::Net::Http {

/**
 * @brief HTTP content backed by a ReadOnlyMemory<byte> view (no copy of the underlying data).
 *
 * C++ counterpart of .NET System.Net.Http.ReadOnlyMemoryContent.
 *
 * @note .NET's ReadOnlyMemoryContent doesn't set a Content-Type by default (its Headers.ContentType
 * stays null until set by the caller). This runtime's simplified HttpContent model requires
 * getContentTypeProperty() to always return a value, so this defaults to
 * "application/octet-stream", matching ByteArrayContent's default.
 */
class ReadOnlyMemoryContent : public HttpContent {
    System::ReadOnlyMemory<SharpRuntime::bytecs> content_;
    std::string mediaType_;

public:
    /**
     * @brief Constructs ReadOnlyMemoryContent from a ReadOnlyMemory<byte> view.
     * @param content   The memory region to use as the content body.
     * @param mediaType MIME type; defaults to "application/octet-stream".
     */
    explicit ReadOnlyMemoryContent(const System::ReadOnlyMemory<SharpRuntime::bytecs>& content,
                                   const std::string& mediaType = "application/octet-stream")
        : content_(content), mediaType_(mediaType) {}

    /** Returns the content body interpreted as a UTF-8 string. */
    [[nodiscard]] std::string ReadAsString() const override {
        return std::string(content_.getPointer(), content_.getPointer() + content_.getLengthProperty());
    }

    /** Returns a copy of the memory-view content body as a byte array. */
    [[nodiscard]] std::vector<SharpRuntime::bytecs> ReadAsByteArray() const override {
        return content_.ToArray();
    }

    /** Returns the MIME type of the content (e.g. "application/octet-stream"). */
    [[nodiscard]] std::string getContentTypeProperty() const override { return mediaType_; }
};

} // namespace System::Net::Http
