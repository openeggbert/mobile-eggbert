// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "System/Text/Encoding.hpp"

namespace System::Text {

    /** ISO-8859-1 (Latin-1) encoding: each byte maps 1:1 to code points U+0000–U+00FF. */
    class Latin1Encoding : public Encoding {
    public:
        /** Returns the encoding name "iso-8859-1". */
        [[nodiscard]] std::string getEncodingNameProperty() const override { return "iso-8859-1"; }
        /** Returns the code page 28591 (ISO-8859-1). */
        [[nodiscard]] int getCodePageProperty() const override { return 28591; }
        /** Latin-1 always uses exactly one byte per character. */
        [[nodiscard]] bool getIsSingleByteProperty() const override { return true; }

        /** Encodes a string to Latin-1 bytes. */
        [[nodiscard]] std::vector<SharpRuntime::bytecs> GetBytes(const std::string& s) const override {
            std::vector<SharpRuntime::bytecs> result;
            result.reserve(s.size());
            for (unsigned char c : s)
                result.push_back(static_cast<SharpRuntime::bytecs>(c));
            return result;
        }

        /** Decodes a Latin-1 byte range to a string. */
        [[nodiscard]] std::string GetString(const SharpRuntime::bytecs* data,
                                             SharpRuntime::intcs index,
                                             SharpRuntime::intcs count) const override {
            std::string result;
            result.reserve(static_cast<std::size_t>(count));
            for (SharpRuntime::intcs i = 0; i < count; ++i)
                result += static_cast<char>(static_cast<unsigned char>(data[index + i]));
            return result;
        }
    };

} // namespace System::Text
