// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Text/ASCIIEncoding.hpp"

namespace System::Text {

namespace {

    // Decodes one UTF-8 sequence starting at s[i], validating continuation bytes and
    // rejecting overlong encodings -- the same conformance logic already applied this
    // session to Rune::TryGetRuneAt/UnicodeEncoding/UTF32Encoding's decode loops. An
    // ill-formed sequence decodes to U+FFFD (length 1, resuming one byte later).
    void decodeUtf8(const std::string& s, size_t i, uint32_t& codePoint, size_t& length) {
        auto isContinuation = [](unsigned char b) { return (b & 0xC0) == 0x80; };
        unsigned char c0 = static_cast<unsigned char>(s[i]);
        uint32_t cp; size_t len;
        if (c0 < 0x80) {
            cp = c0; len = 1;
        } else if ((c0 & 0xE0) == 0xC0 && i + 1 < s.size() &&
                   isContinuation(static_cast<unsigned char>(s[i + 1]))) {
            cp = (static_cast<uint32_t>(c0 & 0x1F) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F);
            len = 2;
            if (cp < 0x80) { codePoint = 0xFFFD; length = 1; return; }
        } else if ((c0 & 0xF0) == 0xE0 && i + 2 < s.size() &&
                   isContinuation(static_cast<unsigned char>(s[i + 1])) &&
                   isContinuation(static_cast<unsigned char>(s[i + 2]))) {
            cp = (static_cast<uint32_t>(c0 & 0x0F) << 12) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6) |
                 (static_cast<unsigned char>(s[i + 2]) & 0x3F);
            len = 3;
            if (cp < 0x800) { codePoint = 0xFFFD; length = 1; return; }
        } else if ((c0 & 0xF8) == 0xF0 && i + 3 < s.size() &&
                   isContinuation(static_cast<unsigned char>(s[i + 1])) &&
                   isContinuation(static_cast<unsigned char>(s[i + 2])) &&
                   isContinuation(static_cast<unsigned char>(s[i + 3]))) {
            cp = (static_cast<uint32_t>(c0 & 0x07) << 18) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12) |
                 ((static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6) | (static_cast<unsigned char>(s[i + 3]) & 0x3F);
            len = 4;
            if (cp < 0x10000) { codePoint = 0xFFFD; length = 1; return; }
        } else {
            codePoint = 0xFFFD;
            length = 1;
            return;
        }
        if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) {
            codePoint = 0xFFFD;
            length = 1;
            return;
        }
        codePoint = cp;
        length = len;
    }

} // namespace

    /**
     * Encodes @p str (this runtime's UTF-8 representation) to ASCII bytes, one output byte
     * per Unicode UTF-16 code unit the string would have in real .NET -- not one per raw
     * UTF-8 byte. The previous implementation iterated the UTF-8-encoded input byte-wise, so
     * a single multi-byte non-ASCII character (e.g. 'é', 2 UTF-8 bytes) produced 2-4 '?'
     * replacement bytes instead of .NET's 1 (real ASCIIEncoding operates on `char[]`/UTF-16
     * code units, verified against ASCIIEncoding.cs). A BMP code point (including ASCII)
     * maps to exactly one output byte; a supplementary-plane code point (>= U+10000) maps to
     * two, matching the two UTF-16 surrogate halves .NET would encode it as (neither half is
     * ASCII-representable, so both are always '?').
     */
    std::vector<SharpRuntime::bytecs> ASCIIEncoding::GetBytes(const std::string& str) const {
        std::vector<SharpRuntime::bytecs> result;
        result.reserve(str.size());
        size_t i = 0;
        while (i < str.size()) {
            uint32_t cp;
            size_t len;
            decodeUtf8(str, i, cp, len);
            i += len;
            if (cp <= 127) {
                result.push_back(static_cast<SharpRuntime::bytecs>(cp));
            } else if (cp < 0x10000) {
                result.push_back('?');
            } else {
                result.push_back('?');
                result.push_back('?');
            }
        }
        return result;
    }

    std::string ASCIIEncoding::GetString(const SharpRuntime::bytecs* data,
                                         SharpRuntime::intcs index,
                                         SharpRuntime::intcs count) const {
        if (data == nullptr || count <= 0) return {};
        std::string result;
        result.reserve(static_cast<size_t>(count));
        for (SharpRuntime::intcs i = 0; i < count; ++i) {
            auto b = data[index + i];
            result.push_back(b <= 127 ? static_cast<char>(b) : '?');
        }
        return result;
    }

} // namespace System::Text
