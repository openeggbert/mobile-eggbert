// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include <vector>
#include "System/Net/Http/Headers/HttpHeaders.hpp"
#include "System/Net/Http/Headers/MediaTypeWithQualityHeaderValue.hpp"
#include "System/Net/Http/Headers/StringWithQualityHeaderValue.hpp"
#include "System/Net/Http/Headers/AuthenticationHeaderValue.hpp"
#include "System/Net/Http/Headers/NameValueWithParametersHeaderValue.hpp"
#include "System/Net/Http/Headers/EntityTagHeaderValue.hpp"
#include "System/Net/Http/Headers/RangeConditionHeaderValue.hpp"
#include "System/Net/Http/Headers/RangeHeaderValue.hpp"
#include "System/Net/Http/Headers/TransferCodingWithQualityHeaderValue.hpp"
#include "System/Net/Http/Headers/TransferCodingHeaderValue.hpp"
#include "System/Net/Http/Headers/ProductInfoHeaderValue.hpp"
#include "System/Net/Http/Headers/ProductHeaderValue.hpp"
#include "System/Net/Http/Headers/ViaHeaderValue.hpp"
#include "System/Net/Http/Headers/WarningHeaderValue.hpp"
#include "System/Net/Http/Headers/NameValueHeaderValue.hpp"
#include "System/Net/Http/Headers/CacheControlHeaderValue.hpp"
#include "System/DateTimeOffset.hpp"
#include "System/Uri.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Typed access to HTTP request headers (Accept, Host, Authorization, etc.), plus the
     * "general" headers shared with responses (Cache-Control, Connection, Date, etc.).
     *
     * C++ counterpart of .NET System.Net.Http.Headers.HttpRequestHeaders.
     *
     * @note .NET splits general headers into an internal shared HttpGeneralHeaders helper class
     * composed by both HttpRequestHeaders and HttpResponseHeaders. That internal type is out of
     * scope here (see NEXT.md/plan.sqlite3: marked ignored, not part of the public surface); the
     * general-header properties are implemented directly on each of HttpRequestHeaders and
     * HttpResponseHeaders instead, following this codebase's established pattern of small
     * duplicated per-file helpers rather than a new shared abstraction.
     * @note List-valued headers (Accept, AcceptCharset, AcceptEncoding, AcceptLanguage, Expect,
     * IfMatch, IfNoneMatch, TE, UserAgent, Connection, Pragma, Trailer, TransferEncoding, Upgrade,
     * Via, Warning) are `HttpHeaderValueCollection<T>` live views in .NET; here they're snapshot
     * getters plus an Add(item) mutator instead of a live mutable collection.
     * @note Protocol is a `:protocol` HTTP/2 pseudo-header — like .NET, it's held as a plain field
     * rather than stored in the header collection (it isn't a real wire header name).
     */
    class HttpRequestHeaders : public HttpHeaders {
        std::string protocol_;
        bool expectContinueSet_ = false;

    public:
        HttpRequestHeaders() = default;

        // --- Request headers ---------------------------------------------------------------

        /** @return The parsed Accept header values, in order. */
        [[nodiscard]] std::vector<MediaTypeWithQualityHeaderValue> getAcceptProperty() const;
        /** @brief Appends @p value to the Accept header. */
        void AddAccept(const MediaTypeWithQualityHeaderValue& value);

        /** @return The parsed Accept-Charset header values, in order. */
        [[nodiscard]] std::vector<StringWithQualityHeaderValue> getAcceptCharsetProperty() const;
        /** @brief Appends @p value to the Accept-Charset header. */
        void AddAcceptCharset(const StringWithQualityHeaderValue& value);

        /** @return The parsed Accept-Encoding header values, in order. */
        [[nodiscard]] std::vector<StringWithQualityHeaderValue> getAcceptEncodingProperty() const;
        /** @brief Appends @p value to the Accept-Encoding header. */
        void AddAcceptEncoding(const StringWithQualityHeaderValue& value);

        /** @return The parsed Accept-Language header values, in order. */
        [[nodiscard]] std::vector<StringWithQualityHeaderValue> getAcceptLanguageProperty() const;
        /** @brief Appends @p value to the Accept-Language header. */
        void AddAcceptLanguage(const StringWithQualityHeaderValue& value);

        /** @return The parsed Authorization header, or empty if not present/parsable. */
        [[nodiscard]] std::optional<AuthenticationHeaderValue> getAuthorizationProperty() const;
        /** @brief Sets (or removes, if empty) the Authorization header. */
        void setAuthorizationProperty(const std::optional<AuthenticationHeaderValue>& value);

        /**
         * @return true if the Expect header contains "100-continue"; false if ExpectContinue was
         * explicitly set to false (or the "100-continue" token explicitly removed); empty if never set.
         */
        [[nodiscard]] std::optional<bool> getExpectContinueProperty() const;
        /** @brief Adds/removes the "100-continue" token in the Expect header. */
        void setExpectContinueProperty(std::optional<bool> value);

        /** @return The parsed Expect header values, in order (excluding the plain "100-continue" token). */
        [[nodiscard]] std::vector<NameValueWithParametersHeaderValue> getExpectProperty() const;
        /** @brief Appends @p value to the Expect header. */
        void AddExpect(const NameValueWithParametersHeaderValue& value);

        /** @return The From header (an email address), or empty if not present. */
        [[nodiscard]] std::optional<std::string> getFromProperty() const;
        /**
         * Sets (or removes, if empty) the From header.
         * @throws System::FormatException if @p value contains a bare CR, LF, or NUL character.
         */
        void setFromProperty(const std::optional<std::string>& value);

        /** @return The Host header, or empty if not present. */
        [[nodiscard]] std::optional<std::string> getHostProperty() const;
        /**
         * Sets (or removes, if empty) the Host header.
         * @throws System::FormatException if @p value is not a valid host string.
         */
        void setHostProperty(const std::optional<std::string>& value);

        /** @return The parsed If-Match header values, in order. */
        [[nodiscard]] std::vector<EntityTagHeaderValue> getIfMatchProperty() const;
        /** @brief Appends @p value to the If-Match header. */
        void AddIfMatch(const EntityTagHeaderValue& value);

        /** @return The parsed If-Modified-Since date, or empty if not present/parsable. */
        [[nodiscard]] std::optional<System::DateTimeOffset> getIfModifiedSinceProperty() const;
        /** @brief Sets (or removes, if empty) the If-Modified-Since header. */
        void setIfModifiedSinceProperty(const std::optional<System::DateTimeOffset>& value);

        /** @return The parsed If-None-Match header values, in order. */
        [[nodiscard]] std::vector<EntityTagHeaderValue> getIfNoneMatchProperty() const;
        /** @brief Appends @p value to the If-None-Match header. */
        void AddIfNoneMatch(const EntityTagHeaderValue& value);

        /** @return The parsed If-Range header, or empty if not present/parsable. */
        [[nodiscard]] std::optional<RangeConditionHeaderValue> getIfRangeProperty() const;
        /** @brief Sets (or removes, if empty) the If-Range header. */
        void setIfRangeProperty(const std::optional<RangeConditionHeaderValue>& value);

        /** @return The parsed If-Unmodified-Since date, or empty if not present/parsable. */
        [[nodiscard]] std::optional<System::DateTimeOffset> getIfUnmodifiedSinceProperty() const;
        /** @brief Sets (or removes, if empty) the If-Unmodified-Since header. */
        void setIfUnmodifiedSinceProperty(const std::optional<System::DateTimeOffset>& value);

        /** @return The Max-Forwards header, or empty if not present/not a valid integer. */
        [[nodiscard]] std::optional<SharpRuntime::intcs> getMaxForwardsProperty() const;
        /** @brief Sets (or removes, if empty) the Max-Forwards header. */
        void setMaxForwardsProperty(std::optional<SharpRuntime::intcs> value);

        /** @return The ":protocol" HTTP/2 pseudo-header value, or empty if not set. */
        [[nodiscard]] std::optional<std::string> getProtocolProperty() const;
        /**
         * Sets (or removes, if empty) the ":protocol" pseudo-header value.
         * @throws System::FormatException if @p value contains a bare CR, LF, or NUL character.
         */
        void setProtocolProperty(const std::optional<std::string>& value);

        /** @return The parsed Proxy-Authorization header, or empty if not present/parsable. */
        [[nodiscard]] std::optional<AuthenticationHeaderValue> getProxyAuthorizationProperty() const;
        /** @brief Sets (or removes, if empty) the Proxy-Authorization header. */
        void setProxyAuthorizationProperty(const std::optional<AuthenticationHeaderValue>& value);

        /** @return The parsed Range header, or empty if not present/parsable. */
        [[nodiscard]] std::optional<RangeHeaderValue> getRangeProperty() const;
        /** @brief Sets (or removes, if empty) the Range header. */
        void setRangeProperty(const std::optional<RangeHeaderValue>& value);

        /** @return The parsed Referer header URI, or empty if not present/parsable. */
        [[nodiscard]] std::optional<System::Uri> getReferrerProperty() const;
        /** @brief Sets (or removes, if empty) the Referer header. */
        void setReferrerProperty(const std::optional<System::Uri>& value);

        /** @return The parsed TE header values, in order. */
        [[nodiscard]] std::vector<TransferCodingWithQualityHeaderValue> getTEProperty() const;
        /** @brief Appends @p value to the TE header. */
        void AddTE(const TransferCodingWithQualityHeaderValue& value);

        /** @return The parsed User-Agent header values, in order. */
        [[nodiscard]] std::vector<ProductInfoHeaderValue> getUserAgentProperty() const;
        /** @brief Appends @p value to the User-Agent header. */
        void AddUserAgent(const ProductInfoHeaderValue& value);

        // --- General headers -----------------------------------------------------------------

        /** @return The parsed Cache-Control header, or empty if not present/parsable. */
        [[nodiscard]] std::optional<CacheControlHeaderValue> getCacheControlProperty() const;
        /** @brief Sets (or removes, if empty) the Cache-Control header. */
        void setCacheControlProperty(const std::optional<CacheControlHeaderValue>& value);

        /** @return The tokens in the Connection header, in order. */
        [[nodiscard]] std::vector<std::string> getConnectionProperty() const;
        /** @brief Appends @p token to the Connection header. */
        void AddConnection(const std::string& token);

        /** @return true if the Connection header contains "close"; false/empty otherwise. */
        [[nodiscard]] std::optional<bool> getConnectionCloseProperty() const;
        /** @brief Adds/removes the "close" token in the Connection header. */
        void setConnectionCloseProperty(std::optional<bool> value);

        /** @return The parsed Date header, or empty if not present/parsable. */
        [[nodiscard]] std::optional<System::DateTimeOffset> getDateProperty() const;
        /** @brief Sets (or removes, if empty) the Date header. */
        void setDateProperty(const std::optional<System::DateTimeOffset>& value);

        /** @return The parsed Pragma header values, in order. */
        [[nodiscard]] std::vector<NameValueHeaderValue> getPragmaProperty() const;
        /** @brief Appends @p value to the Pragma header. */
        void AddPragma(const NameValueHeaderValue& value);

        /** @return The tokens in the Trailer header, in order. */
        [[nodiscard]] std::vector<std::string> getTrailerProperty() const;
        /** @brief Appends @p token to the Trailer header. */
        void AddTrailer(const std::string& token);

        /** @return The parsed Transfer-Encoding header values, in order. */
        [[nodiscard]] std::vector<TransferCodingHeaderValue> getTransferEncodingProperty() const;
        /** @brief Appends @p value to the Transfer-Encoding header. */
        void AddTransferEncoding(const TransferCodingHeaderValue& value);

        /** @return true if the Transfer-Encoding header contains "chunked"; false/empty otherwise. */
        [[nodiscard]] std::optional<bool> getTransferEncodingChunkedProperty() const;
        /** @brief Adds/removes the "chunked" token in the Transfer-Encoding header. */
        void setTransferEncodingChunkedProperty(std::optional<bool> value);

        /** @return The parsed Upgrade header values, in order. */
        [[nodiscard]] std::vector<ProductHeaderValue> getUpgradeProperty() const;
        /** @brief Appends @p value to the Upgrade header. */
        void AddUpgrade(const ProductHeaderValue& value);

        /** @return The parsed Via header values, in order. */
        [[nodiscard]] std::vector<ViaHeaderValue> getViaProperty() const;
        /** @brief Appends @p value to the Via header. */
        void AddVia(const ViaHeaderValue& value);

        /** @return The parsed Warning header values, in order. */
        [[nodiscard]] std::vector<WarningHeaderValue> getWarningProperty() const;
        /** @brief Appends @p value to the Warning header. */
        void AddWarning(const WarningHeaderValue& value);
    };

} // namespace System::Net::Http::Headers
