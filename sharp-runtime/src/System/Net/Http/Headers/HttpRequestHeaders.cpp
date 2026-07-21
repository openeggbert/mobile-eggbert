// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/HttpRequestHeaders.hpp"
#include "System/FormatException.hpp"
#include "System/TimeSpan.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string_view>

namespace System::Net::Http::Headers {

    namespace {
        std::string trim(const std::string& s) {
            size_t start = s.find_first_not_of(" \t");
            if (start == std::string::npos) return "";
            size_t end = s.find_last_not_of(" \t");
            return s.substr(start, end - start + 1);
        }

        // Splits on top-level commas, respecting quoted strings (matches the established pattern
        // in CacheControlHeaderValue.cpp/other multi-value header parsers in this namespace).
        std::vector<std::string> splitTopLevel(const std::string& input) {
            std::vector<std::string> parts;
            std::string current;
            bool inQuotes = false;
            for (char c : input) {
                if (c == '"') {
                    inQuotes = !inQuotes;
                    current += c;
                } else if (c == ',' && !inQuotes) {
                    parts.push_back(current);
                    current.clear();
                } else {
                    current += c;
                }
            }
            parts.push_back(current);
            return parts;
        }

        // Collects every value stored under `name` and splits each on top-level commas, since a
        // list-valued header may be added either as one comma-joined Add() call or as several
        // separate Add() calls (both are equivalent on the wire).
        template <typename T, typename TryParseFn>
        std::vector<T> parseList(const HttpHeaders& headers, const std::string& name, TryParseFn tryParse, T placeholder) {
            std::vector<T> result;
            std::vector<std::string> raw;
            if (!headers.TryGetValues(name, raw)) return result;
            for (const auto& line : raw) {
                for (const auto& part : splitTopLevel(line)) {
                    std::string token = trim(part);
                    if (token.empty()) continue;
                    T value = placeholder;
                    if (tryParse(token, value)) result.push_back(value);
                }
            }
            return result;
        }

        void checkNoNewlineOrNul(const std::string& value) {
            if (value.find_first_of("\r\n") != std::string::npos || value.find('\0') != std::string::npos) {
                throw System::FormatException("New-line or NUL characters are not allowed in header values.");
            }
        }

        bool isValidHost(const std::string& value) {
            if (value.empty()) return false;
            // Unlike checkNoNewlineOrNul() (used by From/Protocol below), this didn't reject an
            // embedded NUL byte -- an inconsistency within this same file, not a deliberate
            // design choice (this is otherwise already a known, accepted simplification of real
            // HttpRequestHeaders.cs's Host setter, which uses HttpRuleParser.GetHostLength() for
            // full RFC host-syntax validation, not a simple char blacklist -- reimplementing
            // that is out of scope here, but rejecting a literal NUL byte isn't).
            static constexpr std::string_view invalid(" \t\r\n\0", 5);
            return value.find_first_of(invalid) == std::string::npos;
        }

        bool tryParseRfc1123(const std::string& s, System::DateTimeOffset& result) {
            static constexpr std::array<const char*, 12> months = {
                "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

            char dayName[4] = {};
            char monthName[4] = {};
            int day = 0, year = 0, hour = 0, minute = 0, second = 0;
            char gmt[4] = {};

            int matched = std::sscanf(s.c_str(), "%3[A-Za-z], %d %3[A-Za-z] %d %d:%d:%d %3s",
                                       dayName, &day, monthName, &year, &hour, &minute, &second, gmt);
            if (matched != 8) return false;
            if (std::strcmp(gmt, "GMT") != 0) return false;

            int monthIndex = -1;
            for (int i = 0; i < 12; ++i) {
                if (std::strcmp(monthName, months[static_cast<size_t>(i)]) == 0) { monthIndex = i; break; }
            }
            if (monthIndex < 0) return false;

            try {
                result = System::DateTimeOffset(year, monthIndex + 1, day, hour, minute, second, System::TimeSpan::Zero);
                return true;
            } catch (...) {
                return false;
            }
        }

        bool equalsIgnoreCase(const std::string& a, const std::string& b) {
            return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) {
                return std::tolower(x) == std::tolower(y);
            });
        }

        bool containsTokenCaseInsensitive(const std::vector<std::string>& tokens, const std::string& target) {
            return std::any_of(tokens.begin(), tokens.end(), [&](const std::string& t) { return equalsIgnoreCase(t, target); });
        }
    }

    // --- Request headers -------------------------------------------------------------------

    std::vector<MediaTypeWithQualityHeaderValue> HttpRequestHeaders::getAcceptProperty() const {
        return parseList<MediaTypeWithQualityHeaderValue>(*this, "Accept", MediaTypeWithQualityHeaderValue::TryParse, MediaTypeWithQualityHeaderValue("x/x"));
    }
    void HttpRequestHeaders::AddAccept(const MediaTypeWithQualityHeaderValue& value) { Add("Accept", value.ToString()); }

    std::vector<StringWithQualityHeaderValue> HttpRequestHeaders::getAcceptCharsetProperty() const {
        return parseList<StringWithQualityHeaderValue>(*this, "Accept-Charset", StringWithQualityHeaderValue::TryParse, StringWithQualityHeaderValue("x"));
    }
    void HttpRequestHeaders::AddAcceptCharset(const StringWithQualityHeaderValue& value) { Add("Accept-Charset", value.ToString()); }

    std::vector<StringWithQualityHeaderValue> HttpRequestHeaders::getAcceptEncodingProperty() const {
        return parseList<StringWithQualityHeaderValue>(*this, "Accept-Encoding", StringWithQualityHeaderValue::TryParse, StringWithQualityHeaderValue("x"));
    }
    void HttpRequestHeaders::AddAcceptEncoding(const StringWithQualityHeaderValue& value) { Add("Accept-Encoding", value.ToString()); }

    std::vector<StringWithQualityHeaderValue> HttpRequestHeaders::getAcceptLanguageProperty() const {
        return parseList<StringWithQualityHeaderValue>(*this, "Accept-Language", StringWithQualityHeaderValue::TryParse, StringWithQualityHeaderValue("x"));
    }
    void HttpRequestHeaders::AddAcceptLanguage(const StringWithQualityHeaderValue& value) { Add("Accept-Language", value.ToString()); }

    std::optional<AuthenticationHeaderValue> HttpRequestHeaders::getAuthorizationProperty() const {
        std::string raw = getRawValue("Authorization");
        if (raw.empty()) return std::nullopt;
        AuthenticationHeaderValue result("x");
        if (!AuthenticationHeaderValue::TryParse(raw, result)) return std::nullopt;
        return result;
    }
    void HttpRequestHeaders::setAuthorizationProperty(const std::optional<AuthenticationHeaderValue>& value) {
        setRawValue("Authorization", value.has_value() ? value->ToString() : "");
    }

    std::optional<bool> HttpRequestHeaders::getExpectContinueProperty() const {
        for (const auto& item : getExpectProperty()) {
            if (item.getNameProperty() == "100-continue") return true;
        }
        if (expectContinueSet_) return false;
        return std::nullopt;
    }
    void HttpRequestHeaders::setExpectContinueProperty(std::optional<bool> value) {
        if (value == true) {
            expectContinueSet_ = true;
            if (!getExpectContinueProperty().value_or(false)) {
                AddExpect(NameValueWithParametersHeaderValue("100-continue"));
            }
        } else {
            expectContinueSet_ = value.has_value();
            std::vector<std::string> raw;
            if (TryGetValues("Expect", raw)) {
                std::vector<std::string> kept;
                for (const auto& line : raw) {
                    for (const auto& part : splitTopLevel(line)) {
                        std::string token = trim(part);
                        if (token.empty() || token == "100-continue") continue;
                        kept.push_back(token);
                    }
                }
                Remove("Expect");
                for (const auto& token : kept) Add("Expect", token);
            }
        }
    }

    std::vector<NameValueWithParametersHeaderValue> HttpRequestHeaders::getExpectProperty() const {
        return parseList<NameValueWithParametersHeaderValue>(*this, "Expect", NameValueWithParametersHeaderValue::TryParse, NameValueWithParametersHeaderValue("x"));
    }
    void HttpRequestHeaders::AddExpect(const NameValueWithParametersHeaderValue& value) { Add("Expect", value.ToString()); }

    std::optional<std::string> HttpRequestHeaders::getFromProperty() const {
        std::string raw = getRawValue("From");
        if (raw.empty()) return std::nullopt;
        return raw;
    }
    void HttpRequestHeaders::setFromProperty(const std::optional<std::string>& value) {
        std::string v = value.value_or("");
        checkNoNewlineOrNul(v);
        setRawValue("From", v);
    }

    std::optional<std::string> HttpRequestHeaders::getHostProperty() const {
        std::string raw = getRawValue("Host");
        if (raw.empty()) return std::nullopt;
        return raw;
    }
    void HttpRequestHeaders::setHostProperty(const std::optional<std::string>& value) {
        std::string v = value.value_or("");
        if (!v.empty() && !isValidHost(v)) {
            throw System::FormatException("The specified value is not a valid 'Host' header string.");
        }
        setRawValue("Host", v);
    }

    std::vector<EntityTagHeaderValue> HttpRequestHeaders::getIfMatchProperty() const {
        return parseList<EntityTagHeaderValue>(*this, "If-Match", EntityTagHeaderValue::TryParse, EntityTagHeaderValue("\"x\""));
    }
    void HttpRequestHeaders::AddIfMatch(const EntityTagHeaderValue& value) { Add("If-Match", value.ToString()); }

    std::optional<System::DateTimeOffset> HttpRequestHeaders::getIfModifiedSinceProperty() const {
        std::string raw = getRawValue("If-Modified-Since");
        if (raw.empty()) return std::nullopt;
        System::DateTimeOffset result;
        if (!tryParseRfc1123(raw, result)) return std::nullopt;
        return result;
    }
    void HttpRequestHeaders::setIfModifiedSinceProperty(const std::optional<System::DateTimeOffset>& value) {
        setRawValue("If-Modified-Since", value.has_value() ? value->ToString("r") : "");
    }

    std::vector<EntityTagHeaderValue> HttpRequestHeaders::getIfNoneMatchProperty() const {
        return parseList<EntityTagHeaderValue>(*this, "If-None-Match", EntityTagHeaderValue::TryParse, EntityTagHeaderValue("\"x\""));
    }
    void HttpRequestHeaders::AddIfNoneMatch(const EntityTagHeaderValue& value) { Add("If-None-Match", value.ToString()); }

    std::optional<RangeConditionHeaderValue> HttpRequestHeaders::getIfRangeProperty() const {
        std::string raw = getRawValue("If-Range");
        if (raw.empty()) return std::nullopt;
        RangeConditionHeaderValue result("\"x\"");
        if (!RangeConditionHeaderValue::TryParse(raw, result)) return std::nullopt;
        return result;
    }
    void HttpRequestHeaders::setIfRangeProperty(const std::optional<RangeConditionHeaderValue>& value) {
        setRawValue("If-Range", value.has_value() ? value->ToString() : "");
    }

    std::optional<System::DateTimeOffset> HttpRequestHeaders::getIfUnmodifiedSinceProperty() const {
        std::string raw = getRawValue("If-Unmodified-Since");
        if (raw.empty()) return std::nullopt;
        System::DateTimeOffset result;
        if (!tryParseRfc1123(raw, result)) return std::nullopt;
        return result;
    }
    void HttpRequestHeaders::setIfUnmodifiedSinceProperty(const std::optional<System::DateTimeOffset>& value) {
        setRawValue("If-Unmodified-Since", value.has_value() ? value->ToString("r") : "");
    }

    std::optional<SharpRuntime::intcs> HttpRequestHeaders::getMaxForwardsProperty() const {
        std::string raw = getRawValue("Max-Forwards");
        if (raw.empty()) return std::nullopt;
        try {
            size_t pos = 0;
            long parsed = std::stol(raw, &pos);
            if (pos != raw.size()) return std::nullopt;
            return static_cast<SharpRuntime::intcs>(parsed);
        } catch (...) {
            return std::nullopt;
        }
    }
    void HttpRequestHeaders::setMaxForwardsProperty(std::optional<SharpRuntime::intcs> value) {
        setRawValue("Max-Forwards", value.has_value() ? std::to_string(*value) : "");
    }

    std::optional<std::string> HttpRequestHeaders::getProtocolProperty() const {
        if (protocol_.empty()) return std::nullopt;
        return protocol_;
    }
    void HttpRequestHeaders::setProtocolProperty(const std::optional<std::string>& value) {
        std::string v = value.value_or("");
        checkNoNewlineOrNul(v);
        protocol_ = v;
    }

    std::optional<AuthenticationHeaderValue> HttpRequestHeaders::getProxyAuthorizationProperty() const {
        std::string raw = getRawValue("Proxy-Authorization");
        if (raw.empty()) return std::nullopt;
        AuthenticationHeaderValue result("x");
        if (!AuthenticationHeaderValue::TryParse(raw, result)) return std::nullopt;
        return result;
    }
    void HttpRequestHeaders::setProxyAuthorizationProperty(const std::optional<AuthenticationHeaderValue>& value) {
        setRawValue("Proxy-Authorization", value.has_value() ? value->ToString() : "");
    }

    std::optional<RangeHeaderValue> HttpRequestHeaders::getRangeProperty() const {
        std::string raw = getRawValue("Range");
        if (raw.empty()) return std::nullopt;
        RangeHeaderValue result;
        if (!RangeHeaderValue::TryParse(raw, result)) return std::nullopt;
        return result;
    }
    void HttpRequestHeaders::setRangeProperty(const std::optional<RangeHeaderValue>& value) {
        setRawValue("Range", value.has_value() ? value->ToString() : "");
    }

    std::optional<System::Uri> HttpRequestHeaders::getReferrerProperty() const {
        std::string raw = getRawValue("Referer");
        if (raw.empty()) return std::nullopt;
        try {
            return System::Uri(raw);
        } catch (...) {
            return std::nullopt;
        }
    }
    void HttpRequestHeaders::setReferrerProperty(const std::optional<System::Uri>& value) {
        setRawValue("Referer", value.has_value() ? value->ToString() : "");
    }

    std::vector<TransferCodingWithQualityHeaderValue> HttpRequestHeaders::getTEProperty() const {
        return parseList<TransferCodingWithQualityHeaderValue>(*this, "TE", TransferCodingWithQualityHeaderValue::TryParse, TransferCodingWithQualityHeaderValue("x"));
    }
    void HttpRequestHeaders::AddTE(const TransferCodingWithQualityHeaderValue& value) { Add("TE", value.ToString()); }

    std::vector<ProductInfoHeaderValue> HttpRequestHeaders::getUserAgentProperty() const {
        return parseList<ProductInfoHeaderValue>(*this, "User-Agent", ProductInfoHeaderValue::TryParse, ProductInfoHeaderValue("x", "1"));
    }
    void HttpRequestHeaders::AddUserAgent(const ProductInfoHeaderValue& value) { Add("User-Agent", value.ToString()); }

    // --- General headers -------------------------------------------------------------------

    std::optional<CacheControlHeaderValue> HttpRequestHeaders::getCacheControlProperty() const {
        std::string raw = getRawValue("Cache-Control");
        if (raw.empty()) return std::nullopt;
        CacheControlHeaderValue result;
        if (!CacheControlHeaderValue::TryParse(raw, result)) return std::nullopt;
        return result;
    }
    void HttpRequestHeaders::setCacheControlProperty(const std::optional<CacheControlHeaderValue>& value) {
        setRawValue("Cache-Control", value.has_value() ? value->ToString() : "");
    }

    std::vector<std::string> HttpRequestHeaders::getConnectionProperty() const {
        std::vector<std::string> result;
        std::vector<std::string> raw;
        if (!TryGetValues("Connection", raw)) return result;
        for (const auto& line : raw) {
            for (const auto& part : splitTopLevel(line)) {
                std::string token = trim(part);
                if (!token.empty()) result.push_back(token);
            }
        }
        return result;
    }
    void HttpRequestHeaders::AddConnection(const std::string& token) { Add("Connection", token); }

    std::optional<bool> HttpRequestHeaders::getConnectionCloseProperty() const {
        if (!Contains("Connection")) return std::nullopt;
        return containsTokenCaseInsensitive(getConnectionProperty(), "close");
    }
    void HttpRequestHeaders::setConnectionCloseProperty(std::optional<bool> value) {
        auto tokens = getConnectionProperty();
        bool hasClose = containsTokenCaseInsensitive(tokens, "close");
        if (value == true) {
            if (!hasClose) AddConnection("close");
        } else if (hasClose) {
            std::vector<std::string> kept;
            for (const auto& t : tokens) {
                if (!equalsIgnoreCase(t, "close")) kept.push_back(t);
            }
            Remove("Connection");
            for (const auto& t : kept) AddConnection(t);
        }
    }

    std::optional<System::DateTimeOffset> HttpRequestHeaders::getDateProperty() const {
        std::string raw = getRawValue("Date");
        if (raw.empty()) return std::nullopt;
        System::DateTimeOffset result;
        if (!tryParseRfc1123(raw, result)) return std::nullopt;
        return result;
    }
    void HttpRequestHeaders::setDateProperty(const std::optional<System::DateTimeOffset>& value) {
        setRawValue("Date", value.has_value() ? value->ToString("r") : "");
    }

    std::vector<NameValueHeaderValue> HttpRequestHeaders::getPragmaProperty() const {
        return parseList<NameValueHeaderValue>(*this, "Pragma", NameValueHeaderValue::TryParse, NameValueHeaderValue("x"));
    }
    void HttpRequestHeaders::AddPragma(const NameValueHeaderValue& value) { Add("Pragma", value.ToString()); }

    std::vector<std::string> HttpRequestHeaders::getTrailerProperty() const {
        std::vector<std::string> result;
        std::vector<std::string> raw;
        if (!TryGetValues("Trailer", raw)) return result;
        for (const auto& line : raw) {
            for (const auto& part : splitTopLevel(line)) {
                std::string token = trim(part);
                if (!token.empty()) result.push_back(token);
            }
        }
        return result;
    }
    void HttpRequestHeaders::AddTrailer(const std::string& token) { Add("Trailer", token); }

    std::vector<TransferCodingHeaderValue> HttpRequestHeaders::getTransferEncodingProperty() const {
        return parseList<TransferCodingHeaderValue>(*this, "Transfer-Encoding", TransferCodingHeaderValue::TryParse, TransferCodingHeaderValue("x"));
    }
    void HttpRequestHeaders::AddTransferEncoding(const TransferCodingHeaderValue& value) { Add("Transfer-Encoding", value.ToString()); }

    std::optional<bool> HttpRequestHeaders::getTransferEncodingChunkedProperty() const {
        if (!Contains("Transfer-Encoding")) return std::nullopt;
        for (const auto& item : getTransferEncodingProperty()) {
            if (equalsIgnoreCase(item.getValueProperty(), "chunked")) return true;
        }
        return false;
    }
    void HttpRequestHeaders::setTransferEncodingChunkedProperty(std::optional<bool> value) {
        bool hasChunked = getTransferEncodingChunkedProperty().value_or(false);
        if (value == true) {
            if (!hasChunked) AddTransferEncoding(TransferCodingHeaderValue("chunked"));
        } else if (hasChunked) {
            auto items = getTransferEncodingProperty();
            Remove("Transfer-Encoding");
            for (const auto& item : items) {
                if (!equalsIgnoreCase(item.getValueProperty(), "chunked")) AddTransferEncoding(item);
            }
        }
    }

    std::vector<ProductHeaderValue> HttpRequestHeaders::getUpgradeProperty() const {
        return parseList<ProductHeaderValue>(*this, "Upgrade", ProductHeaderValue::TryParse, ProductHeaderValue("x"));
    }
    void HttpRequestHeaders::AddUpgrade(const ProductHeaderValue& value) { Add("Upgrade", value.ToString()); }

    std::vector<ViaHeaderValue> HttpRequestHeaders::getViaProperty() const {
        return parseList<ViaHeaderValue>(*this, "Via", ViaHeaderValue::TryParse, ViaHeaderValue("1.1", "x"));
    }
    void HttpRequestHeaders::AddVia(const ViaHeaderValue& value) { Add("Via", value.ToString()); }

    std::vector<WarningHeaderValue> HttpRequestHeaders::getWarningProperty() const {
        return parseList<WarningHeaderValue>(*this, "Warning", WarningHeaderValue::TryParse, WarningHeaderValue(0, "x", "\"x\""));
    }
    void HttpRequestHeaders::AddWarning(const WarningHeaderValue& value) { Add("Warning", value.ToString()); }

} // namespace System::Net::Http::Headers
