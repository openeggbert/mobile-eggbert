// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Text::Encodings::Web {

    using SharpRuntime::intcs;

    /** Encodes strings for safe embedding in HTML by escaping special characters. */
    class HtmlEncoder {
    public:
        /** HTML-encodes the entire string (& < > " ' are escaped). */
        static std::string Encode(const std::string& value) {
            std::string out;
            out.reserve(value.size());
            for (char c : value) {
                switch (c) {
                    case '&':  out += "&amp;";  break;
                    case '<':  out += "&lt;";   break;
                    case '>':  out += "&gt;";   break;
                    case '"':  out += "&quot;"; break;
                    case '\'': out += "&#x27;"; break;
                    default:   out += c;        break;
                }
            }
            return out;
        }

        /** Returns the default HtmlEncoder singleton. */
        static const HtmlEncoder& Default() {
            static HtmlEncoder instance;
            return instance;
        }

        /** HTML-encodes a substring defined by startIndex and characterCount. */
        std::string Encode(const std::string& value, intcs startIndex, intcs characterCount) const {
            return Encode(value.substr(static_cast<std::size_t>(startIndex), static_cast<std::size_t>(characterCount)));
        }
    };

} // namespace System::Text::Encodings::Web
