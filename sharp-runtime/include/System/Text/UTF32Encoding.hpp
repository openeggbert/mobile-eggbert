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
     * @brief UTF-32 encoding; defaults to little-endian with a byte-order mark.
     *
     * This runtime represents `System::String`/`std::string` as UTF-8 internally, so GetBytes()
     * decodes the source UTF-8 sequence into Unicode scalar values and re-encodes each as one
     * 4-byte UTF-32 code unit; GetString() reverses this — the full Unicode range is supported,
     * not just ASCII.
     *
     * C++ counterpart of .NET System.Text.UTF32Encoding.
     */
    class UTF32Encoding : public Encoding {
        bool bigEndian_;
        bool byteOrderMark_;

    public:
        /** Constructs a UTF-32 LE encoding with a byte-order mark. */
        UTF32Encoding() : bigEndian_(false), byteOrderMark_(true) {}
        /** Constructs a UTF-32 encoding with the specified endianness and BOM setting. */
        UTF32Encoding(bool bigEndian, bool byteOrderMark)
            : bigEndian_(bigEndian), byteOrderMark_(byteOrderMark) {}

        /** Returns the encoding name "utf-32". */
        [[nodiscard]] std::string getEncodingNameProperty() const override { return "utf-32"; }
        /** Returns the code page (12001 for big-endian, 12000 for little-endian). */
        [[nodiscard]] int getCodePageProperty() const override { return bigEndian_ ? 12001 : 12000; }

        /** @return true if this instance encodes as big-endian UTF-32. */
        [[nodiscard]] bool getIsBigEndianProperty() const { return bigEndian_; }
        /** @return true if GetBytes() prepends a byte-order mark. */
        [[nodiscard]] bool getByteOrderMarkProperty() const { return byteOrderMark_; }

        /**
         * @brief Encodes a string (decoded from UTF-8) to a UTF-32 byte sequence.
         * @note Pre-sizes the output buffer to its exact worst case (every decoded code point
         * emits exactly 4 bytes, and there are at most `s.size()` code points, so
         * `s.size() * 4 + 4` is a provable upper bound) and writes through indexing rather than
         * `reserve()` + `push_back()`. GCC 14's `-Wfree-nonheap-object`/`-Warray-bounds` (both
         * fatal under `-Werror` in a Release build) misanalyze the inlined
         * reserve-then-push_back reallocation path here as a real bug even though the
         * reallocation branch is unreachable in practice; indexed writes into a pre-sized
         * buffer sidestep that code shape entirely.
         */
        [[nodiscard]] std::vector<SharpRuntime::bytecs> GetBytes(const std::string& s) const override {
            std::vector<SharpRuntime::bytecs> result(s.size() * 4 + 4);
            size_t pos = 0;
            if (byteOrderMark_) {
                writeUnit(result, pos, 0x0000FEFF);
            }

            size_t i = 0;
            while (i < s.size()) {
                uint32_t cp;
                size_t len;
                decodeUtf8(s, i, cp, len);
                i += len;
                writeUnit(result, pos, cp);
            }
            result.resize(pos);
            return result;
        }

        /**
         * @brief Decodes a UTF-32 byte range to a UTF-8-encoded string.
         *
         * A 4-byte unit that isn't a valid Unicode scalar value (> U+10FFFF, or a surrogate
         * value U+D800-U+DFFF) is replaced with U+FFFD, matching .NET's default
         * DecoderFallback behavior. Without this check, an arbitrary/corrupt 32-bit value
         * (e.g. 0xFFFFFFFF) would previously reach encodeUtf8() unvalidated and produce a
         * byte sequence that isn't even structurally valid UTF-8, not just the wrong code
         * point.
         */
        [[nodiscard]] std::string GetString(const SharpRuntime::bytecs* data,
                                             SharpRuntime::intcs index,
                                             SharpRuntime::intcs count) const override {
            std::string result;
            SharpRuntime::intcs start = index;
            if (count >= 4) {
                uint32_t maybeBom = readUnit(data, start);
                if (maybeBom == 0x0000FEFF) {
                    start += 4;
                    count -= 4;
                }
            }
            for (SharpRuntime::intcs i = start; i + 3 < start + count; i += 4) {
                uint32_t cp = readUnit(data, i);
                if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) cp = 0xFFFD;
                encodeUtf8(cp, result);
            }
            return result;
        }

    private:
        void writeUnit(std::vector<SharpRuntime::bytecs>& out, size_t& pos, uint32_t cp) const {
            if (!bigEndian_) {
                out[pos++] = static_cast<SharpRuntime::bytecs>(cp & 0xFF);
                out[pos++] = static_cast<SharpRuntime::bytecs>((cp >> 8) & 0xFF);
                out[pos++] = static_cast<SharpRuntime::bytecs>((cp >> 16) & 0xFF);
                out[pos++] = static_cast<SharpRuntime::bytecs>((cp >> 24) & 0xFF);
            } else {
                out[pos++] = static_cast<SharpRuntime::bytecs>((cp >> 24) & 0xFF);
                out[pos++] = static_cast<SharpRuntime::bytecs>((cp >> 16) & 0xFF);
                out[pos++] = static_cast<SharpRuntime::bytecs>((cp >> 8) & 0xFF);
                out[pos++] = static_cast<SharpRuntime::bytecs>(cp & 0xFF);
            }
        }

        [[nodiscard]] uint32_t readUnit(const SharpRuntime::bytecs* data, SharpRuntime::intcs i) const {
            uint8_t b0 = data[i], b1 = data[i + 1], b2 = data[i + 2], b3 = data[i + 3];
            if (!bigEndian_) {
                return static_cast<uint32_t>(b0) | (static_cast<uint32_t>(b1) << 8) | (static_cast<uint32_t>(b2) << 16) |
                       (static_cast<uint32_t>(b3) << 24);
            }
            return (static_cast<uint32_t>(b0) << 24) | (static_cast<uint32_t>(b1) << 16) | (static_cast<uint32_t>(b2) << 8) |
                   static_cast<uint32_t>(b3);
        }

        // Validates and decodes one UTF-8 sequence starting at s[i]. Rejects ill-formed input
        // (bad continuation bytes, overlong encodings, surrogate/out-of-range code points) by
        // substituting U+FFFD for a single byte and resuming there, matching .NET's default
        // DecoderFallback replacement behavior, instead of the previous code's silent
        // acceptance of invalid sequences (verified with a compiled reproduction: a bad
        // continuation byte, "\xC2\x41", used to decode into a bogus code point instead of
        // being rejected).
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
