// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/RangeConditionHeaderValue.hpp"
#include "System/FormatException.hpp"
#include "System/TimeSpan.hpp"
#include <array>
#include <cstring>
#include <cstdio>

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

    RangeConditionHeaderValue::RangeConditionHeaderValue(const System::DateTimeOffset& date) : date_(date) {}

    RangeConditionHeaderValue::RangeConditionHeaderValue(const EntityTagHeaderValue& entityTag) : entityTag_(entityTag) {}

    RangeConditionHeaderValue::RangeConditionHeaderValue(const std::string& entityTag) : entityTag_(EntityTagHeaderValue(entityTag)) {}

    std::string RangeConditionHeaderValue::ToString() const {
        return entityTag_.has_value() ? entityTag_->ToString() : date_->ToString("r");
    }

    bool RangeConditionHeaderValue::Equals(const RangeConditionHeaderValue& other) const {
        if (entityTag_.has_value() != other.entityTag_.has_value()) return false;
        if (entityTag_.has_value()) return *entityTag_ == *other.entityTag_;
        return date_ == other.date_;
    }

    SharpRuntime::intcs RangeConditionHeaderValue::GetHashCode() const {
        return entityTag_.has_value() ? entityTag_->GetHashCode()
                                       : static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(date_->ToString("r")));
    }

    bool RangeConditionHeaderValue::TryParse(const std::string& input, RangeConditionHeaderValue& parsedValue) {
        std::string trimmed = trim(input);
        if (trimmed.size() < 2) return false;

        char first = trimmed.front();
        char second = trimmed[1];
        if (first == '"' || ((first == 'w' || first == 'W') && second == '/')) {
            EntityTagHeaderValue tag = EntityTagHeaderValue::Any();
            if (!EntityTagHeaderValue::TryParse(trimmed, tag)) return false;
            parsedValue = RangeConditionHeaderValue(tag);
            return true;
        }

        System::DateTimeOffset date;
        if (!tryParseRfc1123(trimmed, date)) return false;
        parsedValue = RangeConditionHeaderValue(date);
        return true;
    }

    RangeConditionHeaderValue RangeConditionHeaderValue::Parse(const std::string& input) {
        RangeConditionHeaderValue result;
        if (!TryParse(input, result)) {
            throw System::FormatException("The If-Range header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
