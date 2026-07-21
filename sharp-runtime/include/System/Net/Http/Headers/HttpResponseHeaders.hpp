// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include <vector>
#include "System/Net/Http/Headers/HttpHeaders.hpp"
#include "System/Net/Http/Headers/EntityTagHeaderValue.hpp"
#include "System/Net/Http/Headers/AuthenticationHeaderValue.hpp"
#include "System/Net/Http/Headers/RetryConditionHeaderValue.hpp"
#include "System/Net/Http/Headers/ProductInfoHeaderValue.hpp"
#include "System/Net/Http/Headers/ProductHeaderValue.hpp"
#include "System/Net/Http/Headers/NameValueHeaderValue.hpp"
#include "System/Net/Http/Headers/CacheControlHeaderValue.hpp"
#include "System/Net/Http/Headers/TransferCodingHeaderValue.hpp"
#include "System/Net/Http/Headers/ViaHeaderValue.hpp"
#include "System/Net/Http/Headers/WarningHeaderValue.hpp"
#include "System/DateTimeOffset.hpp"
#include "System/TimeSpan.hpp"
#include "System/Uri.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http::Headers {

    /**
     * @brief Typed access to HTTP response headers (ETag, Location, Server, etc.), plus the
     * "general" headers shared with requests (Cache-Control, Connection, Date, etc.).
     *
     * C++ counterpart of .NET System.Net.Http.Headers.HttpResponseHeaders.
     *
     * @note As with HttpRequestHeaders, .NET's internal shared HttpGeneralHeaders helper is not
     * reproduced (out of scope, see NEXT.md/plan.sqlite3: marked ignored); the general-header
     * properties are duplicated directly here, matching HttpRequestHeaders.
     * @note List-valued headers (AcceptRanges, ProxyAuthenticate, Server, Vary, WwwAuthenticate,
     * Connection, Pragma, Trailer, TransferEncoding, Upgrade, Via, Warning) are snapshot getters
     * plus an Add(item) mutator instead of a live `HttpHeaderValueCollection<T>`.
     * @note The trailing-headers mode (constructor's containsTrailingHeaders parameter, which
     * restricts which header names are allowed) is not reproduced — this runtime's HttpHeaders has
     * no known-header allow-list to restrict against (see its class doc note).
     */
    class HttpResponseHeaders : public HttpHeaders {
    public:
        HttpResponseHeaders() = default;

        // --- Response headers -----------------------------------------------------------------

        /** @return The tokens in the Accept-Ranges header, in order. */
        [[nodiscard]] std::vector<std::string> getAcceptRangesProperty() const;
        /** @brief Appends @p token to the Accept-Ranges header. */
        void AddAcceptRanges(const std::string& token);

        /** @return The Age header (as a TimeSpan of seconds), or empty if not present/not a valid non-negative integer. */
        [[nodiscard]] std::optional<System::TimeSpan> getAgeProperty() const;
        /** @brief Sets (or removes, if empty) the Age header. */
        void setAgeProperty(const std::optional<System::TimeSpan>& value);

        /** @return The parsed ETag header, or empty if not present/parsable. */
        [[nodiscard]] std::optional<EntityTagHeaderValue> getETagProperty() const;
        /** @brief Sets (or removes, if empty) the ETag header. */
        void setETagProperty(const std::optional<EntityTagHeaderValue>& value);

        /** @return The parsed Location header URI, or empty if not present/parsable. */
        [[nodiscard]] std::optional<System::Uri> getLocationProperty() const;
        /** @brief Sets (or removes, if empty) the Location header. */
        void setLocationProperty(const std::optional<System::Uri>& value);

        /** @return The parsed Proxy-Authenticate header values, in order. */
        [[nodiscard]] std::vector<AuthenticationHeaderValue> getProxyAuthenticateProperty() const;
        /** @brief Appends @p value to the Proxy-Authenticate header. */
        void AddProxyAuthenticate(const AuthenticationHeaderValue& value);

        /** @return The parsed Retry-After header, or empty if not present/parsable. */
        [[nodiscard]] std::optional<RetryConditionHeaderValue> getRetryAfterProperty() const;
        /** @brief Sets (or removes, if empty) the Retry-After header. */
        void setRetryAfterProperty(const std::optional<RetryConditionHeaderValue>& value);

        /** @return The parsed Server header values, in order. */
        [[nodiscard]] std::vector<ProductInfoHeaderValue> getServerProperty() const;
        /** @brief Appends @p value to the Server header. */
        void AddServer(const ProductInfoHeaderValue& value);

        /** @return The tokens in the Vary header, in order. */
        [[nodiscard]] std::vector<std::string> getVaryProperty() const;
        /** @brief Appends @p token to the Vary header. */
        void AddVary(const std::string& token);

        /** @return The parsed WWW-Authenticate header values, in order. */
        [[nodiscard]] std::vector<AuthenticationHeaderValue> getWwwAuthenticateProperty() const;
        /** @brief Appends @p value to the WWW-Authenticate header. */
        void AddWwwAuthenticate(const AuthenticationHeaderValue& value);

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
