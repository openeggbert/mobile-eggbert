// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/TransferCodingHeaderValue.hpp"
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

    TransferCodingHeaderValue::TransferCodingHeaderValue(const std::string& value) {
        System::ArgumentException::ThrowIfNullOrEmpty(value, "value");
        if (!isToken(value)) {
            throw System::FormatException("The value is not a valid HTTP token: " + value);
        }
        value_ = value;
    }

    std::string TransferCodingHeaderValue::ToString() const {
        std::string result = value_;
        for (const auto& p : parameters_) {
            result += "; " + p.ToString();
        }
        return result;
    }

    bool TransferCodingHeaderValue::Equals(const TransferCodingHeaderValue& other) const {
        if (!equalsIgnoreCase(value_, other.value_)) return false;
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

    SharpRuntime::intcs TransferCodingHeaderValue::GetHashCode() const {
        SharpRuntime::intcs result = static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(toLowerAscii(value_)));
        for (const auto& p : parameters_) result ^= p.GetHashCode();
        return result;
    }

    bool TransferCodingHeaderValue::TryParse(const std::string& input, TransferCodingHeaderValue& parsedValue) {
        auto segments = splitTopLevel(input, ';');
        std::string value = trim(segments[0]);
        if (!isToken(value)) return false;

        TransferCodingHeaderValue result(value);
        for (size_t i = 1; i < segments.size(); ++i) {
            std::string segment = trim(segments[i]);
            if (segment.empty()) return false;
            try {
                result.getParametersProperty().push_back(NameValueHeaderValue::Parse(segment));
            } catch (...) {
                return false;
            }
        }

        parsedValue = result;
        return true;
    }

    TransferCodingHeaderValue TransferCodingHeaderValue::Parse(const std::string& input) {
        TransferCodingHeaderValue result("x");
        if (!TryParse(input, result)) {
            throw System::FormatException("The transfer-coding header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
