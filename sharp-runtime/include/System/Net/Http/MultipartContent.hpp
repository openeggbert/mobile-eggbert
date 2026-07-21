// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Net/Http/HttpContent.hpp"
#include <memory>
#include <string>
#include <vector>

namespace System::Net::Http {

/**
 * @brief Provides a container for content encoded using a "multipart" MIME type.
 *
 * C++ counterpart of .NET System.Net.Http.MultipartContent.
 *
 * @note This runtime's HttpContent has no generic per-content header bag (see HttpContent's
 * class doc note), so each part's serialized headers are limited to "Content-Type" (and
 * "Content-Disposition" for MultipartFormDataContent's parts) rather than .NET's arbitrary
 * Headers.NonValidated collection. Iteration is exposed via getContentsProperty() rather than
 * .NET's IEnumerable<HttpContent> (no iterator-adapter idiom used elsewhere in this content model).
 */
class MultipartContent : public HttpContent {
protected:
    /** One nested part: its content plus any extra pre-formatted header line(s) (e.g. Content-Disposition). */
    struct Part {
        std::shared_ptr<HttpContent> content;
        std::string extraHeaderLines; ///< Already includes trailing "\r\n" per line, or empty.
    };

    std::vector<Part> parts_;
    std::string boundary_;
    std::string mediaType_;

    static std::string generateBoundary();
    static void validateBoundary(const std::string& boundary);
    static std::string buildMediaType(const std::string& subtype, const std::string& boundary);

    /** Adds @p content with an extra pre-formatted header line (used by MultipartFormDataContent). */
    void AddWithHeaderLine(const std::shared_ptr<HttpContent>& content, const std::string& extraHeaderLine);

public:
    /** Constructs a multipart/mixed container with a randomly generated boundary. */
    MultipartContent();
    /** Constructs a multipart/{subtype} container with a randomly generated boundary. */
    explicit MultipartContent(const std::string& subtype);
    /**
     * @brief Constructs a multipart/{subtype} container with the given boundary.
     * @throws System::ArgumentException if @p subtype or @p boundary is empty/whitespace,
     * or @p boundary is invalid per RFC 2046 5.1.1 (too long, trailing space, or disallowed chars).
     */
    MultipartContent(const std::string& subtype, const std::string& boundary);

    /** Appends @p content as a new part with no extra headers beyond Content-Type. */
    virtual void Add(const std::shared_ptr<HttpContent>& content);

    /** @return The nested parts, in the order they were added. */
    [[nodiscard]] std::vector<std::shared_ptr<HttpContent>> getContentsProperty() const;

    /** @return The multipart body: boundary-delimited parts, each preceded by its headers. */
    [[nodiscard]] std::string ReadAsString() const override;
    /** @return The multipart body as raw bytes. */
    [[nodiscard]] std::vector<SharpRuntime::bytecs> ReadAsByteArray() const override;
    /** @return "multipart/{subtype}; boundary=\"{boundary}\"". */
    [[nodiscard]] std::string getContentTypeProperty() const override { return mediaType_; }
};

} // namespace System::Net::Http
