// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/RangeHeaderValue.hpp"
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

        bool tryParseRangeItem(const std::string& segment, RangeItemHeaderValue& out) {
            size_t dash = segment.find('-');
            if (dash == std::string::npos) return false;

            std::string fromStr = trim(segment.substr(0, dash));
            std::string toStr = trim(segment.substr(dash + 1));
            if (fromStr.empty() && toStr.empty()) return false;

            std::optional<SharpRuntime::longcs> from, to;
            if (!fromStr.empty()) {
                SharpRuntime::longcs v;
                if (!tryParseNonNegativeLong(fromStr, v)) return false;
                from = v;
            }
            if (!toStr.empty()) {
                SharpRuntime::longcs v;
                if (!tryParseNonNegativeLong(toStr, v)) return false;
                to = v;
            }
            if (from.has_value() && to.has_value() && *from > *to) return false;

            try {
                out = RangeItemHeaderValue(from, to);
            } catch (...) {
                return false;
            }
            return true;
        }
    }

    RangeHeaderValue::RangeHeaderValue(std::optional<SharpRuntime::longcs> from, std::optional<SharpRuntime::longcs> to) {
        ranges_.push_back(RangeItemHeaderValue(from, to));
    }

    void RangeHeaderValue::setUnitProperty(const std::string& value) {
        if (!isToken(value)) {
            throw System::FormatException("The unit is not a valid HTTP token: " + value);
        }
        unit_ = value;
    }

    std::string RangeHeaderValue::ToString() const {
        std::string result = unit_ + "=";
        for (size_t i = 0; i < ranges_.size(); ++i) {
            if (i != 0) result += ", ";
            result += ranges_[i].ToString();
        }
        return result;
    }

    bool RangeHeaderValue::Equals(const RangeHeaderValue& other) const {
        if (!equalsIgnoreCase(unit_, other.unit_)) return false;
        if (ranges_.size() != other.ranges_.size()) return false;

        std::vector<bool> matched(other.ranges_.size(), false);
        for (const auto& r : ranges_) {
            bool found = false;
            for (size_t i = 0; i < other.ranges_.size(); ++i) {
                if (!matched[i] && r == other.ranges_[i]) { matched[i] = true; found = true; break; }
            }
            if (!found) return false;
        }
        return true;
    }

    SharpRuntime::intcs RangeHeaderValue::GetHashCode() const {
        std::string lowerUnit = unit_;
        std::transform(lowerUnit.begin(), lowerUnit.end(), lowerUnit.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        SharpRuntime::intcs result = static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(lowerUnit));
        for (const auto& r : ranges_) result ^= r.GetHashCode();
        return result;
    }

    bool RangeHeaderValue::TryParse(const std::string& input, RangeHeaderValue& parsedValue) {
        size_t eq = input.find('=');
        if (eq == std::string::npos) return false;

        std::string unit = trim(input.substr(0, eq));
        if (!isToken(unit)) return false;

        RangeHeaderValue result;
        result.unit_ = unit;

        std::string rest = input.substr(eq + 1);
        size_t start = 0;
        bool any = false;
        while (start <= rest.size()) {
            size_t next = rest.find(',', start);
            std::string segment = trim(next == std::string::npos ? rest.substr(start) : rest.substr(start, next - start));
            if (!segment.empty()) {
                RangeItemHeaderValue item(std::nullopt, 0);
                if (!tryParseRangeItem(segment, item)) return false;
                result.ranges_.push_back(item);
                any = true;
            }
            if (next == std::string::npos) break;
            start = next + 1;
        }

        if (!any) return false;

        parsedValue = result;
        return true;
    }

    RangeHeaderValue RangeHeaderValue::Parse(const std::string& input) {
        RangeHeaderValue result;
        if (!TryParse(input, result)) {
            throw System::FormatException("The Range header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
