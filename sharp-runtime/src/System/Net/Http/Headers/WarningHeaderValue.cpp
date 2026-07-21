// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/WarningHeaderValue.hpp"
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

        bool isValidAgent(const std::string& s) {
            if (s.empty()) return false;
            if (isToken(s)) return true;
            return s.find_first_of(" \t\r/,") == std::string::npos;
        }

        bool isValidQuotedString(const std::string& value) {
            if (value.size() < 2 || value.front() != '"' || value.back() != '"') return false;
            for (size_t i = 1; i + 1 < value.size(); ) {
                if (value[i] == '\\') {
                    if (i + 1 >= value.size() - 1) return false;
                    i += 2;
                } else if (value[i] == '"') {
                    return false;
                } else {
                    ++i;
                }
            }
            return true;
        }

        // Given s[pos] == '"', finds the end of the quoted-string (handling \-escapes) and returns
        // the substring including both quotes, or empty on failure.
        bool extractQuotedString(const std::string& s, size_t pos, std::string& out, size_t& endPos) {
            if (pos >= s.size() || s[pos] != '"') return false;
            size_t i = pos + 1;
            while (i < s.size()) {
                if (s[i] == '\\') {
                    if (i + 1 >= s.size()) return false;
                    i += 2;
                    continue;
                }
                if (s[i] == '"') {
                    out = s.substr(pos, i - pos + 1);
                    endPos = i + 1;
                    return true;
                }
                ++i;
            }
            return false;
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

    WarningHeaderValue::WarningHeaderValue(SharpRuntime::intcs code, const std::string& agent, const std::string& text) {
        System::ArgumentOutOfRangeException::ThrowIfNegative(code, "code");
        System::ArgumentOutOfRangeException::ThrowIfGreaterThan(code, 999, "code");
        System::ArgumentException::ThrowIfNullOrEmpty(agent, "agent");
        if (!isValidAgent(agent)) {
            throw System::FormatException("The agent value is not valid: " + agent);
        }
        if (!isValidQuotedString(text)) {
            throw System::FormatException("The text is not a valid quoted-string: " + text);
        }

        code_ = code;
        agent_ = agent;
        text_ = text;
    }

    WarningHeaderValue::WarningHeaderValue(SharpRuntime::intcs code, const std::string& agent, const std::string& text, System::DateTimeOffset date)
        : WarningHeaderValue(code, agent, text) {
        date_ = date;
    }

    std::string WarningHeaderValue::ToString() const {
        char codeBuf[8];
        std::snprintf(codeBuf, sizeof(codeBuf), "%03d", code_);
        std::string result = std::string(codeBuf) + " " + agent_ + " " + text_;
        if (date_.has_value()) {
            result += " \"" + date_->ToString("r") + "\"";
        }
        return result;
    }

    bool WarningHeaderValue::Equals(const WarningHeaderValue& other) const {
        return code_ == other.code_ &&
               equalsIgnoreCase(agent_, other.agent_) &&
               text_ == other.text_ &&
               date_ == other.date_;
    }

    SharpRuntime::intcs WarningHeaderValue::GetHashCode() const {
        return System::HashCode::Combine(code_, hashIgnoreCase(agent_), text_, date_.has_value());
    }

    bool WarningHeaderValue::TryParse(const std::string& input, WarningHeaderValue& parsedValue) {
        std::string trimmed = trim(input);

        size_t ws1 = trimmed.find_first_of(" \t");
        if (ws1 == std::string::npos || ws1 == 0 || ws1 > 3) return false;
        std::string codeStr = trimmed.substr(0, ws1);
        if (!std::all_of(codeStr.begin(), codeStr.end(), [](unsigned char c) { return std::isdigit(c) != 0; })) return false;

        SharpRuntime::intcs code;
        try {
            code = static_cast<SharpRuntime::intcs>(std::stoi(codeStr));
        } catch (...) {
            return false;
        }
        if (code > 999) return false;

        size_t agentStart = trimmed.find_first_not_of(" \t", ws1);
        if (agentStart == std::string::npos) return false;
        size_t ws2 = trimmed.find_first_of(" \t", agentStart);
        if (ws2 == std::string::npos) return false;
        std::string agent = trimmed.substr(agentStart, ws2 - agentStart);
        if (!isValidAgent(agent)) return false;

        size_t textStart = trimmed.find_first_not_of(" \t", ws2);
        if (textStart == std::string::npos || trimmed[textStart] != '"') return false;

        std::string text;
        size_t afterText = 0;
        if (!extractQuotedString(trimmed, textStart, text, afterText)) return false;

        std::optional<System::DateTimeOffset> date;
        size_t dateStart = trimmed.find_first_not_of(" \t", afterText);
        if (dateStart != std::string::npos) {
            if (trimmed[dateStart] != '"') return false;
            std::string dateQuoted;
            size_t afterDate = 0;
            if (!extractQuotedString(trimmed, dateStart, dateQuoted, afterDate)) return false;
            std::string dateInner = dateQuoted.substr(1, dateQuoted.size() - 2);
            System::DateTimeOffset parsedDate;
            if (!tryParseRfc1123(dateInner, parsedDate)) return false;
            date = parsedDate;
            if (trim(trimmed.substr(afterDate)) != "") return false;
        }

        try {
            WarningHeaderValue result = date.has_value()
                ? WarningHeaderValue(code, agent, text, *date)
                : WarningHeaderValue(code, agent, text);
            parsedValue = result;
        } catch (...) {
            return false;
        }
        return true;
    }

    WarningHeaderValue WarningHeaderValue::Parse(const std::string& input) {
        WarningHeaderValue result;
        if (!TryParse(input, result)) {
            throw System::FormatException("The Warning header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
