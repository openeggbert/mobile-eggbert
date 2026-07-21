// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/HttpHeaders.hpp"
#include "System/ArgumentException.hpp"
#include "System/FormatException.hpp"
#include "System/InvalidOperationException.hpp"
#include <algorithm>
#include <cctype>
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

        // Real .NET's System.Net.Http.Headers.HttpHeaders validates values via
        // HttpRuleParser.ContainsNewLineOrNull -- a FLAT rejection of any '\r', '\n', or '\0'
        // anywhere in the value, citing RFC 9110 section 5.5-5 ("Field values containing CR, LF,
        // or NUL characters are invalid and dangerous"). This is deliberately stricter than the
        // separate, older System.Net.WebHeaderCollection.CheckBadHeaderValueChars (ported as
        // WebHeaderCollection::CheckBadHeaderValueChars in this codebase), which still tolerates
        // a "\r\n " / "\r\n\t" obs-fold sequence for HttpWebRequest-era backward compatibility --
        // real .NET's own comment on that older helper says so explicitly ("we want to be
        // permissive in what we accept... it would be a breaking change to reject this"). An
        // earlier version of this function copied that older, more permissive state machine
        // instead of this type's own stricter reference behavior, which would have let an
        // embedded "\r\n evil: header" obs-fold sequence through Add() -- a genuine HTTP header
        // injection vector once ToString() serializes it back out verbatim.
        void checkValueChars(const std::string& value) {
            if (value.find_first_of("\r\n") != std::string::npos || value.find('\0') != std::string::npos) {
                throw System::FormatException("The value contains invalid CR, LF, or NUL characters.");
            }
        }
    }

    void HttpHeaders::Add(const std::string& name, const std::string& value) {
        System::ArgumentException::ThrowIfNullOrEmpty(name, "name");
        if (!isToken(name)) {
            throw System::FormatException("The header name is not a valid HTTP token: " + name);
        }
        checkValueChars(value);
        headers_.Add(name, value);
    }

    void HttpHeaders::Add(const std::string& name, const std::vector<std::string>& values) {
        for (const auto& v : values) Add(name, v);
    }

    bool HttpHeaders::TryAddWithoutValidation(const std::string& name, const std::string& value) {
        if (name.empty()) return false;
        headers_.Add(name, value);
        return true;
    }

    bool HttpHeaders::TryAddWithoutValidation(const std::string& name, const std::vector<std::string>& values) {
        if (name.empty()) return false;
        for (const auto& v : values) headers_.Add(name, v);
        return true;
    }

    std::vector<std::string> HttpHeaders::GetValues(const std::string& name) const {
        std::vector<std::string> values;
        if (!TryGetValues(name, values)) {
            throw System::InvalidOperationException("The header '" + name + "' was not found.");
        }
        return values;
    }

    bool HttpHeaders::TryGetValues(const std::string& name, std::vector<std::string>& values) const {
        if (!Contains(name)) return false;
        values = headers_.GetValues(name);
        return true;
    }

    bool HttpHeaders::Contains(const std::string& name) const {
        return std::any_of(headers_.AllKeys().begin(), headers_.AllKeys().end(), [&](const std::string& k) {
            return k.size() == name.size() && std::equal(k.begin(), k.end(), name.begin(), [](unsigned char a, unsigned char b) {
                return std::tolower(a) == std::tolower(b);
            });
        });
    }

    bool HttpHeaders::Remove(const std::string& name) {
        if (!Contains(name)) return false;
        headers_.Remove(name);
        return true;
    }

    void HttpHeaders::Clear() {
        headers_.Clear();
    }

    SharpRuntime::intcs HttpHeaders::getCountProperty() const {
        return headers_.getCountProperty();
    }

    const std::vector<std::string>& HttpHeaders::getHeaderNamesProperty() const {
        return headers_.AllKeys();
    }

    std::string HttpHeaders::ToString() const {
        std::string result;
        for (const auto& key : headers_.AllKeys()) {
            result += key + ": " + headers_.Get(key) + "\r\n";
        }
        result += "\r\n";
        return result;
    }

    std::string HttpHeaders::getRawValue(const std::string& name) const {
        return Contains(name) ? headers_.Get(name) : "";
    }

    void HttpHeaders::setRawValue(const std::string& name, const std::string& value) {
        if (value.empty()) {
            Remove(name);
        } else {
            headers_.Set(name, value);
        }
    }

} // namespace System::Net::Http::Headers
