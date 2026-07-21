// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

namespace System::Net {

    /**
     * Provides static methods for encoding and decoding URLs and HTML strings.
     *
     * C++ counterpart of .NET System.Net.WebUtility.
     *
     * @note HtmlEncode here only encodes the 5 ASCII special characters
     * ('&', '<', '>', '"', '\''); .NET additionally numeric-entity-encodes the
     * Latin-1 supplement range (U+00A0-U+00FF) and non-BMP characters, which would
     * require decoding UTF-8 into code points — not done here since std::string is
     * treated as an opaque byte sequence elsewhere in this class (UrlEncode/UrlDecode
     * already work correctly on UTF-8 bytes without needing to decode code points).
     * @note HtmlDecode supports the 5 basic named entities plus numeric character
     * references (&amp;#NNN; and &amp;#xHH;) and a handful of common named entities
     * (nbsp, copy, reg, apos, trade) — not .NET's full ~250-entry HTML5 named-entity
     * table.
     * @note TextWriter-based overloads and the byte[]-based UrlEncodeToBytes/
     * UrlDecodeToBytes overloads are not ported (no TextWriter/byte[] idiom used
     * elsewhere in this class).
     */
    class WebUtility {
        static void appendUtf8(std::string& out, unsigned int codepoint) {
            if (codepoint <= 0x7F) {
                out += static_cast<char>(codepoint);
            } else if (codepoint <= 0x7FF) {
                out += static_cast<char>(0xC0 | (codepoint >> 6));
                out += static_cast<char>(0x80 | (codepoint & 0x3F));
            } else if (codepoint <= 0xFFFF) {
                out += static_cast<char>(0xE0 | (codepoint >> 12));
                out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                out += static_cast<char>(0x80 | (codepoint & 0x3F));
            } else {
                out += static_cast<char>(0xF0 | (codepoint >> 18));
                out += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
                out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                out += static_cast<char>(0x80 | (codepoint & 0x3F));
            }
        }

    public:
        WebUtility() = delete;

        /**
         * Encodes special HTML characters in @p value (e.g. '&' becomes "&amp;").
         * @return The HTML-encoded string.
         */
        static std::string HtmlEncode(const std::string& value) {
            std::string out;
            out.reserve(value.size());
            for (unsigned char c : value) {
                switch (c) {
                    case '&':  out += "&amp;";  break;
                    case '<':  out += "&lt;";   break;
                    case '>':  out += "&gt;";   break;
                    case '"':  out += "&quot;"; break;
                    case '\'': out += "&#39;";  break;
                    default:   out += c;        break;
                }
            }
            return out;
        }

        /**
         * Decodes HTML entities in @p value back to their character equivalents.
         * @return The HTML-decoded string.
         */
        static std::string HtmlDecode(const std::string& value) {
            std::string out;
            out.reserve(value.size());
            std::size_t i = 0;
            while (i < value.size()) {
                if (value[i] != '&') { out += value[i++]; continue; }
                std::size_t semi = value.find(';', i);
                if (semi == std::string::npos) { out += value[i++]; continue; }
                std::string entity = value.substr(i + 1, semi - i - 1);

                if (!entity.empty() && entity[0] == '#') {
                    std::string numPart = entity.substr(1);
                    bool isHex = !numPart.empty() && (numPart[0] == 'x' || numPart[0] == 'X');
                    if (isHex) numPart = numPart.substr(1);
                    bool validDigits = !numPart.empty() && std::all_of(numPart.begin(), numPart.end(), [&](char c) {
                        return isHex ? static_cast<bool>(std::isxdigit(static_cast<unsigned char>(c)))
                                     : static_cast<bool>(std::isdigit(static_cast<unsigned char>(c)));
                    });
                    if (validDigits) {
                        appendUtf8(out, static_cast<unsigned int>(std::stoul(numPart, nullptr, isHex ? 16 : 10)));
                        i = semi + 1;
                        continue;
                    }
                    out += value.substr(i, semi - i + 1);
                    i = semi + 1;
                    continue;
                }

                if      (entity == "amp")  out += '&';
                else if (entity == "lt")   out += '<';
                else if (entity == "gt")   out += '>';
                else if (entity == "quot") out += '"';
                else if (entity == "apos") out += '\'';
                else if (entity == "nbsp") appendUtf8(out, 0xA0);
                else if (entity == "copy") appendUtf8(out, 0xA9);
                else if (entity == "reg")  appendUtf8(out, 0xAE);
                else if (entity == "trade") appendUtf8(out, 0x2122);
                else { out += value.substr(i, semi - i + 1); }
                i = semi + 1;
            }
            return out;
        }

        /**
         * Percent-encodes @p value for use in a URL (spaces become '+').
         * @return The URL-encoded string.
         *
         * @note Verified against WebUtility.cs's s_safeUrlChars ("Url safe chars as defined by
         * RFC 1738.4, minus '+'"): the safe set is letters, digits, `-_.!*()` -- notably
         * including `!*()` and excluding `~`. This previously used `-_.~`, which both
         * percent-encoded `!*()` that real .NET leaves literal and left `~` literal where real
         * .NET percent-encodes it -- silently different output for any URL containing those
         * characters.
         */
        static std::string UrlEncode(const std::string& value) {
            auto isSafe = [](unsigned char c) {
                return std::isalnum(c) || c == '-' || c == '_' || c == '.' ||
                       c == '!' || c == '*' || c == '(' || c == ')';
            };
            std::ostringstream oss;
            for (unsigned char c : value) {
                if (isSafe(c))
                    oss << c;
                else if (c == ' ')
                    oss << '+';
                else
                    oss << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)c;
            }
            return oss.str();
        }

        /**
         * Decodes a percent-encoded URL string ('+' becomes space).
         * @return The URL-decoded string.
         *
         * @note Verified against WebUtility.cs's UrlDecodeInternal: a malformed percent
         * sequence (either of the two characters after '%' isn't a valid hex digit, or fewer
         * than two characters remain) is never an error — real .NET's HexConverter.FromChar-
         * based check simply leaves '%' as a literal character and continues, decoding nothing.
         * The previous implementation used std::stoi(..., 16) to parse the two hex characters,
         * which throws std::invalid_argument (an uncatchable-by-System::-exception-handlers,
         * uncaught type) when neither of the two characters is a valid hex digit, e.g. on
         * "100%complete".
         */
        static std::string UrlDecode(const std::string& value) {
            auto hexVal = [](char c) -> int {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return -1;
            };
            std::string out;
            for (std::size_t i = 0; i < value.size(); ++i) {
                if (value[i] == '+') {
                    out += ' ';
                } else if (value[i] == '%' && i + 2 < value.size()) {
                    int h1 = hexVal(value[i + 1]);
                    int h2 = hexVal(value[i + 2]);
                    if (h1 >= 0 && h2 >= 0) {
                        out += static_cast<char>((h1 << 4) | h2);
                        i += 2;
                    } else {
                        out += value[i]; // malformed sequence: '%' passes through literally
                    }
                } else {
                    out += value[i];
                }
            }
            return out;
        }
    };

} // namespace System::Net
