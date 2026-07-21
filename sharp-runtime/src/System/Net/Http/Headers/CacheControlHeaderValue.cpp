// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Net/Http/Headers/CacheControlHeaderValue.hpp"
#include "System/FormatException.hpp"
#include "System/HashCode.hpp"
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

        std::string toLowerAscii(const std::string& s) {
            std::string out = s;
            std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return out;
        }

        bool equalsIgnoreCase(const std::string& a, const std::string& b) {
            if (a.size() != b.size()) return false;
            return std::equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) {
                return std::tolower(x) == std::tolower(y);
            });
        }

        // Splits a comma-separated list at the top level, ignoring commas inside a quoted string.
        std::vector<std::string> splitTopLevel(const std::string& input) {
            std::vector<std::string> parts;
            std::string current;
            bool inQuotes = false;
            for (size_t i = 0; i < input.size(); ++i) {
                char c = input[i];
                if (c == '"') {
                    inQuotes = !inQuotes;
                    current += c;
                } else if (c == ',' && !inQuotes) {
                    parts.push_back(current);
                    current.clear();
                } else {
                    current += c;
                }
            }
            parts.push_back(current);
            return parts;
        }

        // Verified against HeaderUtilities.cs's TryParseInt32, which delegates to
        // int.TryParse(value, NumberStyles.None, ...): NumberStyles.None requires an unsigned,
        // sign-free digit string (matching HTTP's delta-seconds grammar, 1*DIGIT) and fails
        // (returns false, no throw) on overflow rather than wrapping. This previously accepted
        // a leading '-' (via std::stol) and, on overflow, silently narrowed a `long` down to
        // `intcs` with no range check at all -- an out-of-range max-age value (e.g.
        // "99999999999") wrapped to an arbitrary, possibly negative intcs instead of failing
        // to parse.
        bool tryParseSeconds(const std::string& value, SharpRuntime::intcs& result) {
            if (value.empty() ||
                !std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c) != 0; }))
                return false;
            try {
                size_t pos = 0;
                long parsed = std::stol(value, &pos);
                if (pos != value.size()) return false;
                if (parsed > static_cast<long>(SharpRuntime::INTCS_MAX)) return false;
                result = static_cast<SharpRuntime::intcs>(parsed);
                return true;
            } catch (...) {
                return false;
            }
        }

        // Parses a quoted, comma-separated token list (e.g. "Set-Cookie, X-Foo") into individual tokens.
        bool tryParseQuotedTokenList(const std::string& value, std::vector<std::string>& out) {
            if (value.size() < 2 || value.front() != '"' || value.back() != '"') return false;
            std::string inner = value.substr(1, value.size() - 2);
            for (const auto& part : splitTopLevel(inner)) {
                std::string token = trim(part);
                if (token.empty()) continue;
                if (!isToken(token)) return false;
                out.push_back(token);
            }
            return true;
        }

        void appendWithSeparator(std::string& sb, const std::string& value) {
            if (!sb.empty()) sb += ", ";
            sb += value;
        }

        void appendTokenList(std::string& sb, const std::vector<std::string>& values) {
            for (size_t i = 0; i < values.size(); ++i) {
                if (i != 0) sb += ", ";
                sb += values[i];
            }
        }

        bool areTokenListsEqual(const std::vector<std::string>& a, const std::vector<std::string>& b) {
            if (a.size() != b.size()) return false;
            std::vector<bool> matched(b.size(), false);
            for (const auto& x : a) {
                bool found = false;
                for (size_t i = 0; i < b.size(); ++i) {
                    if (!matched[i] && equalsIgnoreCase(x, b[i])) { matched[i] = true; found = true; break; }
                }
                if (!found) return false;
            }
            return true;
        }
    }

    std::string CacheControlHeaderValue::ToString() const {
        std::string sb;

        if (noStore_) appendWithSeparator(sb, "no-store");
        if (noTransform_) appendWithSeparator(sb, "no-transform");
        if (onlyIfCached_) appendWithSeparator(sb, "only-if-cached");
        if (public_) appendWithSeparator(sb, "public");
        if (mustRevalidate_) appendWithSeparator(sb, "must-revalidate");
        if (proxyRevalidate_) appendWithSeparator(sb, "proxy-revalidate");

        if (noCache_) {
            appendWithSeparator(sb, "no-cache");
            if (!noCacheHeaders_.empty()) {
                sb += "=\"";
                appendTokenList(sb, noCacheHeaders_);
                sb += "\"";
            }
        }

        if (maxAge_.has_value()) {
            appendWithSeparator(sb, "max-age");
            sb += "=" + std::to_string(*maxAge_);
        }

        if (sharedMaxAge_.has_value()) {
            appendWithSeparator(sb, "s-maxage");
            sb += "=" + std::to_string(*sharedMaxAge_);
        }

        if (maxStale_) {
            appendWithSeparator(sb, "max-stale");
            if (maxStaleLimit_.has_value()) {
                sb += "=" + std::to_string(*maxStaleLimit_);
            }
        }

        if (minFresh_.has_value()) {
            appendWithSeparator(sb, "min-fresh");
            sb += "=" + std::to_string(*minFresh_);
        }

        if (private_) {
            appendWithSeparator(sb, "private");
            if (!privateHeaders_.empty()) {
                sb += "=\"";
                appendTokenList(sb, privateHeaders_);
                sb += "\"";
            }
        }

        for (const auto& ext : extensions_) {
            appendWithSeparator(sb, ext.ToString());
        }

        return sb;
    }

    bool CacheControlHeaderValue::Equals(const CacheControlHeaderValue& other) const {
        if (noCache_ != other.noCache_ || noStore_ != other.noStore_ || maxStale_ != other.maxStale_ ||
            noTransform_ != other.noTransform_ || onlyIfCached_ != other.onlyIfCached_ ||
            public_ != other.public_ || private_ != other.private_ ||
            mustRevalidate_ != other.mustRevalidate_ || proxyRevalidate_ != other.proxyRevalidate_) {
            return false;
        }
        if (maxAge_ != other.maxAge_ || sharedMaxAge_ != other.sharedMaxAge_ ||
            maxStaleLimit_ != other.maxStaleLimit_ || minFresh_ != other.minFresh_) {
            return false;
        }
        if (!areTokenListsEqual(noCacheHeaders_, other.noCacheHeaders_)) return false;
        if (!areTokenListsEqual(privateHeaders_, other.privateHeaders_)) return false;
        if (extensions_.size() != other.extensions_.size()) return false;

        std::vector<bool> matched(other.extensions_.size(), false);
        for (const auto& e : extensions_) {
            bool found = false;
            for (size_t i = 0; i < other.extensions_.size(); ++i) {
                if (!matched[i] && e == other.extensions_[i]) { matched[i] = true; found = true; break; }
            }
            if (!found) return false;
        }
        return true;
    }

    SharpRuntime::intcs CacheControlHeaderValue::GetHashCode() const {
        SharpRuntime::intcs result = System::HashCode::Combine(
            noCache_, noStore_, maxStale_, noTransform_, onlyIfCached_, public_, private_,
            mustRevalidate_);
        result = System::HashCode::Combine(result, proxyRevalidate_,
            maxAge_.value_or(-1), sharedMaxAge_.value_or(-1), maxStaleLimit_.value_or(-1), minFresh_.value_or(-1));
        for (const auto& h : noCacheHeaders_) result ^= static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(toLowerAscii(h)));
        for (const auto& h : privateHeaders_) result ^= static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(toLowerAscii(h)));
        for (const auto& e : extensions_) result ^= e.GetHashCode();
        return result;
    }

    bool CacheControlHeaderValue::TryParse(const std::string& input, CacheControlHeaderValue& parsedValue) {
        CacheControlHeaderValue result;

        if (trim(input).empty()) {
            parsedValue = result;
            return true;
        }

        for (const auto& rawSegment : splitTopLevel(input)) {
            std::string segment = trim(rawSegment);
            // Empty list elements (leading/trailing/consecutive commas) are ignorable, not a
            // parse error -- matches RFC 7230's #rule ABNF and real .NET's actual behavior.
            // Verified against BaseHeaderParser.TryParseValue + HeaderUtilities.
            // GetNextNonEmptyOrWhitespaceIndex (CacheControlHeaderValue.cs's
            // MultipleValueNameValueParser has supportsMultipleValues=true, which enables the
            // "skipEmptyValues" loop that silently consumes any run of ,-separated empty
            // elements, both leading, trailing, and between real values). This previously
            // rejected the whole header for e.g. "no-cache," or "no-cache,,no-store".
            if (segment.empty()) continue;

            size_t eq = segment.find('=');
            std::string name = trim(eq == std::string::npos ? segment : segment.substr(0, eq));
            std::string value = eq == std::string::npos ? "" : trim(segment.substr(eq + 1));
            bool hasValue = eq != std::string::npos;

            if (name.empty() || !isToken(name)) return false;

            std::string lowerName = toLowerAscii(name);
            SharpRuntime::intcs seconds = 0;

            if (lowerName == "no-cache") {
                result.noCache_ = true;
                if (hasValue && !tryParseQuotedTokenList(value, result.noCacheHeaders_)) return false;
            } else if (lowerName == "no-store") {
                result.noStore_ = true;
            } else if (lowerName == "max-age") {
                if (!hasValue || !tryParseSeconds(value, seconds)) return false;
                result.maxAge_ = seconds;
            } else if (lowerName == "max-stale") {
                result.maxStale_ = true;
                if (hasValue) {
                    if (!tryParseSeconds(value, seconds)) return false;
                    result.maxStaleLimit_ = seconds;
                }
            } else if (lowerName == "min-fresh") {
                if (!hasValue || !tryParseSeconds(value, seconds)) return false;
                result.minFresh_ = seconds;
            } else if (lowerName == "no-transform") {
                result.noTransform_ = true;
            } else if (lowerName == "only-if-cached") {
                result.onlyIfCached_ = true;
            } else if (lowerName == "public") {
                result.public_ = true;
            } else if (lowerName == "private") {
                result.private_ = true;
                if (hasValue && !tryParseQuotedTokenList(value, result.privateHeaders_)) return false;
            } else if (lowerName == "must-revalidate") {
                result.mustRevalidate_ = true;
            } else if (lowerName == "proxy-revalidate") {
                result.proxyRevalidate_ = true;
            } else if (lowerName == "s-maxage") {
                if (!hasValue || !tryParseSeconds(value, seconds)) return false;
                result.sharedMaxAge_ = seconds;
            } else {
                try {
                    result.extensions_.push_back(hasValue ? NameValueHeaderValue(name, value) : NameValueHeaderValue(name));
                } catch (...) {
                    return false;
                }
            }
        }

        parsedValue = result;
        return true;
    }

    CacheControlHeaderValue CacheControlHeaderValue::Parse(const std::string& input) {
        CacheControlHeaderValue result;
        if (!TryParse(input, result)) {
            throw System::FormatException("The Cache-Control header value is not valid: " + input);
        }
        return result;
    }

} // namespace System::Net::Http::Headers
