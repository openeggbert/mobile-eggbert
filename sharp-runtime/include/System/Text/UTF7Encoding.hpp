// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "System/NotImplementedException.hpp"
#include "System/Text/Encoding.hpp"

namespace System::Text {

    /**
     * @brief Partial UTF-7 encoding (SYSLIB0001 obsolete in real .NET); provided for API
     * compatibility only.
     *
     * @note This is NOT a full port: real UTF-7 (RFC 2152) represents non-ASCII text via
     * modified-Base64 shift sequences (`+...-`), which this implementation does not encode
     * or decode. Only pure-ASCII round-tripping is implemented. Per CLAUDE.md's rule that a
     * method that "cannot be meaningfully implemented... should throw NotImplementedException
     * ... never silently return a wrong value," GetBytes/GetString throw for any input
     * containing a non-ASCII character/byte, rather than the prior behavior of silently
     * substituting '?' -- which produced ASCII text that looked plausible but was not a
     * correct UTF-7 encoding/decoding of the input. Full RFC 2152 support is a real feature
     * gap, not a bug fix, and is out of scope here: UTF-7 is deprecated by .NET itself
     * (SYSLIB0001) due to known security issues, so this is unlikely to be needed for ported
     * game code.
     */
    class UTF7Encoding : public Encoding {
        bool allowOptionals_;
    public:
        /** Constructs a UTF-7 encoding with optional characters disallowed. */
        UTF7Encoding() : allowOptionals_(false) {}
        /** Constructs a UTF-7 encoding with the optional characters setting. */
        explicit UTF7Encoding(bool allowOptionals) : allowOptionals_(allowOptionals) {}

        /** Returns the encoding name "utf-7". */
        [[nodiscard]] std::string getEncodingNameProperty() const override { return "utf-7"; }
        /** Returns the code page 65000 (UTF-7). */
        [[nodiscard]] int getCodePageProperty()             const override { return 65000; }
        /** Returns whether optional characters are allowed. */
        [[nodiscard]] bool getAllowOptionals()               const { return allowOptionals_; }

        /**
         * @brief Encodes a pure-ASCII string to bytes unchanged.
         * @throws System::NotImplementedException if @p s contains a non-ASCII character --
         *         real UTF-7's shift-sequence encoding for non-ASCII text is not implemented.
         */
        [[nodiscard]] std::vector<SharpRuntime::bytecs> GetBytes(const std::string& s) const override {
            std::vector<SharpRuntime::bytecs> out;
            out.reserve(s.size());
            for (unsigned char c : s) {
                if (c >= 0x80)
                    throw System::NotImplementedException(
                        "UTF7Encoding.GetBytes: non-ASCII input requires RFC 2152 shift-sequence "
                        "encoding, which is not implemented in this port.");
                out.push_back(c);
            }
            return out;
        }

        /**
         * @brief Decodes a pure-ASCII byte range to a string unchanged.
         * @throws System::NotImplementedException if any byte in the range is >= 0x80 --
         *         real UTF-7 byte streams are always ASCII-only, so such a byte would only
         *         appear inside a shift sequence, which this port does not decode.
         */
        [[nodiscard]] std::string GetString(const SharpRuntime::bytecs* data,
                                            SharpRuntime::intcs index,
                                            SharpRuntime::intcs count) const override {
            std::string s;
            s.reserve(static_cast<size_t>(count));
            for (SharpRuntime::intcs i = 0; i < count; ++i) {
                uint8_t b = data[index + i];
                if (b >= 0x80)
                    throw System::NotImplementedException(
                        "UTF7Encoding.GetString: non-ASCII byte encountered, which would only "
                        "appear inside an RFC 2152 shift sequence; decoding that is not "
                        "implemented in this port.");
                s += static_cast<char>(b);
            }
            return s;
        }
    };

} // namespace System::Text
