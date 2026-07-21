// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/HttpResponseHeaders.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstring>

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

        std::vector<std::string> tokenList(const HttpHeaders& headers, const std::string& name) {
            std::vector<std::string> result;
            std::vector<std::string> raw;
            if (!headers.TryGetValues(name, raw)) return result;
            for (const auto& line : raw) {
                for (const auto& part : splitTopLevel(line)) {
                    std::string token = trim(part);
                    if (!token.empty()) result.push_back(token);
                }
            }
            return result;
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

    // --- Response headers -------------------------------------------------------------------

    std::vector<std::string> HttpResponseHeaders::getAcceptRangesProperty() const { return tokenList(*this, "Accept-Ranges"); }
    void HttpResponseHeaders::AddAcceptRanges(const std::string& token) { Add("Accept-Ranges", token); }

    std::optional<System::TimeSpan> HttpResponseHeaders::getAgeProperty() const {
        // Matches real .NET's TimeSpanHeaderParser (used for Age): HttpRuleParser.GetNumberLength
        // counts a run of pure DIGIT characters (delta-seconds = 1*DIGIT, no sign allowed at
        // all), then HeaderUtilities.TryParseInt32 parses it -- std::stoll's leading-sign
        // tolerance (accepts an optional '+' or '-') let a value like "+5" through, silently
        // accepting a form real .NET rejects entirely (the same bug class already fixed in
        // CacheControlHeaderValue::tryParseSeconds for max-age/s-maxage/etc.). Validate every
        // character is a digit before parsing, exactly as that fix does.
        std::string raw = getRawValue("Age");
        if (raw.empty() ||
            !std::all_of(raw.begin(), raw.end(), [](unsigned char c) { return std::isdigit(c) != 0; }))
            return std::nullopt;
        try {
            size_t pos = 0;
            long long seconds = std::stoll(raw, &pos);
            if (pos != raw.size()) return std::nullopt;
            return System::TimeSpan::FromSeconds(static_cast<double>(seconds));
        } catch (...) {
            return std::nullopt;
        }
    }
    void HttpResponseHeaders::setAgeProperty(const std::optional<System::TimeSpan>& value) {
        setRawValue("Age", value.has_value() ? std::to_string(static_cast<long long>(value->getTotalSecondsProperty())) : "");
    }

    std::optional<EntityTagHeaderValue> HttpResponseHeaders::getETagProperty() const {
        std::string raw = getRawValue("ETag");
        if (raw.empty()) return std::nullopt;
        EntityTagHeaderValue result("\"x\"");
        if (!EntityTagHeaderValue::TryParse(raw, result)) return std::nullopt;
        return result;
    }
    void HttpResponseHeaders::setETagProperty(const std::optional<EntityTagHeaderValue>& value) {
        setRawValue("ETag", value.has_value() ? value->ToString() : "");
    }

    std::optional<System::Uri> HttpResponseHeaders::getLocationProperty() const {
        std::string raw = getRawValue("Location");
        if (raw.empty()) return std::nullopt;
        try {
            return System::Uri(raw);
        } catch (...) {
            return std::nullopt;
        }
    }
    void HttpResponseHeaders::setLocationProperty(const std::optional<System::Uri>& value) {
        setRawValue("Location", value.has_value() ? value->ToString() : "");
    }

    std::vector<AuthenticationHeaderValue> HttpResponseHeaders::getProxyAuthenticateProperty() const {
        return parseList<AuthenticationHeaderValue>(*this, "Proxy-Authenticate", AuthenticationHeaderValue::TryParse, AuthenticationHeaderValue("x"));
    }
    void HttpResponseHeaders::AddProxyAuthenticate(const AuthenticationHeaderValue& value) { Add("Proxy-Authenticate", value.ToString()); }

    std::optional<RetryConditionHeaderValue> HttpResponseHeaders::getRetryAfterProperty() const {
        std::string raw = getRawValue("Retry-After");
        if (raw.empty()) return std::nullopt;
        RetryConditionHeaderValue result(System::TimeSpan::Zero);
        if (!RetryConditionHeaderValue::TryParse(raw, result)) return std::nullopt;
        return result;
    }
    void HttpResponseHeaders::setRetryAfterProperty(const std::optional<RetryConditionHeaderValue>& value) {
        setRawValue("Retry-After", value.has_value() ? value->ToString() : "");
    }

    std::vector<ProductInfoHeaderValue> HttpResponseHeaders::getServerProperty() const {
        return parseList<ProductInfoHeaderValue>(*this, "Server", ProductInfoHeaderValue::TryParse, ProductInfoHeaderValue("x", "1"));
    }
    void HttpResponseHeaders::AddServer(const ProductInfoHeaderValue& value) { Add("Server", value.ToString()); }

    std::vector<std::string> HttpResponseHeaders::getVaryProperty() const { return tokenList(*this, "Vary"); }
    void HttpResponseHeaders::AddVary(const std::string& token) { Add("Vary", token); }

    std::vector<AuthenticationHeaderValue> HttpResponseHeaders::getWwwAuthenticateProperty() const {
        return parseList<AuthenticationHeaderValue>(*this, "WWW-Authenticate", AuthenticationHeaderValue::TryParse, AuthenticationHeaderValue("x"));
    }
    void HttpResponseHeaders::AddWwwAuthenticate(const AuthenticationHeaderValue& value) { Add("WWW-Authenticate", value.ToString()); }

    // --- General headers -------------------------------------------------------------------

    std::optional<CacheControlHeaderValue> HttpResponseHeaders::getCacheControlProperty() const {
        std::string raw = getRawValue("Cache-Control");
        if (raw.empty()) return std::nullopt;
        CacheControlHeaderValue result;
        if (!CacheControlHeaderValue::TryParse(raw, result)) return std::nullopt;
        return result;
    }
    void HttpResponseHeaders::setCacheControlProperty(const std::optional<CacheControlHeaderValue>& value) {
        setRawValue("Cache-Control", value.has_value() ? value->ToString() : "");
    }

    std::vector<std::string> HttpResponseHeaders::getConnectionProperty() const { return tokenList(*this, "Connection"); }
    void HttpResponseHeaders::AddConnection(const std::string& token) { Add("Connection", token); }

    std::optional<bool> HttpResponseHeaders::getConnectionCloseProperty() const {
        if (!Contains("Connection")) return std::nullopt;
        return containsTokenCaseInsensitive(getConnectionProperty(), "close");
    }
    void HttpResponseHeaders::setConnectionCloseProperty(std::optional<bool> value) {
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

    std::optional<System::DateTimeOffset> HttpResponseHeaders::getDateProperty() const {
        std::string raw = getRawValue("Date");
        if (raw.empty()) return std::nullopt;
        System::DateTimeOffset result;
        if (!tryParseRfc1123(raw, result)) return std::nullopt;
        return result;
    }
    void HttpResponseHeaders::setDateProperty(const std::optional<System::DateTimeOffset>& value) {
        setRawValue("Date", value.has_value() ? value->ToString("r") : "");
    }

    std::vector<NameValueHeaderValue> HttpResponseHeaders::getPragmaProperty() const {
        return parseList<NameValueHeaderValue>(*this, "Pragma", NameValueHeaderValue::TryParse, NameValueHeaderValue("x"));
    }
    void HttpResponseHeaders::AddPragma(const NameValueHeaderValue& value) { Add("Pragma", value.ToString()); }

    std::vector<std::string> HttpResponseHeaders::getTrailerProperty() const { return tokenList(*this, "Trailer"); }
    void HttpResponseHeaders::AddTrailer(const std::string& token) { Add("Trailer", token); }

    std::vector<TransferCodingHeaderValue> HttpResponseHeaders::getTransferEncodingProperty() const {
        return parseList<TransferCodingHeaderValue>(*this, "Transfer-Encoding", TransferCodingHeaderValue::TryParse, TransferCodingHeaderValue("x"));
    }
    void HttpResponseHeaders::AddTransferEncoding(const TransferCodingHeaderValue& value) { Add("Transfer-Encoding", value.ToString()); }

    std::optional<bool> HttpResponseHeaders::getTransferEncodingChunkedProperty() const {
        if (!Contains("Transfer-Encoding")) return std::nullopt;
        for (const auto& item : getTransferEncodingProperty()) {
            if (equalsIgnoreCase(item.getValueProperty(), "chunked")) return true;
        }
        return false;
    }
    void HttpResponseHeaders::setTransferEncodingChunkedProperty(std::optional<bool> value) {
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

    std::vector<ProductHeaderValue> HttpResponseHeaders::getUpgradeProperty() const {
        return parseList<ProductHeaderValue>(*this, "Upgrade", ProductHeaderValue::TryParse, ProductHeaderValue("x"));
    }
    void HttpResponseHeaders::AddUpgrade(const ProductHeaderValue& value) { Add("Upgrade", value.ToString()); }

    std::vector<ViaHeaderValue> HttpResponseHeaders::getViaProperty() const {
        return parseList<ViaHeaderValue>(*this, "Via", ViaHeaderValue::TryParse, ViaHeaderValue("1.1", "x"));
    }
    void HttpResponseHeaders::AddVia(const ViaHeaderValue& value) { Add("Via", value.ToString()); }

    std::vector<WarningHeaderValue> HttpResponseHeaders::getWarningProperty() const {
        return parseList<WarningHeaderValue>(*this, "Warning", WarningHeaderValue::TryParse, WarningHeaderValue(0, "x", "\"x\""));
    }
    void HttpResponseHeaders::AddWarning(const WarningHeaderValue& value) { Add("Warning", value.ToString()); }

} // namespace System::Net::Http::Headers
