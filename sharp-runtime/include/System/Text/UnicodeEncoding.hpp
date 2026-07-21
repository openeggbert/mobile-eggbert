// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "System/Text/Encoding.hpp"

namespace System::Text {

    /**
     * @brief Encoding that uses UTF-16 (matches .NET's UnicodeEncoding — little-endian by
     * default, or big-endian).
     *
     * This runtime represents `System::String`/`std::string` as UTF-8 internally, so GetBytes()
     * decodes the source UTF-8 sequence into Unicode scalar values and re-encodes each as one or
     * two UTF-16 code units (surrogate pairs for U+10000 and above); GetString() reverses this.
     * Unlike an earlier, simpler version of this file, this handles the full Unicode range, not
     * just ASCII.
     *
     * C++ counterpart of .NET System.Text.UnicodeEncoding.
     */
    class UnicodeEncoding : public Encoding {
        bool bigEndian_;
        bool byteOrderMark_;

    public:
        /** Constructs a UTF-16 LE encoding with no byte-order mark. */
        UnicodeEncoding() : bigEndian_(false), byteOrderMark_(false) {}
        /** Constructs a UTF-16 encoding with the specified endianness and BOM setting. */
        UnicodeEncoding(bool bigEndian, bool byteOrderMark) : bigEndian_(bigEndian), byteOrderMark_(byteOrderMark) {}

        /** Returns the encoding name ("utf-16" or "utf-16BE"). */
        [[nodiscard]] std::string getEncodingNameProperty() const override { return bigEndian_ ? "utf-16BE" : "utf-16"; }
        /** Returns the code page (1201 for big-endian, 1200 for little-endian). */
        [[nodiscard]] int getCodePageProperty() const override { return bigEndian_ ? 1201 : 1200; }

        /** @return true if this instance encodes as big-endian UTF-16. */
        [[nodiscard]] bool getIsBigEndianProperty() const { return bigEndian_; }
        /** @return true if GetBytes() prepends a byte-order mark. */
        [[nodiscard]] bool getByteOrderMarkProperty() const { return byteOrderMark_; }

        /**
         * @brief Encodes a string (decoded from UTF-8) to UTF-16 bytes, in the configured endianness.
         * @note Pre-sizes the output buffer to its exact worst case (2 output bytes per input
         * UTF-8 byte -- every decode path here emits at most 2 output bytes per byte consumed,
         * so `s.size() * 2 + 2` is a provable upper bound) and writes through indexing rather
         * than `reserve()` + `push_back()`. GCC 14's `-Wfree-nonheap-object`/`-Warray-bounds`
         * (both fatal under `-Werror` in a Release build) misanalyze the inlined
         * reserve-then-push_back reallocation path here as a real bug even though the
         * reallocation branch is unreachable in practice; indexed writes into a pre-sized
         * buffer sidestep that code shape entirely.
         */
        [[nodiscard]] std::vector<SharpRuntime::bytecs> GetBytes(const std::string& s) const override {
            std::vector<SharpRuntime::bytecs> out(s.size() * 2 + 2);
            size_t pos = 0;
            auto writeUnit = [&](uint16_t unit) {
                if (bigEndian_) {
                    out[pos++] = static_cast<SharpRuntime::bytecs>((unit >> 8) & 0xFF);
                    out[pos++] = static_cast<SharpRuntime::bytecs>(unit & 0xFF);
                } else {
                    out[pos++] = static_cast<SharpRuntime::bytecs>(unit & 0xFF);
                    out[pos++] = static_cast<SharpRuntime::bytecs>((unit >> 8) & 0xFF);
                }
            };
            if (byteOrderMark_) writeUnit(0xFEFF);

            size_t i = 0;
            while (i < s.size()) {
                uint32_t cp;
                size_t len;
                decodeUtf8(s, i, cp, len);
                i += len;
                if (cp < 0x10000) {
                    writeUnit(static_cast<uint16_t>(cp));
                } else {
                    uint32_t v = cp - 0x10000;
                    writeUnit(static_cast<uint16_t>(0xD800 + (v >> 10)));
                    writeUnit(static_cast<uint16_t>(0xDC00 + (v & 0x3FF)));
                }
            }
            out.resize(pos);
            return out;
        }

        /**
         * @brief Decodes a UTF-16 byte range (in the configured endianness) to a UTF-8-encoded string.
         *
         * An unpaired surrogate (a high surrogate not followed by a matching low surrogate, or
         * a lone low surrogate) is replaced with U+FFFD, matching .NET's default
         * DecoderFallback behavior. Without this check, `cp` would previously stay set to the
         * raw surrogate value (0xD800-0xDFFF) and reach encodeUtf8() unvalidated -- real UTF-8
         * forbids encoding a surrogate value directly (that's CESU-8/WTF-8, not UTF-8), so this
         * produced ill-formed output bytes rather than a genuine encoding error.
         */
        [[nodiscard]] std::string GetString(const SharpRuntime::bytecs* data,
                                            SharpRuntime::intcs index,
                                            SharpRuntime::intcs count) const override {
            std::string out;
            SharpRuntime::intcs i = index;
            SharpRuntime::intcs end = index + count;

            if (count >= 2) {
                uint16_t bom = readUnit(data, i);
                if (bom == 0xFEFF) i += 2;
            }

            while (i + 1 < end) {
                uint16_t unit = readUnit(data, i);
                i += 2;
                uint32_t cp = unit;
                if (unit >= 0xD800 && unit <= 0xDBFF) {
                    if (i + 1 < end) {
                        uint16_t low = readUnit(data, i);
                        if (low >= 0xDC00 && low <= 0xDFFF) {
                            cp = 0x10000 + ((static_cast<uint32_t>(unit - 0xD800) << 10) | (low - 0xDC00));
                            i += 2;
                        } else {
                            cp = 0xFFFD; // unpaired high surrogate
                        }
                    } else {
                        cp = 0xFFFD; // unpaired high surrogate at end of input
                    }
                } else if (unit >= 0xDC00 && unit <= 0xDFFF) {
                    cp = 0xFFFD; // lone low surrogate
                }
                encodeUtf8(cp, out);
            }
            return out;
        }

    private:
        [[nodiscard]] uint16_t readUnit(const SharpRuntime::bytecs* data, SharpRuntime::intcs i) const {
            uint8_t b0 = data[i];
            uint8_t b1 = data[i + 1];
            return bigEndian_ ? static_cast<uint16_t>((b0 << 8) | b1) : static_cast<uint16_t>((b1 << 8) | b0);
        }

        // Validates and decodes one UTF-8 sequence starting at s[i]. Rejects ill-formed input
        // (bad continuation bytes, overlong encodings, surrogate/out-of-range code points) by
        // substituting U+FFFD for a single byte and resuming there, matching .NET's default
        // DecoderFallback replacement behavior, instead of the previous code's silent
        // acceptance of invalid sequences (verified with a compiled reproduction: an overlong
        // encoding of U+0000, "\xC0\x80", used to decode straight through to real U+0000).
        static void decodeUtf8(const std::string& s, size_t i, uint32_t& codePoint, size_t& length) {
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

        static void encodeUtf8(uint32_t cp, std::string& out) {
            if (cp < 0x80) {
                out += static_cast<char>(cp);
            } else if (cp < 0x800) {
                out += static_cast<char>(0xC0 | (cp >> 6));
                out += static_cast<char>(0x80 | (cp & 0x3F));
            } else if (cp < 0x10000) {
                out += static_cast<char>(0xE0 | (cp >> 12));
                out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                out += static_cast<char>(0x80 | (cp & 0x3F));
            } else {
                out += static_cast<char>(0xF0 | (cp >> 18));
                out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
                out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                out += static_cast<char>(0x80 | (cp & 0x3F));
            }
        }
    };

} // namespace System::Text
