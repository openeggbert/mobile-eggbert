// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cctype>
#include <cstring>
#include <stdexcept>
#include <string>
#include "System/FormatException.hpp"
#include "System/ArgumentException.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Net::Http {

/**
 * @brief Represents an HTTP method (verb).
 *
 * C++ counterpart of .NET System.Net.Http.HttpMethod. Equality is ordinal
 * case-insensitive, matching .NET.
 */
class HttpMethod {
    std::string method_;

    static bool isTokenChar(unsigned char c) {
        static constexpr const char* extras = "!#$%&'*+-.^_`|~";
        return c != 0 && (std::isalnum(c) || std::strchr(extras, static_cast<char>(c)) != nullptr);
    }

public:
    /**
     * @brief Constructs an HttpMethod with the given verb string.
     * @param method HTTP verb string (e.g. "GET", "POST").
     * @throws System::ArgumentException if @p method is empty or all whitespace.
     * @throws System::FormatException if @p method contains characters not valid in an HTTP token.
     */
    explicit HttpMethod(const std::string& method) : method_(method) {
        System::ArgumentException::ThrowIfNullOrWhiteSpace(method, "method");
        if (!std::all_of(method.begin(), method.end(), [](char c) { return isTokenChar(static_cast<unsigned char>(c)); })) {
            throw System::FormatException("The format of the HTTP method is invalid.");
        }
    }

    /** Returns the HTTP method string (e.g. "GET"). */
    [[nodiscard]] const std::string& getMethodProperty() const { return method_; }
    /** Returns the HTTP method string. */
    [[nodiscard]] std::string ToString() const { return method_; }

    /** Case-insensitive equality, matching .NET's OrdinalIgnoreCase comparison. */
    [[nodiscard]] bool Equals(const HttpMethod& other) const {
        if (method_.size() != other.method_.size()) return false;
        return std::equal(method_.begin(), method_.end(), other.method_.begin(), [](unsigned char a, unsigned char b) {
            return std::tolower(a) == std::tolower(b);
        });
    }
    /** Returns true if both HttpMethod instances represent the same verb (case-insensitive). */
    bool operator==(const HttpMethod& o) const { return Equals(o); }
    /** Returns true if the two HttpMethod instances represent different verbs. */
    bool operator!=(const HttpMethod& o) const { return !Equals(o); }

    /** @return A case-insensitive hash code for this method. */
    [[nodiscard]] SharpRuntime::intcs GetHashCode() const {
        std::string lower = method_;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(lower));
    }

    static const HttpMethod& Get()     { static HttpMethod m("GET");     return m; } ///< The HTTP GET method.
    static const HttpMethod& Put()     { static HttpMethod m("PUT");     return m; } ///< The HTTP PUT method.
    static const HttpMethod& Post()    { static HttpMethod m("POST");    return m; } ///< The HTTP POST method.
    static const HttpMethod& Delete_() { static HttpMethod m("DELETE");  return m; } ///< The HTTP DELETE method.
    static const HttpMethod& Head()    { static HttpMethod m("HEAD");    return m; } ///< The HTTP HEAD method.
    static const HttpMethod& Options() { static HttpMethod m("OPTIONS"); return m; } ///< The HTTP OPTIONS method.
    static const HttpMethod& Trace()   { static HttpMethod m("TRACE");   return m; } ///< The HTTP TRACE method.
    static const HttpMethod& Patch()   { static HttpMethod m("PATCH");   return m; } ///< The HTTP PATCH method.
    static const HttpMethod& Query()   { static HttpMethod m("QUERY");   return m; } ///< The HTTP QUERY method.
    static const HttpMethod& Connect() { static HttpMethod m("CONNECT"); return m; } ///< The HTTP CONNECT method.

    /**
     * @brief Parses @p method into an HttpMethod instance.
     *
     * C++ counterpart of .NET HttpMethod.Parse(ReadOnlySpan&lt;char&gt;). Returns one of the
     * known-verb singletons (Get()/Put()/Post()/etc.) for a case-insensitive match against a
     * known verb name; otherwise constructs (and validates, per the constructor's rules) a new
     * HttpMethod for the given string.
     * @throws System::ArgumentException if @p method is empty or all whitespace.
     * @throws System::FormatException if @p method contains characters not valid in an HTTP token.
     */
    [[nodiscard]] static HttpMethod Parse(const std::string& method) {
        static const HttpMethod* const known[] = {
            &Get(), &Put(), &Post(), &Delete_(), &Head(), &Options(), &Trace(), &Patch(), &Query(), &Connect(),
        };
        for (const HttpMethod* candidate : known) {
            if (method.size() == candidate->method_.size() &&
                std::equal(method.begin(), method.end(), candidate->method_.begin(), [](unsigned char a, unsigned char b) {
                    return std::tolower(a) == std::tolower(b);
                })) {
                return *candidate;
            }
        }
        return HttpMethod(method);
    }
};

} // namespace System::Net::Http
