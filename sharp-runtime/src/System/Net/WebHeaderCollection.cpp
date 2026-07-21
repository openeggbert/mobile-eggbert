// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/WebHeaderCollection.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/InvalidOperationException.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <unordered_set>

namespace System::Net {

    namespace {
        bool isInvalidMethodOrHeaderChar(char c) {
            switch (c) {
                case '(': case ')': case '<': case '>': case '@': case ',': case ';':
                case ':': case '\\': case '"': case '\'': case '/': case '[': case ']':
                case '?': case '=': case '{': case '}': case ' ': case '\t': case '\r': case '\n':
                    return true;
                default:
                    return false;
            }
        }

        bool containsNonAsciiChars(const std::string& s) {
            for (unsigned char c : s) {
                if (c < 0x20 || c > 0x7E) return true;
            }
            return false;
        }

        bool equalsIgnoreCase(const std::string& a, const std::string& b) {
            if (a.size() != b.size()) return false;
            return std::equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) {
                return std::tolower(x) == std::tolower(y);
            });
        }

        const std::unordered_set<std::string>& requestRestrictedHeaders() {
            static const std::unordered_set<std::string> s = {
                "Accept", "Connection", "Content-Type", "Content-Length", "Date", "Expect",
                "Host", "If-Modified-Since", "Proxy-Connection", "Range", "Referer",
                "Transfer-Encoding", "User-Agent",
            };
            return s;
        }

        const std::unordered_set<std::string>& responseRestrictedHeaders() {
            static const std::unordered_set<std::string> s = {
                "Content-Length", "Keep-Alive", "Transfer-Encoding", "WWW-Authenticate",
            };
            return s;
        }

        bool containsIgnoreCase(const std::unordered_set<std::string>& set, const std::string& name) {
            for (const auto& item : set) {
                if (equalsIgnoreCase(item, name)) return true;
            }
            return false;
        }
    }

    std::string WebHeaderCollection::CheckBadHeaderNameChars(const std::string& name) {
        for (char c : name) {
            if (isInvalidMethodOrHeaderChar(c)) {
                throw System::ArgumentException("Specified value has invalid HTTP header characters: " + name, "name");
            }
        }
        if (containsNonAsciiChars(name)) {
            throw System::ArgumentException("Specified value has invalid HTTP header characters: " + name, "name");
        }
        return name;
    }

    std::string WebHeaderCollection::CheckBadHeaderValueChars(const std::string& value) {
        if (value.empty()) return value;

        static const std::string trimChars = "\x09\x0A\x0B\x0C\x0D\x20";
        size_t begin = value.find_first_not_of(trimChars);
        if (begin == std::string::npos) return "";
        size_t end = value.find_last_not_of(trimChars);
        std::string trimmed = value.substr(begin, end - begin + 1);

        int crlf = 0;
        for (char raw : trimmed) {
            unsigned char c = static_cast<unsigned char>(raw);
            switch (crlf) {
                case 0:
                    if (c == '\r') crlf = 1;
                    else if (c == '\n') crlf = 2;
                    else if (c == 127 || (c < ' ' && c != '\t'))
                        throw System::ArgumentException("Specified value has invalid Control characters.", "value");
                    break;
                case 1:
                    if (c == '\n') { crlf = 2; break; }
                    throw System::ArgumentException("Specified value has invalid CRLF characters.", "value");
                case 2:
                    if (c == ' ' || c == '\t') { crlf = 0; break; }
                    throw System::ArgumentException("Specified value has invalid Control characters.", "value");
            }
        }
        if (crlf != 0) {
            throw System::ArgumentException("Specified value has invalid CRLF characters.", "value");
        }
        return trimmed;
    }

    bool WebHeaderCollection::allowHttpRequestHeader() const {
        if (type_ == CollectionType::Unknown) type_ = CollectionType::WebRequestKind;
        return type_ == CollectionType::WebRequestKind;
    }

    bool WebHeaderCollection::allowHttpResponseHeader() const {
        if (type_ == CollectionType::Unknown) type_ = CollectionType::WebResponseKind;
        return type_ == CollectionType::WebResponseKind;
    }

    void WebHeaderCollection::Add(const std::string& name, const std::string& value) {
        if (name.empty()) throw System::ArgumentException("The value cannot be an empty string.", "name");
        std::string checkedName = CheckBadHeaderNameChars(name);
        std::string checkedValue = CheckBadHeaderValueChars(value);
        inner_.Add(checkedName, checkedValue);
    }

    void WebHeaderCollection::Add(const std::string& combinedHeader) {
        if (combinedHeader.empty()) throw System::ArgumentNullException("header");
        size_t colonPos = combinedHeader.find(':');
        if (colonPos == std::string::npos) {
            throw System::ArgumentException("Header must be in \"name: value\" form.", "header");
        }
        Add(combinedHeader.substr(0, colonPos), combinedHeader.substr(colonPos + 1));
    }

    void WebHeaderCollection::Add(HttpRequestHeader header, const std::string& value) {
        if (!allowHttpRequestHeader()) {
            throw System::InvalidOperationException("This collection holds response headers.");
        }
        Add(HttpRequestHeaderGetName(header), value);
    }

    void WebHeaderCollection::Add(HttpResponseHeader header, const std::string& value) {
        if (!allowHttpResponseHeader()) {
            throw System::InvalidOperationException("This collection holds request headers.");
        }
        Add(HttpResponseHeaderGetName(header), value);
    }

    void WebHeaderCollection::Set(const std::string& name, const std::string& value) {
        if (name.empty()) throw System::ArgumentNullException("name");
        std::string checkedName = CheckBadHeaderNameChars(name);
        std::string checkedValue = CheckBadHeaderValueChars(value);
        inner_.Set(checkedName, checkedValue);
    }

    void WebHeaderCollection::Set(HttpRequestHeader header, const std::string& value) {
        if (!allowHttpRequestHeader()) {
            throw System::InvalidOperationException("This collection holds response headers.");
        }
        Set(HttpRequestHeaderGetName(header), value);
    }

    void WebHeaderCollection::Set(HttpResponseHeader header, const std::string& value) {
        if (!allowHttpResponseHeader()) {
            throw System::InvalidOperationException("This collection holds request headers.");
        }
        Set(HttpResponseHeaderGetName(header), value);
    }

    void WebHeaderCollection::Remove(const std::string& name) {
        if (name.empty()) throw System::ArgumentNullException("name");
        CheckBadHeaderNameChars(name);
        inner_.Remove(name);
    }

    void WebHeaderCollection::Remove(HttpRequestHeader header) {
        if (!allowHttpRequestHeader()) {
            throw System::InvalidOperationException("This collection holds response headers.");
        }
        Remove(HttpRequestHeaderGetName(header));
    }

    void WebHeaderCollection::Remove(HttpResponseHeader header) {
        if (!allowHttpResponseHeader()) {
            throw System::InvalidOperationException("This collection holds request headers.");
        }
        Remove(HttpResponseHeaderGetName(header));
    }

    std::string WebHeaderCollection::Get(HttpRequestHeader header) const {
        if (!allowHttpRequestHeader()) {
            throw System::InvalidOperationException("This collection holds response headers.");
        }
        return inner_.Get(HttpRequestHeaderGetName(header));
    }

    std::string WebHeaderCollection::Get(HttpResponseHeader header) const {
        if (!allowHttpResponseHeader()) {
            throw System::InvalidOperationException("This collection holds request headers.");
        }
        return inner_.Get(HttpResponseHeaderGetName(header));
    }

    std::string WebHeaderCollection::ToString() const {
        if (inner_.getCountProperty() == 0) return "\r\n";

        std::string result;
        for (const auto& key : inner_.AllKeys()) {
            result += key;
            result += ": ";
            result += inner_.Get(key);
            result += "\r\n";
        }
        result += "\r\n";
        return result;
    }

    std::vector<bytecs> WebHeaderCollection::ToByteArray() const {
        std::string s = ToString();
        return std::vector<bytecs>(s.begin(), s.end());
    }

    bool WebHeaderCollection::IsRestricted(const std::string& headerName, bool response) {
        CheckBadHeaderNameChars(headerName);
        return response ? containsIgnoreCase(responseRestrictedHeaders(), headerName)
                         : containsIgnoreCase(requestRestrictedHeaders(), headerName);
    }

} // namespace System::Net
