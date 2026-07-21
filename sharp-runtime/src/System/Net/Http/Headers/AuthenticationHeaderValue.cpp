// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/AuthenticationHeaderValue.hpp"
#include "System/ArgumentException.hpp"
#include "System/FormatException.hpp"
#include "System/HashCode.hpp"
#include <algorithm>
#include <cctype>
#include <string_view>

namespace System::Net::Http::Headers {

    namespace {
        // Note: uses std::string_view::find rather than strchr, since strchr(s, '\0') matches a
        // string's own NUL terminator and would incorrectly accept an embedded NUL byte.
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

        SharpRuntime::intcs hashIgnoreCase(const std::string& s) {
            std::string lower = s;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(lower));
        }

        bool containsNewLineOrNull(const std::string& s) {
            return s.find('\r') != std::string::npos || s.find('\n') != std::string::npos || s.find('\0') != std::string::npos;
        }
    }

    AuthenticationHeaderValue::AuthenticationHeaderValue(const std::string& scheme) : AuthenticationHeaderValue(scheme, "") {}

    AuthenticationHeaderValue::AuthenticationHeaderValue(const std::string& scheme, const std::string& parameter)
        : scheme_(scheme), parameter_(parameter) {
        System::ArgumentException::ThrowIfNullOrEmpty(scheme, "scheme");
        if (!isToken(scheme)) {
            throw System::FormatException("The scheme is not a valid HTTP token: " + scheme);
        }
        if (containsNewLineOrNull(parameter)) {
            throw System::FormatException("The parameter contains invalid CR, LF, or NUL characters.");
        }
    }

    std::string AuthenticationHeaderValue::ToString() const {
        if (parameter_.empty()) return scheme_;
        return scheme_ + " " + parameter_;
    }

    bool AuthenticationHeaderValue::Equals(const AuthenticationHeaderValue& other) const {
        if (!equalsIgnoreCase(scheme_, other.scheme_)) return false;
        if (parameter_.empty() && other.parameter_.empty()) return true;
        return parameter_ == other.parameter_;
    }

    SharpRuntime::intcs AuthenticationHeaderValue::GetHashCode() const {
        SharpRuntime::intcs result = hashIgnoreCase(scheme_);
        if (!parameter_.empty()) {
            result ^= static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(parameter_));
        }
        return result;
    }

    bool AuthenticationHeaderValue::TryParse(const std::string& input, AuthenticationHeaderValue& parsedValue) {
        if (containsNewLineOrNull(input)) return false;

        size_t start = input.find_first_not_of(" \t");
        if (start == std::string::npos) return false;
        size_t end = input.find_last_not_of(" \t");
        std::string trimmed = input.substr(start, end - start + 1);

        size_t ws = trimmed.find_first_of(" \t");
        std::string scheme = (ws == std::string::npos) ? trimmed : trimmed.substr(0, ws);
        if (!isToken(scheme)) return false;

        if (ws == std::string::npos) {
            parsedValue = AuthenticationHeaderValue(scheme);
            return true;
        }

        size_t paramStart = trimmed.find_first_not_of(" \t", ws);
        if (paramStart == std::string::npos) {
            parsedValue = AuthenticationHeaderValue(scheme);
            return true;
        }

        std::string parameter = trimmed.substr(paramStart);
        parsedValue = AuthenticationHeaderValue(scheme, parameter);
        return true;
    }

    AuthenticationHeaderValue AuthenticationHeaderValue::Parse(const std::string& input) {
        AuthenticationHeaderValue result("x");
        if (!TryParse(input, result)) {
            throw System::FormatException("The authentication header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
