// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/MediaTypeHeaderValue.hpp"
#include "System/ArgumentException.hpp"
#include "System/FormatException.hpp"
#include <algorithm>
#include <cctype>
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

        // Validates "type/subtype" with no internal/leading/trailing whitespace.
        bool isValidMediaType(const std::string& s) {
            size_t slash = s.find('/');
            if (slash == std::string::npos || slash == 0 || slash == s.size() - 1) return false;
            return isToken(s.substr(0, slash)) && isToken(s.substr(slash + 1));
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
    }

    MediaTypeHeaderValue::MediaTypeHeaderValue(const std::string& mediaType) {
        setMediaTypeProperty(mediaType);
    }

    MediaTypeHeaderValue::MediaTypeHeaderValue(const std::string& mediaType, const std::string& charSet) {
        setMediaTypeProperty(mediaType);
        if (!charSet.empty()) {
            setCharSetProperty(charSet);
        }
    }

    void MediaTypeHeaderValue::setMediaTypeProperty(const std::string& value) {
        System::ArgumentException::ThrowIfNullOrEmpty(value, "mediaType");
        if (!isValidMediaType(value)) {
            throw System::FormatException("The media type is not valid: " + value);
        }
        mediaType_ = value;
    }

    std::string MediaTypeHeaderValue::getCharSetProperty() const {
        for (const auto& p : parameters_) {
            if (equalsIgnoreCase(p.getNameProperty(), "charset")) return p.getValueProperty();
        }
        return "";
    }

    void MediaTypeHeaderValue::setCharSetProperty(const std::string& value) {
        for (auto it = parameters_.begin(); it != parameters_.end(); ++it) {
            if (equalsIgnoreCase(it->getNameProperty(), "charset")) {
                if (value.empty()) {
                    parameters_.erase(it);
                } else {
                    it->setValueProperty(value);
                }
                return;
            }
        }
        if (!value.empty()) {
            parameters_.push_back(NameValueHeaderValue("charset", value));
        }
    }

    std::string MediaTypeHeaderValue::ToString() const {
        std::string result = mediaType_;
        for (const auto& p : parameters_) {
            result += "; " + p.ToString();
        }
        return result;
    }

    bool MediaTypeHeaderValue::Equals(const MediaTypeHeaderValue& other) const {
        if (!equalsIgnoreCase(mediaType_, other.mediaType_)) return false;
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

    SharpRuntime::intcs MediaTypeHeaderValue::GetHashCode() const {
        SharpRuntime::intcs result = static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(toLowerAscii(mediaType_)));
        for (const auto& p : parameters_) result ^= p.GetHashCode();
        return result;
    }

    bool MediaTypeHeaderValue::TryParse(const std::string& input, MediaTypeHeaderValue& parsedValue) {
        auto segments = splitTopLevel(input, ';');
        std::string mediaType = trim(segments[0]);
        if (!isValidMediaType(mediaType)) return false;

        MediaTypeHeaderValue result(mediaType);
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

    MediaTypeHeaderValue MediaTypeHeaderValue::Parse(const std::string& input) {
        MediaTypeHeaderValue result("x/x");
        if (!TryParse(input, result)) {
            throw System::FormatException("The media type header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
