// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/ContentDispositionHeaderValue.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include "System/HashCode.hpp"
#include "System/TimeSpan.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string_view>

namespace System::Net::Http::Headers {

    namespace {
        std::string trim(const std::string& s) {
            size_t start = s.find_first_not_of(" \t");
            if (start == std::string::npos) return "";
            size_t end = s.find_last_not_of(" \t");
            return s.substr(start, end - start + 1);
        }

        bool isHttpTokenChar(unsigned char c) {
            static constexpr std::string_view extras = "!#$%&'*+-.^_`|~";
            return c != 0 && (std::isalnum(c) || extras.find(static_cast<char>(c)) != std::string_view::npos);
        }

        bool isToken(const std::string& s) {
            return !s.empty() && std::all_of(s.begin(), s.end(), [](char c) { return isHttpTokenChar(static_cast<unsigned char>(c)); });
        }

        void checkValidToken(const std::string& value, const std::string& paramName) {
            System::ArgumentException::ThrowIfNullOrEmpty(value, paramName);
            if (!isToken(value)) {
                throw System::FormatException("The value is not a valid HTTP token: " + value);
            }
        }

        bool equalsIgnoreCase(const std::string& a, const std::string& b) {
            if (a.size() != b.size()) return false;
            return std::equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) {
                return std::tolower(x) == std::tolower(y);
            });
        }

        std::string toLowerAscii(const std::string& s) {
            std::string out = s;
            std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return out;
        }

        bool isQuoted(const std::string& s) {
            return s.size() >= 2 && s.front() == '"' && s.back() == '"';
        }

        std::string stripQuotes(const std::string& s) {
            return isQuoted(s) ? s.substr(1, s.size() - 2) : s;
        }

        std::string quote(const std::string& s) {
            return "\"" + s + "\"";
        }

        // Splits a ';'-separated list at the top level, ignoring ';' inside a quoted string.
        std::vector<std::string> splitTopLevel(const std::string& input, char delim) {
            std::vector<std::string> parts;
            std::string current;
            bool inQuotes = false;
            for (char c : input) {
                if (c == '"') {
                    inQuotes = !inQuotes;
                    current += c;
                } else if (c == delim && !inQuotes) {
                    parts.push_back(current);
                    current.clear();
                } else {
                    current += c;
                }
            }
            parts.push_back(current);
            return parts;
        }

        bool isRfc5987AttrChar(unsigned char c) {
            static constexpr std::string_view extras = "!#$&+-.^_`|~";
            return c != 0 && (std::isalnum(c) || extras.find(static_cast<char>(c)) != std::string_view::npos);
        }

        std::string encode5987(const std::string& value) {
            std::string out = "UTF-8''";
            char buf[4];
            for (unsigned char c : value) {
                if (isRfc5987AttrChar(c)) {
                    out += static_cast<char>(c);
                } else {
                    std::snprintf(buf, sizeof(buf), "%%%02X", c);
                    out += buf;
                }
            }
            return out;
        }

        // Parses the RFC 1123 format that DateTimeOffset::ToString("r") produces
        // (e.g. "Wed, 21 Oct 2015 07:28:00 GMT"). DateTimeOffset::TryParse doesn't accept this
        // format (only ISO-8601-style strings), so Content-Disposition's date parameters —
        // which the HTTP spec mandates in this RFC 822/1123 family of formats — need their own
        // parser here to round-trip what ToString("r") emits.
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

        bool tryDecode5987(const std::string& value, std::string& result) {
            size_t firstQuote = value.find('\'');
            if (firstQuote == std::string::npos) return false;
            size_t secondQuote = value.find('\'', firstQuote + 1);
            if (secondQuote == std::string::npos) return false;
            // Real .NET's TryDecode5987 requires *exactly* two single quotes in the whole
            // string (the charset'language'value delimiters):
            // `quoteIndex == lastQuoteIndex || input.IndexOf('\'', quoteIndex + 1) !=
            // lastQuoteIndex` rejects both "fewer than 2" and "more than 2". This previously
            // only checked for the first two, silently accepting a third (or more) quote as
            // part of the value -- e.g. "UTF-8'en'a'b" would decode to "a'b" instead of being
            // rejected as malformed, since a well-formed encoder must percent-encode any
            // literal apostrophe in the value (isRfc5987AttrChar doesn't allow a raw `'`).
            if (value.find('\'', secondQuote + 1) != std::string::npos) return false;

            std::string encoded = value.substr(secondQuote + 1);
            std::string decoded;
            for (size_t i = 0; i < encoded.size(); ++i) {
                if (encoded[i] == '%' && i + 2 < encoded.size()) {
                    auto hexVal = [](char c) -> int {
                        if (c >= '0' && c <= '9') return c - '0';
                        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                        return -1;
                    };
                    int hi = hexVal(encoded[i + 1]);
                    int lo = hexVal(encoded[i + 2]);
                    if (hi < 0 || lo < 0) return false;
                    decoded += static_cast<char>((hi << 4) | lo);
                    i += 2;
                } else {
                    decoded += encoded[i];
                }
            }
            result = decoded;
            return true;
        }
    }

    const NameValueHeaderValue* ContentDispositionHeaderValue::findParameter(const std::string& name) const {
        for (const auto& p : parameters_) {
            if (equalsIgnoreCase(p.getNameProperty(), name)) return &p;
        }
        return nullptr;
    }

    void ContentDispositionHeaderValue::removeParameter(const std::string& name) {
        parameters_.erase(std::remove_if(parameters_.begin(), parameters_.end(),
            [&](const NameValueHeaderValue& p) { return equalsIgnoreCase(p.getNameProperty(), name); }), parameters_.end());
    }

    void ContentDispositionHeaderValue::setOrAddParameter(const std::string& name, const std::string& value) {
        for (auto& p : parameters_) {
            if (equalsIgnoreCase(p.getNameProperty(), name)) {
                p.setValueProperty(value);
                return;
            }
        }
        parameters_.push_back(NameValueHeaderValue(name, value));
    }

    ContentDispositionHeaderValue::ContentDispositionHeaderValue(const std::string& dispositionType) {
        checkValidToken(dispositionType, "dispositionType");
        dispositionType_ = dispositionType;
    }

    void ContentDispositionHeaderValue::setDispositionTypeProperty(const std::string& value) {
        checkValidToken(value, "value");
        dispositionType_ = value;
    }

    std::string ContentDispositionHeaderValue::getNameProperty() const {
        const auto* p = findParameter("name");
        return p ? stripQuotes(p->getValueProperty()) : "";
    }

    void ContentDispositionHeaderValue::setNameProperty(const std::string& value) {
        if (value.empty()) { removeParameter("name"); return; }
        setOrAddParameter("name", quote(value));
    }

    std::string ContentDispositionHeaderValue::getFileNameProperty() const {
        const auto* p = findParameter("filename");
        return p ? stripQuotes(p->getValueProperty()) : "";
    }

    void ContentDispositionHeaderValue::setFileNameProperty(const std::string& value) {
        if (value.empty()) { removeParameter("filename"); return; }
        setOrAddParameter("filename", quote(value));
    }

    std::string ContentDispositionHeaderValue::getFileNameStarProperty() const {
        const auto* p = findParameter("filename*");
        if (!p) return "";
        std::string result;
        return tryDecode5987(p->getValueProperty(), result) ? result : "";
    }

    void ContentDispositionHeaderValue::setFileNameStarProperty(const std::string& value) {
        if (value.empty()) { removeParameter("filename*"); return; }
        setOrAddParameter("filename*", encode5987(value));
    }

    std::optional<System::DateTimeOffset> ContentDispositionHeaderValue::getCreationDateProperty() const {
        const auto* p = findParameter("creation-date");
        if (!p) return std::nullopt;
        System::DateTimeOffset result;
        if (tryParseRfc1123(stripQuotes(p->getValueProperty()), result)) return result;
        return std::nullopt;
    }

    void ContentDispositionHeaderValue::setCreationDateProperty(std::optional<System::DateTimeOffset> value) {
        if (!value.has_value()) { removeParameter("creation-date"); return; }
        setOrAddParameter("creation-date", quote(value->ToString("r")));
    }

    std::optional<System::DateTimeOffset> ContentDispositionHeaderValue::getModificationDateProperty() const {
        const auto* p = findParameter("modification-date");
        if (!p) return std::nullopt;
        System::DateTimeOffset result;
        if (tryParseRfc1123(stripQuotes(p->getValueProperty()), result)) return result;
        return std::nullopt;
    }

    void ContentDispositionHeaderValue::setModificationDateProperty(std::optional<System::DateTimeOffset> value) {
        if (!value.has_value()) { removeParameter("modification-date"); return; }
        setOrAddParameter("modification-date", quote(value->ToString("r")));
    }

    std::optional<System::DateTimeOffset> ContentDispositionHeaderValue::getReadDateProperty() const {
        const auto* p = findParameter("read-date");
        if (!p) return std::nullopt;
        System::DateTimeOffset result;
        if (tryParseRfc1123(stripQuotes(p->getValueProperty()), result)) return result;
        return std::nullopt;
    }

    void ContentDispositionHeaderValue::setReadDateProperty(std::optional<System::DateTimeOffset> value) {
        if (!value.has_value()) { removeParameter("read-date"); return; }
        setOrAddParameter("read-date", quote(value->ToString("r")));
    }

    std::optional<SharpRuntime::longcs> ContentDispositionHeaderValue::getSizeProperty() const {
        const auto* p = findParameter("size");
        if (!p) return std::nullopt;
        try {
            size_t pos = 0;
            long long parsed = std::stoll(p->getValueProperty(), &pos);
            if (pos != p->getValueProperty().size() || parsed < 0) return std::nullopt;
            return static_cast<SharpRuntime::longcs>(parsed);
        } catch (...) {
            return std::nullopt;
        }
    }

    void ContentDispositionHeaderValue::setSizeProperty(std::optional<SharpRuntime::longcs> value) {
        if (!value.has_value()) { removeParameter("size"); return; }
        if (*value < 0) throw System::ArgumentOutOfRangeException("value", "size must not be negative.");
        setOrAddParameter("size", std::to_string(*value));
    }

    std::string ContentDispositionHeaderValue::ToString() const {
        std::string result = dispositionType_;
        for (const auto& p : parameters_) {
            result += "; " + p.ToString();
        }
        return result;
    }

    bool ContentDispositionHeaderValue::Equals(const ContentDispositionHeaderValue& other) const {
        if (!equalsIgnoreCase(dispositionType_, other.dispositionType_)) return false;
        if (parameters_.size() != other.parameters_.size()) return false;

        std::vector<bool> matched(other.parameters_.size(), false);
        for (const auto& p : parameters_) {
            bool found = false;
            for (size_t i = 0; i < other.parameters_.size(); ++i) {
                if (!matched[i] && p == other.parameters_[i]) { matched[i] = true; found = true; break; }
            }
            if (!found) return false;
        }
        return true;
    }

    SharpRuntime::intcs ContentDispositionHeaderValue::GetHashCode() const {
        SharpRuntime::intcs result = static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(toLowerAscii(dispositionType_)));
        for (const auto& p : parameters_) result ^= p.GetHashCode();
        return result;
    }

    bool ContentDispositionHeaderValue::TryParse(const std::string& input, ContentDispositionHeaderValue& parsedValue) {
        auto segments = splitTopLevel(input, ';');
        std::string dispositionType = trim(segments[0]);
        if (!isToken(dispositionType)) return false;

        ContentDispositionHeaderValue result(dispositionType);
        for (size_t i = 1; i < segments.size(); ++i) {
            std::string segment = trim(segments[i]);
            if (segment.empty()) return false;
            try {
                result.parameters_.push_back(NameValueHeaderValue::Parse(segment));
            } catch (...) {
                return false;
            }
        }

        parsedValue = result;
        return true;
    }

    ContentDispositionHeaderValue ContentDispositionHeaderValue::Parse(const std::string& input) {
        ContentDispositionHeaderValue result("x");
        if (!TryParse(input, result)) {
            throw System::FormatException("The Content-Disposition header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
