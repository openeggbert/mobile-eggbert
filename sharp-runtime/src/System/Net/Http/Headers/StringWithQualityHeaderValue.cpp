// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/StringWithQualityHeaderValue.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include "System/HashCode.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace System::Net::Http::Headers {

    namespace {
        bool isHttpTokenChar(unsigned char c) {
            static constexpr const char* extras = "!#$%&'*+-.^_`|~";
            return c != 0 && (std::isalnum(c) || std::strchr(extras, static_cast<char>(c)) != nullptr);
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

        SharpRuntime::intcs hashIgnoreCase(const std::string& s) {
            std::string lower = s;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(lower));
        }

        void checkValidToken(const std::string& value, const std::string& paramName) {
            System::ArgumentException::ThrowIfNullOrEmpty(value, paramName);
            if (!isToken(value)) {
                throw System::FormatException("The value is not a valid HTTP token: " + value);
            }
        }

        // Matches real .NET's HttpRuleParser.GetNumberLength(input, start, allowDecimal: true)
        // grammar: digits with at most one '.', no leading dot (".5" is invalid, "0.5" required),
        // no sign, no exponent. A trailing dot with no digits after ("1.") is valid. std::stod
        // alone is far more permissive (accepts a leading dot, a sign, and scientific notation
        // like "1e0"), which would let TryParse silently accept quality values real .NET rejects.
        bool isValidQualityNumber(const std::string& s) {
            if (s.empty() || s[0] == '.') return false;
            bool haveDot = false, haveDigit = false;
            for (char c : s) {
                if (c >= '0' && c <= '9') haveDigit = true;
                else if (c == '.' && !haveDot) haveDot = true;
                else return false;
            }
            return haveDigit;
        }

        // Formats like .NET's "0.0##" custom numeric format: 1-3 decimal digits, trailing zeros
        // beyond the first decimal digit trimmed.
        std::string formatQuality(double quality) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%.3f", quality);
            std::string s(buf);
            size_t lastNonZero = s.find_last_not_of('0');
            if (lastNonZero != std::string::npos && s[lastNonZero] == '.') lastNonZero++;
            return s.substr(0, lastNonZero + 1);
        }
    }

    StringWithQualityHeaderValue::StringWithQualityHeaderValue(const std::string& value) : value_(value) {
        checkValidToken(value, "value");
    }

    StringWithQualityHeaderValue::StringWithQualityHeaderValue(const std::string& value, double quality)
        : value_(value), quality_(quality) {
        checkValidToken(value, "value");
        if (quality < 0.0) throw System::ArgumentOutOfRangeException("quality", "quality must not be negative.");
        if (quality > 1.0) throw System::ArgumentOutOfRangeException("quality", "quality must not be greater than 1.0.");
    }

    std::string StringWithQualityHeaderValue::ToString() const {
        if (!quality_.has_value()) return value_;
        return value_ + "; q=" + formatQuality(*quality_);
    }

    bool StringWithQualityHeaderValue::Equals(const StringWithQualityHeaderValue& other) const {
        return equalsIgnoreCase(value_, other.value_) && quality_ == other.quality_;
    }

    SharpRuntime::intcs StringWithQualityHeaderValue::GetHashCode() const {
        return System::HashCode::Combine(hashIgnoreCase(value_), quality_.value_or(-1.0));
    }

    bool StringWithQualityHeaderValue::TryParse(const std::string& input, StringWithQualityHeaderValue& parsedValue) {
        size_t start = input.find_first_not_of(" \t");
        if (start == std::string::npos) return false;
        size_t end = input.find_last_not_of(" \t");
        std::string trimmed = input.substr(start, end - start + 1);

        size_t semi = trimmed.find(';');
        std::string value = (semi == std::string::npos) ? trimmed : trimmed.substr(0, semi);
        size_t valueEnd = value.find_last_not_of(" \t");
        if (valueEnd == std::string::npos) return false;
        value = value.substr(0, valueEnd + 1);

        if (!isToken(value)) return false;

        if (semi == std::string::npos) {
            parsedValue = StringWithQualityHeaderValue(value);
            return true;
        }

        std::string qualityPart = trimmed.substr(semi + 1);
        size_t qStart = qualityPart.find_first_not_of(" \t");
        if (qStart == std::string::npos) return false;
        qualityPart = qualityPart.substr(qStart);

        if (qualityPart.size() < 3 || (qualityPart[0] != 'q' && qualityPart[0] != 'Q')) return false;
        size_t i = 1;
        while (i < qualityPart.size() && (qualityPart[i] == ' ' || qualityPart[i] == '\t')) ++i;
        if (i >= qualityPart.size() || qualityPart[i] != '=') return false;
        ++i;
        while (i < qualityPart.size() && (qualityPart[i] == ' ' || qualityPart[i] == '\t')) ++i;

        std::string numberStr = qualityPart.substr(i);
        size_t numEnd = numberStr.find_last_not_of(" \t");
        if (numEnd == std::string::npos) return false;
        numberStr = numberStr.substr(0, numEnd + 1);

        if (!isValidQualityNumber(numberStr)) return false;
        double quality;
        try {
            size_t pos = 0;
            quality = std::stod(numberStr, &pos);
            if (pos != numberStr.size()) return false;
        } catch (...) {
            return false;
        }
        if (quality < 0.0 || quality > 1.0) return false;

        parsedValue = StringWithQualityHeaderValue(value, quality);
        return true;
    }

    StringWithQualityHeaderValue StringWithQualityHeaderValue::Parse(const std::string& input) {
        StringWithQualityHeaderValue result("x");
        if (!TryParse(input, result)) {
            throw System::FormatException("The header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
