// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Mime/ContentType.hpp"
#include "System/ArgumentException.hpp"
#include "System/FormatException.hpp"
#include <algorithm>
#include <string_view>

namespace System::Net::Mime {

    namespace {
        constexpr std::string_view tspecials = "()<>@,;:\\\"/[]?=";

        bool isTokenChar(unsigned char c) {
            return c > 32 && c < 127 && tspecials.find(static_cast<char>(c)) == std::string_view::npos;
        }

        std::string readToken(const std::string& s, size_t& pos) {
            size_t start = pos;
            while (pos < s.size() && isTokenChar(static_cast<unsigned char>(s[pos]))) ++pos;
            return s.substr(start, pos - start);
        }

        // Skips plain whitespace (a practical simplification of RFC 2045's full CFWS/comment
        // grammar — nested "(...)" comments are not supported, matching this codebase's other
        // practical-rather-than-exhaustive header parsers).
        bool skipWhitespace(const std::string& s, size_t& pos) {
            while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t')) ++pos;
            return pos < s.size();
        }

        bool tryReadQuotedString(const std::string& s, size_t& pos, std::string& out) {
            if (pos >= s.size() || s[pos] != '"') return false;
            ++pos;
            std::string result;
            while (pos < s.size() && s[pos] != '"') {
                if (s[pos] == '\\' && pos + 1 < s.size()) ++pos;
                result += s[pos];
                ++pos;
            }
            if (pos >= s.size()) return false;
            ++pos; // closing quote
            out = result;
            return true;
        }

        bool needsQuoting(const std::string& value) {
            if (value.empty()) return true;
            return !std::all_of(value.begin(), value.end(), [](char c) { return isTokenChar(static_cast<unsigned char>(c)); });
        }

        void appendEncoded(std::string& out, const std::string& value) {
            if (!needsQuoting(value)) {
                out += value;
                return;
            }
            out += '"';
            for (char c : value) {
                if (c == '"' || c == '\\') out += '\\';
                out += c;
            }
            out += '"';
        }
    }

    ContentType::ContentType() : ContentType("application/octet-stream") {}

    ContentType::ContentType(const std::string& contentType) {
        System::ArgumentException::ThrowIfNullOrEmpty(contentType, "contentType");
        parseValue(contentType);
    }

    void ContentType::parseValue(const std::string& contentType) {
        size_t pos = 0;
        std::string media = readToken(contentType, pos);
        if (media.empty() || pos >= contentType.size() || contentType[pos] != '/') {
            throw System::FormatException("The specified content type is invalid.");
        }
        ++pos;
        std::string sub = readToken(contentType, pos);
        if (sub.empty()) {
            throw System::FormatException("The specified content type is invalid.");
        }

        System::Collections::Specialized::StringDictionary parsedParams;
        while (skipWhitespace(contentType, pos)) {
            if (contentType[pos] != ';') {
                throw System::FormatException("The specified content type is invalid.");
            }
            ++pos;
            if (!skipWhitespace(contentType, pos)) break;

            std::string name = readToken(contentType, pos);
            if (name.empty() || pos >= contentType.size() || contentType[pos] != '=') {
                throw System::FormatException("The specified content type is invalid.");
            }
            ++pos;
            skipWhitespace(contentType, pos);

            std::string value;
            if (pos < contentType.size() && contentType[pos] == '"') {
                if (!tryReadQuotedString(contentType, pos, value)) {
                    throw System::FormatException("The specified content type is invalid.");
                }
            } else {
                value = readToken(contentType, pos);
                if (value.empty()) {
                    throw System::FormatException("The specified content type is invalid.");
                }
            }
            parsedParams.set(name, value);
        }

        mediaType_ = media;
        subType_ = sub;
        parameters_ = parsedParams;
    }

    void ContentType::setMediaTypeProperty(const std::string& value) {
        System::ArgumentException::ThrowIfNullOrEmpty(value, "value");
        size_t pos = 0;
        std::string media = readToken(value, pos);
        if (media.empty() || pos >= value.size() || value[pos] != '/') {
            throw System::FormatException("The specified media type is invalid.");
        }
        ++pos;
        std::string sub = readToken(value, pos);
        if (sub.empty() || pos < value.size()) {
            throw System::FormatException("The specified media type is invalid.");
        }
        mediaType_ = media;
        subType_ = sub;
    }

    void ContentType::setBoundaryProperty(const std::string& value) {
        if (value.empty()) parameters_.Remove("boundary");
        else parameters_.set("boundary", value);
    }

    void ContentType::setCharSetProperty(const std::string& value) {
        if (value.empty()) parameters_.Remove("charset");
        else parameters_.set("charset", value);
    }

    void ContentType::setNameProperty(const std::string& value) {
        if (value.empty()) parameters_.Remove("name");
        else parameters_.set("name", value);
    }

    std::string ContentType::ToString() const {
        std::string result = mediaType_ + "/" + subType_;
        for (const auto& key : parameters_.getKeysProperty()) {
            result += "; " + key + "=";
            appendEncoded(result, parameters_.GetValue(key));
        }
        return result;
    }

    bool ContentType::Equals(const ContentType& other) const {
        std::string a = ToString(), b = other.ToString();
        return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) {
            return std::tolower(x) == std::tolower(y);
        });
    }

    SharpRuntime::intcs ContentType::GetHashCode() const {
        std::string s = ToString();
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(s));
    }

} // namespace System::Net::Mime
