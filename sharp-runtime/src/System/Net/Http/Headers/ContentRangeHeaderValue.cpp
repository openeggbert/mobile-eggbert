// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/ContentRangeHeaderValue.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include "System/HashCode.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string_view>

namespace System::Net::Http::Headers {

    namespace {
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

        std::string trim(const std::string& s) {
            size_t start = s.find_first_not_of(" \t");
            if (start == std::string::npos) return "";
            size_t end = s.find_last_not_of(" \t");
            return s.substr(start, end - start + 1);
        }

        bool tryParseNonNegativeLong(const std::string& s, SharpRuntime::longcs& out) {
            if (s.empty() || !std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c) != 0; })) return false;
            try {
                size_t pos = 0;
                long long parsed = std::stoll(s, &pos);
                if (pos != s.size()) return false;
                out = static_cast<SharpRuntime::longcs>(parsed);
                return true;
            } catch (...) {
                return false;
            }
        }
    }

    ContentRangeHeaderValue::ContentRangeHeaderValue(SharpRuntime::longcs from, SharpRuntime::longcs to, SharpRuntime::longcs length) {
        if (length < 0) throw System::ArgumentOutOfRangeException("length");
        if (to < 0) throw System::ArgumentOutOfRangeException("to");
        if (to > length) throw System::ArgumentOutOfRangeException("to");
        if (from < 0) throw System::ArgumentOutOfRangeException("from");
        if (from > to) throw System::ArgumentOutOfRangeException("from");

        from_ = from;
        to_ = to;
        length_ = length;
    }

    ContentRangeHeaderValue::ContentRangeHeaderValue(SharpRuntime::longcs length) {
        if (length < 0) throw System::ArgumentOutOfRangeException("length");
        length_ = length;
    }

    ContentRangeHeaderValue::ContentRangeHeaderValue(SharpRuntime::longcs from, SharpRuntime::longcs to) {
        if (to < 0) throw System::ArgumentOutOfRangeException("to");
        if (from < 0) throw System::ArgumentOutOfRangeException("from");
        if (from > to) throw System::ArgumentOutOfRangeException("from");

        from_ = from;
        to_ = to;
    }

    void ContentRangeHeaderValue::setUnitProperty(const std::string& value) {
        if (!isToken(value)) {
            throw System::FormatException("The unit is not a valid HTTP token: " + value);
        }
        unit_ = value;
    }

    std::string ContentRangeHeaderValue::ToString() const {
        std::string result = unit_ + " ";
        if (getHasRangeProperty()) {
            result += std::to_string(*from_) + "-" + std::to_string(*to_);
        } else {
            result += "*";
        }
        result += "/";
        result += getHasLengthProperty() ? std::to_string(*length_) : "*";
        return result;
    }

    bool ContentRangeHeaderValue::Equals(const ContentRangeHeaderValue& other) const {
        return from_ == other.from_ && to_ == other.to_ && length_ == other.length_ &&
               equalsIgnoreCase(unit_, other.unit_);
    }

    SharpRuntime::intcs ContentRangeHeaderValue::GetHashCode() const {
        std::string lowerUnit = unit_;
        std::transform(lowerUnit.begin(), lowerUnit.end(), lowerUnit.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return System::HashCode::Combine(lowerUnit, from_.value_or(-1), to_.value_or(-1), length_.value_or(-1));
    }

    bool ContentRangeHeaderValue::TryParse(const std::string& input, ContentRangeHeaderValue& parsedValue) {
        std::string trimmed = trim(input);
        size_t ws = trimmed.find_first_of(" \t");
        if (ws == std::string::npos) return false;

        std::string unit = trimmed.substr(0, ws);
        if (!isToken(unit)) return false;

        size_t rangeStart = trimmed.find_first_not_of(" \t", ws);
        if (rangeStart == std::string::npos) return false;
        std::string rest = trimmed.substr(rangeStart);

        size_t slash = rest.find('/');
        if (slash == std::string::npos) return false;

        std::string rangePart = trim(rest.substr(0, slash));
        std::string lengthPart = trim(rest.substr(slash + 1));
        if (rangePart.empty() || lengthPart.empty()) return false;

        ContentRangeHeaderValue result;
        result.unit_ = unit;

        if (rangePart != "*") {
            size_t dash = rangePart.find('-');
            if (dash == std::string::npos) return false;
            SharpRuntime::longcs from, to;
            if (!tryParseNonNegativeLong(trim(rangePart.substr(0, dash)), from)) return false;
            if (!tryParseNonNegativeLong(trim(rangePart.substr(dash + 1)), to)) return false;
            if (from > to) return false;
            result.from_ = from;
            result.to_ = to;
        }

        if (lengthPart != "*") {
            SharpRuntime::longcs length;
            if (!tryParseNonNegativeLong(lengthPart, length)) return false;
            if (result.to_.has_value() && *result.to_ >= length) return false;
            result.length_ = length;
        }

        parsedValue = result;
        return true;
    }

    ContentRangeHeaderValue ContentRangeHeaderValue::Parse(const std::string& input) {
        ContentRangeHeaderValue result;
        if (!TryParse(input, result)) {
            throw System::FormatException("The Content-Range header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
