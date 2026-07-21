// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <iomanip>
#include <sstream>
#include <string>

namespace System::Text::Encodings::Web {

    /** Encodes strings for safe embedding in JavaScript literals. */
    class JavaScriptEncoder {
    public:
        /** Escapes special characters in the given string for JavaScript embedding. */
        static std::string Encode(const std::string& value) {
            std::string out;
            out.reserve(value.size());
            for (unsigned char c : value) {
                if (c == '\\') { out += "\\\\"; }
                else if (c == '"')  { out += "\\\""; }
                else if (c == '\n') { out += "\\n";  }
                else if (c == '\r') { out += "\\r";  }
                else if (c == '\t') { out += "\\t";  }
                else if (c < 0x20) {
                    std::ostringstream ss;
                    ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    out += ss.str();
                }
                else { out += static_cast<char>(c); }
            }
            return out;
        }

        /** Returns the default JavaScriptEncoder singleton. */
        static const JavaScriptEncoder& Default() {
            static JavaScriptEncoder instance;
            return instance;
        }
    };

} // namespace System::Text::Encodings::Web
