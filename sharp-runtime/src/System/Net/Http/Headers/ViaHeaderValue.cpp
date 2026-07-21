// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/ViaHeaderValue.hpp"
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

        // Practical approximation of HttpRuleParser.GetHostLength: accepts a token, or any
        // non-empty string containing none of the HTTP list-delimiter characters (space, tab,
        // CR, '/', ',') — covers hostnames, "host:port", and bracketed IPv6 literals.
        bool isValidReceivedBy(const std::string& s) {
            if (s.empty()) return false;
            if (isToken(s)) return true;
            return s.find_first_of(" \t\r/,") == std::string::npos;
        }

        bool isValidComment(const std::string& s) {
            if (s.empty() || s.front() != '(') return false;
            int depth = 0;
            size_t i = 0;
            while (i < s.size()) {
                char c = s[i];
                if (c == '\\') {
                    if (i + 1 >= s.size()) return false;
                    unsigned char next = static_cast<unsigned char>(s[i + 1]);
                    if (next > 127 || next == '\r' || next == '\n' || next == 0) return false;
                    i += 2;
                    continue;
                }
                if (c == '(') {
                    ++depth;
                    if (depth > 5) return false;
                    ++i;
                    continue;
                }
                if (c == ')') {
                    --depth;
                    ++i;
                    if (depth == 0) return i == s.size();
                    if (depth < 0) return false;
                    continue;
                }
                if (static_cast<unsigned char>(c) < 0x20 && c != '\t') return false;
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

        void checkReceivedBy(const std::string& receivedBy) {
            System::ArgumentException::ThrowIfNullOrEmpty(receivedBy, "receivedBy");
            if (!isValidReceivedBy(receivedBy)) {
                throw System::FormatException("The receivedBy value is not valid: " + receivedBy);
            }
        }

        void checkValidToken(const std::string& value, const std::string& paramName) {
            System::ArgumentException::ThrowIfNullOrEmpty(value, paramName);
            if (!isToken(value)) {
                throw System::FormatException("The value is not a valid HTTP token: " + value);
            }
        }
    }

    ViaHeaderValue::ViaHeaderValue(const std::string& protocolVersion, const std::string& receivedBy)
        : ViaHeaderValue(protocolVersion, receivedBy, "", "") {}

    ViaHeaderValue::ViaHeaderValue(const std::string& protocolVersion, const std::string& receivedBy, const std::string& protocolName)
        : ViaHeaderValue(protocolVersion, receivedBy, protocolName, "") {}

    ViaHeaderValue::ViaHeaderValue(const std::string& protocolVersion, const std::string& receivedBy,
                                    const std::string& protocolName, const std::string& comment) {
        checkValidToken(protocolVersion, "protocolVersion");
        checkReceivedBy(receivedBy);

        if (!protocolName.empty()) {
            checkValidToken(protocolName, "protocolName");
            protocolName_ = protocolName;
        }
        if (!comment.empty()) {
            if (!isValidComment(comment)) {
                throw System::FormatException("The comment is not a valid RFC 2616 comment: " + comment);
            }
            comment_ = comment;
        }

        protocolVersion_ = protocolVersion;
        receivedBy_ = receivedBy;
    }

    std::string ViaHeaderValue::ToString() const {
        std::string result;
        if (!protocolName_.empty()) {
            result += protocolName_ + "/";
        }
        result += protocolVersion_ + " " + receivedBy_;
        if (!comment_.empty()) {
            result += " " + comment_;
        }
        return result;
    }

    bool ViaHeaderValue::Equals(const ViaHeaderValue& other) const {
        return equalsIgnoreCase(protocolVersion_, other.protocolVersion_) &&
               equalsIgnoreCase(receivedBy_, other.receivedBy_) &&
               equalsIgnoreCase(protocolName_, other.protocolName_) &&
               comment_ == other.comment_;
    }

    SharpRuntime::intcs ViaHeaderValue::GetHashCode() const {
        SharpRuntime::intcs result = hashIgnoreCase(protocolVersion_) ^ hashIgnoreCase(receivedBy_);
        if (!protocolName_.empty()) result ^= hashIgnoreCase(protocolName_);
        if (!comment_.empty()) result ^= static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(comment_));
        return result;
    }

    bool ViaHeaderValue::TryParse(const std::string& input, ViaHeaderValue& parsedValue) {
        std::string trimmed = trim(input);
        if (trimmed.empty()) return false;

        size_t pos = 0;
        size_t ws1 = trimmed.find_first_of(" \t", pos);
        if (ws1 == std::string::npos) return false;
        std::string firstToken = trimmed.substr(0, ws1);

        std::string protocolName, protocolVersion;
        size_t afterFirst = trimmed.find_first_not_of(" \t", ws1);
        if (afterFirst == std::string::npos) return false;

        if (!firstToken.empty() && firstToken.find('/') != std::string::npos) {
            size_t slash = firstToken.find('/');
            protocolName = firstToken.substr(0, slash);
            protocolVersion = firstToken.substr(slash + 1);
            if (protocolName.empty() || protocolVersion.empty() || !isToken(protocolName) || !isToken(protocolVersion)) return false;
            pos = afterFirst;
        } else {
            protocolVersion = firstToken;
            if (!isToken(protocolVersion)) return false;
            pos = afterFirst;
        }

        size_t ws2 = trimmed.find_first_of(" \t", pos);
        std::string receivedBy = (ws2 == std::string::npos) ? trimmed.substr(pos) : trimmed.substr(pos, ws2 - pos);
        if (!isValidReceivedBy(receivedBy)) return false;

        std::string comment;
        if (ws2 != std::string::npos) {
            size_t commentStart = trimmed.find_first_not_of(" \t", ws2);
            if (commentStart != std::string::npos) {
                comment = trim(trimmed.substr(commentStart));
                if (!isValidComment(comment)) return false;
            }
        }

        try {
            parsedValue = ViaHeaderValue(protocolVersion, receivedBy, protocolName, comment);
        } catch (...) {
            return false;
        }
        return true;
    }

    ViaHeaderValue ViaHeaderValue::Parse(const std::string& input) {
        ViaHeaderValue result;
        if (!TryParse(input, result)) {
            throw System::FormatException("The Via header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
