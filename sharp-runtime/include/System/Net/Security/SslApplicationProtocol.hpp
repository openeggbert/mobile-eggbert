// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"

namespace System::Net::Security {

    /**
     * @brief Represents an Application-Layer Protocol Negotiation (ALPN) TLS extension value.
     *
     * C++ counterpart of .NET System.Net.Security.SslApplicationProtocol (readonly struct).
     */
    class SslApplicationProtocol {
        std::vector<SharpRuntime::bytecs> protocol_;

        // Strict UTF-8 validity check (rejects overlong encodings and out-of-range code points),
        // matching the behavior of .NET's ExceptionFallback UTF-8 decoder closely enough to decide
        // between the plain-text and hex-dump ToString() paths.
        [[nodiscard]] bool isValidUtf8() const {
            size_t i = 0, n = protocol_.size();
            while (i < n) {
                SharpRuntime::bytecs b0 = protocol_[i];
                unsigned c0 = static_cast<unsigned char>(b0);
                size_t extra;
                unsigned minCp;
                if (c0 < 0x80) { ++i; continue; }
                else if ((c0 & 0xE0) == 0xC0) { extra = 1; minCp = 0x80; }
                else if ((c0 & 0xF0) == 0xE0) { extra = 2; minCp = 0x800; }
                else if ((c0 & 0xF8) == 0xF0) { extra = 3; minCp = 0x10000; }
                else return false;
                if (i + extra >= n) return false;
                unsigned cp = c0 & (0xFF >> (extra + 2));
                for (size_t k = 1; k <= extra; ++k) {
                    unsigned cb = static_cast<unsigned char>(protocol_[i + k]);
                    if ((cb & 0xC0) != 0x80) return false;
                    cp = (cp << 6) | (cb & 0x3F);
                }
                if (cp < minCp || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) return false;
                i += extra + 1;
            }
            return true;
        }

    public:
        SslApplicationProtocol() = default;

        /** @brief Constructs from raw ALPN protocol-id bytes (RFC 7301: 1-255 bytes). */
        explicit SslApplicationProtocol(const std::vector<SharpRuntime::bytecs>& protocol) : protocol_(protocol) {
            if (protocol.empty() || protocol.size() > 255) {
                throw System::ArgumentException("Invalid SSL application protocol.", "protocol");
            }
        }

        /** @brief Constructs from a UTF-8-encoded protocol name (e.g. "h2"). */
        explicit SslApplicationProtocol(const std::string& protocol)
            : SslApplicationProtocol(std::vector<SharpRuntime::bytecs>(protocol.begin(), protocol.end())) {}

        /** @return The raw ALPN protocol-id bytes. */
        [[nodiscard]] const std::vector<SharpRuntime::bytecs>& getProtocolProperty() const { return protocol_; }

        [[nodiscard]] bool Equals(const SslApplicationProtocol& other) const { return protocol_ == other.protocol_; }
        bool operator==(const SslApplicationProtocol& o) const { return Equals(o); }
        bool operator!=(const SslApplicationProtocol& o) const { return !Equals(o); }

        [[nodiscard]] SharpRuntime::intcs GetHashCode() const {
            SharpRuntime::intcs hash = 0;
            for (auto b : protocol_) {
                hash = ((hash << 5) + hash) ^ static_cast<SharpRuntime::intcs>(b);
            }
            return hash;
        }

        /** @return The protocol name decoded as UTF-8, or a hex dump if it isn't valid UTF-8. */
        [[nodiscard]] std::string ToString() const {
            if (protocol_.empty()) return std::string();
            if (isValidUtf8()) return std::string(protocol_.begin(), protocol_.end());

            // Real .NET's ExceptionFallback UTF-8 decoder throws on invalid sequences, and
            // ToString() catches that to fall back to a "0xNN 0xNN ..." hex dump (space-separated,
            // lowercase, no trailing space) -- mirrored here since this port has no throwing-decoder
            // equivalent to drive the same try/catch structure.
            static const char* kHex = "0123456789abcdef";
            std::string result;
            result.reserve(protocol_.size() * 5 - 1);
            for (size_t i = 0; i < protocol_.size(); ++i) {
                if (i != 0) result += ' ';
                SharpRuntime::bytecs b = protocol_[i];
                result += "0x";
                result += kHex[(b >> 4) & 0xF];
                result += kHex[b & 0xF];
            }
            return result;
        }

        /** @brief Defines the HTTP/3.0 ALPN protocol id ("h3"). */
        static const SslApplicationProtocol Http3;
        /** @brief Defines the HTTP/2.0 ALPN protocol id ("h2"). */
        static const SslApplicationProtocol Http2;
        /** @brief Defines the HTTP/1.1 ALPN protocol id ("http/1.1"). */
        static const SslApplicationProtocol Http11;
    };

    inline const SslApplicationProtocol SslApplicationProtocol::Http3{std::string("h3")};
    inline const SslApplicationProtocol SslApplicationProtocol::Http2{std::string("h2")};
    inline const SslApplicationProtocol SslApplicationProtocol::Http11{std::string("http/1.1")};

} // namespace System::Net::Security
