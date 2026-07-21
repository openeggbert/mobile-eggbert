// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <iomanip>
#include <sstream>
#include <string>

namespace System::Text::Encodings::Web {

    /** Provides URL percent-encoding and decoding (RFC 3986 unreserved characters are not encoded). */
    class UrlEncoder {
        static bool isUnreserved(unsigned char c) {
            return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                   (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~';
        }

    public:
        /** Percent-encodes a string, leaving RFC 3986 unreserved characters unchanged. */
        static std::string Encode(const std::string& value) {
            std::ostringstream out;
            for (unsigned char c : value) {
                if (isUnreserved(c)) {
                    out << static_cast<char>(c);
                } else {
                    out << '%' << std::hex << std::uppercase
                        << std::setw(2) << std::setfill('0') << static_cast<int>(c);
                }
            }
            return out.str();
        }

        /** Decodes a percent-encoded URL string. */
        static std::string Decode(const std::string& value) {
            std::string out;
            out.reserve(value.size());
            for (std::size_t i = 0; i < value.size(); ++i) {
                if (value[i] == '%' && i + 2 < value.size()) {
                    int hex = std::stoi(value.substr(i + 1, 2), nullptr, 16);
                    out += static_cast<char>(hex);
                    i += 2;
                } else if (value[i] == '+') {
                    out += ' ';
                } else {
                    out += value[i];
                }
            }
            return out;
        }

        /** Returns the default UrlEncoder singleton. */
        static const UrlEncoder& Default() {
            static UrlEncoder instance;
            return instance;
        }
    };

} // namespace System::Text::Encodings::Web
