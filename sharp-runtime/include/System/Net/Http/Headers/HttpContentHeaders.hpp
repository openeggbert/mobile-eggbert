// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include <vector>
#include "System/Net/Http/Headers/HttpHeaders.hpp"
#include "System/Net/Http/Headers/ContentDispositionHeaderValue.hpp"
#include "System/Net/Http/Headers/ContentRangeHeaderValue.hpp"
#include "System/Net/Http/Headers/MediaTypeHeaderValue.hpp"
#include "System/DateTimeOffset.hpp"
#include "System/Uri.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Typed access to the headers that describe an HttpContent body (Content-Type,
     * Content-Length, Content-Disposition, etc.).
     *
     * C++ counterpart of .NET System.Net.Http.Headers.HttpContentHeaders.
     *
     * @note ContentLength here is a plain get/set with no auto-calculation from the content
     * body: .NET's getter falls back to `HttpContent.GetComputedOrBufferLength()` when unset,
     * which would require a length-computation hook this runtime's simplified HttpContent
     * (ReadAsString()/ReadAsByteArray(), see its class doc note) doesn't expose.
     * @note Allow/ContentEncoding/ContentLanguage are `ICollection<string>` live views in .NET
     * (order matters, so no simple property fits); here they're snapshot getters plus an
     * Add(token) mutator instead of a live mutable collection.
     * @note This class does not hold a parent HttpContent reference (no constructor dependency on
     * one) — it's usable standalone, unlike .NET's version which is always created by an
     * HttpContent instance.
     */
    class HttpContentHeaders : public HttpHeaders {
    public:
        HttpContentHeaders() = default;

        /** @return The tokens in the Allow header, in order. */
        [[nodiscard]] std::vector<std::string> getAllowProperty() const;
        /** @brief Appends @p method to the Allow header. */
        void AddAllow(const std::string& method);

        /** @return The parsed Content-Disposition header, or empty if not present/parsable. */
        [[nodiscard]] std::optional<ContentDispositionHeaderValue> getContentDispositionProperty() const;
        /** @brief Sets (or removes, if empty) the Content-Disposition header. */
        void setContentDispositionProperty(const std::optional<ContentDispositionHeaderValue>& value);

        /** @return The tokens in the Content-Encoding header, in order. */
        [[nodiscard]] std::vector<std::string> getContentEncodingProperty() const;
        /** @brief Appends @p encoding to the Content-Encoding header. */
        void AddContentEncoding(const std::string& encoding);

        /** @return The tokens in the Content-Language header, in order. */
        [[nodiscard]] std::vector<std::string> getContentLanguageProperty() const;
        /** @brief Appends @p language to the Content-Language header. */
        void AddContentLanguage(const std::string& language);

        /** @return The Content-Length, or empty if not present/not a valid non-negative integer. */
        [[nodiscard]] std::optional<SharpRuntime::longcs> getContentLengthProperty() const;
        /** @brief Sets (or removes, if empty) the Content-Length header. */
        void setContentLengthProperty(std::optional<SharpRuntime::longcs> value);

        /** @return The parsed Content-Location URI, or empty if not present/parsable. */
        [[nodiscard]] std::optional<System::Uri> getContentLocationProperty() const;
        /** @brief Sets (or removes, if empty) the Content-Location header. */
        void setContentLocationProperty(const std::optional<System::Uri>& value);

        /** @return The Content-MD5 raw bytes (base64-decoded), or empty if not present/parsable. */
        [[nodiscard]] std::optional<std::vector<SharpRuntime::bytecs>> getContentMD5Property() const;
        /** @brief Sets (or removes, if empty) the Content-MD5 header (base64-encoded). */
        void setContentMD5Property(const std::optional<std::vector<SharpRuntime::bytecs>>& value);

        /** @return The parsed Content-Range header, or empty if not present/parsable. */
        [[nodiscard]] std::optional<ContentRangeHeaderValue> getContentRangeProperty() const;
        /** @brief Sets (or removes, if empty) the Content-Range header. */
        void setContentRangeProperty(const std::optional<ContentRangeHeaderValue>& value);

        /** @return The parsed Content-Type header, or empty if not present/parsable. */
        [[nodiscard]] std::optional<MediaTypeHeaderValue> getContentTypeProperty() const;
        /** @brief Sets (or removes, if empty) the Content-Type header. */
        void setContentTypeProperty(const std::optional<MediaTypeHeaderValue>& value);

        /** @return The parsed Expires date, or empty if not present/parsable. */
        [[nodiscard]] std::optional<System::DateTimeOffset> getExpiresProperty() const;
        /** @brief Sets (or removes, if empty) the Expires header. */
        void setExpiresProperty(const std::optional<System::DateTimeOffset>& value);

        /** @return The parsed Last-Modified date, or empty if not present/parsable. */
        [[nodiscard]] std::optional<System::DateTimeOffset> getLastModifiedProperty() const;
        /** @brief Sets (or removes, if empty) the Last-Modified header. */
        void setLastModifiedProperty(const std::optional<System::DateTimeOffset>& value);
    };

} // namespace System::Net::Http::Headers
