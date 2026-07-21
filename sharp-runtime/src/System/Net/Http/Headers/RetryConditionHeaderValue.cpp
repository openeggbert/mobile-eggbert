// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/RetryConditionHeaderValue.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace System::Net::Http::Headers {

    namespace {
        std::string trim(const std::string& s) {
            size_t start = s.find_first_not_of(" \t");
            if (start == std::string::npos) return "";
            size_t end = s.find_last_not_of(" \t");
            return s.substr(start, end - start + 1);
        }

        // Parses the RFC 1123 format that DateTimeOffset::ToString("r") produces (see
        // ContentDispositionHeaderValue.cpp for the same helper and why it's needed).
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
    }

    RetryConditionHeaderValue::RetryConditionHeaderValue(const System::DateTimeOffset& date) : date_(date) {}

    RetryConditionHeaderValue::RetryConditionHeaderValue(const System::TimeSpan& delta) {
        if (delta.getTotalSecondsProperty() > static_cast<double>(std::numeric_limits<SharpRuntime::intcs>::max())) {
            throw System::ArgumentOutOfRangeException("delta", "delta must not exceed INT32_MAX seconds.");
        }
        delta_ = delta;
    }

    std::string RetryConditionHeaderValue::ToString() const {
        return delta_.has_value() ? std::to_string(static_cast<long long>(delta_->getTotalSecondsProperty()))
                                   : date_->ToString("r");
    }

    bool RetryConditionHeaderValue::Equals(const RetryConditionHeaderValue& other) const {
        if (delta_.has_value() != other.delta_.has_value()) return false;
        if (delta_.has_value()) return *delta_ == *other.delta_;
        return date_ == other.date_;
    }

    SharpRuntime::intcs RetryConditionHeaderValue::GetHashCode() const {
        return delta_.has_value()
            ? static_cast<SharpRuntime::intcs>(std::hash<double>{}(delta_->getTotalSecondsProperty()))
            : static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(date_->ToString("r")));
    }

    bool RetryConditionHeaderValue::TryParse(const std::string& input, RetryConditionHeaderValue& parsedValue) {
        std::string trimmed = trim(input);
        if (trimmed.empty()) return false;

        if (std::isdigit(static_cast<unsigned char>(trimmed.front()))) {
            if (!std::all_of(trimmed.begin(), trimmed.end(), [](unsigned char c) { return std::isdigit(c) != 0; })) return false;
            if (trimmed.size() > 10) return false;
            long long seconds;
            try {
                seconds = std::stoll(trimmed);
            } catch (...) {
                return false;
            }
            if (seconds > std::numeric_limits<SharpRuntime::intcs>::max()) return false;

            parsedValue = RetryConditionHeaderValue(System::TimeSpan::FromSeconds(static_cast<double>(seconds)));
            return true;
        }

        System::DateTimeOffset date;
        if (!tryParseRfc1123(trimmed, date)) return false;
        parsedValue = RetryConditionHeaderValue(date);
        return true;
    }

    RetryConditionHeaderValue RetryConditionHeaderValue::Parse(const std::string& input) {
        RetryConditionHeaderValue result;
        if (!TryParse(input, result)) {
            throw System::FormatException("The Retry-After header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
