// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Text/UTF8Encoding.hpp"
#include <cstdint>

namespace System::Text {

namespace {

    // Returns the byte length (1-4) of a well-formed UTF-8 sequence starting at data[i] within
    // [i, end), or 0 if the byte at i does not begin (or complete) a well-formed sequence --
    // same conformance rules (continuation-byte, overlong-encoding, surrogate, and out-of-range
    // rejection) as the decodeUtf8 helper duplicated across ASCIIEncoding.cpp/UnicodeEncoding.hpp/
    // UTF32Encoding.hpp, but reporting a validity length instead of substituting a code point --
    // UTF8Encoding's well-formed bytes pass through unchanged (source and destination are both
    // UTF-8), so there's no re-encoding step to fold the substitution into.
    size_t wellFormedUtf8Length(const SharpRuntime::bytecs* data, size_t i, size_t end) {
        auto isContinuation = [](SharpRuntime::bytecs b) { return (b & 0xC0) == 0x80; };
        SharpRuntime::bytecs c0 = data[i];
        if (c0 < 0x80) return 1;
        if ((c0 & 0xE0) == 0xC0 && i + 1 < end && isContinuation(data[i + 1])) {
            uint32_t cp = (static_cast<uint32_t>(c0 & 0x1F) << 6) | (data[i + 1] & 0x3F);
            return cp >= 0x80 ? 2 : 0;
        }
        if ((c0 & 0xF0) == 0xE0 && i + 2 < end && isContinuation(data[i + 1]) && isContinuation(data[i + 2])) {
            uint32_t cp = (static_cast<uint32_t>(c0 & 0x0F) << 12) |
                          (static_cast<uint32_t>(data[i + 1] & 0x3F) << 6) | (data[i + 2] & 0x3F);
            return (cp >= 0x800 && !(cp >= 0xD800 && cp <= 0xDFFF)) ? 3 : 0;
        }
        if ((c0 & 0xF8) == 0xF0 && i + 3 < end && isContinuation(data[i + 1]) && isContinuation(data[i + 2]) &&
            isContinuation(data[i + 3])) {
            uint32_t cp = (static_cast<uint32_t>(c0 & 0x07) << 18) |
                          (static_cast<uint32_t>(data[i + 1] & 0x3F) << 12) |
                          (static_cast<uint32_t>(data[i + 2] & 0x3F) << 6) | (data[i + 3] & 0x3F);
            return (cp >= 0x10000 && cp <= 0x10FFFF) ? 4 : 0;
        }
        return 0;
    }

} // namespace

    UTF8Encoding::UTF8Encoding() {
        // Real UTF8Encoding.SetDefaultFallbacks() uses a U+FFFD replacement fallback, not the
        // generic Encoding base class's "?" default (which only fits single-byte code pages).
        setEncoderFallbackProperty(std::make_shared<EncoderReplacementFallback>("\xEF\xBF\xBD"));
        setDecoderFallbackProperty(std::make_shared<DecoderReplacementFallback>("\xEF\xBF\xBD"));
    }

    std::vector<SharpRuntime::bytecs> UTF8Encoding::GetBytes(const std::string& str) const {
        const auto* data = reinterpret_cast<const SharpRuntime::bytecs*>(str.data());
        size_t end = str.size();
        std::vector<SharpRuntime::bytecs> result;
        result.reserve(end);
        auto fallback = getEncoderFallbackProperty();
        size_t i = 0;
        while (i < end) {
            size_t len = wellFormedUtf8Length(data, i, end);
            if (len > 0) {
                result.insert(result.end(), data + i, data + i + len);
                i += len;
            } else {
                auto buffer = fallback->CreateFallbackBuffer();
                if (buffer->Fallback(static_cast<char>(data[i]), static_cast<SharpRuntime::intcs>(i))) {
                    while (buffer->getRemainingProperty() > 0) {
                        result.push_back(static_cast<SharpRuntime::bytecs>(buffer->GetNextChar()));
                    }
                }
                ++i;
            }
        }
        return result;
    }

    std::string UTF8Encoding::GetString(const SharpRuntime::bytecs* data,
                                        SharpRuntime::intcs index,
                                        SharpRuntime::intcs count) const {
        if (data == nullptr || count <= 0) return {};
        size_t i = static_cast<size_t>(index);
        size_t end = i + static_cast<size_t>(count);
        std::string result;
        result.reserve(static_cast<size_t>(count));
        auto fallback = getDecoderFallbackProperty();
        while (i < end) {
            size_t len = wellFormedUtf8Length(data, i, end);
            if (len > 0) {
                result.append(reinterpret_cast<const char*>(data + i), len);
                i += len;
            } else {
                std::vector<SharpRuntime::bytecs> bad{data[i]};
                auto buffer = fallback->CreateFallbackBuffer();
                if (buffer->Fallback(bad, static_cast<SharpRuntime::intcs>(i))) {
                    while (buffer->getRemainingProperty() > 0) {
                        result.push_back(buffer->GetNextChar());
                    }
                }
                ++i;
            }
        }
        return result;
    }

} // namespace System::Text
